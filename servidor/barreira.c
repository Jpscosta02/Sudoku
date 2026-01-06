// servidor/barreira.c
#include "barreira.h"
#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

static int contador = 0;
static int limite = 0;
static int ativa = 0;
static int cancelada = 0; /* fica 1 em caso de timeout */

/* ============================================================
   Inicializar barreira
   ============================================================ */
void inicializarBarreira(int max)
{
    pthread_mutex_lock(&mx);
    contador = 0;
    limite = (max < 1) ? 1 : max;
    ativa = 1;
    cancelada = 0;
    pthread_mutex_unlock(&mx);

    printf("[DEBUG] Barreira inicializada para %d jogadores.\n", limite);
}

/* ============================================================
   Entrar na barreira — bloqueia até todos chegarem
   ============================================================ */
int entrarBarreira(void)
{
    pthread_mutex_lock(&mx);

    if (!ativa || cancelada) {
        pthread_mutex_unlock(&mx);
        return cancelada ? -1 : 0;
    }

    if (limite <= 1) {
        ativa = 0;
        pthread_mutex_unlock(&mx);
        return 0;
    }

    contador++;
    //printf("[DEBUG] Barreira: %d/%d chegaram.\n", contador, limite);

    if (contador >= limite) {
        ativa = 0;  // desbloquear uso futuro
        pthread_cond_broadcast(&cv);  // libertar todos
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 30; /* timeout de 30s */

        while (ativa && !cancelada && contador < limite) {
            int rc = pthread_cond_timedwait(&cv, &mx, &ts);
            if (rc == ETIMEDOUT) {
                printf("[DEBUG] Barreira expirada após 30s. A prosseguir com erro.\n");
                ativa = 0;
                cancelada = 1;
                pthread_cond_broadcast(&cv);
                pthread_mutex_unlock(&mx);
                return -1; /* timeout */
            }
        }
    }

    int r = cancelada ? -1 : 0;
    pthread_mutex_unlock(&mx);
    return r;
}
