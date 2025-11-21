// servidor/tratar_cliente.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "../comum/util.h"
#include "../comum/logs.h"
#include "../protocolo/protocolo.h"

#include "tratar_cliente.h"
#include "gestor_ids.h"
#include "jogos.h"
#include "clientes_ligados.h"
#include "validacao_fifo.h"
#include "ranking.h"
#include "equipas.h"
#include "barreira.h"
#include "servidor_tcp.h"

#define MODO_NORMAL      1
#define MODO_COMPETICAO  2

extern sem_t semClientes;

/* ============================================================
   LOOP PRINCIPAL DO CLIENTE
   ============================================================ */
static void loopReceberJogo(int sock,
                            int modo,
                            int equipa,
                            const Jogo *jogo,
                            int idCliente)
{
    char buffer[256];
    char solucaoRecebida[82];
    int idJogo = jogo->id;

    while (1) {

        int n = readline(sock, buffer, sizeof(buffer));
        if (n <= 0) {
            printf("[DEBUG] Cliente %d desconectou.\n", idCliente);
            return;
        }

        if (buffer[n - 1] == '\n')
            buffer[n - 1] = '\0';

        /* -------- SET -------- */
        if (strncmp(buffer, "SET", 3) == 0) {
            int lin, col, val;

            if (sscanf(buffer, "SET %d %d %d", &lin, &col, &val) == 3) {

                printf("[DEBUG] (cliente %d) RECEBI SET (%d,%d)=%d\n",
                       idCliente, lin, col, val);

                if (modo == MODO_COMPETICAO) {
                    enviarUpdateEquipa(equipa, idCliente, lin, col, val);
                }
            }
            continue;
        }

        /* -------- SOLUCAO -------- */
        if (strncmp(buffer, "SOLUCAO", 7) == 0) {

            int idRecebido;
            if (sscanf(buffer, "SOLUCAO %d %81s",
                       &idRecebido, solucaoRecebida) == 2)
            {
                int erros =
                    validarSudokuFIFO(solucaoRecebida, jogo->solucao);

                if (erros == 0) {

                    int ultimo = 0;   /* 1 se esta equipa for a última a terminar */

                    if (modo == MODO_COMPETICAO) {

                        time_t tFim = time(NULL);
                        double tempoEquipa = 0.0;

                        int marcou =
                            registarFimEquipa(equipa, tFim, &tempoEquipa);

                        if (marcou) {
                            printf("[DEBUG] Equipa %d terminou em %.2f s\n",
                                   equipa, tempoEquipa);

                            /* Guardar no ranking */
                            registarResultadoCompeticao(equipa, tempoEquipa);

                            /* Verificar se TODAS as equipas já terminaram */
                            if (todasEquipasTerminaram()) {
                                ultimo = 1;
                            }
                        }
                    }

                    /* Primeiro: enviar SEMPRE o RESULTADO para este cliente */
                    enviarResultadoOK(sock, idJogo);

                    /* Só depois, e apenas uma vez (última equipa), enviar ranking a todos */
                    if (modo == MODO_COMPETICAO && ultimo) {
                        printf("[RANKING] Todas as equipas terminaram! A enviar ranking...\n");
                        enviarRankingATodos();
                    }
                }
                else {
                    enviarResultadoErros(sock, idJogo, erros);
                }
            }
            continue;
        }

        /* -------- SAIR -------- */
        if (strncmp(buffer, "SAIR", 4) == 0) {
            printf("[DEBUG] Cliente %d pediu SAIR.\n", idCliente);
            return;
        }
    }
}

/* ============================================================
   THREAD PRINCIPAL DO CLIENTE
   ============================================================ */
void *tratarCliente(void *arg)
{
    int sock = *(int*)arg;
    free(arg);

    char idBase[64];
    int modo = 0;
    int equipa = 0;

    printf("[DEBUG] Thread criada (sock=%d)\n", sock);

    /* 1) Receber pedido de jogo (inclui modo e equipa) */
    if (receberPedidoJogoServidorModo(sock,
                                      idBase, sizeof(idBase),
                                      &modo, &equipa) <= 0)
    {
        close(sock);
        sem_post(&semClientes);
        return NULL;
    }

    /* 2) Atribuir ID interno */
    int idCliente = atribuirIdCliente();
    enviarIdAtribuidoServidor(sock, idCliente);

    /* 3) Modo competição → registar equipa e esperar barreira */
    if (modo == MODO_COMPETICAO) {

        registarClienteLigado(idCliente, equipa, sock);
        registarEntradaJogador(equipa);

        printf("[DEBUG] Cliente %d A ENTRAR NA BARREIRA...\n", idCliente);
        entrarBarreira();
        printf("[DEBUG] Cliente %d SAIU DA BARREIRA!\n", idCliente);
    }

    /* 4) Obter jogo */
    const Jogo *jogo = obterJogoProximo();
    if (!jogo) {
        enviarErro(sock, "Sem jogos");
        close(sock);
        sem_post(&semClientes);
        return NULL;
    }

    /* 5) Enviar tabuleiro para o cliente */
    if (enviarJogoServidor(sock, jogo->id, jogo->jogo) <= 0) {
        close(sock);
        sem_post(&semClientes);
        return NULL;
    }

    /* 6) Loop de interação */
    loopReceberJogo(sock, modo, equipa, jogo, idCliente);

    /* 7) Cleanup */
    if (modo == MODO_COMPETICAO)
        removerClienteLigado(idCliente);

    libertarIdCliente(idCliente);
    close(sock);
    sem_post(&semClientes);
    return NULL;
}

/* ============================================================
   ACEITAR CLIENTES EM LOOP
   ============================================================ */
void aceitarClientes(int sockListen)
{
    printf("[DEBUG] A aceitar clientes...\n");

    while (1) {

        sem_wait(&semClientes);

        int *pSock = malloc(sizeof(int));
        *pSock = aceitarCliente(sockListen);

        if (*pSock < 0) {
            free(pSock);
            sem_post(&semClientes);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, tratarCliente, pSock);
        pthread_detach(tid);
    }
}
