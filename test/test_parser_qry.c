#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../Unity/unity.h"
#include "../include/parser_qry.h"
#include "../include/hash.h"
#include "../include/quadra.h"
#include "../include/habitante.h"

/* =========================================================================
   CAMINHOS DE ARQUIVOS TEMPORÁRIOS
   ========================================================================= */

#define HF_QUADRAS    "/tmp/tqry_quadras.hf"
#define HF_HAB        "/tmp/tqry_hab.hf"
#define QRY_TEMP      "/tmp/tqry_test.qry"
#define TXT_TEMP      "/tmp/tqry_out.txt"
#define SVG_TEMP      "/tmp/tqry_out.svg"

/* =========================================================================
   VARIÁVEIS GLOBAIS DE FIXTURE
   ========================================================================= */

static Hash *quadras    = NULL;
static Hash *habitantes = NULL;
static FILE *txt        = NULL;
static FILE *svg        = NULL;

/* =========================================================================
   FUNÇÕES AUXILIARES
   ========================================================================= */

/* djb2 — igual à usada internamente em parser_geo/parser_qry */
static uint64_t djb2(const char *str) {
    uint64_t h = 5381;
    int c;
    while ((c = *str++)) h = ((h << 5) + h) + c;
    return h;
}

/* Converte CPF (com ou sem formatação) em chave numérica */
static uint64_t cpf_key(const char *cpf) {
    uint64_t k = 0;
    for (int i = 0; cpf[i]; i++)
        if (cpf[i] >= '0' && cpf[i] <= '9')
            k = k * 10 + (uint64_t)(cpf[i] - '0');
    return k;
}

/* Insere uma quadra no hash de quadras */
static void inserir_quadra(Hash *q_hash, const char *cep,
                            double x, double y, double w, double h) {
    Quadra q = quadra_constructor(cep, x, y, w, h);
    quadra_setCorp(q, "orange");
    quadra_setCorb(q, "black");
    quadra_setSw(q, 1.0);
    hash_insert(q_hash, q, djb2(cep), quadra_getSize());
    quadra_destructor(q);
}

/* Insere um habitante sem endereço no hash de habitantes */
static void inserir_habitante(Hash *h_hash, const char *cpf,
                               const char *nome, const char *sobrenome,
                               char sexo, const char *nasc) {
    Habitante h = habitante_constructor(cpf, nome, sobrenome, sexo, nasc);
    hash_insert(h_hash, h, cpf_key(cpf), habitante_getSize());
    habitante_destructor(h);
}

/* Insere habitante já com endereço */
static void inserir_morador(Hash *h_hash, const char *cpf,
                             const char *nome, const char *sobrenome,
                             char sexo, const char *nasc,
                             const char *cep, char face,
                             double num, const char *compl) {
    Habitante h = habitante_constructor(cpf, nome, sobrenome, sexo, nasc);
    habitante_setEndereco(h, (char *)cep, face, num, (char *)compl);
    hash_insert(h_hash, h, cpf_key(cpf), habitante_getSize());
    habitante_destructor(h);
}

/* Escreve conteúdo no arquivo .qry temporário */
static void escrever_qry(const char *conteudo) {
    FILE *f = fopen(QRY_TEMP, "w");
    TEST_ASSERT_NOT_NULL(f);
    fputs(conteudo, f);
    fclose(f);
}

/* Lê o conteúdo do arquivo .txt de saída em buffer heap-alocado */
static char *ler_txt(void) {
    rewind(txt);
    fseek(txt, 0, SEEK_END);
    long tam = ftell(txt);
    rewind(txt);
    if (tam <= 0) {
        char *vazio = malloc(1);
        TEST_ASSERT_NOT_NULL(vazio);
        vazio[0] = '\0';
        return vazio;
    }
    char *buf = malloc((size_t)tam + 1);
    TEST_ASSERT_NOT_NULL(buf);
    fread(buf, 1, (size_t)tam, txt);
    buf[tam] = '\0';
    return buf;
}

