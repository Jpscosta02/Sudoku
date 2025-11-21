// servidor/ranking.c
#include <pthread.h>
#include <stdio.h>
#include "ranking.h"
#include "../protocolo/protocolo.h"

typedef struct {
    int   equipa;   /* nº da equipa */
    double tempo;   /* tempo em segundos */
} ResultadoCompeticao;

static ResultadoCompeticao resultados[128];
static int totalResultados = 0;

pthread_mutex_t mutexResultados = PTHREAD_MUTEX_INITIALIZER;

/* ============================================================
   LIMPAR RESULTADOS (INÍCIO DO SERVIDOR / NOVA PROVA)
   ============================================================ */
void limparResultadosCompeticao(void)
{
    pthread_mutex_lock(&mutexResultados);
    totalResultados = 0;
    pthread_mutex_unlock(&mutexResultados);
}

/* ============================================================
   REGISTAR RESULTADO DE UMA EQUIPA
   - equipa: nº da equipa
   - tempo: tempo total em segundos
   Também calcula e mostra no ecrã a posição provisória.
   ============================================================ */
void registarResultadoCompeticao(int equipa, double tempo)
{
    pthread_mutex_lock(&mutexResultados);

    if (totalResultados < 128) {

        /* calcular posição provisória com base nos tempos já registados */
        int pos = 1;
        for (int i = 0; i < totalResultados; i++) {
            if (tempo > resultados[i].tempo) {
                pos++;
            }
        }

        resultados[totalResultados].equipa = equipa;
        resultados[totalResultados].tempo  = tempo;
        totalResultados++;

        printf("[RANKING] Equipa %d terminou em %.2f s (posição provisória: %d)\n",
               equipa, tempo, pos);
    }

    pthread_mutex_unlock(&mutexResultados);
}

/* ============================================================
   ENVIAR RANKING PARA UM CLIENTE
   - Ordena por tempo (melhor tempo primeiro)
   - Envia:
       RANKING N
       <equipa> <tempo>
       ...
   ============================================================ */
void enviarRankingCompeticao(int sock)
{
    pthread_mutex_lock(&mutexResultados);

    /* ordenar por tempo (bubble sort simples, dado N pequeno) */
    for (int i = 0; i < totalResultados - 1; i++) {
        for (int j = i + 1; j < totalResultados; j++) {
            if (resultados[j].tempo < resultados[i].tempo) {
                ResultadoCompeticao tmp = resultados[i];
                resultados[i] = resultados[j];
                resultados[j] = tmp;
            }
        }
    }

    char msg[128];

    snprintf(msg, sizeof(msg), "RANKING %d\n", totalResultados);
    enviarMensagem(sock, msg);

    for (int i = 0; i < totalResultados; i++) {
        snprintf(msg, sizeof(msg), "%d %.2f\n",
                 resultados[i].equipa,
                 resultados[i].tempo);
        enviarMensagem(sock, msg);
    }

    pthread_mutex_unlock(&mutexResultados);
}

void enviarFimCompeticao(int sock)
{
    enviarMensagem(sock, "FIM_COMPETICAO\n");
}
