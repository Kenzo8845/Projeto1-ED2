#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include "../include/parser_geo.h"
#include "../include/quadra.h"
#include "../include/svg.h"

static uint64_t hash_string_djb2(const char *str) {
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) hash = ((hash << 5) + hash) + c;
    return hash;
}

void parser_geo_processar(const char *caminho_arquivo, Hash *tabela_quadras, FILE *svg) {
    FILE *ficheiro = fopen(caminho_arquivo, "r");
    if (ficheiro == NULL) {
        perror("Erro ao abrir o arquivo .geo");
        return;
    }

    char linha[256];
    char cor_fill[64] = "black";
    char cor_strk[64] = "black";
    double sw_atual   = 1.0;

    while (fgets(linha, sizeof(linha), ficheiro) != NULL) {
        char comando[10];
        if (sscanf(linha, "%9s", comando) != 1) continue;

        if (strcmp(comando, "cq") == 0) {
            char sw_str[32] = "1.0";
            sscanf(linha, "%*s %31s %63s %63s", sw_str, cor_fill, cor_strk);
            sw_atual = atof(sw_str);
        }
        else if (strcmp(comando, "q") == 0) {
            char   cep[64];
            double x, y, w, h;
            sscanf(linha, "%*s %63s %lf %lf %lf %lf", cep, &x, &y, &w, &h);

            Quadra q = quadra_constructor(cep, x, y, w, h);
            quadra_setCorp(q, cor_fill);
            quadra_setCorb(q, cor_strk);
            quadra_setSw(q, sw_atual);

            if (tabela_quadras != NULL)
                hash_insert(tabela_quadras, q, hash_string_djb2(cep), quadra_getSize());

            if (svg != NULL)
                svg_desenhar_quadra(svg, q);

            quadra_destructor(q);
        }
    }

    fclose(ficheiro);
}