/* Verifica se o arquivo txt contém a substring esperada */
static void assert_txt_contem(const char *sub) {
    char *buf = ler_txt();
    int ok = (strstr(buf, sub) != NULL);
    free(buf);
    TEST_ASSERT_TRUE_MESSAGE(ok, sub);
}

/* =========================================================================
   SETUP / TEARDOWN
   ========================================================================= */

void setUp(void) {
    quadras    = hash_create(HF_QUADRAS, 2048);
    habitantes = hash_create(HF_HAB,     2048);
    txt        = fopen(TXT_TEMP, "w+");
    svg        = fopen(SVG_TEMP, "w+");
    TEST_ASSERT_NOT_NULL(quadras);
    TEST_ASSERT_NOT_NULL(habitantes);
    TEST_ASSERT_NOT_NULL(txt);
    TEST_ASSERT_NOT_NULL(svg);
}

void tearDown(void) {
    hash_destroy(quadras);
    hash_destroy(habitantes);
    if (txt) { fclose(txt); txt = NULL; }
    if (svg) { fclose(svg); svg = NULL; }
    remove(HF_QUADRAS);
    remove(HF_HAB);
    remove(QRY_TEMP);
    remove(TXT_TEMP);
    remove(SVG_TEMP);
}

/* =========================================================================
   TESTES — ROBUSTEZ BÁSICA
   ========================================================================= */

void test_arquivo_vazio_nao_trava(void) {
    escrever_qry("");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    TEST_PASS();
}

void test_arquivo_inexistente_nao_trava(void) {
    /* parser_qry abre com assert — se o arquivo não existe ele perror e retorna */
    /* O assert interno exige args não-nulos, então passamos args válidos        */
    /* Nota: a implementação faz perror e return se fopen falha — não aborta.   */
    parser_qry_processar("/tmp/nao_existe_xyz_qry.qry",
                         quadras, habitantes, txt, svg);
    TEST_PASS();
}

