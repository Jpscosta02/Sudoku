#include <stdio.h>
#include "cliente_ui.h"

void mostrarTabuleiroColorido(int tab[9][9], int original[9][9])
{
    /* Limpa o ecrã e move o cursor para o topo */
    printf("\033[2J\033[H");

    printf(COR_INFO "======= SUDOKU =======\n" COR_RESET);
    for (int i = 0; i < 9; i++) {
        if (i % 3 == 0)
            printf("+-------+-------+-------+\n");

        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0)
                printf("| ");

            int v = tab[i][j];

            if (v == 0) {
                printf(". ");
            } else {
                if (original[i][j])
                    printf(COR_FIXO "%d " COR_RESET, v);
                else
                    printf(COR_USER "%d " COR_RESET, v);
            }
        }
        printf("|\n");
    }
    printf("+-------+-------+-------+\n\n");
}