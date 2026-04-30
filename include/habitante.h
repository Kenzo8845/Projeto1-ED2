#ifndef HABITANTE_H
#define HABITANTE_H

#include <stdbool.h>
#include <stdio.h>

/**  Este Modulo define as funções básicas para a manipulação de habitantes e seus endereços.
 *  Um habitante é composto por: nome, cpf, sobrenome, sexo e data de nascimento.
 *  Quando um habitante é criado, se define por padrão como sem teto, até que um endereço
 *  seja vinculado a ele, assim se transformando em um morador.
 *  Um endereço é composto por: CEP, face, numero e complemento.
 *  
 *  Este modulo é definido de maneira opaca e segura (encapsulamento), com uso de metodos set e get.
 *  Com o endereço sendo atribuido diretamente ao habitante, não é necessario a modulariação da casa.
 */


typedef struct stHabitante* Habitante;
typedef struct stEndereco* Enderecoo;


//******************************************** */
//           Constructor e Destuctror
//******************************************** */

/**
 * @brief Cria um novo habitante com os dados passados pelo arquivo pm.
 * @param cpf numero de CPF de um habitante.
 * @param nome Nome do habitante.
 * @param sobrenome Sobrenome do habitante.
 * @param sexo char que define o sexo do habitante.
 * @param data_nascimento Data de nascimento de um habitante, em formato de DD/MM/AA 
 * @return Habitante um ponteiro para o habitante criado.
 */
Habitante habitante_constructor(const char *cpf, const char *nome,
    const char *sobrenome, char sexo, const char *data_nascimento);


/**
 * @brief Libera a memoria alocada para um habitante.
 * @param h o habitante a ser excluido.
 */
void habitante_destructor(Habitante h);

//******************************************** */
//                Metodos set 
//******************************************** */

/**
 * @brief Atualiza o endereço de um habitante, e caso ele ser sem-teto, retira o status sem-teto.
 * @param h Ponterio para o habitante.
 * @param cep Novo CEP da quadra onde o novo endereço esta localizado.
 * @param face Face da quadra (N, L, O, S). 
 * @param numero Distancia da casa da face da quadra.
 * @param complemento Complemento do novo endereço.
 */
void habitante_setEndereco(Habitante h, char *cep, char face, double numero, char *complemento);


/** @brief Atualiza o CPF do habitante.
 *  @param h Ponteiro para o habitante.
 * @param cpf Novo CPF.
 */
void habitante_setCpf(Habitante h, const char *cpf);

/** @brief Atualiza o nome do habitante.
 * @param h Ponteiro para o habitante.
 * @param nome Novo nome.
 */
void habitante_setNome(Habitante h, const char *nome);

/**  @brief Atualiza o sobrenome do habitante.
 * @param h Ponteiro para o habitante.
 *  @param sobrenome Novo sobrenome.
 */
void habitante_setSobrenome(Habitante h, const char *sobrenome);

/** @brief Atualiza o sexo do habitante.
 *  @param h Ponteiro para o habitante.
 *  @param sexo Novo caractere de sexo.
 */
void habitante_setSexo(Habitante h, char sexo);

/** @brief Atualiza a data de nascimento do habitante.
 *  @param h Ponteiro para o habitante.
 *  @param data Nova data de nascimento.
 */
void habitante_setDataNascimento(Habitante h, const char *data);

/** @brief Força a mudança do status de moradia (sem-teto).
 *  @param h Ponteiro para o habitante.
 *  @param status True para sem-teto, false se possuir moradia.
 */
void habitante_setSemTeto(Habitante h, bool status);

// Setters de endereço.

/** @brief Atualiza o CEP da moradia.
 * @param h Ponteiro para o habitante.
 * @param cep Novo CEP.
 */
void habitante_setCep(Habitante h, const char *cep);

/** @brief Atualiza a face da moradia.
 * @param h Ponteiro para o habitante.
 * @param face Nova face ('N', 'S', 'L' ou 'O').
 */
void habitante_setFace(Habitante h, char face);

/** @brief Atualiza a distância/número da casa.
 * @param h Ponteiro para o habitante.
 * @param numero Novo numero da casa.
 */
void habitante_setNumerCcasa(Habitante h, double numero);

/** @brief Atualiza o complemento da moradia.
 * @param h Ponteiro para o habitante.
 * @param complemento Novo complemento.
 */
void habitante_setComplemento(Habitante h, const char *complemento);


//******************************************** */
//                Metodos get 
//******************************************** */

/** @brief Obtém o CPF de um habitante.
 * @param h Ponteiro constante para o habitante.
 *  @return String constante com o CPF.
 */
const char* habitante_getCpf(const Habitante h);

/** @brief Obtém o nome de um habitante.
 * @param h Ponteiro constante para o habitante.
 * @return String constante com o nome.
 */
const char* habitante_getNome(const Habitante h);

/** @brief Obtém o sobrenome de um habitante.
 * @param h Ponteiro constante para o habitante.
 * @return String com o sobrenome.
 */
const char* habitante_getSobrenome(const Habitante h);

/** @brief Obtém o sexo de um habitante.
 * @param h Ponteiro constante para o habitante.
 * @return Caractere representante do sexo.
 */
char habitante_getSexo(const Habitante h);

/** @brief Obtém a data de nascimento de um habitante.
 * @param h Ponteiro constante para o habitante.
 * @return String constante com a data de nascimento.
 */
const char* habitante_getDataNascimento(const Habitante h);

// Getters de endereço.

const char* habitante_getCep(const Habitante h);

/** @brief Obtém a face da quadra onde o habitante mora.
 * @param h Ponteiro constante para o habitante.
 * @return Caractere da face, ou '\0' se for sem-teto.
 */
char habitante_getFace(const Habitante h);

/** @brief Obtém o número da casa onde o habitante mora.
 * @param h Ponteiro constante para o habitante.
 * @return Valor numérico da casa, ou -1.0 se for sem-teto.
 */
double habitante_getNumeroCasa(const Habitante h);

/** @brief Obtém o complemento do endereço do habitante.
 * @param h Ponteiro constante para o habitante.
 * @return String constante com o complemento, ou NULL se for sem-teto.
 */
const char* habitante_getComplemento(const Habitante h);


//******************************************** */
//                 Funções
//******************************************** */


/** @brief Insere em um arquivo de texto, as informações gerais (nome, cpf, etc) de um habitante.
 * @param txt Ponteiro para o arquivo.
 * @param h Ponteiro para o habitante.
 */
void habitante_printInfo(FILE *txt, const Habitante h);

/** @brief Insere em um arquivo de texto, as informações do endereço de um habitante, já formatadas.
 * @param txt Ponteiro para o arquivo.
 * @param h Ponteiro para o habitante.
 */
void habitante_printInfoEndereco(FILE *txt, const Habitante h);

/** @brief Verifica se o habitante é um sem-teto.
 * @param hab Ponteiro constante para o habitante.
 * @return Retorna true se for sem-teto, ou false caso morador.
 */
bool habitante_isSemTeto(const Habitante h);

#endif