#ifndef PARSER_PM_H
#define PARSER_PM_H

#include "hash.h"

/** 
 * Este modulo lida com a leitura do arquivo pm.
 * O arquivo pm é o arquivo que cria os habitantes e endereços (moradodores).
 * O arquivo pm tem 2 tipos de comando.
 * p: cria um habitante de bitnopolis, com cpf, nome, sobrenome, sexo e data de nascimento.
 * m: pega um morador pelo cpf, e atribui a ele um endereço, retirando o status de sem teto.
 * Quando um habitante é criado, ele é adicionado a um hash de habitantes, e depois, apagado da memoria RAM.
 */

/** 
 * @brief Lê o arquivo .pm e popula a tabela hash de habitantes.
 * @param caminho_arquivo Caminho completo para o arquivo .pm
 * @param tabela_habitantes Ponteiro para a Hash Extensível de Habitantes
 */
void parser_pm_processar(const char* caminho_arquivo, Hash* tabela_habitantes);

#endif