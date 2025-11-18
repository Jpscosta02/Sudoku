#ifndef TRATAR_CLIENTE_H
#define TRATAR_CLIENTE_H

#include "barreira.h"
#include "ranking.h"
#include "validacao_fifo.h"
#include "sincronizacao.h"


void *tratarCliente(void *arg);

/* Funções de ranking definidas em servidor.c */
void registarResultadoCompeticao(int idCliente, double tempo);
void enviarRankingCompeticao(int sock);


#endif
