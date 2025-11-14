#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jogos.h"

#define MAX_JOGOS 1024

static Jogo jogos[MAX_JOGOS];
static int totalJogos = 0;
static int indiceProximo = 0;

int carregarJogosServidor(const char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        perror("Erro ao abrir ficheiro de jogos");
        return 0;
    }

    char linha[256];
    Jogo j;

    totalJogos = 0;
    indiceProximo = 0;

    while (fgets(linha, sizeof(linha), f) && totalJogos < MAX_JOGOS) {
        char puzzle[82], sol[82];

        if (sscanf(linha, "%d,%81[^,],%81s",
                   &j.id, puzzle, sol) == 3) {

            strcpy(j.jogo, puzzle);
            strcpy(j.solucao, sol);
            jogos[totalJogos++] = j;
        }
    }

    fclose(f);

    printf("Foram carregados %d jogo(s).\n", totalJogos);
    return totalJogos > 0;
}

const Jogo *obterJogoProximo(void)
{
    if (totalJogos == 0) return NULL;

    const Jogo *ptr = &jogos[indiceProximo];
    indiceProximo = (indiceProximo + 1) % totalJogos;
    return ptr;
}
