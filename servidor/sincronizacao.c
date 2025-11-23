#include "sincronizacao.h"
#include <semaphore.h>

int GRUPO_COMPETICAO = 0;
sem_t semClientes;

void inicializarSincronizacao(int maxClientes)
{
    GRUPO_COMPETICAO = maxClientes;
    if (GRUPO_COMPETICAO < 1) GRUPO_COMPETICAO = 1;

    sem_init(&semClientes, 0, maxClientes);
}
