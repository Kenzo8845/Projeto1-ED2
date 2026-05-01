#ifndef HASH_H
#define HASH_H

#include <stddef.h>
#include <stdbool.h>
#include <inttypes.h>

/** Este módulo define as funções básicas para criação e manipulação dos arquivos Hash Extensivel.
 * O hash, é uma forma de organiação de dados com persistência em disco.
 * O usuário ao passar uma chave e o dadona ser guardado, seu conteúdo 
 * é alocado ao um bucket (espaço físico no disco).
 * 
 * Hash file extensivel:
 * Por se tratar de um Hash Extensivel, utilizado em sistemas com grande densidade de dados,
 * os buckets se extendes sozinhos, dobrando de tamanho quando necessario.
 * Os dados sobrevivem o fechamento do programada.
 * A busca de um dado por meio da chave é rápida.
 */


typedef struct Hash Hash;


/**
 * @brief Cria um hash file novo.
 * @param file_name Nome do arquivo que sera utilizado no hash.
 * @param bucket_size tamanho maximo do bucket em bytes.
 * @return Hash* um ponteiro para o hash criado.
 */
Hash* hash_create(const char* file_name, size_t bucket_size);

/**
 * Libera toda a memória associada ao hash.
 */
void hash_destroy(Hash* h);


/**
 * @brief Insere um dado no hash, precisando da chave, e do tamanho do dado.
 * @param h O hash a ter o dado inserido.
 * @param data O dado, que por se tratar de um void*, é generico.
 * @param key A chave de hashing.
 * @param size o tamanho em bytes do dado a ser inserido.
 * @return true Caso bem sucedido.
 * @return false Caso algum erro.
 */
bool hash_insert(Hash *h, void *data, uint64_t key, size_t size);

/**
 * @brief Procura um dado no hash.
 * @param h o hash map a ser procurado.
 * @param key a chave do dado.
 * @param size ponteiro usado como saida, com o tamanho em bytes do dado encontrado.
 * @return void* Um ponteiro para o dado caso encontrado. 
 * @return NULL caso não encontrado, ou em caso de erro.
 */
void *hash_search(Hash *h, uint64_t key, size_t *size);

/**
 * @brief remove um dado do hash map.
 * @param h o hash map a ter o dado removido.
 * @param key a chave do dado removido.
 * @param size tamanho do dado removido.
 * @return void* um ponteiro para o item removido, ou NULL em caso de erro.
 */
void *hash_remove(Hash *h, uint64_t key, size_t *size);

#endif