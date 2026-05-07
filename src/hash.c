#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/hash.h"

/* ==========================================================================
   ESTRUTURAS INTERNAS
   ========================================================================== */

//Cabeçalho gravado no início de cada bucket no arquivo .hf.
typedef struct {
    int local_depth;   // profundidade local deste bucket
    int num_records;   // quantidade de registros válidos
    int max_records;   // capacidade máxima (informativo)
} BucketHeader;

//Cabeçalho gravado imediatamente antes dos bytes de cada registro.
typedef struct {
    uint64_t key;        /* chave do registro  */
    size_t   data_size;  /* tamanho do dado em bytes */
    int      valid;      /* 1 = válido, 0 = removido */
} RecordHeader;

//Estrutura principal — opaca para o usuário (definida apenas aqui).
struct Hash {
    FILE   *file;          // arquivo .hf aberto em modo "w+b"      
    int     global_depth;  // profundidade global atual              
    size_t  bucket_size;   // tamanho fixo de cada bucket em bytes   
    int     num_buckets;   // quantos buckets existem no arquivo     
    int    *directory;     // directory[i] = índice do bucket para i 
    int     dir_size;      // tamanho do diretório (2^global_depth)  
    int     max_records;   // capacidade por bucket (calculada 1 vez)
};

/* ==========================================================================
   FUNÇÕES AUXILIARES INTERNAS
   ========================================================================== */

//Retorna os bits menos significativos de key (índice no diretório). 
static int hash_index(uint64_t key, int depth) {
    return (int)(key & ((1ULL << depth) - 1));
}

/* Calcula quantos registros cabem no payload de um bucket.
   Usa o tamanho mínimo possível de registro (dado de 1 byte) como estimativa
   conservadora — na prática registros serão maiores, então nunca estoura. */
static int calc_max_records(size_t bucket_size) {
    size_t header  = sizeof(BucketHeader);
    size_t rec_min = sizeof(RecordHeader) + 1;
    if (bucket_size <= header) return 0;
    return (int)((bucket_size - header) / rec_min);
}

//Retorna o offset em bytes do início do bucket bucket_idx no arquivo. 
static long bucket_offset(const Hash *h, int bucket_idx) {
    assert(bucket_idx >= 0 && bucket_idx < h->num_buckets);
    return (long)bucket_idx * (long)h->bucket_size;
}

/* Lê e retorna o BucketHeader do bucket indicado.
   Posiciona o cursor logo APÓS o cabeçalho (pronto para ler registros). */
static BucketHeader read_bucket_header(Hash *h, int bucket_idx) {
    BucketHeader bh;
    fseek(h->file, bucket_offset(h, bucket_idx), SEEK_SET);
    fread(&bh, sizeof(BucketHeader), 1, h->file);
    return bh;
}

/* Grava o BucketHeader no bucket indicado.
   Não altera o cursor de leitura/escrita de registros. */
static void write_bucket_header(Hash *h, int bucket_idx, BucketHeader *bh) {
    fseek(h->file, bucket_offset(h, bucket_idx), SEEK_SET);
    fwrite(bh, sizeof(BucketHeader), 1, h->file);
}

//Zera o bloco inteiro do bucket e grava um cabeçalho limpo. 
static void init_bucket(Hash *h, int bucket_idx, int local_depth) {
    long off = bucket_offset(h, bucket_idx);

    void *zeros = calloc(1, h->bucket_size);
    assert(zeros != NULL);
    fseek(h->file, off, SEEK_SET);
    fwrite(zeros, h->bucket_size, 1, h->file);
    free(zeros);

    BucketHeader bh;
    bh.local_depth = local_depth;
    bh.num_records = 0;
    bh.max_records = h->max_records;
    write_bucket_header(h, bucket_idx, &bh);
}

//Dobra o diretório e incrementa a profundidade global. 
static void double_directory(Hash *h) {
    int old_size = h->dir_size;
    int new_size = old_size * 2;
    int *new_dir = realloc(h->directory, (size_t)new_size * sizeof(int));
    assert(new_dir != NULL);
    h->directory = new_dir;

    // A segunda metade aponta para os mesmos buckets da primeira 
    for (int i = 0; i < old_size; i++) {
        h->directory[old_size + i] = h->directory[i];
    }
    h->dir_size     = new_size;
    h->global_depth += 1;
}

