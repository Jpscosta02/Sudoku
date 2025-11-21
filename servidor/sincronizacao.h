#ifndef SINCRONIZACAO_H
#define SINCRONIZACAO_H

#include <semaphore.h>

extern int GRUPO_COMPETICAO;
extern sem_t semClientes;

void inicializarSincronizacao(int maxClientes);

#endif
