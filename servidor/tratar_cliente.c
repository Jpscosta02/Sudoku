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

#include "sincronizacao.h"
#include "tratar_cliente.h"
#include "gestor_ids.h"
#include "jogos.h"
#include "clientes_ligados.h"
#include "validacao_fifo.h"
#include "ranking.h"
#include "equipas.h"
#include "barreira.h"
#include "servidor_tcp.h"

/* ============================================================
   DEFINIÇÕES DE MODO DE JOGO
   ============================================================ */
#define MODO_NORMAL      1
#define MODO_COMPETICAO  2

/* Semáforo que controla número de clientes aceites */
extern sem_t semClientes;

/* Mutex para garantir que apenas 1 thread escolhe o jogo da competição */
static pthread_mutex_t mxJogoCompeticao = PTHREAD_MUTEX_INITIALIZER;

/* Ponteiro para o jogo escolhido para todas as equipas */
static const Jogo *jogoCompeticao = NULL;

/* ============================================================
   LOOP PRINCIPAL DO CLIENTE
   ------------------------------------------------------------
   Trata TODAS as mensagens enviadas pelo cliente durante o jogo:
     - SET
     - SOLUCAO
     - SAIR
   Este loop corre até o cliente sair ou fechar socket.
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

        /* Lê linha completa enviada pelo cliente */
        int n = readline(sock, buffer, sizeof(buffer));
        if (n <= 0) {
            printf("[DEBUG] Cliente %d desconectou.\n", idCliente);
            return;
        }

        /* Remover newline final */
        if (buffer[n - 1] == '\n')
            buffer[n - 1] = '\0';

        /* ======================================================
           SET — Jogada do jogador
           ====================================================== */
        if (strncmp(buffer, "SET", 3) == 0) {
            int lin, col, val;

            if (sscanf(buffer, "SET %d %d %d", &lin, &col, &val) == 3) {

                printf("[DEBUG] (cliente %d) RECEBI SET (%d,%d)=%d\n",
                       idCliente, lin, col, val);

                /* No modo competição, reenviar jogada aos restantes
                   jogadores da mesma equipa */
                if (modo == MODO_COMPETICAO) {
                    enviarUpdateEquipa(equipa, idCliente, lin, col, val);
                }
            }
            continue;
        }

        /* ======================================================
           SOLUCAO — Cliente enviou solução final
           ====================================================== */
        if (strncmp(buffer, "SOLUCAO", 7) == 0) {

            int idRecebido;
            if (sscanf(buffer, "SOLUCAO %d %81s",
                       &idRecebido, solucaoRecebida) == 2)
            {
                /* Validação assíncrona via FIFO */
                int erros = validarSudokuFIFO(solucaoRecebida, jogo->solucao);

                /* === SOLUÇÃO CORRETA === */
                if (erros == 0) {

                    int ultimo = 0;   /* Fica 1 se for a última equipa a terminar */

                    if (modo == MODO_COMPETICAO) {

                        /* Registar tempo final da equipa */
                        time_t tFim = time(NULL);
                        double tempoEquipa = 0.0;

                        int marcou = registarFimEquipa(equipa, tFim, &tempoEquipa);

                        /* Apenas a primeira vez marca */
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

                    /* Enviar SEMPRE o resultado OK */
                    enviarResultadoOK(sock, idJogo);

                    /* Se for a última equipa → enviar ranking + reset da competição */
                    if (modo == MODO_COMPETICAO && ultimo) {
                        printf("[RANKING] Todas as equipas terminaram! A enviar ranking...\n");
                        enviarRankingATodos();

                        /* ======================================================
                           RESET TOTAL DA COMPETIÇÃO
                           ====================================================== */

                        pthread_mutex_lock(&mxJogoCompeticao);
                        jogoCompeticao = NULL;   // permite nova competição
                        pthread_mutex_unlock(&mxJogoCompeticao);

                        limparResultadosCompeticao();    
                        inicializarEquipas();            
                        inicializarBarreira(GRUPO_COMPETICAO);

                        printf("[RESET] Estado da competição reinicializado.\n");
                        /* ====================================================== */
                    }
                }
                /* === SOLUÇÃO COM ERROS === */
                else {
                    enviarResultadoErros(sock, idJogo, erros);
                }
            }
            continue;
        }

        /* ======================================================
           SAIR — Cliente encerrou
           ====================================================== */
        if (strncmp(buffer, "SAIR", 4) == 0) {
            printf("[DEBUG] Cliente %d pediu SAIR.\n", idCliente);
            return;
        }
    }
}

