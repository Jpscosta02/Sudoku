#include "sincronizacao.h"
#include <semaphore.h>

int GRUPO_COMPETICAO = 0;
sem_t semClientes;

void inicializarSincronizacao(int maxClientes)
{
    /* maxClientes vem do .conf; garantir limites razoáveis. */
    GRUPO_COMPETICAO = maxClientes;
    if (GRUPO_COMPETICAO < 1) GRUPO_COMPETICAO = 1;
    if (GRUPO_COMPETICAO > 100) GRUPO_COMPETICAO = 100;

    /* sem_init recebe unsigned; nunca passar valores <=0. */
    sem_init(&semClientes, 0, (unsigned int)GRUPO_COMPETICAO);
}