static bool insert_into_bucket(Hash *h, int bucket_idx, void *data, uint64_t key, size_t size);

 //Divide o bucket bucket_idx em dois e redistribui seus registros.
static void split_bucket(Hash *h, int bucket_idx) {
    //Lê o cabeçalho ANTES de qualquer modificação no diretório 
    BucketHeader old_bh = read_bucket_header(h, bucket_idx);
    int old_depth = old_bh.local_depth;
    int old_nrec = old_bh.num_records;

    //Se a profundidade local já atingiu a global, dobra o diretório 
    if (old_depth == h->global_depth) {
        double_directory(h);
    }

    // Cria o novo bucket no final do arquivo 
    int new_bucket_idx = h->num_buckets;
    h->num_buckets++;
    init_bucket(h, new_bucket_idx, old_depth + 1);

    /* Atualiza entradas do diretório: as que tinham o bit old_depth = 1
       passam a apontar para o novo bucket */
    for (int i = 0; i < h->dir_size; i++) {
        if (h->directory[i] == bucket_idx && ((i >> old_depth) & 1)) {
            h->directory[i] = new_bucket_idx;
        }
    }

    //Lê todo o payload do bucket antigo em memória para redistribuir.

    size_t payload = h->bucket_size - sizeof(BucketHeader);
    uint8_t *buf = malloc(payload);
    assert(buf != NULL);

    long payload_off = bucket_offset(h, bucket_idx) + (long)sizeof(BucketHeader);
    fseek(h->file, payload_off, SEEK_SET);
    fread(buf, payload, 1, h->file);

    // Zera o bucket antigo (cabeçalho + payload) para reinserção limpa
    old_bh.local_depth = old_depth + 1;
    old_bh.num_records = 0;
    write_bucket_header(h, bucket_idx, &old_bh);
    {
        void *zeros = calloc(1, payload);
        assert(zeros != NULL);
        fseek(h->file, payload_off, SEEK_SET);
        fwrite(zeros, payload, 1, h->file);
        free(zeros);
    }
    size_t cursor = 0;
    for (int r = 0; r < old_nrec; r++) {
        if (cursor + sizeof(RecordHeader) > payload) break;

        RecordHeader *rh = (RecordHeader *)(buf + cursor);
        size_t rsize = sizeof(RecordHeader) + rh->data_size;

        if (cursor + rsize > payload) break;

        if (rh->valid) {
            void *rdata   = buf + cursor + sizeof(RecordHeader);
            int   new_idx = h->directory[hash_index(rh->key, h->global_depth)];
            insert_into_bucket(h, new_idx, rdata, rh->key, rh->data_size);
        }
        cursor += rsize;
    }

    free(buf);
}

 //Tenta inserir (key, data, size) no bucket bucket_idx.
 //Se não houver espaço, divide o bucket e tenta novamente.

static bool insert_into_bucket(Hash *h, int bucket_idx, void *data, uint64_t key, size_t size) {
    /* Lê o cabeçalho — cursor fica logo após o cabeçalho */
    BucketHeader bh = read_bucket_header(h, bucket_idx);
    size_t payload = h->bucket_size - sizeof(BucketHeader);
    size_t needed  = sizeof(RecordHeader) + size;

    //Calcula quanto espaço já está usado pelos registros existentes
    size_t used = 0;
    for (int i = 0; i < bh.num_records; i++) {
        RecordHeader rh;
        fread(&rh, sizeof(RecordHeader), 1, h->file);
        fseek(h->file, (long)rh.data_size, SEEK_CUR);
        used += sizeof(RecordHeader) + rh.data_size;
    }

    //Sem espaço: divide e redireciona
    if (used + needed > payload) {
        split_bucket(h, bucket_idx);
        int new_idx = h->directory[hash_index(key, h->global_depth)];
        return insert_into_bucket(h, new_idx, data, key, size);
    }

    //Escreve o novo registro após os existentes
    long write_pos = bucket_offset(h, bucket_idx)
                     + (long)sizeof(BucketHeader)
                     + (long)used;
    fseek(h->file, write_pos, SEEK_SET);

    RecordHeader rh;
    memset(&rh, 0, sizeof(RecordHeader));
    rh.key = key;
    rh.data_size = size;
    rh.valid = 1;
    fwrite(&rh, sizeof(RecordHeader), 1, h->file);
    fwrite(data, size, 1, h->file);
    fflush(h->file);

    bh.num_records++;
    write_bucket_header(h, bucket_idx, &bh);
    return true;
}

