#ifndef CONFIGURACAO_H
#define CONFIGURACAO_H

typedef struct {
    char ficheiroJogos[128];
    char ficheiroSolucoes[128];
    int porta;
    int maxClientes;     // configurado no server.conf
} ConfigServidor;

typedef struct {
    char ipServidor[64];
    int porta;
    char idCliente[64];
} ConfigCliente;

/* -------- PROTÓTIPOS CORRETOS -------- */

int carregarConfiguracaoServidor(const char *ficheiro, ConfigServidor *cfg);
int carregarConfiguracaoCliente(const char *ficheiro, ConfigCliente *cfg);

#endif
