// comum/configuracao.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "configuracao.h"

static char *trim(char *s)
{
    while (*s && isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s);
    while (end > s && isspace((unsigned char)end[-1])) end--;
    *end = '\0';
    return s;
}

static void cortarComentario(char *s)
{
    /* Suporta comentários do tipo '# ...' */
    for (char *p = s; *p; p++) {
        if (*p == '#') {
            *p = '\0';
            return;
        }
    }
}

static int parseInt(const char *s, int *out)
{
    if (!s || !*s) return 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (!end) return 0;
    while (*end && isspace((unsigned char)*end)) end++;
    if (*end != '\0') return 0;
    if (v < (long)INT_MIN || v > (long)INT_MAX) return 0;
    *out = (int)v;
    return 1;
}

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

    char linha[256];

    // valores por omissão
    cfg->ficheiroJogos[0] = '\0';
    cfg->porta = 0;
    cfg->maxClientes = 2;

    while (fgets(linha, sizeof(linha), f)) {
        cortarComentario(linha);

        char *p = trim(linha);
        if (*p == '\0') continue;

        char *sep = strchr(p, ':');
        if (!sep) continue;
        *sep = '\0';

        char *chave = trim(p);
        char *valor = trim(sep + 1);
        if (*chave == '\0' || *valor == '\0') continue;

        if (strcmp(chave, "FICHEIRO_JOGOS") == 0) {
            strncpy(cfg->ficheiroJogos, valor, sizeof(cfg->ficheiroJogos) - 1);
            cfg->ficheiroJogos[sizeof(cfg->ficheiroJogos) - 1] = '\0';
        }
        else if (strcmp(chave, "PORTA") == 0) {
            (void)parseInt(valor, &cfg->porta);
        }
        else if (strcmp(chave, "MAX_CLIENTES") == 0) {
            (void)parseInt(valor, &cfg->maxClientes);
        }
        /* Linhas antigas como FICHEIRO_SOLUCOES (se existirem)
           são simplesmente ignoradas. */
    }

    fclose(f);

    if (cfg->ficheiroJogos[0] == '\0') {
        fprintf(stderr, "Falta FICHEIRO_JOGOS em %s\n", ficheiro);
        return 0;
    }
    if (cfg->porta < 1 || cfg->porta > 65535) {
        fprintf(stderr, "PORTA inválida em %s\n", ficheiro);
        return 0;
    }
    if (cfg->maxClientes < 1 || cfg->maxClientes > 100) {
        fprintf(stderr, "MAX_CLIENTES inválido em %s (1..100)\n", ficheiro);
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

    char linha[256];

    // valores por omissão
    cfg->ipServidor[0] = '\0';
    cfg->porta = 0;
    cfg->idCliente[0] = '\0';
    cfg->equipa = 1;    // por omissão fica na equipa 1

    while (fgets(linha, sizeof(linha), f)) {
        cortarComentario(linha);

        char *p = trim(linha);
        if (*p == '\0') continue;

        char *sep = strchr(p, ':');
        if (!sep) continue;
        *sep = '\0';

        char *chave = trim(p);
        char *valor = trim(sep + 1);
        if (*chave == '\0' || *valor == '\0') continue;

        if (strcmp(chave, "IP_SERVIDOR") == 0) {
            strncpy(cfg->ipServidor, valor, sizeof(cfg->ipServidor) - 1);
            cfg->ipServidor[sizeof(cfg->ipServidor) - 1] = '\0';
        }
        else if (strcmp(chave, "PORTA") == 0) {
            (void)parseInt(valor, &cfg->porta);
        }
        else if (strcmp(chave, "ID_CLIENTE") == 0) {
            strncpy(cfg->idCliente, valor, sizeof(cfg->idCliente) - 1);
            cfg->idCliente[sizeof(cfg->idCliente) - 1] = '\0';
        }
        else if (strcmp(chave, "EQUIPA") == 0) {
            (void)parseInt(valor, &cfg->equipa);
        }
    }

    fclose(f);

    if (cfg->ipServidor[0] == '\0') {
        fprintf(stderr, "Falta IP_SERVIDOR em %s\n", ficheiro);
        return 0;
    }
    if (cfg->porta < 1 || cfg->porta > 65535) {
        fprintf(stderr, "PORTA inválida em %s\n", ficheiro);
        return 0;
    }
    if (cfg->idCliente[0] == '\0') {
        fprintf(stderr, "Falta ID_CLIENTE em %s\n", ficheiro);
        return 0;
    }

    if (cfg->equipa < 1) cfg->equipa = 1;

    return 1;
}
