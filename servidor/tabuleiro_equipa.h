// servidor/tabuleiro_equipa.h
#ifndef TABULEIRO_EQUIAPA_H
#define TABULEIRO_EQUIAPA_H

#define DIM 81  // sudoku 9x9 em string

void inicializarTabuleirosEquipas(const char *tabInicial);
void aplicarJogadaEquipa(int idEquipa, int lin, int col, int val);
const char *obterTabuleiroEquipa(int idEquipa);

#endif