/* ==========================================================================
   FUNÇÕES PÚBLICAS — CICLO DE VIDA
   ========================================================================== */

Hash *hash_create(const char *file_name, size_t bucket_size) {
    assert(file_name != NULL);
    assert(bucket_size > sizeof(BucketHeader) + sizeof(RecordHeader) + 1);

    Hash *h = malloc(sizeof(Hash));
    assert(h != NULL);

    h->bucket_size = bucket_size;
    h->global_depth = 1;
    h->dir_size = 2;
    h->num_buckets = 2;
    h->max_records = calc_max_records(bucket_size);

    h->directory = malloc((size_t)h->dir_size * sizeof(int));
    assert(h->directory != NULL);
    h->directory[0] = 0;
    h->directory[1] = 1;

    h->file = fopen(file_name, "w+b");
    assert(h->file != NULL);

    init_bucket(h, 0, 1);
    init_bucket(h, 1, 1);

    return h;
}

void hash_destroy(Hash *h) {
    if (h == NULL) return;
    if (h->file) fclose(h->file);
    if (h->directory) free(h->directory);
    free(h);
}

/* ==========================================================================
   FUNÇÕES PÚBLICAS — OPERAÇÕES PRINCIPAIS
   ========================================================================== */

bool hash_insert(Hash *h, void *data, uint64_t key, size_t size) {
    assert(h != NULL);
    assert(data != NULL);
    assert(size > 0);

    int bucket_idx = h->directory[hash_index(key, h->global_depth)];
    return insert_into_bucket(h, bucket_idx, data, key, size);
}

void *hash_search(Hash *h, uint64_t key, size_t *size) {
    assert(h != NULL);

    int bucket_idx = h->directory[hash_index(key, h->global_depth)];

    /* read_bucket_header já faz fseek para o início do bucket e deixa o
       cursor logo após o cabeçalho — pronto para ler registros. */
    BucketHeader bh = read_bucket_header(h, bucket_idx);

    for (int i = 0; i < bh.num_records; i++) {
        RecordHeader rh;
        fread(&rh, sizeof(RecordHeader), 1, h->file);

        if (rh.valid && rh.key == key) {
            void *buf = malloc(rh.data_size);
            assert(buf != NULL);
            fread(buf, rh.data_size, 1, h->file);
            if (size) *size = rh.data_size;
            return buf;
        }
        fseek(h->file, (long)rh.data_size, SEEK_CUR);
    }
    return NULL;
}

