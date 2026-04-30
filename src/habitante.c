#include "../include/habitante.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct stEndereco {
    char *cep;
    char face;
    double numero;
    char *complemento;
};

struct stHabitante {
    char *cpf;
    char *nome;
    char *sobrenome;
    char sexo;
    char *data_nascimento;
    bool semTeto;
    struct stEndereco *endereco;
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
    assert(dest != NULL);
    strcpy(dest, src);
    return dest;
}

//=============================================================================
// Construtor e Destrutor
//=============================================================================

Habitante habitante_constructor(const char *cpf, const char *nome,
    const char *sobrenome, char sexo, const char *data_nascimento) {
    
    Habitante h = malloc(sizeof(struct stHabitante));
    assert(h != NULL); // Garante que a memória foi alocada[cite: 3]

    h->cpf = clone_string(cpf);
    h->nome = clone_string(nome);
    h->sobrenome = clone_string(sobrenome);
    h->sexo = sexo;
    h->data_nascimento = clone_string(data_nascimento);
    
    h->semTeto = true;
    h->endereco = NULL;

    return h;
}

void habitante_destructor(Habitante h) {
    if (h == NULL) return;

    free(h->cpf);
    free(h->nome);
    free(h->sobrenome);
    free(h->data_nascimento);

    if (h->endereco != NULL) {
        free(h->endereco->cep);
        free(h->endereco->complemento);
        free(h->endereco);
    }

    free(h);
}

//=============================================================================
// Métodos Setters
//=============================================================================

void habitante_setEndereco(Habitante h, char *cep, char face, double numero, char *complemento) {
    assert(h != NULL);

    if (h->endereco == NULL) {
        h->endereco = malloc(sizeof(struct stEndereco));
        assert(h->endereco != NULL);
        h->endereco->cep = NULL;
        h->endereco->complemento = NULL;
    }

    habitante_setCep(h, cep);
    habitante_setFace(h, face);
    habitante_setNumerCcasa(h, numero);
    habitante_setComplemento(h, complemento);
    
    h->semTeto = false;
}

void habitante_setCpf(Habitante h, const char *cpf) {
    assert(h != NULL);
    free(h->cpf);
    h->cpf = clone_string(cpf);
}

void habitante_setNome(Habitante h, const char *nome) {
    assert(h != NULL);
    free(h->nome);
    h->nome = clone_string(nome);
}

void habitante_setSobrenome(Habitante h, const char *sobrenome) {
    assert(h != NULL);
    free(h->sobrenome);
    h->sobrenome = clone_string(sobrenome);
}

void habitante_setSexo(Habitante h, char sexo) {
    assert(h != NULL);
    h->sexo = sexo;
}

void habitante_setDataNascimento(Habitante h, const char *data) {
    assert(h != NULL);
    free(h->data_nascimento);
    h->data_nascimento = clone_string(data);
}

void habitante_setSemTeto(Habitante h, bool status) {
    assert(h != NULL);
    h->semTeto = status;
    
    // Se tornou sem-teto, limpa os dados de endereço
    if (status == true && h->endereco != NULL) {
        free(h->endereco->cep);
        free(h->endereco->complemento);
        free(h->endereco);
        h->endereco = NULL;
    }
}

void habitante_setCep(Habitante h, const char *cep) {
    assert(h != NULL && h->endereco != NULL);
    free(h->endereco->cep);
    h->endereco->cep = clone_string(cep);
}

void habitante_setFace(Habitante h, char face) {
    assert(h != NULL && h->endereco != NULL);
    h->endereco->face = face;
}

void habitante_setNumerCcasa(Habitante h, double numero) {
    assert(h != NULL && h->endereco != NULL);
    h->endereco->numero = numero;
}

void habitante_setComplemento(Habitante h, const char *complemento) {
    assert(h != NULL && h->endereco != NULL);
    free(h->endereco->complemento);
    h->endereco->complemento = clone_string(complemento);
}

//=============================================================================
// Métodos Getters 
// NOTA: Para compilar, os retornos de string DEVEM ser `const char*` no .h
//=============================================================================

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
    if (h->semTeto || h->endereco == NULL) return NULL;
    return h->endereco->cep;
}

char habitante_getFace(const Habitante h) {
    assert(h != NULL);
    if (h->semTeto || h->endereco == NULL) return '\0';
    return h->endereco->face;
}

double habitante_getNumeroCasa(const Habitante h) {
    assert(h != NULL);
    if (h->semTeto || h->endereco == NULL) return -1.0;
    return h->endereco->numero;
}

const char* habitante_getComplemento(const Habitante h) {
    assert(h != NULL);
    if (h->semTeto || h->endereco == NULL) return NULL;
    return h->endereco->complemento;
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
                h->endereco->cep, h->endereco->face, h->endereco->numero);
        if (h->endereco->complemento != NULL && strlen(h->endereco->complemento) > 0) {
            fprintf(txt, " - Compl: %s\n", h->endereco->complemento);
        } else {
            fprintf(txt, "\n");
        }
    }
}