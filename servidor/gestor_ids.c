#include "gestor_ids.h"
#include <pthread.h>

#define MAX_CLIENTES 100

static int idsOcupados[MAX_CLIENTES + 1] = {0};

/* MUTEX para garantir exclusão mútua nos IDs */
static pthread_mutex_t mutexIds = PTHREAD_MUTEX_INITIALIZER;

int atribuirIdCliente(void)
{
    pthread_mutex_lock(&mutexIds);

    for (int i = 1; i <= MAX_CLIENTES; i++) {
        if (!idsOcupados[i]) {
            idsOcupados[i] = 1;
            pthread_mutex_unlock(&mutexIds);
            return i;
        }
    }

    pthread_mutex_unlock(&mutexIds);
    return -1;
}

void libertarIdCliente(int id)
{
    pthread_mutex_lock(&mutexIds);

    if (id >= 1 && id <= MAX_CLIENTES)
        idsOcupados[id] = 0;

    pthread_mutex_unlock(&mutexIds);
}
