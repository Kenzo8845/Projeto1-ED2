#ifndef QUADRA_H
#define QUADRA_H

#include <stdbool.h>
#include <stdio.h>

/**  Este módulo define as funções para a manipulação de quadras.
 * Uma quadra tem como ID seu CEP, e é representada por um retângulo no arquivo SVG.
 * As quadras são locais onde os habitantes de bitnópolis vivem, e onde as casas dos moradores se localizam.
 * largura (w), altura (h), um CEP identificador, e propriedades visuais para o SVG.
 */


typedef struct stQuadra* Quadra;



//******************************************** //
//           Constructor e Destructor
//******************************************** //

/**
 * @brief  Cria uma nova quadra com dados passados pelo arquivo.geo.
 * @param cep CEP da quadra a ser criada.
 * @param x Coordenada x da âncora da quadra a ser criada.
 * @param y Coordenada y da âncora da quadra a ser criada.
 * @param w Largura da quadra a ser criada.
 * @param h Altura da quadra a ser criada.

 * @return Quadra 
 */
Quadra quadra_constructor(const char *cep, double x, double y, double w, double h);

/**
 * @brief Destroi a quadra liberando a memória alocada a ela. (não libera memoria dos habitantes que nela estão).
 * @param q Quadra a ser destruída.
 */
void quadra_destructor(Quadra q);


//******************************************** //
//                Metodos get 
//******************************************** //

/**
 * @brief Pega o CEP de um determinada quadra.
 * @param q A quadra escolhida.
 * @return const char* O CEP da quadra.
 */
const char* quadra_getCep(const Quadra q);

/**
 * @brief Pega a coordenada X da âncora de uma determinada quadra.
 * @param q A quadra escolhida.
 * @return double A coordenada X da quadra.
 */
double quadra_getX(const Quadra q);

/**
 * @brief Pega a coordenada Y da âncora de uma determinada quadra.
 * @param q A quadra escolhida.
 * @return double A coordenada Y da quadra.
 */
double quadra_getY(const Quadra q);

/**
 * @brief Pega a largura de uma determinada quadra.
 * @param q A quadra escolhida.
 * @return double A largura da quadra.
 */
double quadra_getW(const Quadra q);

/**
 * @brief Pega a altura de uma determinada quadra.
 * @param q A quadra escolhida.
 * @return double A altura da quadra.
 */
double quadra_getH(const Quadra q);

/**
 * @brief Pega a cor de preenchimento de uma determinada quadra.
 * @param q A quadra escolhida.
 * @return char* Cor de preenchimento da quadra.
 */
const char* quadra_getCorp(const Quadra q);

/**
 * @brief Pega a cor de borda de uma determinada quadra.
 * @param q A quadra escolhida.
 * @return char* Cor de borda da quadra.
 */
const char* quadra_getCorb(const Quadra q);

/**
 * @brief Pega a largura de borda de uma determinada quadra.
 * @param q A quadra escolhida.
 * @return Double Largura de borda da quadra.
 */
double quadra_getSw(const Quadra q);


//******************************************** //
//                Metodos set 
//******************************************** //

/**
 * @brief Altera o CEP de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoCep O novo CEP.
 */
void quadra_setCep(const Quadra q, const char* novoCep);

/**
 * @brief Altera a coordenada X da âncora de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoX A nova coordenada X da âncora.
 */
void quadra_setX(const Quadra q, double novoX);

/**
 * @brief Altera a coordenada Y da âncora de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoY A nova coordenada Y da âncora.
 */
void quadra_setY(const Quadra q, double novoY);

/**
 * @brief Altera a largura de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoW A nova largura.
 */
void quadra_setW(const Quadra q, double novoW);

/**
 * @brief Altera a altura de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoH A nova altura.
 */
void quadra_setH(const Quadra q, double novoH);

/**
 * @brief Altera a cor de preenchimento de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoCorp A nova cor de preenchimento.
 */
void quadra_setCorp(const Quadra q, const char* novoCorp);


/**
 * @brief Altera a cor de borda de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoCorp A nova cor de borda.
 */
void quadra_setCorb(const Quadra q, const char* novoCorb);

/**
 * @brief Altera a largura de borda de uma determinada quadra.
 * @param q A quadra a ser alterada.
 * @param novoSw A nova largura de borda.
 */
void quadra_setSw(const Quadra q, double novoSw);


//******************************************** //
//                 Funções
//******************************************** //


#endif