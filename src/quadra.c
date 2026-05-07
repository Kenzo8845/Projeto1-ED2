#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/quadra.h"

struct stQuadra {
    char cep[16];       //CEP identificador da quadra 
    double x;           //Coordenada X da âncora (canto sudeste) 
    double y;           //Coordenada Y da âncora (canto sudeste)
    double w;           //Largura da quadra 
    double h;           //Altura da quadra 
    
    char corp[64];      // Cor de preenchimento (fill) 
    char corb[64];      // Cor da borda (stroke) 
    double sw;          //Espessura da borda (stroke-width)
};


//=============================================================================
// Constructor e Destructor
//=============================================================================

Quadra quadra_constructor(const char *cep, double x, double y, double w, double h) {
    Quadra q = calloc(1, sizeof(struct stQuadra));
    assert(q != NULL); 

    // Inicializa os atributos obrigatórios da quadra
    strncpy(q->cep, cep, 16);
    q->x = x;
    q->y = y;
    q->w = w;
    q->h = h;
    
    q->sw = 0.0;

    return q;
}

void quadra_destructor(Quadra q) {
    if (q == NULL) return;

    free(q);
}

//=============================================================================
// Métodos Get 
//=============================================================================

size_t quadra_getSize() {
    return sizeof(struct stQuadra);
}

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
// Métodos Set 
//=============================================================================


void quadra_setCep(const Quadra q, const char* novoCep) {
    assert(q != NULL);

    strncpy(q->cep, novoCep, 16);
    q->cep[15] = '\0';
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

    strncpy(q->corp, novoCorp, 64);
    q->corp[63] = '\0';
}

void quadra_setCorb(const Quadra q, const char* novoCorb) {
    assert(q != NULL);
    
    strncpy(q->corb, novoCorb, 64);
    q->corb[63] = '\0';
}

void quadra_setSw(const Quadra q, double novoSw) {
    assert(q != NULL);
    q->sw = novoSw;
}


