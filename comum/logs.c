// comum/logs.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#include "logs.h"

/* Garante que existe a pasta logs/ */
void criarPastaLogs(void)
{
    struct stat st;
    if (stat("logs", &st) == -1) {
        mkdir("logs", 0755);
    }
}

/* Função auxiliar para obter hora AAAA-MM-DD HH:MM:SS */
static void horaAtual(char *out, int max)
{
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    if (!tm_info) {
        snprintf(out, max, "-");
        return;
    }
    strftime(out, max, "%Y-%m-%d %H:%M:%S", tm_info);
}

/* Nova função de logging estruturado:
   ID | Utilizador | Hora | Acontecimento | Descricao */
void logEvento(const char *ficheiro,
               int id,
               const char *utilizador,
               const char *acontecimento,
               const char *descricao)
{
    criarPastaLogs();

    FILE *f = fopen(ficheiro, "a");
    if (!f) return;

    char hora[32];
    horaAtual(hora, sizeof(hora));

    fprintf(f, "%d | %s | %s | %s | %s\n",
            id,
            (utilizador ? utilizador : "-"),
            hora,
            (acontecimento ? acontecimento : "-"),
            (descricao ? descricao : "-"));

    fclose(f);
}

/* Implementações antigas em cima de logEvento,
   para não partir código existente. */

void registarEvento(const char *ficheiro, const char *descricao)
{
    logEvento(ficheiro, 0, "-", "INFO", descricao);
}

void registarEventoID(const char *ficheiro, int id, const char *descricao)
{
    logEvento(ficheiro, id, "-", "INFO", descricao);
}
