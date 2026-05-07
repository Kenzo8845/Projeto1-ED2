#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

#include "../include/parser_qry.h"
#include "../include/quadra.h"
#include "../include/habitante.h"
#include "../include/svg.h"

/* ==========================================================================
   FUNÇÕES AUXILIARES DE HASHING
   ========================================================================== */

static uint64_t hash_djb2(const char *str) {
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) { hash = ((hash << 5) + hash) + c; }
    return hash;
}

static uint64_t hash_cpf(const char *cpf) {
    uint64_t chave = 0;
    for (int i = 0; cpf[i] != '\0'; i++) {
        if (cpf[i] >= '0' && cpf[i] <= '9')
            chave = chave * 10 + (cpf[i] - '0');
    }
    return chave;
}


//Espelha as structs internas de hash.c para leitura direta do arquivo .hf.
typedef struct { int local_depth; int num_records; int max_records; } _BucketHeader;
typedef struct { uint64_t key; size_t data_size; int valid; } _RecordHeader;

static void varrer_habitantes(Hash *hab_hash, void (*callback)(Habitante, void *), void *userdata) {
    FILE *f  = hash_get_file(hab_hash);
    int num_buckets = hash_get_num_buckets(hab_hash);
    size_t bucket_size = hash_get_bucket_size(hab_hash);

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
                callback((Habitante)buf, userdata);
            } else {
                fseek(f, (long)rh.data_size, SEEK_CUR);
            }
        }
    }
    free(visitado);
}

/* ==========================================================================
   CALLBACKS 
   ========================================================================== */

typedef struct {
    const char *cep_alvo;
    FILE *txt;
    Hash *hab_hash;
} CtxRq;


static void cb_rq(Habitante h, void *userdata) {
    CtxRq *ctx = (CtxRq *)userdata;
    if (!habitante_isSemTeto(h) &&
        strcmp(habitante_getCep(h), ctx->cep_alvo) == 0)
    {
        fprintf(ctx->txt, "  CPF: %s, Nome: %s %s\n",
                habitante_getCpf(h),
                habitante_getNome(h),
                habitante_getSobrenome(h));

        habitante_setSemTeto(h, true);
        uint64_t chave = hash_cpf(habitante_getCpf(h));
        size_t sz_removido;
        void *removido = hash_remove(ctx->hab_hash, chave, &sz_removido);
        free(removido);
        hash_insert(ctx->hab_hash, h, chave, habitante_getSize());
    }
    habitante_destructor(h);
}

typedef struct {
    const char *cep_alvo;
    int total, faceN, faceS, faceL, faceO;
} CtxPq;

static void cb_pq(Habitante h, void *userdata) {
    CtxPq *ctx = (CtxPq *)userdata;
    if (!habitante_isSemTeto(h) &&
        strcmp(habitante_getCep(h), ctx->cep_alvo) == 0)
    {
        ctx->total++;
        char face = habitante_getFace(h);
        if (face == 'N') ctx->faceN++;
        else if (face == 'S') ctx->faceS++;
        else if (face == 'L') ctx->faceL++;
        else if (face == 'O') ctx->faceO++;
    }
    habitante_destructor(h);
}

typedef struct {
    int total, moradores, semTeto;
    int homens, mulheres;
    int moradores_h, moradores_m;
    int semTeto_h, semTeto_m;
} CtxCenso;

static void cb_censo(Habitante h, void *userdata) {
    CtxCenso *ctx = (CtxCenso *)userdata;
    ctx->total++;
    char sexo = habitante_getSexo(h);
    int  eh_h = (sexo == 'M' || sexo == 'm');

    if (eh_h) ctx->homens++; else ctx->mulheres++;

    if (habitante_isSemTeto(h)) {
        ctx->semTeto++;
        if (eh_h) ctx->semTeto_h++; else ctx->semTeto_m++;
    } else {
        ctx->moradores++;
        if (eh_h) ctx->moradores_h++; else ctx->moradores_m++;
    }
    habitante_destructor(h);
}

