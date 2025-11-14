#ifndef JOGOS_H
#define JOGOS_H

typedef struct {
    int id;
    char jogo[82];
    char solucao[82];
} Jogo;

int carregarJogosServidor(const char *ficheiro);
const Jogo *obterJogoProximo(void);

#endif
