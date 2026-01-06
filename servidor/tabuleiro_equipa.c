#include "tabuleiro_equipa.h"
#include <string.h>
#include <pthread.h>

static char tab1[DIM+1];
static char tab2[DIM+1];
static pthread_mutex_t m1 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t m2 = PTHREAD_MUTEX_INITIALIZER;

void inicializarTabuleirosEquipas(const char *tabInicial)
{
    pthread_mutex_lock(&m1);
    memcpy(tab1, tabInicial, DIM+1);
    pthread_mutex_unlock(&m1);

    pthread_mutex_lock(&m2);
    memcpy(tab2, tabInicial, DIM+1);
    pthread_mutex_unlock(&m2);
}

void aplicarJogadaEquipa(int idEquipa, int lin, int col, int val)
{
    int idx = lin*9 + col;
    char c = '0' + val;

    if (idEquipa == 1) {
        pthread_mutex_lock(&m1);
        tab1[idx] = c;
        pthread_mutex_unlock(&m1);
    } else {
        pthread_mutex_lock(&m2);
        tab2[idx] = c;
        pthread_mutex_unlock(&m2);
    }
}

const char *obterTabuleiroEquipa(int idEquipa)
{
    if (idEquipa == 1) return tab1;
    return tab2;
}
