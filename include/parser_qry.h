#ifndef PARSER_QRY_H
#define PARSER_QRY_H

#include <stdio.h>
#include "hash.h"

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