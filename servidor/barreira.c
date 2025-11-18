#include <pthread.h>
#include <semaphore.h>

#include "barreira.h"
#include "sincronizacao.h"
#include "ranking.h"
#include "jogos.h"     // <-- Necessário

pthread_mutex_t mutexBarreira = PTHREAD_MUTEX_INITIALIZER;
sem_t semBarreira;
int contadorCompeticao = 0;

/* variável visível em todo o servidor */
const Jogo *jogoCompeticao = NULL;

void inicializarBarreira(void)
{
    sem_init(&semBarreira, 0, 0);
}

void barreiraCompeticao(void)
{
    pthread_mutex_lock(&mutexBarreira);

    contadorCompeticao++;

    if (contadorCompeticao == 1) {
        jogoCompeticao = obterJogoProximo();
        limparResultadosCompeticao();
    }

    if (contadorCompeticao == GRUPO_COMPETICAO) {

        for (int i = 0; i < GRUPO_COMPETICAO - 1; i++)
            sem_post(&semBarreira);

        contadorCompeticao = 0;
        pthread_mutex_unlock(&mutexBarreira);
        return;
    }

    pthread_mutex_unlock(&mutexBarreira);
    sem_wait(&semBarreira);
}
