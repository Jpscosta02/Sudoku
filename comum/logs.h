// comum/logs.h
#ifndef LOGS_H
#define LOGS_H

/* Cria a pasta "logs" se ainda não existir */
void criarPastaLogs(void);

/* Versões antigas, mantidas por compatibilidade.
   Escrevem apenas uma descrição simples no ficheiro indicado. */
void registarEvento(const char *ficheiro, const char *descricao);
void registarEventoID(const char *ficheiro, int id, const char *descricao);

/* Nova função de logging estruturado:
   formato: ID | Utilizador | Hora | Acontecimento | Descricao */
void logEvento(const char *ficheiro,
               int id,
               const char *utilizador,
               const char *acontecimento,
               const char *descricao);

/* Macros de conveniência para servidor e cliente */

#define LOG_SRV(id, utilizador, acao, descricao) \
    logEvento("logs/servidor.log", (id), (utilizador), (acao), (descricao))

#define LOG_CLI(id, utilizador, acao, descricao, ficheiroLog) \
    logEvento((ficheiroLog), (id), (utilizador), (acao), (descricao))

#endif /* LOGS_H */
