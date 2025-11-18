#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "protocolo.h"

/* ================= MENSAGENS BASE ================= */

int enviarMensagem(int sock, const char *msg)
{
    return write(sock, msg, strlen(msg));
}

int receberMensagem(int sock, char *buffer, int max)
{
    int n = read(sock, buffer, max - 1);
    if (n <= 0) return n;
    buffer[n] = '\0';
    return n;
}

/* ================= PEDIR_JOGO / MODOS ================= */

/* Versão antiga – mantém compatibilidade, trata como modo normal */
int pedirJogo(int sock, const char *idClienteBase)
{
    return pedirJogoNormal(sock, idClienteBase);
}

/* NOVO: modo normal explícito */
int pedirJogoNormal(int sock, const char *idClienteBase)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "JOGO_NORMAL %s\n", idClienteBase);
    return enviarMensagem(sock, msg);
}

/* NOVO: modo competição explícito */
int pedirJogoCompeticao(int sock, const char *idClienteBase)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "JOGO_COMPETICAO %s\n", idClienteBase);
    return enviarMensagem(sock, msg);
}

/* Versão antiga – mantém, mas sem info de modo */
int receberPedidoJogoServidor(int sock, char *idClienteBase, int max)
{
    int modoLixo;
    return receberPedidoJogoServidorModo(sock, idClienteBase, max, &modoLixo);
}

/* NOVO: versão que deteta o modo (0=normal, 1=competição) */
int receberPedidoJogoServidorModo(int sock, char *idClienteBase, int max, int *modo)
{
    char buffer[128];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    /* Modo novo: JOGO_NORMAL <id> */
    if (sscanf(buffer, "JOGO_NORMAL %s", idClienteBase) == 1) {
        if (modo) *modo = 0;
        return 1;
    }

    /* Modo novo: JOGO_COMPETICAO <id> */
    if (sscanf(buffer, "JOGO_COMPETICAO %s", idClienteBase) == 1) {
        if (modo) *modo = 1;
        return 1;
    }

    /* Compatibilidade: comando antigo PEDIR_JOGO <id> → trata como normal */
    if (sscanf(buffer, "PEDIR_JOGO %s", idClienteBase) == 1) {
        if (modo) *modo = 0;
        return 1;
    }

    return -1;
}

/* ================= ID_ATRIBUIDO ================= */

int enviarIdAtribuidoServidor(int sock, int idNovo)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "ID_ATRIBUIDO %d\n", idNovo);
    return enviarMensagem(sock, msg);
}

int receberIdAtribuidoCliente(int sock, int *idNovo)
{
    char buffer[128];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (sscanf(buffer, "ID_ATRIBUIDO %d", idNovo) == 1)
        return 1;

    return -1;
}

/* ================= JOGO ================= */

int enviarJogoServidor(int sock, int idJogo, const char *tab)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "JOGO %d %s\n", idJogo, tab);
    return enviarMensagem(sock, msg);
}

int receberJogo(int sock, int *idJogo, char *tab)
{
    char buffer[256];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (sscanf(buffer, "JOGO %d %s", idJogo, tab) == 2)
        return 1;

    return -1;
}

/* ================= SOLUCAO ================= */

int enviarSolucao(int sock, int idJogo, const char *sol)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "SOLUCAO %d %s\n", idJogo, sol);
    return enviarMensagem(sock, msg);
}

int receberSolucaoServidor(int sock, int *idJogo, char *sol)
{
    char buffer[256];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (sscanf(buffer, "SOLUCAO %d %s", idJogo, sol) == 2)
        return 1;

    return -1;
}

/* ================= RESULTADO ================= */

int enviarResultadoOK(int sock, int idJogo)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "RESULTADO %d OK\n", idJogo);
    return enviarMensagem(sock, msg);
}

int enviarResultadoErros(int sock, int idJogo, int erros)
{
    char msg[128];
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

    if (strstr(buffer, "ERROS")) {
        sscanf(buffer, "RESULTADO %d ERROS %d", idJogo, erros);
        return 1;
    }

    return -1;
}

/* ================= SAIR ================= */

int enviarSair(int sock)
{
    return enviarMensagem(sock, "SAIR\n");
}

int receberSairServidor(int sock)
{
    char buffer[64];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (strncmp(buffer, "SAIR", 4) == 0)
        return 1;

    return -1;
}

/* ================= ERRO ================= */

int enviarErro(int sock, const char *descricao)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "ERRO %s\n", descricao);
    return enviarMensagem(sock, msg);
}

int receberErro(int sock, char *descricao, int maxLen)
{
    char buffer[256];
    int n = receberMensagem(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (strncmp(buffer, "ERRO", 4) != 0)
        return -1;

    char *p = buffer + 5;
    char *nl = strchr(p, '\n');
    if (nl) *nl = '\0';

    strncpy(descricao, p, maxLen - 1);
    descricao[maxLen - 1] = '\0';

    return 1;
}

/* ================= RANKING (COMPETIÇÃO) ================= */

#include "../comum/util.h"

int receberRankingHeader(int sock, int *nEntradas)
{
    char buffer[128];

    int n = readline(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (sscanf(buffer, "RANKING %d", nEntradas) == 1)
        return 1;

    return -1;
}

int receberRankingLinha(int sock, int *idCliente, double *tempo)
{
    char buffer[128];

    int n = readline(sock, buffer, sizeof(buffer));
    if (n <= 0) return n;

    if (sscanf(buffer, "%d %lf", idCliente, tempo) == 2)
        return 1;

    return -1;
}
