// comum/configuracao.h
#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

/* ================== SERVIDOR ================== */

typedef struct {
    char ficheiroJogos[256];  // ficheiro com "id,puzzle,solucao"
    int porta;
    int maxClientes;          // nº máximo de clientes concorrentes (ex: 4)
} ConfigServidor;

/* ================== CLIENTE ================== */

typedef struct {
    char ipServidor[64];
    int porta;
    char idCliente[64];
    int equipa;               // 1 ou 2 (para modo competição em equipas)
} ConfigCliente;

/* ================== FUNÇÕES ================== */

int carregarConfiguracaoServidor(const char *ficheiro, ConfigServidor *cfg);
int carregarConfiguracaoCliente(const char *ficheiro, ConfigCliente *cfg);

#endif
