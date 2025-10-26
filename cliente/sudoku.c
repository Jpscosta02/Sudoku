#include <stdio.h>
#include "sudoku.h"

/**
 * Lê um Sudoku de uma string de 81 dígitos.
 */
void lerSudokuDeString(const char *str, int tab[TAM][TAM]) {
    for (int i = 0; i < TAM * TAM; i++)
        tab[i / TAM][i % TAM] = str[i] - '0';
}

/**
 * Verifica se a string de Sudoku tem exatamente 81 caracteres.
 */
int verificarValidezTamanho(const char* str) {
    int cont = 0;
    for (int i = 0; str[i] != '\0'; ++i)
        cont++;
    return cont == 81;
}

/**
 * Verifica se todos os números do Sudoku são válidos (0–9).
 */
int verificarValoresValidos(int sudoku[TAM][TAM]) {
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (sudoku[i][j] < 0 || sudoku[i][j] > 9) {
                printf(" Valor inválido em (%d,%d): %d\n", i + 1, j + 1, sudoku[i][j]);
                return -1;
            }
        }
    }
    return 0;
}

/**
 * Verifica repetições nas linhas.
 */
int verificarLinhas(int sudoku[TAM][TAM]) {
    int erros = 0;
    for (int i = 0; i < TAM; i++) {
        int vistos[10] = {0};
        for (int j = 0; j < TAM; j++) {
            int num = sudoku[i][j];
            if (num == 0) continue;
            if (vistos[num]) {
                printf(" Repetição do número %d na linha %d\n", num, i + 1);
                erros++;
            }
            vistos[num] = 1;
        }
    }
    return erros;
}

/**
 * Verifica repetições nas colunas.
 */
int verificarColunas(int sudoku[TAM][TAM]) {
    int erros = 0;
    for (int j = 0; j < TAM; j++) {
        int vistos[10] = {0};
        for (int i = 0; i < TAM; i++) {
            int num = sudoku[i][j];
            if (num == 0) continue;
            if (vistos[num]) {
                printf(" Repetição do número %d na coluna %d\n", num, j + 1);
                erros++;
            }
            vistos[num] = 1;
        }
    }
    return erros;
}

/**
 * Verifica repetições nos blocos 3x3.
 */
int verificarBlocos(int sudoku[TAM][TAM]) {
    int erros = 0;
    for (int bi = 0; bi < TAM; bi += 3) {
        for (int bj = 0; bj < TAM; bj += 3) {
            int vistos[10] = {0};
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int num = sudoku[bi + i][bj + j];
                    if (num == 0) continue;
                    if (vistos[num]) {
                        printf(" Repetição do número %d no bloco 3x3 (%d,%d)\n",
                               num, bi / 3 + 1, bj / 3 + 1);
                        erros++;
                    }
                    vistos[num] = 1;
                }
            }
        }
    }
    return erros;
}

/**
 * Compara com a solução.
 */
int compararComSolucao(int sudoku[TAM][TAM], int solucao[TAM][TAM]) {
    int dif = 0;
    for (int i = 0; i < TAM; i++)
        for (int j = 0; j < TAM; j++)
            if (sudoku[i][j] != 0 && sudoku[i][j] != solucao[i][j])
                dif++;
    if (dif > 0)
        printf("  Sudoku difere da solução em %d posições.\n", dif);
    return dif;
}

/**
 * Função principal de verificação — junta todas as verificações anteriores.
 */
int verificarSudoku(int sudoku[TAM][TAM], int solucao[TAM][TAM]) {
    int erros = 0;

    if (verificarValoresValidos(sudoku) == -1)
        return -1;

    erros += verificarLinhas(sudoku);
    erros += verificarColunas(sudoku);
    erros += verificarBlocos(sudoku);

    int dif = compararComSolucao(sudoku, solucao);

    if (erros == 0 && dif == 0)
        printf(" Sudoku válido e correto!\n");
    else if (erros == 0)
        printf(" Sudoku válido mas diferente da solução.\n");
    else
        printf(" Sudoku inválido (%d erros encontrados)\n", erros);

    return erros + dif;
}

/**
 * Mostra o tabuleiro de Sudoku formatado.
 */
void mostrarSudoku(int sudoku[TAM][TAM]) {
    for (int i = 0; i < TAM; i++) {
        if (i % 3 == 0 && i != 0) printf("---------------------\n");
        for (int j = 0; j < TAM; j++) {
            if (j % 3 == 0 && j != 0) printf("| ");
            printf("%d ", sudoku[i][j]);
        }
        printf("\n");
    }
}
