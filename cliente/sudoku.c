#include <stdio.h>
#include "sudoku.h"

/**
 * @brief Lê um tabuleiro de Sudoku a partir de uma string de 81 caracteres.
 *
 * @param str String contendo os números do Sudoku (81 dígitos de '0' a '9')
 * @param tab Matriz 9x9 (TAM x TAM) onde será armazenado o tabuleiro
 */
void lerSudokuDeString(const char *str, int tab[TAM][TAM]) {
    for (int i = 0; i < TAM * TAM; i++)
        tab[i / TAM][i % TAM] = str[i] - '0';
}

/**
 * @brief Verifica se a string de entrada contém exatamente 81 caracteres.
 *
 * @param str String contendo o Sudoku em formato linear
 * @return int 1 se o tamanho for válido, 0 caso contrário
 */
int verificarValidezTamanho(const char* str) {
    int cont = 0;
    for (int i = 0; str[i] != '\0'; ++i)
        cont++;
    return cont == 81;
}

/**
 * @brief Verifica se todos os valores no Sudoku são válidos (entre 0 e 9).
 *
 * @param sudoku Matriz 9x9 a verificar
 * @return int 0 se tudo ok, -1 se houver valores inválidos
 */
int verificarValoresValidos(int sudoku[TAM][TAM]) {
    for (int i = 0; i < TAM; i++) {
        for (int j = 0; j < TAM; j++) {
            if (sudoku[i][j] < 0 || sudoku[i][j] > 9) {
                printf("❌ Valor inválido em (%d,%d): %d\n", i + 1, j + 1, sudoku[i][j]);
                return -1;
            }
        }
    }
    return 0;
}

/**
 * @brief Verifica se existem repetições nas linhas.
 *
 * @param sudoku Matriz 9x9 a verificar
 * @return int Número de repetições encontradas
 */
int verificarLinhas(int sudoku[TAM][TAM]) {
    int erros = 0;
    for (int i = 0; i < TAM; i++) {
        int vistos[10] = {0};
        for (int j = 0; j < TAM; j++) {
            int num = sudoku[i][j];
            if (num == 0) continue;
            if (vistos[num]) {
                printf("❌ Repetição do número %d na linha %d\n", num, i + 1);
                erros++;
            }
            vistos[num] = 1;
        }
    }
    return erros;
}

/**
 * @brief Verifica se existem repetições nas colunas.
 *
 * @param sudoku Matriz 9x9 a verificar
 * @return int Número de repetições encontradas
 */
int verificarColunas(int sudoku[TAM][TAM]) {
    int erros = 0;
    for (int j = 0; j < TAM; j++) {
        int vistos[10] = {0};
        for (int i = 0; i < TAM; i++) {
            int num = sudoku[i][j];
            if (num == 0) continue;
            if (vistos[num]) {
                printf("❌ Repetição do número %d na coluna %d\n", num, j + 1);
                erros++;
            }
            vistos[num] = 1;
        }
    }
    return erros;
}

/**
 * @brief Verifica se existem repetições nos blocos 3x3 do Sudoku.
 *
 * @param sudoku Matriz 9x9 a verificar
 * @return int Número de repetições encontradas
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
                        printf("❌ Repetição do número %d no bloco 3x3 (%d,%d)\n",
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
 * @brief Compara o Sudoku do jogador com a solução correta.
 *
 * @param sudoku Matriz 9x9 preenchida pelo jogador
 * @param solucao Matriz 9x9 com a solução correta
 * @return int Número de diferenças encontradas
 */
int compararComSolucao(int sudoku[TAM][TAM], int solucao[TAM][TAM]) {
    int dif = 0;
    for (int i = 0; i < TAM; i++)
        for (int j = 0; j < TAM; j++)
            if (sudoku[i][j] != 0 && sudoku[i][j] != solucao[i][j])
                dif++;
    if (dif > 0)
        printf("⚠️  Sudoku difere da solução em %d posições.\n", dif);
    return dif;
}

/**
 * @brief Verifica a validade completa do Sudoku:
 * - Verifica valores válidos
 * - Verifica linhas, colunas e blocos
 * - Compara com a solução (se fornecida)
 *
 * @param sudoku Matriz 9x9 a verificar
 * @param solucao Matriz 9x9 com a solução correta
 * @return int 0 se válido e igual à solução,
 *             >0 se válido mas diferente,
 *             -1 se inválido
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
        printf("✅ Sudoku válido e correto!\n");
    else if (erros == 0)
        printf("✅ Sudoku válido mas diferente da solução.\n");
    else
        printf("❌ Sudoku inválido (%d erros encontrados)\n", erros);

    return erros + dif;
}

/**
 * @brief Mostra o tabuleiro de Sudoku formatado no ecrã.
 *
 * @param sudoku Matriz 9x9 a exibir
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
