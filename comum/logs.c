#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include "logs.h"

void criarPastaLogs(void) {
    struct stat st = {0};
    if (stat("logs", &st) == -1)
        mkdir("logs", 0755);
}

/* versão simples */
void registarEvento(const char *ficheiro, const char *descricao) {
    criarPastaLogs();
    FILE *f = fopen(ficheiro, "a");
    if (!f) return;

    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);

    fprintf(f, "[%02d:%02d:%02d] %s\n",
            tmp->tm_hour, tmp->tm_min, tmp->tm_sec, descricao);

    fclose(f);
}

/* versão com ID */
void registarEventoID(const char *ficheiro, int id, const char *descricao) {
    criarPastaLogs();
    FILE *f = fopen(ficheiro, "a");
    if (!f) return;

    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);

    fprintf(f, "%d %02d:%02d:%02d %s\n",
            id,
            tmp->tm_hour, tmp->tm_min, tmp->tm_sec,
            descricao);

    fclose(f);
}
