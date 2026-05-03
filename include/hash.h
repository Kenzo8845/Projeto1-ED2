#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>
#include <stdio.h>

/**
 * Este módulo define as funções para criação e manipulação de Hash Extensível
 * com persistência em disco (arquivo binário .hf).
 *
 * O hash extensível cresce dinamicamente: quando um bucket fica cheio, ele é
 * dividido e o diretório pode ser dobrado. Registros são identificados por uma
 * chave numérica de 64 bits e o dado é genérico (void*) com tamanho variável.
 *
 * Remoções usam compactação in-place: o slot do registro removido é preenchido
 * pelo último registro válido do bucket, evitando acúmulo de tombstones que
 * causariam splits desnecessários.
 */

typedef struct Hash Hash;


/**
 * @brief Cria um novo hash extensível zerado, associado ao arquivo file_name.
 *        O arquivo é aberto em modo "w+b" (cria ou trunca).
 * @param file_name Caminho do arquivo .hf a criar.
 * @param bucket_size Tamanho em bytes de cada bucket (deve ser grande o
 *        suficiente para conter pelo menos um registro + cabeçalho).
 * @return Ponteiro para o Hash criado, ou NULL em caso de erro.
 */
Hash *hash_create(const char *file_name, size_t bucket_size);

/**
 * @brief Fecha o arquivo e libera toda a memória associada ao hash.
 * @param h Hash a destruir. NULL é ignorado.
 */
void hash_destroy(Hash *h);


/* ==========================================================================
   Operações principais
   ========================================================================== */

/**
 * @brief Insere um dado no hash.
 * @param h     Hash de destino.
 * @param data  Ponteiro para o dado a ser copiado no arquivo.
 * @param key   Chave de hashing (64 bits).
 * @param size  Tamanho em bytes do dado apontado por data.
 * @return true em caso de sucesso, false em caso de erro.
 */
bool hash_insert(Hash *h, void *data, uint64_t key, size_t size);

/**
 * @brief Busca um dado no hash pela chave.
 * @param h    Hash a consultar.
 * @param key  Chave do registro procurado.
 * @param size Ponteiro de saída: receberá o tamanho em bytes do dado retornado.
 *             Pode ser NULL se o tamanho não for necessário.
 * @return Ponteiro para uma cópia heap-alocada do dado, ou NULL se não
 *         encontrado. O chamador é responsável por chamar free().
 */
void *hash_search(Hash *h, uint64_t key, size_t *size);

/**
 * @brief Remove um dado do hash pela chave.
 *        O slot é compactado internamente (sem tombstones).
 * @param h    Hash a modificar.
 * @param key  Chave do registro a remover.
 * @param size Ponteiro de saída com o tamanho do dado removido. Pode ser NULL.
 * @return Ponteiro para uma cópia heap-alocada do dado removido, ou NULL se
 *         não encontrado. O chamador é responsável por chamar free().
 */
void *hash_remove(Hash *h, uint64_t key, size_t *size);


/* ==========================================================================
   Acesso interno (necessário para varredura sequencial pelo parser_qry)
   ========================================================================== */

/**
 * @brief Retorna o FILE* do arquivo .hf aberto internamente.
 *        Usado pela varredura sequencial de parser_qry.c.
 */
FILE *hash_get_file(Hash *h);

/**
 * @brief Retorna o número de buckets físicos alocados no arquivo.
 */
int hash_get_num_buckets(Hash *h);

/**
 * @brief Retorna o tamanho em bytes de cada bucket.
 */
size_t hash_get_bucket_size(Hash *h);


/* ==========================================================================
   Dump textual (.hfd) 
   ========================================================================== */

/**
 * @brief Gera um arquivo-texto legível (.hfd) com a representação esquemática
 *        do conteúdo do hash: diretório, buckets, registros e expansões.
 * @param h         Hash a ser despejado.
 * @param file_name Caminho do arquivo .hfd a criar.
 */
void hash_dump(Hash *h, const char *file_name);

#endif