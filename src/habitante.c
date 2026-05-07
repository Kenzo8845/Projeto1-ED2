#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../include/habitante.h"
struct stEndereco {
    char cep[16];
    char face;
    double numero;
    char complemento[64];
};

struct stHabitante {
    char cpf[16];
    char nome[64];
    char sobrenome[64];
    char sexo;
    char data_nascimento[12];
    bool semTeto;
    struct stEndereco endereco;
};


//=============================================================================
// Construtor e Destrutor
//=============================================================================

Habitante habitante_constructor(const char *cpf, const char *nome,
    const char *sobrenome, char sexo, const char *data_nascimento) {
    
    Habitante h = calloc(1, sizeof(struct stHabitante)); 
    assert(h != NULL); 

    strncpy(h->cpf, cpf, 15);
    h->cpf[15] = '\0';

    strncpy(h->nome, nome, 63);
    h->nome[63] = '\0';

    strncpy(h->sobrenome, sobrenome, 63);
    h->sobrenome[63] = '\0';

    h->sexo = sexo;

    strncpy(h->data_nascimento, data_nascimento, 11);
    h->data_nascimento[11] = '\0';
    
    h->semTeto = true; 

    return h;
}

void habitante_destructor(Habitante h) {
    if (h == NULL) return;

    free(h);
}


//=============================================================================
// Métodos Setters
//=============================================================================

void habitante_setEndereco(Habitante h, char *cep, char face, double numero, char *complemento) {
    assert(h != NULL);

    habitante_setCep(h, cep);
    habitante_setFace(h, face);
    habitante_setNumerCcasa(h, numero);
    habitante_setComplemento(h, complemento);
    
    h->semTeto = false;
}

void habitante_setCpf(Habitante h, const char *cpf) {
    assert(h != NULL);
    strncpy(h->cpf, cpf, 16);
    h->cpf[15] = '\0';
    
}

void habitante_setNome(Habitante h, const char *nome) {
    assert(h != NULL);
    strncpy(h->nome, nome, 64);
    h->nome[63] = '\0';
}

void habitante_setSobrenome(Habitante h, const char *sobrenome) {
    assert(h != NULL);
    strncpy(h->sobrenome, sobrenome, 64);
    h->sobrenome[63] = '\0';
}

void habitante_setSexo(Habitante h, char sexo) {
    assert(h != NULL);
    h->sexo = sexo;
}

void habitante_setDataNascimento(Habitante h, const char *data) {
    assert(h != NULL);
    strncpy(h->data_nascimento, data, 12);
    h->data_nascimento[11] = '\0';
}

void habitante_setSemTeto(Habitante h, bool status) {
    assert(h != NULL);
    h->semTeto = status;
}

void habitante_setCep(Habitante h, const char *cep) {
    assert(h != NULL);
    strncpy(h->endereco.cep, cep, 16);
    h->endereco.cep[15] = '\0';
}

void habitante_setFace(Habitante h, char face) {
    assert(h != NULL);
    h->endereco.face = face;
}

void habitante_setNumerCcasa(Habitante h, double numero) {
    assert(h != NULL);
    h->endereco.numero = numero;
}

void habitante_setComplemento(Habitante h, const char *complemento) {
    assert(h != NULL);
    strncpy(h->endereco.complemento, complemento, 64);
    h->endereco.complemento[63] = '\0';
}

//=============================================================================
// Métodos Getters 
//=============================================================================

size_t habitante_getSize() {
    return sizeof(struct stHabitante);
}

const char* habitante_getCpf(const Habitante h) {
    assert(h != NULL);
    return h->cpf;
}

const char* habitante_getNome(const Habitante h) {
    assert(h != NULL);
    return h->nome;
}

const char* habitante_getSobrenome(const Habitante h) {
    assert(h != NULL);
    return h->sobrenome;
}

char habitante_getSexo(const Habitante h) {
    assert(h != NULL);
    return h->sexo;
}

const char* habitante_getDataNascimento(const Habitante h) {
    assert(h != NULL);
    return h->data_nascimento;
}

const char* habitante_getCep(const Habitante h) {
    assert(h != NULL);
    if (h->semTeto) return NULL;
    return h->endereco.cep;
}

char habitante_getFace(const Habitante h) {
    assert(h != NULL);
    if (h->semTeto) return '\0';
    return h->endereco.face;
}

double habitante_getNumeroCasa(const Habitante h) {
    assert(h != NULL);
    if (h->semTeto ) return -1.0;
    return h->endereco.numero;
}

const char* habitante_getComplemento(const Habitante h) {
    assert(h != NULL);
    if (h->semTeto ) return NULL;
    return h->endereco.complemento;
}

//=============================================================================
// Funções Gerais
//=============================================================================

bool habitante_isSemTeto(const Habitante h) {
    assert(h != NULL);
    return h->semTeto;
}

void habitante_printInfo(FILE *txt, const Habitante h) {
    assert(txt != NULL && h != NULL);
    fprintf(txt, "CPF: %s, Nome: %s %s, Sexo: %c, Nasc: %s\n", 
            h->cpf, h->nome, h->sobrenome, h->sexo, h->data_nascimento);
}

void habitante_printInfoEndereco(FILE *txt, const Habitante h) {
    assert(txt != NULL && h != NULL);
    if (h->semTeto) {
        fprintf(txt, "Status: Sem-teto\n");
    } else {
        fprintf(txt, "Endereço: %s/%c/%.2f", 
                h->endereco.cep, h->endereco.face, h->endereco.numero);
        if (strlen(h->endereco.complemento) > 0) {
            fprintf(txt, " - Compl: %s\n", h->endereco.complemento);
        } else {
            fprintf(txt, "\n");
        }
    }
}