#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../include/parser_pm.h"
#include "../include/hash.h"
#include "../include/habitante.h"
#include "../Unity/unity.h"

#define PM_TEMP "/tmp/test.pm"
#define HF_TEMP "/tmp/test_pm.hf"

static Hash *tabela = NULL;

/* Converte CPF em chave — idêntico ao usado em parser_pm.c */
static uint64_t cpf_para_chave(const char *cpf) {
    uint64_t chave = 0;
    for (int i = 0; cpf[i] != '\0'; i++)
        if (cpf[i] >= '0' && cpf[i] <= '9')
            chave = chave * 10 + (cpf[i] - '0');
    return chave;
}

static void escrever_pm(const char *conteudo) {
    FILE *f = fopen(PM_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fputs(conteudo, f);
    fclose(f);
}

void setUp(void) {
    tabela = hash_create(HF_TEMP, 2048);
}

void tearDown(void) {
    hash_destroy(tabela);
    remove(HF_TEMP);
    remove(PM_TEMP);
}

/* --- habitante inserido com comando p --- */

void test_habitante_inserido(void) {
    escrever_pm("p 12345678900 Joao Silva M 01/01/1990\n");
    parser_pm_processar(PM_TEMP, tabela);

    size_t sz;
    Habitante h = hash_search(tabela, cpf_para_chave("12345678900"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_STRING("Joao", habitante_getNome(h));
    habitante_destructor(h);
}

/* --- novo habitante começa como sem-teto --- */

void test_habitante_inicia_sem_teto(void) {
    escrever_pm("p 12345678900 Joao Silva M 01/01/1990\n");
    parser_pm_processar(PM_TEMP, tabela);

    size_t sz;
    Habitante h = hash_search(tabela, cpf_para_chave("12345678900"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_TRUE(habitante_isSemTeto(h));
    habitante_destructor(h);
}

/* --- comando m atribui endereço e remove sem-teto --- */

void test_comando_m_define_endereco(void) {
    escrever_pm("p 12345678900 Joao Silva M 01/01/1990\n"
                "m 12345678900 CEP01 S 25.0 Apto3\n");
    parser_pm_processar(PM_TEMP, tabela);

    size_t sz;
    Habitante h = hash_search(tabela, cpf_para_chave("12345678900"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_FALSE(habitante_isSemTeto(h));
    TEST_ASSERT_EQUAL_STRING("CEP01", habitante_getCep(h));
    TEST_ASSERT_EQUAL_CHAR('S', habitante_getFace(h));
    habitante_destructor(h);
}

/* --- múltiplos habitantes --- */

void test_multiplos_habitantes(void) {
    escrever_pm("p 11111111111 Ana Souza F 10/05/1985\n"
                "p 22222222222 Carlos Lima M 22/03/1970\n");
    parser_pm_processar(PM_TEMP, tabela);

    size_t sz;
    Habitante h1 = hash_search(tabela, cpf_para_chave("11111111111"), &sz);
    Habitante h2 = hash_search(tabela, cpf_para_chave("22222222222"), &sz);
    TEST_ASSERT_NOT_NULL(h1);
    TEST_ASSERT_NOT_NULL(h2);
    TEST_ASSERT_EQUAL_STRING("Ana",    habitante_getNome(h1));
    TEST_ASSERT_EQUAL_STRING("Carlos", habitante_getNome(h2));
    habitante_destructor(h1);
    habitante_destructor(h2);
}

/* --- m com CPF inexistente não trava --- */

void test_m_cpf_inexistente_nao_trava(void) {
    escrever_pm("m 99999999999 CEP01 N 10.0 Casa\n");
    parser_pm_processar(PM_TEMP, tabela);
    TEST_PASS();
}

/* --- arquivo vazio não trava --- */

void test_arquivo_vazio_nao_trava(void) {
    escrever_pm("");
    parser_pm_processar(PM_TEMP, tabela);
    TEST_PASS();
}

/* --- arquivo inexistente não trava --- */

void test_arquivo_inexistente_nao_trava(void) {
    parser_pm_processar("/tmp/nao_existe_xyz.pm", tabela);
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_habitante_inserido);
    RUN_TEST(test_habitante_inicia_sem_teto);
    RUN_TEST(test_comando_m_define_endereco);
    RUN_TEST(test_multiplos_habitantes);
    RUN_TEST(test_m_cpf_inexistente_nao_trava);
    RUN_TEST(test_arquivo_vazio_nao_trava);
    RUN_TEST(test_arquivo_inexistente_nao_trava);
    return UNITY_END();
}