//Remove o registro com a chave dada e compacta o bucket.
void *hash_remove(Hash *h, uint64_t key, size_t *out_size) {
    assert(h != NULL);

    int bucket_idx = h->directory[hash_index(key, h->global_depth)];

    BucketHeader bh = read_bucket_header(h, bucket_idx);

    long base = bucket_offset(h, bucket_idx) + (long)sizeof(BucketHeader);

    //Primeira passagem: localiza o registro 
    fseek(h->file, base, SEEK_SET);
    for (int i = 0; i < bh.num_records; i++) {
        RecordHeader rh;
        fread(&rh, sizeof(RecordHeader), 1, h->file);

        if (rh.valid && rh.key == key) {
            void *buf = malloc(rh.data_size);
            assert(buf != NULL);
            fread(buf, rh.data_size, 1, h->file);
            if (out_size) *out_size = rh.data_size;


            if (i < bh.num_records - 1) {
                /* Precisa recalcular o offset do registro i e do último */
                /* Relê o bucket do início para encontrar os offsets */
                fseek(h->file, base, SEEK_SET);

                long off_target = base;
                long off_last   = base;

                for (int j = 0; j < bh.num_records; j++) {
                    RecordHeader tmp;
                    long cur = ftell(h->file);
                    fread(&tmp, sizeof(RecordHeader), 1, h->file);
                    fseek(h->file, (long)tmp.data_size, SEEK_CUR);

                    if (j == i) off_target = cur;
                    if (j == bh.num_records - 1) off_last = cur;
                }

                //lê o último registro
                fseek(h->file, off_last, SEEK_SET);
                RecordHeader last_rh;
                fread(&last_rh, sizeof(RecordHeader), 1, h->file);
                void *last_data = malloc(last_rh.data_size);
                assert(last_data != NULL);
                fread(last_data, last_rh.data_size, 1, h->file);

                /* Escreve o último no lugar do removido */
                fseek(h->file, off_target, SEEK_SET);
                fwrite(&last_rh,  sizeof(RecordHeader), 1, h->file);
                fwrite(last_data, last_rh.data_size, 1, h->file);
                free(last_data);

                /* Zera os bytes do antigo último registro para não deixar
                   lixo no payload (opcional, mas torna o .hfd mais legível) */
                fseek(h->file, off_last, SEEK_SET);
                size_t last_total = sizeof(RecordHeader) + last_rh.data_size;
                void  *zeros = calloc(1, last_total);
                fwrite(zeros, last_total, 1, h->file);
                free(zeros);
            } else {
            // É o último: apenas zera no lugar
                long off_self = ftell(h->file) - (long)(sizeof(RecordHeader) + rh.data_size);
                size_t total = sizeof(RecordHeader) + rh.data_size;
                void *zeros = calloc(1, total);
                fseek(h->file, off_self, SEEK_SET);
                fwrite(zeros, total, 1, h->file);
                free(zeros);
            }

            fflush(h->file);

            bh.num_records--;
            write_bucket_header(h, bucket_idx, &bh);

            return buf;
        }
        fseek(h->file, (long)rh.data_size, SEEK_CUR);
    }

    return NULL; /* chave não encontrada */
}

/* ==========================================================================
   FUNÇÕES PÚBLICAS — ACESSO INTERNO (para varredura em parser_qry)
   ========================================================================== */

FILE *hash_get_file(Hash *h) {
    assert(h != NULL);
    return h->file;
}

int hash_get_num_buckets(Hash *h) {
    assert(h != NULL);
    return h->num_buckets;
}

size_t hash_get_bucket_size(Hash *h) {
    assert(h != NULL);
    return h->bucket_size;
}

/* ==========================================================================
   DUMP TEXTUAL (.hfd) — exigido pelo enunciado
   ========================================================================== */

void hash_dump(Hash *h, const char *file_name) {
    assert(h         != NULL);
    assert(file_name != NULL);

    FILE *out = fopen(file_name, "w");
    if (out == NULL) {
        perror("[hash_dump] Erro ao criar arquivo .hfd");
        return;
    }

    /* --- Metadados globais --- */
    fprintf(out, "=== HASH EXTENSIVEL DUMP ===\n");
    fprintf(out, "Profundidade global : %d\n",    h->global_depth);
    fprintf(out, "Num. de buckets     : %d\n",    h->num_buckets);
    fprintf(out, "Tamanho do bucket   : %zu B\n", h->bucket_size);
    fprintf(out, "Tamanho do diretorio: %d\n\n",  h->dir_size);

    /* --- Diretório --- */
    fprintf(out, "--- DIRETORIO ---\n");
    for (int i = 0; i < h->dir_size; i++) {
        fprintf(out, "  dir[%3d] -> bucket %d\n", i, h->directory[i]);
    }
    fprintf(out, "\n");

    /* --- Buckets --- */
    fprintf(out, "--- BUCKETS ---\n");
    for (int b = 0; b < h->num_buckets; b++) {
        BucketHeader bh = read_bucket_header(h, b);
        fprintf(out,
                "Bucket %3d | depth_local=%d | registros=%d/%d\n",
                b, bh.local_depth, bh.num_records, bh.max_records);

        for (int r = 0; r < bh.num_records; r++) {
            RecordHeader rh;
            fread(&rh, sizeof(RecordHeader), 1, h->file);
            fprintf(out,
                    "  [%d] chave=%" PRIu64 "  tamanho=%zu B  valido=%d\n",
                    r, rh.key, rh.data_size, rh.valid);
            fseek(h->file, (long)rh.data_size, SEEK_CUR);
        }
        fprintf(out, "\n");
    }

    fprintf(out, "=== FIM DO DUMP ===\n");
    fclose(out);
}