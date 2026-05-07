#ifndef SVG_H
#define SVG_H

#include <stdio.h>

#include "../include/hash.h"
#include "quadra.h"

/**
 * @brief Escreve as tags iniciais de um arquivo SVG.
 * @param arquivo Arquivo .svg aberto para escrita.
 */
void svg_iniciar(FILE *arquivo);

/**
 * @brief Escreve a tag de fechamento do arquivo SVG.
 * @param arquivo Arquivo .svg aberto para escrita.
 */
void svg_fechar(FILE *arquivo);

/**
 * @brief Desenha o retângulo de uma quadra com seu CEP centralizado.
 * @param arquivo Arquivo .svg aberto.
 * @param q Quadra a desenhar.
 */
void svg_desenhar_quadra(FILE *arquivo, const Quadra q);

/**
 * @brief Varre o hash de quadras e desenha todas as quadras presentes.
 * Deve ser chamada após o processamento do .qry para que apenas
 * as quadras que sobreviveram (não removidas por rq) sejam desenhadas.
 * @param arquivo Ponteiro para o arquivo svg.
 * @param tabela_quadras ponteiro para o hash de quadras para conseguir pegar o estado final delas.
 */
void svg_desenhar_todas_quadras(FILE *arquivo, Hash *tabela_quadras);

/**
 * @brief Desenha um X vermelho na âncora da quadra (usado em rq).
 * @param arquivo Arquivo .svg aberto.
 * @param q  Quadra removida.
 */
void svg_marcar_remocao_quadra(FILE *arquivo, const Quadra q);

/**
 * @brief Desenha a contagem de moradores por face e o total no centro da quadra (usado em pq).
 * @param arquivo Arquivo .svg aberto.
 * @param q Quadra consultada.
 * @param faceN Moradores na face N.
 * @param faceS Moradores na face S.
 * @param faceL Moradores na face L.
 * @param faceO Moradores na face O.
 * @param total Total de moradores.
 */
void svg_desenhar_contagem_moradores(FILE *arquivo, const Quadra q, int faceN, int faceS, int faceL, int faceO, int total);

/**
 * @brief Desenha uma cruz vermelha no endereço de um falecido (usado em rip).
 * @param arquivo Arquivo .svg aberto.
 * @param q Quadra onde o habitante morava.
 * @param face Face da quadra onde morava.
 * @param num Número (distância) da casa na face.
 */
void svg_marcar_obito(FILE *arquivo, const Quadra q, char face, double num);

/**
 * @brief Desenha um quadrado vermelho com o CPF no endereço de destino de uma mudança (usado em mud).
 * @param arquivo Arquivo .svg aberto.
 * @param q Quadra de destino.
 * @param face Face de destino.
 * @param num Número de destino.
 * @param cpf CPF do morador que se mudou.
 */
void svg_marcar_mudanca(FILE *arquivo, const Quadra q, char face, double num, const char *cpf);

/**
 * @brief Desenha um círculo preto no endereço onde ocorreu o despejo (usado em dspj).
 * @param arquivo Arquivo .svg aberto.
 * @param q Quadra onde ocorreu o despejo.
 * @param face Face da quadra.
 * @param num Número da casa.
 */
void svg_marcar_despejo(FILE *arquivo, const Quadra q, char face, double num);

/**
 * @brief Copia todos os elementos de um svg para outro, usado depois do qry.
 * gera um svg somente com as marcações do qry, e copia elas por sima do outro svg,
 * que terá somente as quadras finais.
 */
void svg_copiar(FILE *dst, FILE *src);

#endif