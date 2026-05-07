#include <string.h>

#include "../Unity/unity.h"
#include "../include/habitante.h"

static Habitante h = NULL;

void setUp(void) {
    h = habitante_constructor("123.456.789-00", "Joao", "Silva", 'M', "01/01/1990");
}

void tearDown(void) {
    habitante_destructor(h);
}

/* --- constructor --- */

void test_constructor_nao_retorna_null(void) {
    TEST_ASSERT_NOT_NULL(h);
}

void test_constructor_dados_iniciais(void) {
    TEST_ASSERT_EQUAL_STRING("123.456.789-00", habitante_getCpf(h));
    TEST_ASSERT_EQUAL_STRING("Joao", habitante_getNome(h));
    TEST_ASSERT_EQUAL_STRING("Silva", habitante_getSobrenome(h));
    TEST_ASSERT_EQUAL_CHAR('M', habitante_getSexo(h));
    TEST_ASSERT_EQUAL_STRING("01/01/1990",habitante_getDataNascimento(h));
}

void test_constructor_novo_habitante_e_sem_teto(void) {
    TEST_ASSERT_TRUE(habitante_isSemTeto(h));
}

/* --- getSize --- */

void test_get_size_maior_que_zero(void) {
    TEST_ASSERT_GREATER_THAN(0, (int)habitante_getSize());
}

/* --- setEndereco --- */

void test_set_endereco_remove_status_sem_teto(void) {
    habitante_setEndereco(h, "CEP01", 'S', 30.0, "Apto 2");
    TEST_ASSERT_FALSE(habitante_isSemTeto(h));
}

void test_set_endereco_valores_corretos(void) {
    habitante_setEndereco(h, "CEP01", 'N', 15.5, "Casa");
    TEST_ASSERT_EQUAL_STRING("CEP01", habitante_getCep(h));
    TEST_ASSERT_EQUAL_CHAR('N', habitante_getFace(h));
    TEST_ASSERT_FLOAT_WITHIN(0.001, 15.5, habitante_getNumeroCasa(h));
    TEST_ASSERT_EQUAL_STRING("Casa", habitante_getComplemento(h));
}

/* --- getters de endereço retornam valores sentinela se sem-teto --- */

void test_cep_null_se_sem_teto(void) {
    TEST_ASSERT_NULL(habitante_getCep(h));
}

void test_face_nula_se_sem_teto(void) {
    TEST_ASSERT_EQUAL_CHAR('\0', habitante_getFace(h));
}

void test_numero_negativo_se_sem_teto(void) {
    TEST_ASSERT_FLOAT_WITHIN(0.001, -1.0, habitante_getNumeroCasa(h));
}

void test_complemento_null_se_sem_teto(void) {
    TEST_ASSERT_NULL(habitante_getComplemento(h));
}

/* --- setters individuais --- */

void test_set_get_cpf(void) {
    habitante_setCpf(h, "000.000.000-00");
    TEST_ASSERT_EQUAL_STRING("000.000.000-00", habitante_getCpf(h));
}

void test_set_get_nome(void) {
    habitante_setNome(h, "Maria");
    TEST_ASSERT_EQUAL_STRING("Maria", habitante_getNome(h));
}

void test_set_get_sobrenome(void) {
    habitante_setSobrenome(h, "Souza");
    TEST_ASSERT_EQUAL_STRING("Souza", habitante_getSobrenome(h));
}

void test_set_get_sexo(void) {
    habitante_setSexo(h, 'F');
    TEST_ASSERT_EQUAL_CHAR('F', habitante_getSexo(h));
}

void test_set_get_data_nascimento(void) {
    habitante_setDataNascimento(h, "15/06/2000");
    TEST_ASSERT_EQUAL_STRING("15/06/2000", habitante_getDataNascimento(h));
}

void test_set_sem_teto_manual(void) {
    habitante_setEndereco(h, "CEP01", 'S', 10.0, "");
    TEST_ASSERT_FALSE(habitante_isSemTeto(h));

    habitante_setSemTeto(h, true);
    TEST_ASSERT_TRUE(habitante_isSemTeto(h));
}

/* --- destructor com NULL não deve travar --- */

void test_destructor_null_seguro(void) {
    habitante_destructor(NULL);
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_constructor_nao_retorna_null);
    RUN_TEST(test_constructor_dados_iniciais);
    RUN_TEST(test_constructor_novo_habitante_e_sem_teto);
    RUN_TEST(test_get_size_maior_que_zero);
    RUN_TEST(test_set_endereco_remove_status_sem_teto);
    RUN_TEST(test_set_endereco_valores_corretos);
    RUN_TEST(test_cep_null_se_sem_teto);
    RUN_TEST(test_face_nula_se_sem_teto);
    RUN_TEST(test_numero_negativo_se_sem_teto);
    RUN_TEST(test_complemento_null_se_sem_teto);
    RUN_TEST(test_set_get_cpf);
    RUN_TEST(test_set_get_nome);
    RUN_TEST(test_set_get_sobrenome);
    RUN_TEST(test_set_get_sexo);
    RUN_TEST(test_set_get_data_nascimento);
    RUN_TEST(test_set_sem_teto_manual);
    RUN_TEST(test_destructor_null_seguro);
    return UNITY_END();
}