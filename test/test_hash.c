#include <stdlib.h>
#include <string.h>

#include "../Unity/unity.h"
#include "../include/hash.h"

static Hash *h = NULL;

void setUp(void) {
    h = hash_create("/tmp/test_hash.hf", 512);
}

void tearDown(void) {
    hash_destroy(h);
    remove("/tmp/test_hash.hf");
}

/* --- insert + search --- */

void test_insert_e_search_simples(void) {
    int dado = 42;
    TEST_ASSERT_TRUE(hash_insert(h, &dado, 1, sizeof(int)));

    size_t sz;
    int *ret = hash_search(h, 1, &sz);
    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_EQUAL_INT(sizeof(int), sz);
    TEST_ASSERT_EQUAL_INT(42, *ret);
    free(ret);
}

void test_search_chave_inexistente_retorna_null(void) {
    size_t sz;
    void *ret = hash_search(h, 9999, &sz);
    TEST_ASSERT_NULL(ret);
}

void test_insert_multiplos_registros(void) {
    for (uint64_t i = 0; i < 10; i++) {
        int val = (int)(i * 100);
        TEST_ASSERT_TRUE(hash_insert(h, &val, i, sizeof(int)));
    }

    for (uint64_t i = 0; i < 10; i++) {
        size_t sz;
        int *ret = hash_search(h, i, &sz);
        TEST_ASSERT_NOT_NULL(ret);
        TEST_ASSERT_EQUAL_INT((int)(i * 100), *ret);
        free(ret);
    }
}

/* --- remove --- */

void test_remove_retorna_dado_correto(void) {
    int dado = 7;
    hash_insert(h, &dado, 10, sizeof(int));

    size_t sz;
    int *ret = hash_remove(h, 10, &sz);
    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_EQUAL_INT(7, *ret);
    free(ret);
}

void test_search_apos_remove_retorna_null(void) {
    int dado = 7;
    hash_insert(h, &dado, 10, sizeof(int));

    size_t sz;
    free(hash_remove(h, 10, &sz));

    void *ret = hash_search(h, 10, &sz);
    TEST_ASSERT_NULL(ret);
}

void test_remove_chave_inexistente_retorna_null(void) {
    size_t sz;
    void *ret = hash_remove(h, 8888, &sz);
    TEST_ASSERT_NULL(ret);
}

/* --- split / crescimento do diretório --- */

void test_hash_cresce_sem_perder_dados(void) {
    /*
     * Com bucket_size=512 e registros de int (4 bytes), forçamos vários
     * splits ao inserir muitos registros com chaves variadas.
     */
    int n = 200;
    for (int i = 0; i < n; i++) {
        TEST_ASSERT_TRUE(hash_insert(h, &i, (uint64_t)i, sizeof(int)));
    }

    for (int i = 0; i < n; i++) {
        size_t sz;
        int *ret = hash_search(h, (uint64_t)i, &sz);
        TEST_ASSERT_NOT_NULL(ret);
        TEST_ASSERT_EQUAL_INT(i, *ret);
        free(ret);
    }
}

/* --- dado de tamanho variável --- */

void test_dado_tamanho_variavel(void) {
    char texto[] = "estrutura de dados";
    TEST_ASSERT_TRUE(hash_insert(h, texto, 77, strlen(texto) + 1));

    size_t sz;
    char *ret = hash_search(h, 77, &sz);
    TEST_ASSERT_NOT_NULL(ret);
    TEST_ASSERT_EQUAL_STRING("estrutura de dados", ret);
    free(ret);
}

/* --- acesso interno (necessário para varredura) --- */

void test_get_file_nao_nulo(void) {
    TEST_ASSERT_NOT_NULL(hash_get_file(h));
}

void test_get_num_buckets_pelo_menos_dois(void) {
    TEST_ASSERT_GREATER_OR_EQUAL(2, hash_get_num_buckets(h));
}

void test_get_bucket_size_correto(void) {
    TEST_ASSERT_EQUAL_size_t(512, hash_get_bucket_size(h));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_insert_e_search_simples);
    RUN_TEST(test_search_chave_inexistente_retorna_null);
    RUN_TEST(test_insert_multiplos_registros);
    RUN_TEST(test_remove_retorna_dado_correto);
    RUN_TEST(test_search_apos_remove_retorna_null);
    RUN_TEST(test_remove_chave_inexistente_retorna_null);
    RUN_TEST(test_hash_cresce_sem_perder_dados);
    RUN_TEST(test_dado_tamanho_variavel);
    RUN_TEST(test_get_file_nao_nulo);
    RUN_TEST(test_get_num_buckets_pelo_menos_dois);
    RUN_TEST(test_get_bucket_size_correto);
    return UNITY_END();
}