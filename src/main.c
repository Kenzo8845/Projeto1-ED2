#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/hash.h"
#include "../include/parser_geo.h"
#include "../include/parser_pm.h"
#include "../include/parser_qry.h"
#include "../include/svg.h"

#define PATH_MAX_LEN  512
#define NAME_MAX_LEN  256
#define BUCKET_SIZE   4096

static void extrair_nome_base(const char *caminho, char *saida, size_t max) {
    const char *ultimo_sep = strrchr(caminho, '/');
    const char *inicio = ultimo_sep ? ultimo_sep + 1 : caminho;
    strncpy(saida, inicio, max - 1);
    saida[max - 1] = '\0';
    char *ponto = strrchr(saida, '.');
    if (ponto) *ponto = '\0';
}

static void normalizar_dir(char *dir) {
    size_t len = strlen(dir);
    if (len > 0 && dir[len - 1] != '/' && len + 1 < PATH_MAX_LEN) {
        dir[len] = '/';
        dir[len + 1] = '\0';
    }
}

int main(int argc, char *argv[]) {
    char dir_entrada[PATH_MAX_LEN] = "./";
    char arq_geo[NAME_MAX_LEN] = "";
    char dir_saida[PATH_MAX_LEN] = "";
    char arq_qry[NAME_MAX_LEN] = "";
    char arq_pm[NAME_MAX_LEN] = "";

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-e")  == 0 && i+1 < argc) strncpy(dir_entrada, argv[++i], PATH_MAX_LEN-1);
        else if (strcmp(argv[i], "-f") == 0 && i+1 < argc) strncpy(arq_geo, argv[++i], NAME_MAX_LEN-1);
        else if (strcmp(argv[i], "-o") == 0 && i+1 < argc) strncpy(dir_saida, argv[++i], PATH_MAX_LEN-1);
        else if (strcmp(argv[i], "-q") == 0 && i+1 < argc) strncpy(arq_qry, argv[++i], NAME_MAX_LEN-1);
        else if (strcmp(argv[i], "-pm") == 0 && i+1 < argc) strncpy(arq_pm, argv[++i], NAME_MAX_LEN-1);
    }

    if (strlen(arq_geo) == 0 || strlen(dir_saida) == 0) {
        fprintf(stderr, "Uso: ted [-e path] -f arq.geo -o dir [-q arq.qry] [-pm arq.pm]\n");
        return EXIT_FAILURE;
    }

    normalizar_dir(dir_entrada);
    normalizar_dir(dir_saida);

    char caminho_geo[PATH_MAX_LEN];
    char caminho_pm[PATH_MAX_LEN] = "";
    char caminho_qry[PATH_MAX_LEN] = "";
    snprintf(caminho_geo, PATH_MAX_LEN, "%s%s", dir_entrada, arq_geo);
    if (strlen(arq_pm) > 0) snprintf(caminho_pm, PATH_MAX_LEN, "%s%s", dir_entrada, arq_pm);
    if (strlen(arq_qry) > 0) snprintf(caminho_qry, PATH_MAX_LEN, "%s%s", dir_entrada, arq_qry);

    char base_geo[NAME_MAX_LEN];
    char base_qry[NAME_MAX_LEN] = "";
    extrair_nome_base(arq_geo, base_geo, NAME_MAX_LEN);
    if (strlen(arq_qry) > 0) extrair_nome_base(arq_qry, base_qry, NAME_MAX_LEN);

    char hf_quadras[PATH_MAX_LEN], hf_habitantes[PATH_MAX_LEN];
    snprintf(hf_quadras, PATH_MAX_LEN, "%s%s_quadras.hf", dir_saida, base_geo);
    snprintf(hf_habitantes, PATH_MAX_LEN, "%s%s_habitantes.hf", dir_saida, base_geo);

    Hash *quadras = hash_create(hf_quadras, BUCKET_SIZE);
    Hash *habitantes = hash_create(hf_habitantes, BUCKET_SIZE);
    if (!quadras || !habitantes) {
        fprintf(stderr, "Erro ao criar hashfiles.\n");
        return EXIT_FAILURE;
    }

    /* -----------------------------------------------------------------------
     * SVG do .geo  (base.svg)
     * parser_geo desenha cada quadra enquanto a processa.
     * ----------------------------------------------------------------------- */
    char caminho_svg_geo[PATH_MAX_LEN];
    snprintf(caminho_svg_geo, PATH_MAX_LEN, "%s%s.svg", dir_saida, base_geo);

    FILE *svg_geo = fopen(caminho_svg_geo, "w");
    if (!svg_geo) {
        perror("Erro ao criar SVG do .geo");
        hash_destroy(quadras);
        hash_destroy(habitantes);
        return EXIT_FAILURE;
    }
    svg_iniciar(svg_geo);
    parser_geo_processar(caminho_geo, quadras, svg_geo);
    svg_fechar(svg_geo);
    fclose(svg_geo);

    // Processa .pm
    if (strlen(caminho_pm) > 0)
        parser_pm_processar(caminho_pm, habitantes);

    // Processa ,qry
    if (strlen(caminho_qry) > 0) {
        char caminho_svg_qry[PATH_MAX_LEN], caminho_txt_qry[PATH_MAX_LEN];
        snprintf(caminho_svg_qry, PATH_MAX_LEN, "%s%s-%s.svg", dir_saida, base_geo, base_qry);
        snprintf(caminho_txt_qry, PATH_MAX_LEN, "%s%s-%s.txt", dir_saida, base_geo, base_qry);

        // Arquivo temporário em memória para os marcadores do .qry
        FILE *svg_marcadores = tmpfile();
        FILE *txt_qry = fopen(caminho_txt_qry, "w");

        if (!svg_marcadores || !txt_qry) {
            perror("Erro ao criar arquivos de saída do .qry");
            hash_destroy(quadras);
            hash_destroy(habitantes);
            return EXIT_FAILURE;
        }

        // Processa o .qry — marcadores vão para svg_marcadores
        parser_qry_processar(caminho_qry, quadras, habitantes, txt_qry, svg_marcadores);
        fclose(txt_qry);

        // Monta o SVG final na ordem correta
        FILE *svg_qry = fopen(caminho_svg_qry, "w");
        if (!svg_qry) {
            perror("Erro ao criar SVG do .qry");
            fclose(svg_marcadores);
            hash_destroy(quadras);
            hash_destroy(habitantes);
            return EXIT_FAILURE;
        }

        svg_iniciar(svg_qry);

        // Camada 1: apenas as quadras que ainda existem no hash 
        svg_desenhar_todas_quadras(svg_qry, quadras);

        // Camada 2: marcadores gerados pelo .qry
        svg_copiar(svg_qry, svg_marcadores);
        fclose(svg_marcadores);

        svg_fechar(svg_qry);
        fclose(svg_qry);
    }

    /* Dumps .hfd */
    char hfd_quadras[PATH_MAX_LEN], hfd_habitantes[PATH_MAX_LEN];
    snprintf(hfd_quadras, PATH_MAX_LEN, "%s%s_quadras.hfd", dir_saida, base_geo);
    snprintf(hfd_habitantes, PATH_MAX_LEN, "%s%s_habitantes.hfd", dir_saida, base_geo);
    hash_dump(quadras, hfd_quadras);
    hash_dump(habitantes, hfd_habitantes);

    hash_destroy(quadras);
    hash_destroy(habitantes);
    return EXIT_SUCCESS;
}