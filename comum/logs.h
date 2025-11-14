#ifndef LOGS_H
#define LOGS_H

void criarPastaLogs(void);
void registarEvento(const char *ficheiro, const char *descricao);
void registarEventoID(const char *ficheiro, int id, const char *descricao);

#endif