void test_comando_desconhecido_nao_trava(void) {
    escrever_qry("xyzzy CEP01\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    TEST_PASS();
}

/* =========================================================================
   TESTES — COMANDO rq (remoção de quadra)
   ========================================================================= */

void test_rq_remove_quadra_do_hash(void) {
    inserir_quadra(quadras, "CEP01", 0, 0, 100, 50);
    escrever_qry("rq CEP01\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    size_t sz;
    void *ret = hash_search(quadras, djb2("CEP01"), &sz);
    TEST_ASSERT_NULL(ret);
}

void test_rq_escreve_cep_no_txt(void) {
    inserir_quadra(quadras, "CEP01", 0, 0, 100, 50);
    escrever_qry("rq CEP01\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("CEP01");
}

void test_rq_quadra_inexistente_reporta_no_txt(void) {
    escrever_qry("rq CEPXX\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    /* deve reportar algo — não pode travar e o txt deve ter conteúdo */
    char *buf = ler_txt();
    TEST_ASSERT_GREATER_THAN(0, (int)strlen(buf));
    free(buf);
}

void test_rq_moradores_tornam_sem_teto(void) {
    inserir_quadra(quadras, "CEP01", 0, 0, 100, 50);
    inserir_morador(habitantes, "11111111111", "Ana", "Lima", 'F',
                    "01/01/1990", "CEP01", 'N', 10.0, "");

    escrever_qry("rq CEP01\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    /* Após rq, o habitante deve estar sem-teto */
    size_t sz;
    Habitante h = hash_search(habitantes, cpf_key("11111111111"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_TRUE(habitante_isSemTeto(h));
    habitante_destructor(h);
}

void test_rq_lista_cpf_do_morador_desabrigado(void) {
    inserir_quadra(quadras, "CEP01", 0, 0, 100, 50);
    inserir_morador(habitantes, "11111111111", "Ana", "Lima", 'F',
                    "01/01/1990", "CEP01", 'N', 10.0, "");

    escrever_qry("rq CEP01\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    assert_txt_contem("11111111111");
}

void test_rq_escreve_no_svg(void) {
    inserir_quadra(quadras, "CEP01", 10, 20, 100, 50);
    escrever_qry("rq CEP01\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    rewind(svg);
    fseek(svg, 0, SEEK_END);
    long tam = ftell(svg);
    TEST_ASSERT_GREATER_THAN(0, tam);
}

/* =========================================================================
   TESTES — COMANDO pq (contagem de moradores)
   ========================================================================= */

void test_pq_quadra_inexistente_reporta_no_txt(void) {
    escrever_qry("pq CEPXX\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    char *buf = ler_txt();
    TEST_ASSERT_GREATER_THAN(0, (int)strlen(buf));
    free(buf);
}

void test_pq_sem_moradores_total_zero(void) {
    inserir_quadra(quadras, "CEP02", 0, 0, 80, 40);
    escrever_qry("pq CEP02\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("Total=0");
}

void test_pq_conta_morador_correto(void) {
    inserir_quadra(quadras, "CEP02", 0, 0, 80, 40);
    inserir_morador(habitantes, "22222222222", "Carlos", "Souza", 'M',
                    "10/05/1985", "CEP02", 'S', 15.0, "");

    escrever_qry("pq CEP02\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("Total=1");
}

void test_pq_conta_faces_corretas(void) {
    inserir_quadra(quadras, "CEP02", 0, 0, 80, 40);
    inserir_morador(habitantes, "22222222222", "Carlos", "Souza", 'M',
                    "10/05/1985", "CEP02", 'N', 10.0, "");
    inserir_morador(habitantes, "33333333333", "Maria",  "Costa", 'F',
                    "20/06/1992", "CEP02", 'S', 20.0, "");

    escrever_qry("pq CEP02\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("N=1");
    assert_txt_contem("S=1");
}

void test_pq_nao_conta_sem_teto(void) {
    inserir_quadra(quadras, "CEP02", 0, 0, 80, 40);
    /* insere sem endereço — sem-teto */
    inserir_habitante(habitantes, "44444444444", "Jose", "Silva", 'M',
                      "05/05/1980");

    escrever_qry("pq CEP02\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("Total=0");
}

/* =========================================================================
   TESTES — COMANDO censo
   ========================================================================= */

void test_censo_sem_habitantes_nao_trava(void) {
    escrever_qry("censo\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("CENSO");
}

void test_censo_conta_total_habitantes(void) {
    inserir_habitante(habitantes, "11111111111", "Ana",    "Lima",  'F', "01/01/1990");
    inserir_habitante(habitantes, "22222222222", "Carlos", "Souza", 'M', "10/05/1985");

    escrever_qry("censo\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("2");
}

void test_censo_distingue_moradores_e_sem_teto(void) {
    inserir_quadra(quadras, "CEP03", 0, 0, 50, 50);
    inserir_morador(habitantes, "11111111111", "Ana", "Lima", 'F',
                    "01/01/1990", "CEP03", 'N', 5.0, "");
    inserir_habitante(habitantes, "22222222222", "Carlos", "Souza", 'M',
                      "10/05/1985");

    escrever_qry("censo\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    /* deve reportar 1 morador e 1 sem-teto */
    assert_txt_contem("moradores");
    assert_txt_contem("Sem-teto");
}

void test_censo_reporta_percentuais(void) {
    inserir_habitante(habitantes, "11111111111", "Ana",    "Lima",  'F', "01/01/1990");
    inserir_habitante(habitantes, "22222222222", "Carlos", "Souza", 'M', "10/05/1985");

    escrever_qry("censo\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    /* percentuais aparecem com '%' */
    assert_txt_contem("%");
}

/* =========================================================================
   TESTES — COMANDO h? (consulta de habitante)
   ========================================================================= */

void test_h_interrogacao_encontra_habitante(void) {
    inserir_habitante(habitantes, "55555555555", "Lucas", "Ferreira", 'M',
                      "12/12/1995");

    escrever_qry("h? 55555555555\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("Lucas");
}

void test_h_interrogacao_cpf_inexistente_reporta(void) {
    escrever_qry("h? 99999999999\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    char *buf = ler_txt();
    TEST_ASSERT_GREATER_THAN(0, (int)strlen(buf));
    free(buf);
}

void test_h_interrogacao_morador_reporta_endereco(void) {
    inserir_morador(habitantes, "55555555555", "Lucas", "Ferreira", 'M',
                    "12/12/1995", "CEP04", 'S', 30.0, "Apto1");

    escrever_qry("h? 55555555555\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("CEP04");
}

/* =========================================================================
   TESTES — COMANDO nasc (nascimento)
   ========================================================================= */

void test_nasc_insere_habitante_no_hash(void) {
    escrever_qry("nasc 66666666666 Pedro Santos M 15/08/2000\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    size_t sz;
    Habitante h = hash_search(habitantes, cpf_key("66666666666"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_EQUAL_STRING("Pedro", habitante_getNome(h));
    habitante_destructor(h);
}

void test_nasc_novo_habitante_e_sem_teto(void) {
    escrever_qry("nasc 66666666666 Pedro Santos M 15/08/2000\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    size_t sz;
    Habitante h = hash_search(habitantes, cpf_key("66666666666"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_TRUE(habitante_isSemTeto(h));
    habitante_destructor(h);
}

void test_nasc_reporta_no_txt(void) {
    escrever_qry("nasc 66666666666 Pedro Santos M 15/08/2000\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("Pedro");
}

/* =========================================================================
   TESTES — COMANDO rip (falecimento)
   ========================================================================= */

void test_rip_remove_habitante_do_hash(void) {
    inserir_habitante(habitantes, "77777777777", "Bia", "Ramos", 'F',
                      "03/03/1975");

    escrever_qry("rip 77777777777\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    size_t sz;
    void *ret = hash_search(habitantes, cpf_key("77777777777"), &sz);
    TEST_ASSERT_NULL(ret);
}

void test_rip_reporta_dados_no_txt(void) {
    inserir_habitante(habitantes, "77777777777", "Bia", "Ramos", 'F',
                      "03/03/1975");

    escrever_qry("rip 77777777777\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("Bia");
}

void test_rip_cpf_inexistente_reporta(void) {
    escrever_qry("rip 00000000000\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    char *buf = ler_txt();
    TEST_ASSERT_GREATER_THAN(0, (int)strlen(buf));
    free(buf);
}

void test_rip_morador_marca_svg(void) {
    inserir_quadra(quadras, "CEP05", 10, 10, 80, 40);
    inserir_morador(habitantes, "77777777777", "Bia", "Ramos", 'F',
                    "03/03/1975", "CEP05", 'N', 20.0, "");

    escrever_qry("rip 77777777777\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    rewind(svg);
    fseek(svg, 0, SEEK_END);
    long tam = ftell(svg);
    TEST_ASSERT_GREATER_THAN(0, tam);
}

/* =========================================================================
   TESTES — COMANDO mud (mudança de endereço)
   ========================================================================= */

void test_mud_atualiza_endereco(void) {
    inserir_quadra(quadras, "CEP06", 0, 0, 100, 50);
    inserir_morador(habitantes, "88888888888", "Tiago", "Nunes", 'M',
                    "07/07/1988", "CEP06", 'S', 10.0, "");

    escrever_qry("mud 88888888888 CEP06 N 25.0 Apto5\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    size_t sz;
    Habitante h = hash_search(habitantes, cpf_key("88888888888"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_FALSE(habitante_isSemTeto(h));
    TEST_ASSERT_EQUAL_CHAR('N', habitante_getFace(h));
    habitante_destructor(h);
}

void test_mud_reporta_no_txt(void) {
    inserir_quadra(quadras, "CEP06", 0, 0, 100, 50);
    inserir_morador(habitantes, "88888888888", "Tiago", "Nunes", 'M',
                    "07/07/1988", "CEP06", 'S', 10.0, "");

    escrever_qry("mud 88888888888 CEP06 N 25.0 Apto5\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("88888888888");
}

void test_mud_cpf_inexistente_nao_trava(void) {
    escrever_qry("mud 00000000001 CEP06 N 5.0 Casa\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    TEST_PASS();
}

/* =========================================================================
   TESTES — COMANDO dspj (despejo)
   ========================================================================= */

void test_dspj_transforma_morador_em_sem_teto(void) {
    inserir_quadra(quadras, "CEP07", 0, 0, 60, 60);
    inserir_morador(habitantes, "99999999999", "Lara", "Vaz", 'F',
                    "22/11/1993", "CEP07", 'O', 5.0, "");

    escrever_qry("dspj 99999999999\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    size_t sz;
    Habitante h = hash_search(habitantes, cpf_key("99999999999"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_TRUE(habitante_isSemTeto(h));
    habitante_destructor(h);
}

void test_dspj_reporta_no_txt(void) {
    inserir_quadra(quadras, "CEP07", 0, 0, 60, 60);
    inserir_morador(habitantes, "99999999999", "Lara", "Vaz", 'F',
                    "22/11/1993", "CEP07", 'O', 5.0, "");

    escrever_qry("dspj 99999999999\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    assert_txt_contem("Lara");
}

void test_dspj_sem_teto_ignorado(void) {
    /* sem-teto não pode ser despejado — deve reportar aviso e não travar */
    inserir_habitante(habitantes, "99999999999", "Lara", "Vaz", 'F',
                      "22/11/1993");

    escrever_qry("dspj 99999999999\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    /* continua sem-teto */
    size_t sz;
    Habitante h = hash_search(habitantes, cpf_key("99999999999"), &sz);
    TEST_ASSERT_NOT_NULL(h);
    TEST_ASSERT_TRUE(habitante_isSemTeto(h));
    habitante_destructor(h);
}

void test_dspj_cpf_inexistente_nao_trava(void) {
    escrever_qry("dspj 00000000002\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);
    TEST_PASS();
}

void test_dspj_marca_circulo_no_svg(void) {
    inserir_quadra(quadras, "CEP07", 0, 0, 60, 60);
    inserir_morador(habitantes, "99999999999", "Lara", "Vaz", 'F',
                    "22/11/1993", "CEP07", 'O', 5.0, "");

    escrever_qry("dspj 99999999999\n");
    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    rewind(svg);
    fseek(svg, 0, SEEK_END);
    long tam = ftell(svg);
    TEST_ASSERT_GREATER_THAN(0, tam);
}

/* =========================================================================
   TESTES — SEQUÊNCIAS DE MÚLTIPLOS COMANDOS
   ========================================================================= */

void test_sequencia_nasc_mud_h_interrogacao(void) {
    inserir_quadra(quadras, "CEP08", 0, 0, 120, 60);

    escrever_qry("nasc 12312312312 Julia Moura F 30/09/2001\n"
                 "mud 12312312312 CEP08 L 40.0 Sala2\n"
                 "h? 12312312312\n");

    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    /* h? deve ter reportado o endereço atualizado */
    assert_txt_contem("CEP08");
    assert_txt_contem("Julia");
}

void test_sequencia_nasc_rip_busca_null(void) {
    escrever_qry("nasc 55566677788 Fulano Tal M 01/01/2000\n"
                 "rip 55566677788\n");

    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    size_t sz;
    void *ret = hash_search(habitantes, cpf_key("55566677788"), &sz);
    TEST_ASSERT_NULL(ret);
}

void test_sequencia_rq_depois_pq_nao_encontra(void) {
    inserir_quadra(quadras, "CEP09", 0, 0, 50, 50);

    escrever_qry("rq CEP09\n"
                 "pq CEP09\n");

    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    /* pq após rq deve reportar não-encontrada */
    assert_txt_contem("CEP09");
}

void test_sequencia_censo_apos_varios_comandos(void) {
    inserir_quadra(quadras, "CEP10", 0, 0, 100, 50);

    escrever_qry("nasc 11122233344 Rosa Lima F 11/11/1991\n"
                 "mud 11122233344 CEP10 S 5.0 \n"
                 "nasc 55544433322 Joao Braga M 22/02/1980\n"
                 "censo\n");

    parser_qry_processar(QRY_TEMP, quadras, habitantes, txt, svg);

    /* censo deve reportar 2 habitantes */
    assert_txt_contem("2");
}

/* =========================================================================
   MAIN
   ========================================================================= */

int main(void) {
    UNITY_BEGIN();

    /* Robustez básica */
    RUN_TEST(test_arquivo_vazio_nao_trava);
    RUN_TEST(test_arquivo_inexistente_nao_trava);
    RUN_TEST(test_comando_desconhecido_nao_trava);

    /* rq */
    RUN_TEST(test_rq_remove_quadra_do_hash);
    RUN_TEST(test_rq_escreve_cep_no_txt);
    RUN_TEST(test_rq_quadra_inexistente_reporta_no_txt);
    RUN_TEST(test_rq_moradores_tornam_sem_teto);
    RUN_TEST(test_rq_lista_cpf_do_morador_desabrigado);
    RUN_TEST(test_rq_escreve_no_svg);

    /* pq */
    RUN_TEST(test_pq_quadra_inexistente_reporta_no_txt);
    RUN_TEST(test_pq_sem_moradores_total_zero);
    RUN_TEST(test_pq_conta_morador_correto);
    RUN_TEST(test_pq_conta_faces_corretas);
    RUN_TEST(test_pq_nao_conta_sem_teto);

    /* censo */
    RUN_TEST(test_censo_sem_habitantes_nao_trava);
    RUN_TEST(test_censo_conta_total_habitantes);
    RUN_TEST(test_censo_distingue_moradores_e_sem_teto);
    RUN_TEST(test_censo_reporta_percentuais);

    /* h? */
    RUN_TEST(test_h_interrogacao_encontra_habitante);
    RUN_TEST(test_h_interrogacao_cpf_inexistente_reporta);
    RUN_TEST(test_h_interrogacao_morador_reporta_endereco);

    /* nasc */
    RUN_TEST(test_nasc_insere_habitante_no_hash);
    RUN_TEST(test_nasc_novo_habitante_e_sem_teto);
    RUN_TEST(test_nasc_reporta_no_txt);

    /* rip */
    RUN_TEST(test_rip_remove_habitante_do_hash);
    RUN_TEST(test_rip_reporta_dados_no_txt);
    RUN_TEST(test_rip_cpf_inexistente_reporta);
    RUN_TEST(test_rip_morador_marca_svg);

    /* mud */
    RUN_TEST(test_mud_atualiza_endereco);
    RUN_TEST(test_mud_reporta_no_txt);
    RUN_TEST(test_mud_cpf_inexistente_nao_trava);

    /* dspj */
    RUN_TEST(test_dspj_transforma_morador_em_sem_teto);
    RUN_TEST(test_dspj_reporta_no_txt);
    RUN_TEST(test_dspj_sem_teto_ignorado);
    RUN_TEST(test_dspj_cpf_inexistente_nao_trava);
    RUN_TEST(test_dspj_marca_circulo_no_svg);

    /* sequências */
    RUN_TEST(test_sequencia_nasc_mud_h_interrogacao);
    RUN_TEST(test_sequencia_nasc_rip_busca_null);
    RUN_TEST(test_sequencia_rq_depois_pq_nao_encontra);
    RUN_TEST(test_sequencia_censo_apos_varios_comandos);

    return UNITY_END();
}