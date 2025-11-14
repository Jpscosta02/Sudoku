#include "protocolo.h"
#include "../comum/util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ============================================================
   Funções base: enviar / receber mensagens
   ============================================================ */

int enviarMensagem(int sock, const char *msg)
{
    int len = strlen(msg);
    int n = writen(sock, (char *)msg, len);
    return (n == len) ? n : -1;
}

int receberMensagem(int sock, char *buffer, int max)
{
    int n = readline(sock, buffer, max);
    if (n <= 0) return n;
    buffer[n] = '\0';
    return n;
}

/* ============================================================
   PEDIR_JOGO  (Cliente -> Servidor)
   ============================================================ */

int pedirJogo(int sock, const char *idClienteBase)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "PEDIR_JOGO %s\n", idClienteBase);
    return enviarMensagem(sock, msg);
}

int receberPedidoJogoServidor(int sock, char *idClienteBase, int maxLen)
{
    char buffer[256];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (strncmp(buffer, "PEDIR_JOGO", 10) != 0)
        return -1;

    sscanf(buffer, "PEDIR_JOGO %s", idClienteBase);
    idClienteBase[maxLen - 1] = '\0';
    return 1;
}

/* ============================================================
   ID ATRIBUIDO  (Servidor -> Cliente)
   ============================================================ */

int enviarIdAtribuidoServidor(int sock, int idNovo)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "ID_ATRIBUIDO %d\n", idNovo);
    return enviarMensagem(sock, msg);
}

int receberIdAtribuidoCliente(int sock, int *idNovo)
{
    char buffer[128];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    sscanf(buffer, "ID_ATRIBUIDO %d", idNovo);
    return 1;
}

/* ============================================================
   JOGO  (Servidor -> Cliente)
   ============================================================ */

int enviarJogoServidor(int sock, int idJogo, const char *tabuleiro81)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "JOGO %d %s\n", idJogo, tabuleiro81);
    return enviarMensagem(sock, msg);
}

int receberJogo(int sock, int *idJogo, char *tabuleiro81)
{
    char buffer[256];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    sscanf(buffer, "JOGO %d %81s", idJogo, tabuleiro81);
    return 1;
}

/* ============================================================
   SOLUCAO  (Cliente -> Servidor)
   ============================================================ */

int enviarSolucao(int sock, int idJogo, const char *sol81)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "SOLUCAO %d %s\n", idJogo, sol81);
    return enviarMensagem(sock, msg);
}

int receberSolucaoServidor(int sock, int *idJogo, char *sol81)
{
    char buffer[256];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    sscanf(buffer, "SOLUCAO %d %81s", idJogo, sol81);
    return 1;
}

/* ============================================================
   RESULTADO (Servidor -> Cliente)
   ============================================================ */

int enviarResultadoOK(int sock, int idJogo)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "RESULTADO %d OK\n", idJogo);
    return enviarMensagem(sock, msg);
}

int enviarResultadoErros(int sock, int idJogo, int erros)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "RESULTADO %d ERROS %d\n", idJogo, erros);
    return enviarMensagem(sock, msg);
}

int receberResultado(int sock, int *idJogo, int *erros)
{
    char buffer[128];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (strstr(buffer, "OK")) {
        sscanf(buffer, "RESULTADO %d OK", idJogo);
        *erros = 0;
        return 1;
    }

    sscanf(buffer, "RESULTADO %d ERROS %d", idJogo, erros);
    return 1;
}
