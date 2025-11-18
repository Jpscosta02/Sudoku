#include <pthread.h>
#include <stdio.h>
#include "ranking.h"
#include "../protocolo/protocolo.h"

typedef struct {
    int idCliente;
    double tempo;
} ResultadoCompeticao;

static ResultadoCompeticao resultados[128];
static int totalResultados = 0;

pthread_mutex_t mutexResultados = PTHREAD_MUTEX_INITIALIZER;

void limparResultadosCompeticao(void)
{
    pthread_mutex_lock(&mutexResultados);
    totalResultados = 0;
    pthread_mutex_unlock(&mutexResultados);
}

void registarResultadoCompeticao(int idCliente, double tempo)
{
    pthread_mutex_lock(&mutexResultados);

    if (totalResultados < 128) {
        resultados[totalResultados].idCliente = idCliente;
        resultados[totalResultados].tempo = tempo;
        totalResultados++;
    }
    pthread_mutex_unlock(&mutexResultados);
}

void enviarRankingCompeticao(int sock)
{
    pthread_mutex_lock(&mutexResultados);

    // ordenar
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
                 resultados[i].idCliente,
                 resultados[i].tempo);
        enviarMensagem(sock, msg);
    }

    pthread_mutex_unlock(&mutexResultados);
}
