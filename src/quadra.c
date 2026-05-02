#include "../include/quadra.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//=============================================================================
// Estruturas Ocultas (Encapsulamento)
//=============================================================================

/**
 * @brief Estrutura real da quadra. 
 * Escondida no .c para garantir o encapsulamento.
 */
struct stQuadra {
    char* cep;          /**< CEP identificador da quadra */
    double x;           /**< Coordenada X da âncora (canto sudeste) */
    double y;           /**< Coordenada Y da âncora (canto sudeste) */
    double w;           /**< Largura da quadra */
    double h;           /**< Altura da quadra */
    
    char* corp;         /**< Cor de preenchimento (fill) */
    char* corb;         /**< Cor da borda (stroke) */
    double sw;          /**< Espessura da borda (stroke-width) */
};

//=============================================================================
// Funções Auxiliares Internas
//=============================================================================

/*
 * Função auxiliar para duplicar strings de forma segura,
 * garantindo compatibilidade estrita com o padrão C99.
 */
static char* clone_string(const char* src) {
    if (src == NULL) return NULL;
    char* dest = malloc(strlen(src) + 1);
    assert(dest != NULL); // Garante que a memória foi alocada com sucesso[cite: 3]
    strcpy(dest, src);
    return dest;
}

//=============================================================================
// Constructor e Destructor
//=============================================================================

Quadra quadra_constructor(const char *cep, double x, double y, double w, double h) {
    Quadra q = malloc(sizeof(struct stQuadra));
    assert(q != NULL); 

    // Inicializa os atributos obrigatórios da quadra
    q->cep = clone_string(cep);
    q->x = x;
    q->y = y;
    q->w = w;
    q->h = h;
    

    q->corp = NULL;
    q->corb = NULL;
    q->sw = 0.0;

    return q;
}

void quadra_destructor(Quadra q) {
    if (q == NULL) return;

    free(q->cep);
    free(q->corp);
    free(q->corb);

    free(q);
}

//=============================================================================
// Métodos Get (Acessadores)
//=============================================================================

const char* quadra_getCep(const Quadra q) {
    assert(q != NULL); 
    return q->cep;
}

double quadra_getX(const Quadra q) {
    assert(q != NULL);
    return q->x;
}

double quadra_getY(const Quadra q) {
    assert(q != NULL);
    return q->y;
}

double quadra_getW(const Quadra q) {
    assert(q != NULL);
    return q->w;
}

double quadra_getH(const Quadra q) {
    assert(q != NULL);
    return q->h;
}

const char* quadra_getCorp(const Quadra q) {
    assert(q != NULL);
    return q->corp;
}

const char* quadra_getCorb(const Quadra q) {
    assert(q != NULL);
    return q->corb;
}

double quadra_getSw(const Quadra q) {
    assert(q != NULL);
    return q->sw;
}

//=============================================================================
// Métodos Set (Modificadores)
//=============================================================================


void quadra_setCep(const Quadra q, const char* novoCep) {
    assert(q != NULL);
    free(q->cep); // Libera o CEP antigo antes de atribuir o novo
    q->cep = clone_string(novoCep);
}

void quadra_setX(const Quadra q, double novoX) {
    assert(q != NULL);
    q->x = novoX;
}

void quadra_setY(const Quadra q, double novoY) {
    assert(q != NULL);
    q->y = novoY;
}

void quadra_setW(const Quadra q, double novoW) {
    assert(q != NULL);
    q->w = novoW;
}

void quadra_setH(const Quadra q, double novoH) {
    assert(q != NULL);
    q->h = novoH;
}

void quadra_setCorp(const Quadra q, const char* novoCorp) {
    assert(q != NULL);
    free(q->corp); // Libera a cor antiga, se existir
    q->corp = clone_string(novoCorp);
}

void quadra_setCorb(const Quadra q, const char* novoCorb) {
    assert(q != NULL);
    free(q->corb); // Libera a cor antiga, se existir
    q->corb = clone_string(novoCorb);
}

void quadra_setSw(const Quadra q, double novoSw) {
    assert(q != NULL);
    q->sw = novoSw;
}

//=============================================================================
// Funções
//=============================================================================

