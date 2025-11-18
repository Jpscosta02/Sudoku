// servidor/servidor.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "../comum/configuracao.h"
#include "../comum/logs.h"
#include "servidor_tcp.h"
#include "tratar_cliente.h"
#include "sincronizacao.h"
#include "barreira.h"
#include "validacao_fifo.h"

int main(int argc, char *argv[])
{
    ConfigServidor cfg;

    if (argc < 2) {
        printf("Uso: %s server.conf\n", argv[0]);
        return 1;
    }

    if (!carregarConfiguracaoServidor(argv[1], &cfg)) return 1;

    printf("MAX_CLIENTES = %d (competição com %d jogadores)\n",
           cfg.maxClientes, cfg.maxClientes);

    inicializarSincronizacao(cfg.maxClientes);
    inicializarBarreira();
    iniciarValidadorFIFO();

    carregarJogosServidor(cfg.ficheiroJogos);

    int sock = criarSocketServidor(cfg.porta);
    printf("Servidor pronto.\n");

    while (1) {

        int *sockCliente = malloc(sizeof(int));
        *sockCliente = aceitarCliente(sock);

        pthread_t tid;
        pthread_create(&tid, NULL, tratarCliente, sockCliente);
        pthread_detach(tid);
    }

    return 0;
}
