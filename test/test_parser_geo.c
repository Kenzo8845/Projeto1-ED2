#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "../Unity/unity.h"
#include "../include/parser_geo.h"
#include "../include/hash.h"


#define GEO_TEMP "test_parser_geo_tmp.geo"
#define HF_TEMP  "test_parser_geo_tmp.hf"

static Hash *tabela = NULL;

static uint64_t djb2(const char *str) {
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

static void escrever_geo(const char *conteudo) {
    FILE *f = fopen(GEO_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fputs(conteudo, f);
    fclose(f);
}

void setUp(void) {
    tabela = hash_create(HF_TEMP, 1024);
}

void tearDown(void) {
    hash_destroy(tabela);
    remove(HF_TEMP);
    remove(GEO_TEMP);
}

void test_quadra_inserida_no_hash(void) {
    escrever_geo("q CEP01 10.0 20.0 100.0 50.0\n");
    parser_geo_processar(GEO_TEMP, tabela, NULL);

    size_t sz;
    void *ret = hash_search(tabela, djb2("CEP01"), &sz);
    TEST_ASSERT_NOT_NULL(ret);
    free(ret);
}

void test_multiplas_quadras(void) {
    escrever_geo("q CEP01 0 0 50 50\n"
                 "q CEP02 60 0 50 50\n"
                 "q CEP03 120 0 50 50\n");
    parser_geo_processar(GEO_TEMP, tabela, NULL);

    size_t sz;
    void *r1 = hash_search(tabela, djb2("CEP01"), &sz); TEST_ASSERT_NOT_NULL(r1); free(r1);
    void *r2 = hash_search(tabela, djb2("CEP02"), &sz); TEST_ASSERT_NOT_NULL(r2); free(r2);
    void *r3 = hash_search(tabela, djb2("CEP03"), &sz); TEST_ASSERT_NOT_NULL(r3); free(r3);
}

void test_cq_nao_insere_quadra(void) {
    escrever_geo("cq 1.0 orange black\n");
    parser_geo_processar(GEO_TEMP, tabela, NULL);

    size_t sz;
    void *ret = hash_search(tabela, djb2("orange"), &sz);
    TEST_ASSERT_NULL(ret);
}

void test_arquivo_vazio_nao_trava(void) {
    escrever_geo("");
    parser_geo_processar(GEO_TEMP, tabela, NULL);
    TEST_PASS();
}

void test_arquivo_inexistente_nao_trava(void) {
    parser_geo_processar("nao_existe_xyz.geo", tabela, NULL);
    TEST_PASS();
}

void test_quadra_desenhada_no_svg(void) {
    escrever_geo("q CEP01 10.0 20.0 100.0 50.0\n");

    FILE *svg = fopen("test_parser_geo_tmp.svg", "w+");
    TEST_ASSERT_NOT_NULL(svg);

    parser_geo_processar(GEO_TEMP, tabela, svg);

    rewind(svg);
    fseek(svg, 0, SEEK_END);
    long tam = ftell(svg);
    TEST_ASSERT_GREATER_THAN(0, tam);

    fclose(svg);
    remove("test_parser_geo_tmp.svg");
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_quadra_inserida_no_hash);
    RUN_TEST(test_multiplas_quadras);
    RUN_TEST(test_cq_nao_insere_quadra);
    RUN_TEST(test_arquivo_vazio_nao_trava);
    RUN_TEST(test_arquivo_inexistente_nao_trava);
    RUN_TEST(test_quadra_desenhada_no_svg);
    return UNITY_END();
}