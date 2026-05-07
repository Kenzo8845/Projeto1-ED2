#include <string.h>

#include "../include/quadra.h"
#include "../Unity/unity.h"

static Quadra q = NULL;

void setUp(void) {
    q = quadra_constructor("CEP01", 10.0, 20.0, 100.0, 50.0);
}

void tearDown(void) {
    quadra_destructor(q);
}

/* --- constructor --- */

void test_constructor_nao_retorna_null(void) {
    TEST_ASSERT_NOT_NULL(q);
}

void test_constructor_valores_iniciais(void) {
    TEST_ASSERT_EQUAL_STRING("CEP01", quadra_getCep(q));
    TEST_ASSERT_EQUAL_DOUBLE(10.0,  quadra_getX(q));
    TEST_ASSERT_EQUAL_DOUBLE(20.0,  quadra_getY(q));
    TEST_ASSERT_EQUAL_DOUBLE(100.0, quadra_getW(q));
    TEST_ASSERT_EQUAL_DOUBLE(50.0,  quadra_getH(q));
}

/* --- getSize --- */

void test_get_size_maior_que_zero(void) {
    TEST_ASSERT_GREATER_THAN(0, (int)quadra_getSize());
}

/* --- setters e getters --- */

void test_set_get_cep(void) {
    quadra_setCep(q, "NOVOCEP");
    TEST_ASSERT_EQUAL_STRING("NOVOCEP", quadra_getCep(q));
}

void test_set_get_x(void) {
    quadra_setX(q, 99.5);
    TEST_ASSERT_EQUAL_DOUBLE(99.5, quadra_getX(q));
}

void test_set_get_y(void) {
    quadra_setY(q, 33.0);
    TEST_ASSERT_EQUAL_DOUBLE(33.0, quadra_getY(q));
}

void test_set_get_w(void) {
    quadra_setW(q, 200.0);
    TEST_ASSERT_EQUAL_DOUBLE(200.0, quadra_getW(q));
}

void test_set_get_h(void) {
    quadra_setH(q, 75.0);
    TEST_ASSERT_EQUAL_DOUBLE(75.0, quadra_getH(q));
}

void test_set_get_corp(void) {
    quadra_setCorp(q, "blue");
    TEST_ASSERT_EQUAL_STRING("blue", quadra_getCorp(q));
}

void test_set_get_corb(void) {
    quadra_setCorb(q, "red");
    TEST_ASSERT_EQUAL_STRING("red", quadra_getCorb(q));
}

void test_set_get_sw(void) {
    quadra_setSw(q, 2.5);
    TEST_ASSERT_FLOAT_WITHIN(0.001, 2.5, quadra_getSw(q));
}

/* --- destructor com NULL não deve travar --- */

void test_destructor_null_seguro(void) {
    quadra_destructor(NULL);
    TEST_PASS();
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_constructor_nao_retorna_null);
    RUN_TEST(test_constructor_valores_iniciais);
    RUN_TEST(test_get_size_maior_que_zero);
    RUN_TEST(test_set_get_cep);
    RUN_TEST(test_set_get_x);
    RUN_TEST(test_set_get_y);
    RUN_TEST(test_set_get_w);
    RUN_TEST(test_set_get_h);
    RUN_TEST(test_set_get_corp);
    RUN_TEST(test_set_get_corb);
    RUN_TEST(test_set_get_sw);
    RUN_TEST(test_destructor_null_seguro);
    return UNITY_END();
}