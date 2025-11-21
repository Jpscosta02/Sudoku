#ifndef CLIENTE_TABULEIRO_H
#define CLIENTE_TABULEIRO_H

/* Inicializa tabuleiro e marca células originais (não editáveis) */
void inicializarTabuleiro(const char *tabStr, int tab[9][9], int original[9][9]);

/* Insere valor; retorna 0 se OK, -1 se célula original ou inválida */
int inserirValor(int tab[9][9], int original[9][9], int lin, int col, int val);

/* Apaga valor; retorna 0 se OK, -1 se célula original */
int apagarValor(int tab[9][9], int original[9][9], int lin, int col);

/* Verificações locais (ignorando zeros) */
int validarLinhas(int tab[9][9]);
int validarColunas(int tab[9][9]);
int validarQuadrados(int tab[9][9]);

/* Converte matriz 9x9 em string de 81 chars ('0'..'9') + '\0' */
void matrizParaString(int tab[9][9], char out[82]);

#endif