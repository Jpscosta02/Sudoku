// servidor/servidor.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "../comum/configuracao.h"
#include "servidor_tcp.h"
#include "tratar_cliente.h"

#include "barreira.h"
#include "jogos.h"
#include "equipas.h"
#include "clientes_ligados.h"
#include "sincronizacao.h"
#include "ranking.h"  

/* Flag global para shutdown (Ctrl+C) */
volatile sig_atomic_t pararServidor = 0;
static int sockGlobal = -1;

static void sigint_handler(int sig)
{
    (void)sig;
    pararServidor = 1;
    if (sockGlobal >= 0) {
        close(sockGlobal); /* desbloqueia accept() */
    }
    printf("\n[SHUTDOWN] SIGINT recebido. A encerrar servidor...\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Uso: %s <ficheiro-config>\n", argv[0]);
        return 1;
    }

    ConfigServidor cfg;

    if (!carregarConfiguracaoServidor(argv[1], &cfg)) {
        printf("Erro ao carregar configuração.\n");
        return 1;
    }

    /* Instalar handler SIGINT (Ctrl+C) */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    printf("MAX_CLIENTES=%d (modo competição)\n", cfg.maxClientes);

    /* ======================
       Inicializar módulos
       ====================== */

    inicializarBarreira(cfg.maxClientes);          // barreira para arranque simultâneo
    inicializarSincronizacao(cfg.maxClientes);     // semáforo de clientes
    inicializarEquipas();                          // estado das equipas
    inicializarClientesLigados();                  // lista de clientes ligados
    limparResultadosCompeticao();                  // ranking vazio no início

    if (!carregarJogosServidor(cfg.ficheiroJogos)) {
        fprintf(stderr, "Erro: não foi possível carregar jogos de '%s'\n", cfg.ficheiroJogos);
        return 1;
    }

    /* ======================
       Criar socket TCP servidor
       ====================== */

    int sockListen = criarSocketServidor(cfg.porta);
    if (sockListen < 0) {
        perror("Erro no socket servidor");
        return 1;
    }
    sockGlobal = sockListen;

    printf("Servidor TCP à escuta na porta %d...\n", cfg.porta);
    printf("Servidor pronto.\n");

    /* ======================
       Aceitar clientes
       ====================== */

    aceitarClientes(sockListen);

    close(sockListen);
    return 0;
}
