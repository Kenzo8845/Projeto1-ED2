#include "../include/hash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * Cabeçalho de cada bucket gravado no arquivo .hf
 * Fica no início de cada bloco de tamanho bucket_size.
 */
typedef struct {
    int   local_depth;   /* profundidade local deste bucket */
    int   num_records;   /* quantos registros estão ocupados */
    int   max_records;   /* capacidade máxima (calculada no create) */
} BucketHeader;

/*
 * Cabeçalho de cada registro dentro do bucket.
 * Vem logo antes dos bytes do dado.
 */
typedef struct {
    uint64_t key;        /* chave do registro */
    size_t   data_size;  /* tamanho em bytes do dado */
    int      valid;      /* 1 = ocupado, 0 = removido (tombstone) */
} RecordHeader;

/*
 * Estrutura principal do hash (opaca para o usuário).
 */
struct Hash {
    FILE   *file;           /* arquivo .hf aberto */
    int     global_depth;   /* profundidade global atual */
    size_t  bucket_size;    /* tamanho em bytes de cada bucket */
    int     num_buckets;    /* número de buckets alocados no arquivo */
    long   *directory;      /* vetor de offsets: directory[i] = posição do bucket i no arquivo */
    int     dir_size;       /* tamanho do diretório (2^global_depth) */
    int     max_records;    /* registros por bucket (calculado uma vez) */
};

/* =========================================================
   FUNÇÕES AUXILIARES INTERNAS
   ========================================================= */

static int hash_index(uint64_t key, int depth) {
    return (int)(key & ((1ULL << depth) - 1));
}

static int calc_max_records(size_t bucket_size) {
    size_t header_size = sizeof(BucketHeader);
    size_t record_min  = sizeof(RecordHeader) + 1; 
    if (bucket_size <= header_size) return 0;
    return (int)((bucket_size - header_size) / record_min);
}

static long bucket_offset(const Hash *h, int bucket_idx) {
    assert(bucket_idx >= 0 && bucket_idx < h->num_buckets);
    return (long)bucket_idx * (long)h->bucket_size;
}

static BucketHeader read_bucket_header(Hash *h, int bucket_idx) {
    BucketHeader bh;
    long off = bucket_offset(h, bucket_idx);
    fseek(h->file, off, SEEK_SET);
    fread(&bh, sizeof(BucketHeader), 1, h->file);
    return bh;
}

static void write_bucket_header(Hash *h, int bucket_idx, BucketHeader *bh) {
    long off = bucket_offset(h, bucket_idx);
    fseek(h->file, off, SEEK_SET);
    fwrite(bh, sizeof(BucketHeader), 1, h->file);
}