/* ============================================================
   THREAD PRINCIPAL DE CADA CLIENTE
   ------------------------------------------------------------
   Esta função:
     1) Recebe pedido de jogo (inclui modo e equipa)
     2) Atribui ID interno
     3) Se competição → entra barreira
     4) Obtém jogo (aleatório ou partilhado)
     5) Envia jogo ao cliente
     6) Entra na função loopReceberJogo()
     7) Cleanup final e liberta ID
   ============================================================ */
void *tratarCliente(void *arg)
{
    int sock = *(int*)arg;
    free(arg);

    char idBase[64];
    int modo = 0;
    int equipa = 0;

    printf("[DEBUG] Thread criada (sock=%d)\n", sock);

    /* 1) Receber pedido do cliente */
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

    /* 3) Modo competição → sincronização e registo */
    if (modo == MODO_COMPETICAO) {

        registarClienteLigado(idCliente, equipa, sock);
        registarEntradaJogador(equipa);

        printf("[DEBUG] Cliente %d A ENTRAR NA BARREIRA...\n", idCliente);

        /* Bloqueia até todas as equipas estarem prontas */
        entrarBarreira();

        printf("[DEBUG] Cliente %d SAIU DA BARREIRA!\n", idCliente);
    }

    /* 4) Escolher jogo */
    const Jogo *jogo = NULL;

    if (modo == MODO_COMPETICAO) {

        pthread_mutex_lock(&mxJogoCompeticao);

        /* Primeira thread escolhe o jogo — todas as equipas usam o mesmo */
        if (jogoCompeticao == NULL) {
            jogoCompeticao = obterJogoProximo();
            if (jogoCompeticao) {
                printf("[DEBUG] Jogo competição escolhido: ID=%d\n",
                       jogoCompeticao->id);
            }
        }

        jogo = jogoCompeticao;
        pthread_mutex_unlock(&mxJogoCompeticao);

    } else {
        /* Modo normal usa jogo aleatório individual */
        jogo = obterJogoProximo();
    }

    if (!jogo) {
        enviarErro(sock, "Sem jogos disponíveis");
        close(sock);
        sem_post(&semClientes);
        return NULL;
    }

    /* 5) Enviar tabuleiro inicial */
    if (enviarJogoServidor(sock, jogo->id, jogo->jogo) <= 0) {
        close(sock);
        sem_post(&semClientes);
        return NULL;
    }

    /* 6) Entrar no loop de mensagens */
    loopReceberJogo(sock, modo, equipa, jogo, idCliente);

    /* 7) Cleanup final */
    if (modo == MODO_COMPETICAO)
        removerClienteLigado(idCliente);

    libertarIdCliente(idCliente);
    close(sock);
    sem_post(&semClientes);
    return NULL;
}

/* ============================================================
   ACEITAR CLIENTES — LOOP INFINITO
   ------------------------------------------------------------
   Cada cliente aceite gera uma nova thread tratarCliente().
   Usa semáforo para limitar nº máximo de clientes.
   ============================================================ */
void aceitarClientes(int sockListen)
{
    printf("[DEBUG] A aceitar clientes...\n");

    while (1) {

        /* Bloqueia quando limite de clientes simultâneos é atingido */
        sem_wait(&semClientes);

        /* Aceitar ligação */
        int *pSock = malloc(sizeof(int));
        *pSock = aceitarCliente(sockListen);

        if (*pSock < 0) {
            free(pSock);
            sem_post(&semClientes);
            continue;
        }

        /* Criar thread para este novo cliente */
        pthread_t tid;
        pthread_create(&tid, NULL, tratarCliente, pSock);
        pthread_detach(tid);
    }
}