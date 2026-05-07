#include "../include/svg.h"
#include "../include/hash.h"

#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/*
 * Espelha as structs internas de hash.c — idêntico ao que está em parser_qry.c.
 * Necessário para a varredura sequencial do hash de quadras.
 */
typedef struct { int local_depth; int num_records; int max_records; } _BucketHeader;
typedef struct { uint64_t key; size_t data_size; int valid; } _RecordHeader;

static void calcular_posicao_face(const Quadra q, char face, double num, double *px, double *py)
{
    double qx = quadra_getX(q);
    double qy = quadra_getY(q);
    double qw = quadra_getW(q);
    double qh = quadra_getH(q);

    switch (face) {
        case 'S': *px = qx + num; *py = qy; break;
        case 'N': *px = qx + num; *py = qy + qh; break;
        case 'L': *px = qx; *py = qy + num; break;
        case 'O': *px = qx + qw;  *py = qy + num; break;
        default:  *px = qx; *py = qy; break;
    }
}

void svg_iniciar(FILE *arquivo) {
    assert(arquivo != NULL);
    fprintf(arquivo,
            "<svg xmlns=\"http://www.w3.org/2000/svg\" "
            "width=\"100%%\" height=\"100%%\">\n");
}

void svg_fechar(FILE *arquivo) {
    assert(arquivo != NULL);
    fprintf(arquivo, "</svg>\n");
}

void svg_desenhar_quadra(FILE *arquivo, const Quadra q) {
    assert(arquivo != NULL && q != NULL);

    double x = quadra_getX(q);
    double y = quadra_getY(q);
    double w = quadra_getW(q);
    double h = quadra_getH(q);
    const char *corp = quadra_getCorp(q);
    const char *corb = quadra_getCorb(q);
    double sw = quadra_getSw(q);

    fprintf(arquivo,
            "\t<rect x=\"%.2f\" y=\"%.2f\" width=\"%.2f\" height=\"%.2f\" "
            "fill=\"%s\" stroke=\"%s\" stroke-width=\"%.2f\"/>\n",
            x, y, w, h, corp, corb, sw);

    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"black\" font-size=\"12\" "
            "text-anchor=\"middle\" dominant-baseline=\"middle\">%s</text>\n",
            x + w / 2.0, y + h / 2.0, quadra_getCep(q));
}

void svg_desenhar_todas_quadras(FILE *arquivo, Hash *tabela_quadras) {
    assert(arquivo != NULL && tabela_quadras != NULL);

    FILE *f = hash_get_file(tabela_quadras);
    int num_buckets = hash_get_num_buckets(tabela_quadras);
    size_t bucket_size = hash_get_bucket_size(tabela_quadras);

    int *visitado = calloc(num_buckets, sizeof(int));
    assert(visitado != NULL);

    for (int b = 0; b < num_buckets; b++) {
        if (visitado[b]) continue;
        visitado[b] = 1;

        fseek(f, (long)b * (long)bucket_size, SEEK_SET);

        _BucketHeader bh;
        fread(&bh, sizeof(_BucketHeader), 1, f);

        for (int r = 0; r < bh.num_records; r++) {
            _RecordHeader rh;
            fread(&rh, sizeof(_RecordHeader), 1, f);

            if (rh.valid) {
                void *buf = malloc(rh.data_size);
                assert(buf != NULL);
                fread(buf, rh.data_size, 1, f);
                svg_desenhar_quadra(arquivo, (Quadra)buf);
                free(buf);
            } else {
                fseek(f, (long)rh.data_size, SEEK_CUR);
            }
        }
    }

    free(visitado);
}

void svg_copiar(FILE *dst, FILE *src) {
    assert(dst != NULL && src != NULL);
    rewind(src);
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0)
        fwrite(buf, 1, n, dst);
}

void svg_marcar_remocao_quadra(FILE *arquivo, const Quadra q) {
    assert(arquivo != NULL && q != NULL);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"red\" font-size=\"14\" "
            "text-anchor=\"middle\">X</text>\n",
            quadra_getX(q), quadra_getY(q));
}

void svg_desenhar_contagem_moradores(FILE *arquivo, const Quadra q,
                                     int faceN, int faceS,
                                     int faceL, int faceO, int total)
{
    assert(arquivo != NULL && q != NULL);

    double x = quadra_getX(q);
    double y = quadra_getY(q);
    double w = quadra_getW(q);
    double h = quadra_getH(q);
    double cx = x + w / 2.0;
    double cy = y + h / 2.0;

    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"blue\" font-size=\"12\" "
            "text-anchor=\"middle\" dominant-baseline=\"middle\">%d</text>\n",
            cx, cy, total);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"blue\" font-size=\"10\" "
            "text-anchor=\"middle\">S:%d</text>\n",
            cx, y + 12.0, faceS);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"blue\" font-size=\"10\" "
            "text-anchor=\"middle\">N:%d</text>\n",
            cx, y + h - 4.0, faceN);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"blue\" font-size=\"10\" "
            "text-anchor=\"start\">L:%d</text>\n",
            x + 4.0, cy, faceL);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"blue\" font-size=\"10\" "
            "text-anchor=\"end\">O:%d</text>\n",
            x + w - 4.0, cy, faceO);
}

void svg_marcar_obito(FILE *arquivo, const Quadra q, char face, double num) {
    assert(arquivo != NULL && q != NULL);
    double px, py;
    calcular_posicao_face(q, face, num, &px, &py);
    fprintf(arquivo,
            "\t<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "stroke=\"red\" stroke-width=\"2\"/>\n",
            px - 5.0, py - 5.0, px + 5.0, py + 5.0);
    fprintf(arquivo,
            "\t<line x1=\"%.2f\" y1=\"%.2f\" x2=\"%.2f\" y2=\"%.2f\" "
            "stroke=\"red\" stroke-width=\"2\"/>\n",
            px + 5.0, py - 5.0, px - 5.0, py + 5.0);
}

void svg_marcar_mudanca(FILE *arquivo, const Quadra q,
                        char face, double num, const char *cpf)
{
    assert(arquivo != NULL && q != NULL && cpf != NULL);
    double px, py;
    calcular_posicao_face(q, face, num, &px, &py);
    fprintf(arquivo,
            "\t<rect x=\"%.2f\" y=\"%.2f\" width=\"30\" height=\"14\" "
            "fill=\"red\" fill-opacity=\"0.7\"/>\n",
            px - 15.0, py - 7.0);
    fprintf(arquivo,
            "\t<text x=\"%.2f\" y=\"%.2f\" fill=\"white\" font-size=\"5\" "
            "text-anchor=\"middle\" dominant-baseline=\"middle\">%s</text>\n",
            px, py, cpf);
}

void svg_marcar_despejo(FILE *arquivo, const Quadra q, char face, double num) {
    assert(arquivo != NULL && q != NULL);
    double px, py;
    calcular_posicao_face(q, face, num, &px, &py);
    fprintf(arquivo,
            "\t<circle cx=\"%.2f\" cy=\"%.2f\" r=\"5\" "
            "fill=\"black\"/>\n",
            px, py);
}