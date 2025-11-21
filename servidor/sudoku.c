#include "sudoku.h"

/* ---------- Declarações antecipadas ---------- */
static int ehDigitoValido(char c);

/* ---------- Funções auxiliares internas ---------- */

static int ehDigitoValido(char c) {
    return (c >= '1' && c <= '9');
}

static void stringParaMatriz(const char *str, int mat[9][9]) {
    for (int i = 0; i < 81; i++) {
        char c = str[i];
        if (ehDigitoValido(c)) {
            mat[i / 9][i % 9] = c - '0';
        } else {
            mat[i / 9][i % 9] = 0;
        }
    }
}

static int linhaValida(int mat[9][9], int linha) {
    int visto[10] = {0};
    for (int col = 0; col < 9; col++) {
        int v = mat[linha][col];
        if (v < 1 || v > 9) return 0;
        if (visto[v]) return 0;
        visto[v] = 1;
    }
    return 1;
}

static int colunaValida(int mat[9][9], int col) {
    int visto[10] = {0};
    for (int lin = 0; lin < 9; lin++) {
        int v = mat[lin][col];
        if (v < 1 || v > 9) return 0;
        if (visto[v]) return 0;
        visto[v] = 1;
    }
    return 1;
}

static int quadradoValido(int mat[9][9], int linIni, int colIni) {
    int visto[10] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            int v = mat[linIni + i][colIni + j];
            if (v < 1 || v > 9) return 0;
            if (visto[v]) return 0;
            visto[v] = 1;
        }
    }
    return 1;
}

static int verificarRegrasSudoku(int mat[9][9]) {
    int erros = 0;

    for (int i = 0; i < 9; i++)
        if (!linhaValida(mat, i)) erros++;

    for (int j = 0; j < 9; j++)
        if (!colunaValida(mat, j)) erros++;

    for (int i = 0; i < 9; i += 3)
        for (int j = 0; j < 9; j += 3)
            if (!quadradoValido(mat, i, j)) erros++;

    return erros;
}

/* ---------- Função principal ---------- */

int verificarSudokuStrings(const char *resposta, const char *correta)
{
    if (!resposta || !correta) return -1;

    int erros = 0;

    for (int i = 0; i < 81; i++) {
        char rc = resposta[i];
        char sc = correta[i];

        if (sc == '\0' || rc == '\0') {
            erros++;
            continue;
        }

        if (!ehDigitoValido(rc)) {
            erros++;
            continue;
        }

        if (rc != sc)
            erros++;
    }

    int mat[9][9];
    stringParaMatriz(resposta, mat);

    erros += verificarRegrasSudoku(mat);

    return erros;
}
