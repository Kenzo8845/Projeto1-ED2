#include "../include/hash.h"
#include "../Unity/unity.h"
#include <stdlib.h>
#include <stdio.h>

static Hash *hash;

void setUp() {
    hash = hash_create("test_hash.dat", 256);
}

void tearDown() {
    hash_destroy(hash);
    remove("test_hash.dat");
}

void test_hash_deve_ser_criado() {
    TEST_ASSERT_NOT_NULL(hash);
}

void test_insert_deve_retornar_true() {

    int value = 67;

    bool result = hash_insert(hash, &value, 1, sizeof(int));

    TEST_ASSERT_TRUE(result);
}

void test_search_deve_encontrar_elemento() {

    int value = 99;

    hash_insert(hash, &value, 10, sizeof(int));

    size_t size;
    int *result = hash_search(hash, 10, &size);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_INT(sizeof(int), size);
    TEST_ASSERT_EQUAL_INT(99, *result);

    free(result);
}

void test_search_deve_retornar_null_para_chave_inexistente(void) {

    size_t size;
    void *result = hash_search(hash, 6161, &size);

    TEST_ASSERT_NULL(result);
}

void test_remove_deve_remover_elemento(void) {

    int value = 123;

    hash_insert(hash, &value, 50, sizeof(int));

    size_t size;
    int *removed = hash_remove(hash, 50, &size);

    TEST_ASSERT_NOT_NULL(removed);
    TEST_ASSERT_EQUAL_INT(123, *removed);

    free(removed);
}

void test_remove_deve_retornar_null_se_nao_existir(void) {

    size_t size;
    void *removed = hash_remove(hash, 500, &size);

    TEST_ASSERT_NULL(removed);
}

void test_insert_varios_elementos(void) {

    int a = 67;
    int b = 28;
    int c = 9;

    TEST_ASSERT_TRUE(hash_insert(hash, &a, 1, sizeof(int)));
    TEST_ASSERT_TRUE(hash_insert(hash, &b, 2, sizeof(int)));
    TEST_ASSERT_TRUE(hash_insert(hash, &c, 3, sizeof(int)));

    size_t size;

    int *ra = hash_search(hash, 1, &size);
    int *rb = hash_search(hash, 2, &size);
    int *rc = hash_search(hash, 3, &size);

    TEST_ASSERT_EQUAL_INT(67, *ra);
    TEST_ASSERT_EQUAL_INT(28, *rb);
    TEST_ASSERT_EQUAL_INT(9, *rc);

    free(ra);
    free(rb);
    free(rc);
}

void test_insert_e_remover_varios(void) {

    int a = 100;
    int b = 200;

    hash_insert(hash, &a, 11, sizeof(int));
    hash_insert(hash, &b, 22, sizeof(int));

    size_t size;
    int *ra = hash_remove(hash, 11, &size);
    TEST_ASSERT_NOT_NULL(ra);
    TEST_ASSERT_EQUAL_INT(100, *ra);
    free(ra);

    void *check = hash_search(hash, 11, &size);

    TEST_ASSERT_NULL(check);
}


typedef struct {
    int id;
    char nome[20];
} Cachorro;

void test_insert_struct(void) {

    Cachorro c = {1, "Max"};

    hash_insert(hash, &c, 777, sizeof(Cachorro));

    size_t size;

    Cachorro *result = hash_search(hash, 777, &size);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_INT(sizeof(Cachorro), size);
    TEST_ASSERT_EQUAL_INT(1, result->id);
    TEST_ASSERT_EQUAL_STRING("Ana", result->nome);

    free(result);
}

void test_insert_chave_duplicada(void) {

    int a = 5;
    int b = 7;

    hash_insert(hash, &a, 1, sizeof(int));
    hash_insert(hash, &b, 1, sizeof(int));

    size_t size;
    int *result = hash_search(hash, 1, &size);

    TEST_ASSERT_NOT_NULL(result);

    free(result);
}

void test_bucket_split_funciona(void) {
    int values[20];

    for (int i = 0; i < 20; i++) {
        values[i] = i * 10;
        TEST_ASSERT_TRUE(hash_insert(hash, &values[i], i, sizeof(int)));
    }

    size_t size;

    for (int i = 0; i < 20; i++) {
        int *result = hash_search(hash, i, &size);

        TEST_ASSERT_NOT_NULL(result);
        TEST_ASSERT_EQUAL_INT(values[i], *result);

        free(result);
    }
}

int main(void) {

    UNITY_BEGIN();

    RUN_TEST(test_hash_deve_ser_criado);
    RUN_TEST(test_insert_deve_retornar_true);
    RUN_TEST(test_search_deve_encontrar_elemento);
    RUN_TEST(test_search_deve_retornar_null_para_chave_inexistente);
    RUN_TEST(test_remove_deve_remover_elemento);
    RUN_TEST(test_remove_deve_retornar_null_se_nao_existir);

    RUN_TEST(test_insert_varios_elementos);
    RUN_TEST(test_insert_e_remover_varios);
    RUN_TEST(test_insert_struct);
    RUN_TEST(test_insert_chave_duplicada);
    RUN_TEST(test_bucket_split_funciona);

    return UNITY_END();
}
