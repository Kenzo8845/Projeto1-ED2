#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../include/habitante.h"
#include "../include/hash.h"


uint64_t converter_cpf_para_chave(const char* cpf) {
    uint64_t chave = 0;
    for (int i = 0; cpf[i] != '\0'; i++) {
        if (cpf[i] >= '0' && cpf[i] <= '9') {
            chave = chave * 10 + (cpf[i] - '0');
        }
    }
    return chave;
}

void parser_pm_processar(const char *caminho_ficheiro, Hash *tabela_habitantes) {
    FILE *ficheiro = fopen(caminho_ficheiro, "r");
    if (ficheiro == NULL) {
        perror("Erro ao abrir o ficheiro .pm");
        return;
    }

    char linha[256];
    
    while (fgets(linha, sizeof(linha), ficheiro) != NULL) {
        char comando[10];
        if (sscanf(linha, "%s", comando) != 1) continue;

        //comando p.
        if (strcmp(comando, "p") == 0) {
            char cpf[20], nome[64], sobrenome[64], data_nasc[15];
            char sexo;
            
            sscanf(linha, "%*s %19s %63s %63s %c %14s", 
                   cpf, nome, sobrenome, &sexo, data_nasc);
            
            Habitante novo_hab = habitante_constructor(cpf, nome, sobrenome, sexo, data_nasc);
            
            uint64_t chave = converter_cpf_para_chave(cpf);
            
            size_t tamanho = habitante_getSize(); 
            hash_insert(tabela_habitantes, novo_hab, chave, tamanho);
            
            habitante_destructor(novo_hab);
        } 

        //comando m.
        else if (strcmp(comando, "m") == 0) {
            char cpf[20], cep[20], face, complemento[64];
            double num;
            
            sscanf(linha, "%*s %19s %19s %c %lf %63[^\n]", 
                   cpf, cep, &face, &num, complemento);
            
            uint64_t chave = converter_cpf_para_chave(cpf);
            size_t tamanho;
            
            Habitante hab = (Habitante) hash_search(tabela_habitantes, chave, &tamanho);
            if (hab != NULL) {
                habitante_setEndereco(hab, cep, face, num, complemento);

                size_t sz_removido;
                void *removido = hash_remove(tabela_habitantes, chave, &sz_removido);
                free(removido);  

                hash_insert(tabela_habitantes, hab, chave, tamanho);
                habitante_destructor(hab);
            } else {
                printf("Aviso: Tentativa de dar endereco a CPF inexistente: %s\n", cpf);
            }
        }
    }
    
    fclose(ficheiro);
}