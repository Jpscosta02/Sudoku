#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "../comum/configuracao.h"
#include "../comum/logs.h"
#include "../protocolo/protocolo.h"
#include "servidor_tcp.h"
#include "jogos.h"
#include "tratar_cliente.h"

int main(int argc, char *argv[])
{
    ConfigServidor cfg;
    int sockfd;

    if (argc < 2) {
        printf("Uso: %s <ficheiro_config>\n", argv[0]);
        return 1;
    }

    if (!carregarConfiguracaoServidor(argv[1], &cfg)) {
        return 1;
    }

    registarEvento("logs/servidor.log", "Servidor iniciado");

    /* Carregar jogos do ficheiro */
    if (!carregarJogosServidor(cfg.ficheiroJogos)) {
        fprintf(stderr, "Erro a carregar jogos. A sair.\n");
        return 1;
    }

    /* Criar socket TCP */
    sockfd = criarSocketServidor(cfg.porta);
    printf("Servidor pronto na porta %d\n", cfg.porta);

    while (1) {
        pthread_t tid;
        int *sockCliente = malloc(sizeof(int));

        printf("\nÀ espera de cliente...\n");

        *sockCliente = aceitarCliente(sockfd);
        if (*sockCliente < 0) {
            perror("Erro ao aceitar cliente");
            free(sockCliente);
            continue;
        }

        printf("Cliente ligado.\n");
        registarEvento("logs/servidor.log", "Cliente conectado");

        pthread_create(&tid, NULL, tratarCliente, sockCliente);
        pthread_detach(tid);
    }
    sleep(30);
    close(sockfd);
    return 0;
}
