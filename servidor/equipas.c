// servidor/equipas.c
#include "equipas.h"
#include <string.h>
#include <pthread.h>
#include <stdio.h>

static EstadoEquipa equipas[MAX_EQUIPAS];
static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

/* ============================================================
   INICIALIZAR
   ============================================================ */
void inicializarEquipas(void)
{
    pthread_mutex_lock(&mx);

    for (int i = 0; i < MAX_EQUIPAS; i++) {
        equipas[i].idEquipa = i + 1;
        equipas[i].jogadoresAtivos = 0;
        equipas[i].sudokuTerminado = 0;
        equipas[i].tInicio = 0;
        equipas[i].tempoFinal = 0.0;

        memset(equipas[i].tabuleiro, '0', 81);
        equipas[i].tabuleiro[81] = '\0';
    }

    pthread_mutex_unlock(&mx);
}

/* ============================================================
   OBTER ESTADO COMPLETO DA EQUIPA
   ============================================================ */
EstadoEquipa *obterEstadoEquipa(int equipa)
{
    if (equipa < 1 || equipa > MAX_EQUIPAS)
        return NULL;

    return &equipas[equipa - 1];
}

/* ============================================================
   ATUALIZAR TABULEIRO OFICIAL DA EQUIPA
   ============================================================ */
void atualizarTabuleiroEquipa(int equipa, const char tab81[82])
{
    if (equipa < 1 || equipa > MAX_EQUIPAS)
        return;

    pthread_mutex_lock(&mx);
    strncpy(equipas[equipa - 1].tabuleiro, tab81, 82);
    pthread_mutex_unlock(&mx);
}

/* ============================================================
   REGISTAR ENTRADA DE JOGADOR
   ============================================================ */
void registarEntradaJogador(int equipa)
{
    if (equipa < 1 || equipa > MAX_EQUIPAS)
        return;

    pthread_mutex_lock(&mx);

    EstadoEquipa *e = &equipas[equipa - 1];
    e->jogadoresAtivos++;

    // Apenas o primeiro jogador arranca o cronómetro
    if (e->jogadoresAtivos == 1) {
        e->tInicio = time(NULL);
    }

    pthread_mutex_unlock(&mx);
}

/* ============================================================
   REGISTAR FIM DO SUDOKU PELA EQUIPA
   ============================================================ */
int registarFimEquipa(int equipa, time_t tFim, double *tempoEquipaOut)
{
    if (equipa < 1 || equipa > MAX_EQUIPAS)
        return 0;

    pthread_mutex_lock(&mx);

    EstadoEquipa *e = &equipas[equipa - 1];
    int marcou = 0;

    if (!e->sudokuTerminado) {
        e->sudokuTerminado = 1;

        if (e->tInicio == 0)
            e->tInicio = tFim;

        e->tempoFinal = difftime(tFim, e->tInicio);
        marcou = 1;
    }

    if (tempoEquipaOut)
        *tempoEquipaOut = e->tempoFinal;

    pthread_mutex_unlock(&mx);
    return marcou;
}

/* ============================================================
   EQUIPA JÁ TERMINOU?
   ============================================================ */
int equipaJaTerminou(int equipa)
{
    if (equipa < 1 || equipa > MAX_EQUIPAS)
        return 0;

    pthread_mutex_lock(&mx);
    int r = equipas[equipa - 1].sudokuTerminado;
    pthread_mutex_unlock(&mx);

    return r;
}

/* ============================================================
   OBTER TEMPO FINAL
   ============================================================ */
double obterTempoEquipa(int equipa)
{
    if (equipa < 1 || equipa > MAX_EQUIPAS)
        return 0;

    pthread_mutex_lock(&mx);
    double t = equipas[equipa - 1].tempoFinal;
    pthread_mutex_unlock(&mx);

    return t;
}
