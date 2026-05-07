#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../Unity/unity.h"
#include "../include/svg.h"
#include "../include/quadra.h"

#define SVG_TEMP "/tmp/test_svg_out.svg"

static FILE *f = NULL;
static Quadra  q = NULL;

void setUp(void) {
    f = fopen(SVG_TEMP, "w+");
    q = quadra_constructor("CEP01", 10.0, 20.0, 100.0, 50.0);
    quadra_setCorp(q, "orange");
    quadra_setCorb(q, "black");
    quadra_setSw(q, 1.0);
}

void tearDown(void) {
    if (f) { fclose(f); f = NULL; }
    quadra_destructor(q);
    remove(SVG_TEMP);
}

/* Lê o conteúdo do arquivo temporário em um buffer heap-alocado. */
static char *ler_arquivo(void) {
    rewind(f);
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    rewind(f);
    char *buf = malloc((size_t)tam + 1);
    TEST_ASSERT_NOT_NULL(buf);
    fread(buf, 1, (size_t)tam, f);
    buf[tam] = '\0';
    return buf;
}

/* --- svg_iniciar / svg_fechar --- */

void test_iniciar_contem_tag_svg(void) {
    svg_iniciar(f);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "<svg"));
    free(buf);
}

void test_fechar_contem_fechamento_svg(void) {
    svg_iniciar(f);
    svg_fechar(f);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "</svg>"));
    free(buf);
}

/* --- svg_desenhar_quadra --- */

void test_desenhar_quadra_contem_rect(void) {
    svg_desenhar_quadra(f, q);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "<rect"));
    free(buf);
}

void test_desenhar_quadra_contem_cep(void) {
    svg_desenhar_quadra(f, q);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "CEP01"));
    free(buf);
}

/* --- svg_marcar_remocao_quadra --- */

void test_remocao_quadra_contem_x_vermelho(void) {
    svg_marcar_remocao_quadra(f, q);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "red"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "X"));
    free(buf);
}

/* --- svg_desenhar_contagem_moradores --- */

void test_contagem_moradores_contem_total(void) {
    svg_desenhar_contagem_moradores(f, q, 1, 2, 3, 4, 10);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "10"));
    free(buf);
}

void test_contagem_moradores_contem_todas_faces(void) {
    svg_desenhar_contagem_moradores(f, q, 1, 2, 3, 4, 10);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "N:"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "S:"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "L:"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "O:"));
    free(buf);
}

/* --- svg_marcar_obito --- */

void test_obito_contem_linhas_vermelhas(void) {
    svg_marcar_obito(f, q, 'S', 20.0);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "<line"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "red"));
    free(buf);
}

/* --- svg_marcar_mudanca --- */

void test_mudanca_contem_rect_e_cpf(void) {
    svg_marcar_mudanca(f, q, 'N', 10.0, "111.222.333-44");
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "<rect"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "111.222.333-44"));
    free(buf);
}

/* --- svg_marcar_despejo --- */

void test_despejo_contem_circulo_preto(void) {
    svg_marcar_despejo(f, q, 'L', 5.0);
    char *buf = ler_arquivo();
    TEST_ASSERT_NOT_NULL(strstr(buf, "<circle"));
    TEST_ASSERT_NOT_NULL(strstr(buf, "black"));
    free(buf);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_iniciar_contem_tag_svg);
    RUN_TEST(test_fechar_contem_fechamento_svg);
    RUN_TEST(test_desenhar_quadra_contem_rect);
    RUN_TEST(test_desenhar_quadra_contem_cep);
    RUN_TEST(test_remocao_quadra_contem_x_vermelho);
    RUN_TEST(test_contagem_moradores_contem_total);
    RUN_TEST(test_contagem_moradores_contem_todas_faces);
    RUN_TEST(test_obito_contem_linhas_vermelhas);
    RUN_TEST(test_mudanca_contem_rect_e_cpf);
    RUN_TEST(test_despejo_contem_circulo_preto);
    return UNITY_END();
}