static void init_bucket(Hash *h, int bucket_idx, int local_depth) {
    /* Zera o bloco inteiro, depois grava o cabeçalho */
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

static void double_directory(Hash *h) {
    int old_size = h->dir_size;
    int new_size = old_size * 2;
    long *new_dir = realloc(h->directory, new_size * sizeof(long));
    assert(new_dir != NULL);
    h->directory = new_dir;

    for (int i = 0; i < old_size; i++) {
        h->directory[old_size + i] = h->directory[i];
    }
    h->dir_size      = new_size;
    h->global_depth += 1;
}


static void split_bucket(Hash *h, int bucket_idx); 

static bool insert_into_bucket(Hash *h, int bucket_idx, void *data, uint64_t key, size_t size);

static void split_bucket(Hash *h, int bucket_idx) {
    BucketHeader old_bh = read_bucket_header(h, bucket_idx);
    int old_local_depth  = old_bh.local_depth;

    if (old_local_depth == h->global_depth) {
        double_directory(h);
    }

    /* Cria novo bucket no final do arquivo */
    int new_bucket_idx = h->num_buckets;
    h->num_buckets++;

    init_bucket(h, new_bucket_idx, old_local_depth + 1);

    old_bh.local_depth = old_local_depth + 1;

    
    for (int i = 0; i < h->dir_size; i++) {
        if (h->directory[i] == bucket_idx) {
            /* Verifica o bit na posição old_local_depth */
            if ((i >> old_local_depth) & 1) {
                h->directory[i] = new_bucket_idx;
            }
            /* senão continua apontando para bucket_idx */
        }
    }

    long off = bucket_offset(h, bucket_idx) + sizeof(BucketHeader);
    fseek(h->file, off, SEEK_SET);

    /* Aloca buffer para ler o bucket inteiro de uma vez */
    size_t payload = h->bucket_size - sizeof(BucketHeader);
    uint8_t *buf = malloc(payload);
    assert(buf != NULL);
    fread(buf, payload, 1, h->file);

    old_bh.num_records = 0;
    write_bucket_header(h, bucket_idx, &old_bh);
    {
        void *zeros = calloc(1, payload);
        fseek(h->file, off, SEEK_SET);
        fwrite(zeros, payload, 1, h->file);
        free(zeros);
    }

    size_t cursor = 0;
    while (cursor + sizeof(RecordHeader) <= payload) {
        RecordHeader *rh = (RecordHeader *)(buf + cursor);
        if (rh->valid == 1) {
            void *rdata = buf + cursor + sizeof(RecordHeader);
            /* Determina bucket correto pelo novo hash */
            int new_idx = (int)(h->directory[hash_index(rh->key, h->global_depth)]);
            insert_into_bucket(h, new_idx, rdata, rh->key, rh->data_size);
            cursor += sizeof(RecordHeader) + rh->data_size;
        } else {
            break; 
        }
    }
    free(buf);
}

static bool insert_into_bucket(Hash *h, int bucket_idx, void *data, uint64_t key, size_t size) {
    BucketHeader bh = read_bucket_header(h, bucket_idx);

    long off        = bucket_offset(h, bucket_idx) + sizeof(BucketHeader);
    size_t payload  = h->bucket_size - sizeof(BucketHeader);
    size_t needed   = sizeof(RecordHeader) + size;

    fseek(h->file, off, SEEK_SET);
    size_t used = 0;
    for (int i = 0; i < bh.num_records; i++) {
        RecordHeader rh;
        fread(&rh, sizeof(RecordHeader), 1, h->file);
        fseek(h->file, (long)rh.data_size, SEEK_CUR);
        used += sizeof(RecordHeader) + rh.data_size;
    }

    if (used + needed > payload) {
        split_bucket(h, bucket_idx);
        int new_idx = (int)(h->directory[hash_index(key, h->global_depth)]);
        return insert_into_bucket(h, new_idx, data, key, size);
    }

    /* Escreve o novo registro no fim dos registros existentes */
    long write_pos = bucket_offset(h, bucket_idx) + (long)sizeof(BucketHeader) + (long)used;
    fseek(h->file, write_pos, SEEK_SET);

    RecordHeader rh;
    rh.key       = key;
    rh.data_size = size;
    rh.valid     = 1;
    fwrite(&rh,   sizeof(RecordHeader), 1, h->file);
    fwrite(data,  size,                 1, h->file);
    fflush(h->file);

    bh.num_records++;
    write_bucket_header(h, bucket_idx, &bh);
    return true;
}


Hash *hash_create(const char *file_name, size_t bucket_size) {
    assert(file_name != NULL);
    assert(bucket_size > sizeof(BucketHeader) + sizeof(RecordHeader) + 1);

    Hash *h = malloc(sizeof(Hash));
    assert(h != NULL);

    h->bucket_size   = bucket_size;
    h->global_depth  = 1;
    h->dir_size      = 2;   
    h->num_buckets   = 2;
    h->max_records   = calc_max_records(bucket_size);

    h->directory = malloc(h->dir_size * sizeof(long));
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
    if (h->file)      fclose(h->file);
    if (h->directory) free(h->directory);
    free(h);
}

bool hash_insert(Hash *h, void *data, uint64_t key, size_t size) {
    assert(h != NULL);
    assert(data != NULL);
    assert(size > 0);

    int dir_idx    = hash_index(key, h->global_depth);
    int bucket_idx = (int)h->directory[dir_idx];
    return insert_into_bucket(h, bucket_idx, data, key, size);
}

void *hash_search(Hash *h, uint64_t key, size_t *size) {
    assert(h != NULL);

    int dir_idx    = hash_index(key, h->global_depth);
    int bucket_idx = (int)h->directory[dir_idx];

    long off = bucket_offset(h, bucket_idx) + sizeof(BucketHeader);
    fseek(h->file, off, SEEK_SET);

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
        /* Pula os bytes do dado deste registro */
        fseek(h->file, (long)rh.data_size, SEEK_CUR);
    }
    return NULL; 
}

void *hash_remove(Hash *h, uint64_t key, size_t *size) {
    assert(h != NULL);

    int dir_idx    = hash_index(key, h->global_depth);
    int bucket_idx = (int)h->directory[dir_idx];

    long base = bucket_offset(h, bucket_idx) + sizeof(BucketHeader);
    fseek(h->file, base, SEEK_SET);

    BucketHeader bh = read_bucket_header(h, bucket_idx);

    long record_start = base;
    for (int i = 0; i < bh.num_records; i++) {
        RecordHeader rh;
        fread(&rh, sizeof(RecordHeader), 1, h->file);
        if (rh.valid && rh.key == key) {
            /* Lê o dado antes de invalidar */
            void *buf = malloc(rh.data_size);
            assert(buf != NULL);
            fread(buf, rh.data_size, 1, h->file);
            if (size) *size = rh.data_size;

            rh.valid = 0;
            fseek(h->file, record_start, SEEK_SET);
            fwrite(&rh, sizeof(RecordHeader), 1, h->file);
            fflush(h->file);

    
            bh.num_records--;
            write_bucket_header(h, bucket_idx, &bh);
            return buf;
        }
        record_start += (long)(sizeof(RecordHeader) + rh.data_size);
        fseek(h->file, (long)rh.data_size, SEEK_CUR);
    }
    return NULL;  
}