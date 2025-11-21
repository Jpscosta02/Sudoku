#include "cliente_tabuleiro.h"

void inicializarTabuleiro(const char *tabStr, int tab[9][9], int original[9][9])
{
    for (int i = 0; i < 81; i++) {
        char c = tabStr[i];
        int lin = i / 9;
        int col = i % 9;

        if (c >= '1' && c <= '9') {
            tab[lin][col] = c - '0';
            original[lin][col] = 1;
        } else {
            tab[lin][col] = 0;
            original[lin][col] = 0;
        }
    }
}

int inserirValor(int tab[9][9], int original[9][9], int lin, int col, int val)
{
    if (lin < 0 || lin >= 9 || col < 0 || col >= 9)
        return -1;
    if (val < 1 || val > 9)
        return -1;
    if (original[lin][col])
        return -1;

    tab[lin][col] = val;
    return 0;
}

int apagarValor(int tab[9][9], int original[9][9], int lin, int col)
{
    if (lin < 0 || lin >= 9 || col < 0 || col >= 9)
        return -1;
    if (original[lin][col])
        return -1;

    tab[lin][col] = 0;
    return 0;
}

int validarLinhas(int tab[9][9])
{
    int erros = 0;
    for (int lin = 0; lin < 9; lin++) {
        int visto[10] = {0};
        for (int col = 0; col < 9; col++) {
            int v = tab[lin][col];
            if (v == 0) continue;
            if (v < 1 || v > 9 || visto[v]) {
                erros++;
                break;
            }
            visto[v] = 1;
        }
    }
    return erros;
}

int validarColunas(int tab[9][9])
{
    int erros = 0;
    for (int col = 0; col < 9; col++) {
        int visto[10] = {0};
        for (int lin = 0; lin < 9; lin++) {
            int v = tab[lin][col];
            if (v == 0) continue;
            if (v < 1 || v > 9 || visto[v]) {
                erros++;
                break;
            }
            visto[v] = 1;
        }
    }
    return erros;
}

int validarQuadrados(int tab[9][9])
{
    int erros = 0;
    for (int li = 0; li < 9; li += 3) {
        for (int co = 0; co < 9; co += 3) {
            int visto[10] = {0};
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    int v = tab[li + i][co + j];
                    if (v == 0) continue;
                    if (v < 1 || v > 9 || visto[v]) {
                        erros++;
                        i = j = 3; // sair dos dois ciclos
                        break;
                    }
                    visto[v] = 1;
                }
            }
        }
    }
    return erros;
}

void matrizParaString(int tab[9][9], char out[82])
{
    int k = 0;
    for (int lin = 0; lin < 9; lin++) {
        for (int col = 0; col < 9; col++) {
            int v = tab[lin][col];
            if (v >= 1 && v <= 9)
                out[k++] = '0' + v;
            else
                out[k++] = '0';
        }
    }
    out[81] = '\0';
}
