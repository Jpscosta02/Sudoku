// comum/configuracao.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuracao.h"

/* ============================================================
   CARREGAR CONFIGURAÇÃO DO SERVIDOR
   ficheiro de exemplo (server.conf):
       FICHEIRO_JOGOS: jogos.txt
       PORTA: 5050
       MAX_CLIENTES: 4
   ============================================================ */

int carregarConfiguracaoServidor(const char *ficheiro, ConfigServidor *cfg)
{
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        perror("Erro ao abrir ficheiro de configuração do servidor");
        return 0;
    }

    char linha[256], chave[64], valor[192];

    // valores por omissão
    cfg->ficheiroJogos[0] = '\0';
    cfg->porta = 0;
    cfg->maxClientes = 2;

    while (fgets(linha, sizeof(linha), f)) {

        if (sscanf(linha, "%[^:]: %s", chave, valor) == 2) {

            if (strcmp(chave, "FICHEIRO_JOGOS") == 0) {
                strncpy(cfg->ficheiroJogos, valor, sizeof(cfg->ficheiroJogos) - 1);
                cfg->ficheiroJogos[sizeof(cfg->ficheiroJogos) - 1] = '\0';
            }
            else if (strcmp(chave, "PORTA") == 0) {
                cfg->porta = atoi(valor);
            }
            else if (strcmp(chave, "MAX_CLIENTES") == 0) {
                cfg->maxClientes = atoi(valor);
            }
            /* Linhas antigas como FICHEIRO_SOLUCOES (se existirem)
               são simplesmente ignoradas. */
        }
    }

    fclose(f);

    if (cfg->ficheiroJogos[0] == '\0') {
        fprintf(stderr, "Falta FICHEIRO_JOGOS em %s\n", ficheiro);
        return 0;
    }
    if (cfg->porta == 0) {
        fprintf(stderr, "Falta PORTA em %s\n", ficheiro);
        return 0;
    }

    return 1;
}

/* ============================================================
   CARREGAR CONFIGURAÇÃO DO CLIENTE
   ficheiro de exemplo (client1.conf):
       IP_SERVIDOR: 127.0.0.1
       PORTA: 5050
       ID_CLIENTE: Cliente1
       EQUIPA: 1
   ============================================================ */

int carregarConfiguracaoCliente(const char *ficheiro, ConfigCliente *cfg)
{
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        perror("Erro ao abrir configuração do cliente");
        return 0;
    }

    char linha[256], chave[64], valor[192];

    // valores por omissão
    cfg->ipServidor[0] = '\0';
    cfg->porta = 0;
    cfg->idCliente[0] = '\0';
    cfg->equipa = 1;    // por omissão fica na equipa 1

    while (fgets(linha, sizeof(linha), f)) {
        if (sscanf(linha, "%[^:]: %s", chave, valor) == 2) {

            if (strcmp(chave, "IP_SERVIDOR") == 0) {
                strncpy(cfg->ipServidor, valor, sizeof(cfg->ipServidor) - 1);
                cfg->ipServidor[sizeof(cfg->ipServidor) - 1] = '\0';
            }
            else if (strcmp(chave, "PORTA") == 0) {
                cfg->porta = atoi(valor);
            }
            else if (strcmp(chave, "ID_CLIENTE") == 0) {
                strncpy(cfg->idCliente, valor, sizeof(cfg->idCliente) - 1);
                cfg->idCliente[sizeof(cfg->idCliente) - 1] = '\0';
            }
            else if (strcmp(chave, "EQUIPA") == 0) {
                cfg->equipa = atoi(valor);
            }
        }
    }

    fclose(f);

    if (cfg->ipServidor[0] == '\0') {
        fprintf(stderr, "Falta IP_SERVIDOR em %s\n", ficheiro);
        return 0;
    }
    if (cfg->porta == 0) {
        fprintf(stderr, "Falta PORTA em %s\n", ficheiro);
        return 0;
    }
    if (cfg->idCliente[0] == '\0') {
        fprintf(stderr, "Falta ID_CLIENTE em %s\n", ficheiro);
        return 0;
    }

    return 1;
}
