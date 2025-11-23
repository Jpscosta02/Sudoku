// servidor/jogos.c

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "jogos.h"

static Jogo listaJogos[MAX_JOGOS];
static int totalJogos = 0;

/* Carrega jogos do ficheiro */
int carregarJogosServidor(const char *ficheiro)
{
    FILE *f = fopen(ficheiro, "r");
    if (!f) {
        perror("Erro ao abrir ficheiro de jogos");
        return 0;
    }

    char linha[256];
    int id;
    char puzzle[82], sol[82];

    totalJogos = 0;

    while (fgets(linha, sizeof(linha), f) && totalJogos < MAX_JOGOS) {

        if (sscanf(linha, "%d,%81[^,],%81s",
                   &id, puzzle, sol) == 3) {

            Jogo j;
            j.id = id;
            strcpy(j.jogo, puzzle);
            strcpy(j.solucao, sol);

            listaJogos[totalJogos++] = j;
        }
    }

    fclose(f);

    printf("Foram carregados %d jogo(s).\n", totalJogos);

    /* iniciar semente aleatória para escolha de jogos */
    srand(time(NULL));

    return (totalJogos > 0);
}

/* Devolve um jogo aleatório */
const Jogo *obterJogoProximo(void)
{
    if (totalJogos == 0)
        return NULL;

    int indice = rand() % totalJogos;

    printf("[DEBUG] A devolver jogo com índice %d (ID=%d)\n",
           indice, listaJogos[indice].id);

    return &listaJogos[indice];
}