/* ==========================================================================
   COMANDOS
   ========================================================================== */

static void cmd_rq(Hash *quadras, Hash *habitantes, const char *cep, FILE *txt, FILE *svg) {
    uint64_t chave_q = hash_djb2(cep);
    size_t tamanho;
    Quadra q = hash_remove(quadras, chave_q, &tamanho);

    if (q == NULL) {
        fprintf(txt, "[rq] Quadra %s nao encontrada.\n", cep);
        return;
    }

    svg_marcar_remocao_quadra(svg, q);

    fprintf(txt, "[rq] Quadra %s removida. Moradores desabrigados:\n", cep);

    CtxRq ctx = { cep, txt, habitantes };
    varrer_habitantes(habitantes, cb_rq, &ctx);


    quadra_destructor(q);
}

static void cmd_pq(Hash *quadras, Hash *habitantes, const char *cep, FILE *txt, FILE *svg) {
    uint64_t chave_q = hash_djb2(cep);
    size_t tamanho;
    Quadra q = hash_search(quadras, chave_q, &tamanho);

    if (q == NULL) {
        fprintf(txt, "[pq] Quadra %s nao encontrada.\n", cep);
        return;
    }

    CtxPq ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.cep_alvo = cep;
    varrer_habitantes(habitantes, cb_pq, &ctx);

    fprintf(txt, "[pq] Quadra %s: Total=%d  N=%d  S=%d  L=%d  O=%d\n",
            cep, ctx.total, ctx.faceN, ctx.faceS, ctx.faceL, ctx.faceO);

    svg_desenhar_contagem_moradores(svg, q, ctx.faceN, ctx.faceS, ctx.faceL, ctx.faceO, ctx.total);

    quadra_destructor(q);
}

static void cmd_censo(Hash *habitantes, FILE *txt) {
    fprintf(txt, "--- CENSO DE BITNOPOLIS ---\n");

    CtxCenso ctx;
    memset(&ctx, 0, sizeof(ctx));
    varrer_habitantes(habitantes, cb_censo, &ctx);

    double prop_mor = ctx.total > 0 ? (ctx.moradores * 100.0 / ctx.total)  : 0.0;
    double pct_hom = ctx.total > 0 ? (ctx.homens * 100.0 / ctx.total) : 0.0;
    double pct_mul = ctx.total > 0 ? (ctx.mulheres * 100.0 / ctx.total) : 0.0;
    double pct_mor_h = ctx.moradores > 0 ? (ctx.moradores_h * 100.0 / ctx.moradores) : 0.0;
    double pct_mor_m = ctx.moradores > 0 ? (ctx.moradores_m * 100.0 / ctx.moradores) : 0.0;
    double pct_st_h = ctx.semTeto > 0 ? (ctx.semTeto_h * 100.0 / ctx.semTeto) : 0.0;
    double pct_st_m = ctx.semTeto > 0 ? (ctx.semTeto_m * 100.0 / ctx.semTeto) : 0.0;

    fprintf(txt, "Total de habitantes   : %d\n", ctx.total);
    fprintf(txt, "Total de moradores    : %d\n", ctx.moradores);
    fprintf(txt, "Prop. moradores/hab.  : %.1f%%\n", prop_mor);
    fprintf(txt, "Homens                : %d (%.1f%%)\n", ctx.homens, pct_hom);
    fprintf(txt, "Mulheres              : %d (%.1f%%)\n", ctx.mulheres, pct_mul);
    fprintf(txt, "Moradores homens      : %d (%.1f%%)\n", ctx.moradores_h, pct_mor_h);
    fprintf(txt, "Moradores mulheres    : %d (%.1f%%)\n", ctx.moradores_m, pct_mor_m);
    fprintf(txt, "Sem-teto              : %d\n", ctx.semTeto);
    fprintf(txt, "Sem-teto homens       : %d (%.1f%%)\n", ctx.semTeto_h, pct_st_h);
    fprintf(txt, "Sem-teto mulheres     : %d (%.1f%%)\n", ctx.semTeto_m, pct_st_m);
    fprintf(txt, "---------------------------\n");
}

