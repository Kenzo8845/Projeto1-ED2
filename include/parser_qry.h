#ifndef PARSER_QRY_H
#define PARSER_QRY_H

#include <stdio.h>
#include "hash.h"

/**
 * Este modulo lida com a leitura do arquivo qry.
 * O arquivo qry é o principal arquivo de instruções do programa.
 * Apos a leitura do geo e do pm, onde as quadras e os habitantes iniciais estão criados e cada um em seu hash,
 * os comandos qry alteram, adicionam ou removem valores desses (habitantes ou quadras).
 * Alguns comandos geram texto no arquivo txt do final da execução, e/ou imagnes no svg final. 
 */

/**
 * @brief Processa o arquivo .qry, executando comandos e gerando saídas.
 * @param caminho_arquivo Caminho do arquivo .qry
 * @param quadras Hash de Quadras
 * @param habitantes Hash de Habitantes
 * @param txt Arquivo texto de saída aberto para escrita (modo "w")
 * @param svg Arquivo SVG aberto no qual os novos elementos serão anexados (modo "a")
 */
void parser_qry_processar(const char* caminho_arquivo, Hash* quadras, Hash* habitantes, FILE* txt, FILE* svg);

#endif