#ifndef SUDOKU_H
#define SUDOKU_H

#define TAM 9  // Tamanho do tabuleiro de Sudoku (9x9)

void lerSudokuDeString(const char *str, int tab[TAM][TAM]);
int verificarValidezTamanho(const char *str);
int verificarValoresValidos(int sudoku[TAM][TAM]);
int verificarLinhas(int sudoku[TAM][TAM]);
int verificarColunas(int sudoku[TAM][TAM]);
int verificarBlocos(int sudoku[TAM][TAM]);
int compararComSolucao(int sudoku[TAM][TAM], int solucao[TAM][TAM]);
int verificarSudoku(int sudoku[TAM][TAM], int solucao[TAM][TAM]);
void mostrarSudoku(int sudoku[TAM][TAM]);

#endif
