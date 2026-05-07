#ifndef PARSER_GEO_H
#define PARSER_GEO_H

#include "hash.h"

/** 
 * Este módulo lida com a leitura do arquivo .geo, e preenche um arquivo hash com as quadras.
 * O arquivo geo contem 2 comandos:
 * q: cria uma quadra com os dados do comando (cep, x, y, w, h), e define a cor da quadra com 
 * as especificações de cores atuais da função.
 * cq: muda as cores atuais da função, toda quadra criada após esse comando terá as cores dadas
 * por ele (corb, corp).
 * Quando uma quadra é criada, ela é adicionada ao arquivo hash, e depois é limpa da memória RAM, 
 * previnindo memory leaks.
 * Ao criar quadras, elas ja sao desenhaas num arquivo svg, que é uma das saidas.
 */


/**
 * @brief Lê o arquivo .geo e popula a tabela hash de quadras.
 * @param caminho_arquivo Caminho completo para o arquivo .geo a ser lido.
 * @param tabela_quadras Ponteiro para a Hash Extensível onde as quadras serão salvas.
 * @param svg_geo Ponteiro para o arquivo SVG de saída onde serão desenhadaas as quadras iniciais.
 */
void parser_geo_processar(const char* caminho_arquivo, Hash* tabela_quadras, FILE* svg_geo);

#endif