static void cmd_h_interrogacao(Hash *habitantes, const char *cpf, FILE *txt) {
    uint64_t  chave = hash_cpf(cpf);
    size_t    tamanho;
    Habitante h = hash_search(habitantes, chave, &tamanho);

    if (h != NULL) {
        habitante_printInfo(txt, h);
        habitante_printInfoEndereco(txt, h);
        habitante_destructor(h);
    } else {
        fprintf(txt, "[h?] Habitante %s nao encontrado.\n", cpf);
    }
}

static void cmd_nasc(Hash *habitantes, 
                     const char *cpf, const char *nome,
                     const char *sobrenome, char sexo,
                     const char *nasc, FILE *txt)
{
    Habitante h = habitante_constructor(cpf, nome, sobrenome, sexo, nasc);
    uint64_t chave = hash_cpf(cpf);

    hash_insert(habitantes, h, chave, habitante_getSize());

    fprintf(txt, "[nasc] Nasceu: %s %s (CPF: %s)\n", nome, sobrenome, cpf);
    habitante_destructor(h);
}

static void cmd_rip(Hash *habitantes, Hash *quadras, const char *cpf, FILE *txt, FILE *svg) {
    uint64_t  chave = hash_cpf(cpf);
    size_t    tamanho;
    Habitante h = hash_remove(habitantes, chave, &tamanho);

    if (h == NULL) {
        fprintf(txt, "[rip] Habitante %s nao encontrado.\n", cpf);
        return;
    }

    fprintf(txt, "[rip] Falecimento registrado:\n");
    habitante_printInfo(txt, h);

    if (!habitante_isSemTeto(h)) {
        habitante_printInfoEndereco(txt, h);

        size_t tq;
        Quadra q = hash_search(quadras, hash_djb2(habitante_getCep(h)), &tq);
        if (q != NULL) {
            svg_marcar_obito(svg, q, habitante_getFace(h), habitante_getNumeroCasa(h));
            quadra_destructor(q);
        }
    }

    habitante_destructor(h);
}

static void cmd_mud(Hash *habitantes, Hash *quadras,
                    const char *cpf, const char *cep,
                    char face, double num, const char *cmpl,
                    FILE *txt, FILE *svg)
{
    uint64_t chave = hash_cpf(cpf);
    size_t tamanho;
    Habitante h = hash_search(habitantes, chave, &tamanho);

    if (h == NULL) {
        fprintf(txt, "[mud] Habitante %s nao encontrado.\n", cpf);
        return;
    }

    habitante_setEndereco(h, (char *)cep, face, num, (char *)cmpl);

    size_t sz_removido;
    void *removido = hash_remove(habitantes, chave, &sz_removido);
    free(removido); 
    hash_insert(habitantes, h, chave, habitante_getSize());

    fprintf(txt, "[mud] %s mudou-se para %s/%c/%.2f (%s)\n",
            cpf, cep, face, num, cmpl);

    size_t tq;
    Quadra q = hash_search(quadras, hash_djb2(cep), &tq);
    if (q != NULL) {
        svg_marcar_mudanca(svg, q, face, num, cpf);
        quadra_destructor(q);
    }

    habitante_destructor(h);
}

