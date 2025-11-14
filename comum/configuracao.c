#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuracao.h"

/* =====================================================
   LER CONFIGURAÇÃO DO SERVIDOR (server.conf)
   FICHEIRO_JOGOS: jogos.txt
   FICHEIRO_SOLUCOES: jogos.txt
   PORTA: 5050
   ===================================================== */

int carregarConfiguracaoServidor(const char *ficheiro, ConfigServidor *cfg)
{
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        perror("Erro ao abrir ficheiro de configuração do servidor");
        return 0;
    }

    char linha[256], chave[64], valor[192];

    while (fgets(linha, sizeof(linha), f)) {

        if (sscanf(linha, "%[^:]: %s", chave, valor) == 2) {

            if (strcmp(chave, "FICHEIRO_JOGOS") == 0) {
                strcpy(cfg->ficheiroJogos, valor);
            }
            else if (strcmp(chave, "FICHEIRO_SOLUCOES") == 0) {
                strcpy(cfg->ficheiroSolucoes, valor);
            }
            else if (strcmp(chave, "PORTA") == 0) {
                cfg->porta = atoi(valor);
            }
        }
    }

    fclose(f);
    return 1;
}

/* =====================================================
   LER CONFIGURAÇÃO DO CLIENTE (clientX.conf)
   IP_SERVIDOR: 127.0.0.1
   PORTA: 5050
   ID_CLIENTE: Cliente1
   ===================================================== */

int carregarConfiguracaoCliente(const char *ficheiro, ConfigCliente *cfg)
{
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        perror("Erro ao abrir ficheiro de configuração do cliente");
        return 0;
    }

    char linha[256], chave[64], valor[192];

    while (fgets(linha, sizeof(linha), f)) {

        if (sscanf(linha, "%[^:]: %s", chave, valor) == 2) {

            if (strcmp(chave, "IP_SERVIDOR") == 0) {
                strcpy(cfg->ipServidor, valor);
            }
            else if (strcmp(chave, "PORTA") == 0) {
                cfg->porta = atoi(valor);
            }
            else if (strcmp(chave, "ID_CLIENTE") == 0) {
                strcpy(cfg->idCliente, valor);
            }
        }
    }

    fclose(f);
    return 1;
}
