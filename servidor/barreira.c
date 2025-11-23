// servidor/barreira.c
#include "barreira.h"
#include <pthread.h>
#include <stdio.h>

static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

static int contador = 0;
static int limite = 0;
static int ativa = 0;

/* ============================================================
   Inicializar barreira
   ============================================================ */
void inicializarBarreira(int max)
{
    pthread_mutex_lock(&mx);
    contador = 0;
    limite = max;
    ativa = 1;
    pthread_mutex_unlock(&mx);

    printf("[DEBUG] Barreira inicializada para %d jogadores.\n", max);
}

/* ============================================================
   Entrar na barreira — bloqueia até todos chegarem
   ============================================================ */
void entrarBarreira(void)
{
    pthread_mutex_lock(&mx);

    if (!ativa) {
        pthread_mutex_unlock(&mx);
        return;
    }

    contador++;
    //printf("[DEBUG] Barreira: %d/%d chegaram.\n", contador, limite);

    if (contador >= limite) {
        ativa = 0;  // desbloquear uso futuro
        pthread_cond_broadcast(&cv);  // libertar todos
    } else {
        pthread_cond_wait(&cv, &mx);
    }

    pthread_mutex_unlock(&mx);
}