static void cmd_dspj(Hash *habitantes, Hash *quadras, const char *cpf, FILE *txt, FILE *svg) {
    uint64_t chave = hash_cpf(cpf);
    size_t tamanho;
    Habitante h = hash_search(habitantes, chave, &tamanho);

    if (h == NULL) {
        fprintf(txt, "[dspj] Habitante %s nao encontrado.\n", cpf);
        return;
    }

    if (habitante_isSemTeto(h)) {
        fprintf(txt, "[dspj] %s ja e sem-teto; despejo ignorado.\n", cpf);
        habitante_destructor(h);
        return;
    }

    fprintf(txt, "[dspj] Despejo registrado:\n");
    habitante_printInfo(txt, h);
    habitante_printInfoEndereco(txt, h);

    size_t tq;
    Quadra q = hash_search(quadras, hash_djb2(habitante_getCep(h)), &tq);
    if (q != NULL) {
        svg_marcar_despejo(svg, q, habitante_getFace(h), habitante_getNumeroCasa(h));
        quadra_destructor(q);
    }

    habitante_setSemTeto(h, true);
    size_t sz_removido;
    void *removido = hash_remove(habitantes, chave, &sz_removido);
    free(removido); 
    hash_insert(habitantes, h, chave, habitante_getSize());

    habitante_destructor(h);
}

/* ==========================================================================
   PROCESSADOR PRINCIPAL
   ========================================================================== */

void parser_qry_processar(const char *caminho_arquivo, Hash *quadras, Hash *habitantes, FILE *txt, FILE *svg) {
    assert(caminho_arquivo != NULL);
    assert(quadras != NULL && habitantes != NULL);
    assert(txt != NULL && svg != NULL);

    FILE *ficheiro = fopen(caminho_arquivo, "r");
    if (ficheiro == NULL) {
        perror("[parser_qry] Erro ao abrir .qry");
        return;
    }

    char linha[512];

    while (fgets(linha, sizeof(linha), ficheiro) != NULL) {
        char comando[32];
        if (sscanf(linha, "%31s", comando) != 1) continue;

        fprintf(txt, "[*] %s", linha);

        if (strcmp(comando, "rq") == 0) {
            char cep[64];
            sscanf(linha, "%*s %63s", cep);
            cmd_rq(quadras, habitantes, cep, txt, svg);
        }
        else if (strcmp(comando, "pq") == 0) {
            char cep[64];
            sscanf(linha, "%*s %63s", cep);
            cmd_pq(quadras, habitantes, cep, txt, svg);
        }
        else if (strcmp(comando, "censo") == 0) {
            cmd_censo(habitantes, txt);
        }
        else if (strcmp(comando, "h?") == 0) {
            char cpf[32];
            sscanf(linha, "%*s %31s", cpf);
            cmd_h_interrogacao(habitantes, cpf, txt);
        }
        else if (strcmp(comando, "nasc") == 0) {
            char cpf[32], nome[64], sobrenome[64], nasc[15];
            char sexo;
            sscanf(linha, "%*s %31s %63s %63s %c %14s", 
                    cpf, nome, sobrenome, &sexo, nasc);
            cmd_nasc(habitantes, cpf, nome, sobrenome, sexo, nasc, txt);
        }
        else if (strcmp(comando, "rip") == 0) {
            char cpf[32];
            sscanf(linha, "%*s %31s", cpf);
            cmd_rip(habitantes, quadras, cpf, txt, svg);
        }
        else if (strcmp(comando, "mud") == 0) {
            char cpf[32], cep[64], cmpl[64];
            char face;
            double num;
            cmpl[0] = '\0';
            sscanf(linha, "%*s %31s %63s %c %lf %63[^\n]",
                   cpf, cep, &face, &num, cmpl);
            cmd_mud(habitantes, quadras, cpf, cep, face, num, cmpl, txt, svg);
        }
        else if (strcmp(comando, "dspj") == 0) {
            char cpf[32];
            sscanf(linha, "%*s %31s", cpf);
            cmd_dspj(habitantes, quadras, cpf, txt, svg);
        }
        else {
            fprintf(txt, "[parser_qry] Comando desconhecido: %s\n", comando);
        }
    }

    fclose(ficheiro);
    printf("[parser_qry] Processamento do .qry concluido.\n");
}