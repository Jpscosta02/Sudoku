// servidor/servidor.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../comum/configuracao.h"
#include "servidor_tcp.h"
#include "tratar_cliente.h"

#include "barreira.h"
#include "jogos.h"
#include "equipas.h"
#include "clientes_ligados.h"
#include "sincronizacao.h"
#include "ranking.h"    /* 👉 ranking das equipas */

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

    printf("MAX_CLIENTES=%d (modo competição)\n", cfg.maxClientes);

    /* ======================
       Inicializar módulos
       ====================== */

    inicializarBarreira(cfg.maxClientes);          // barreira de arranque
    inicializarSincronizacao(cfg.maxClientes);     // semáforo de clientes
    inicializarEquipas();                          // estado das equipas
    inicializarClientesLigados();                  // lista de clientes ligados
    limparResultadosCompeticao();                  // ranking vazio no início

    carregarJogosServidor(cfg.ficheiroJogos);

    /* ======================
       Criar socket TCP servidor
       ====================== */

    int sockListen = criarSocketServidor(cfg.porta);
    if (sockListen < 0) {
        perror("Erro no socket servidor");
        return 1;
    }

    printf("Servidor TCP à escuta na porta %d...\n", cfg.porta);
    printf("Servidor pronto.\n");

    /* ======================
       Aceitar clientes
       ====================== */

    aceitarClientes(sockListen);

    close(sockListen);
    return 0;
}
