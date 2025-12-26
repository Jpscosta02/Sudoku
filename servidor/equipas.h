#ifndef EQUIPAS_H
#define EQUIPAS_H

#include <time.h>

#define MAX_EQUIPAS 2

typedef struct {
    int idEquipa;
    int jogadoresAtivos;

    int sudokuTerminado;
    time_t tInicio;
    double tempoFinal;

    char tabuleiro[82];  // tabuleiro OFICIAL da equipa
} EstadoEquipa;

void inicializarEquipas(void);

/* Obter estado completo da equipa */
EstadoEquipa *obterEstadoEquipa(int equipa);

/* Atualiza tabuleiro oficial da equipa */
void atualizarTabuleiroEquipa(int equipa, const char tab81[82]);

/* Marca que um jogador entrou na equipa */
int registarEntradaJogador(int equipa);

/* Marca a conclusão do sudoku pela equipa */
int registarFimEquipa(int equipa, time_t tFim, double *tempoEquipa);

/* Verifica se já terminou */
int equipaJaTerminou(int equipa);

/* Obtém tempo final da equipa */
double obterTempoEquipa(int equipa);

int todasEquipasTerminaram(void);

#endif
