#ifndef RANKING_H
#define RANKING_H

void limparResultadosCompeticao(void);
void registarResultadoCompeticao(int idCliente, double tempo);
void enviarRankingCompeticao(int sock);
void enviarFimCompeticao(int sock);


#endif
