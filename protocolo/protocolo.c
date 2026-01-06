// protocolo/protocolo.c
// ======================================================================
// Este módulo implementa todo o protocolo de comunicação entre
// CLIENTE e SERVIDOR.
//
// É responsável por:
//   • Ler mensagens linha-a-linha do socket
//   • Enviar pedidos de jogo (normal e competição)
//   • Enviar/receber ID atribuído
//   • Enviar/receber jogo
//   • Enviar/receber solução
//   • Enviar/receber SET/UPDATE (sync equipas)
//   • Enviar/receber RESULTADO
//   • Enviar/receber SAIR
//   • Enviar/receber ERRO
// ======================================================================

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "protocolo.h"

/* ============================================================
   LEITURA LINHA-A-LINHA (função interna)
   ------------------------------------------------------------
   Lê do socket caracter a caracter até encontrar '\n'
   ou até atingir max-1 caracteres.
   - Retorna nº de bytes lidos
   - Retorna 0 ou <0 em erro ou ligação fechada
   ============================================================ */

static int receberLinha(int sock, char *buffer, int max)
{
    int pos = 0;

    while (pos < max - 1) {
        char c;
        int n = read(sock, &c, 1);       // Lê 1 byte
        if (n <= 0) {
            if (pos == 0) return n;      // ligação fechou logo no início
            break;                       // termina linha lida parcialmente
        }
        buffer[pos++] = c;
        if (c == '\n') break;            // linha completa
    }

    buffer[pos] = '\0';
    return pos;
}

/* ============================================================
   MENSAGEM BASE — enviar string para o socket
   ============================================================ */

int enviarMensagem(int sock, const char *msg)
{
    return write(sock, msg, strlen(msg));
}

/* ============================================================
   PEDIR JOGO (CLIENTE → SERVIDOR)
   ------------------------------------------------------------
   Existem dois tipos de pedido:
     • JOGO_NORMAL <idCliente>
     • JOGO_COMPETICAO <idCliente> <equipa>
   ============================================================ */

int pedirJogoNormal(int sock, const char *id)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "JOGO_NORMAL %s\n", id);
    return enviarMensagem(sock, msg);
}

int pedirJogoCompeticao(int sock, const char *id, int equipa)
{
    char msg[128];
    snprintf(msg, sizeof(msg), "JOGO_COMPETICAO %s %d\n", id, equipa);
    return enviarMensagem(sock, msg);
}

/* ------------------------------------------------------------
   VERSÃO ANTIGA — só devolve o ID
   (mantida por compatibilidade)
   ------------------------------------------------------------ */
int receberPedidoJogoServidor(int sock, char *idOut, int max)
{
    int modoDummy, equipaDummy;
    return receberPedidoJogoServidorModo(sock, idOut, max, &modoDummy, &equipaDummy);
}

/* ------------------------------------------------------------
   VERSÃO COMPLETA:
   lê pedido e devolve:
     modoOut = 1 → normal
                2 → competição
     equipaOut → nº da equipa no modo competição
   ------------------------------------------------------------ */
int receberPedidoJogoServidorModo(int sock,
                                  char *idOut,
                                  int max,
                                  int *modoOut,
                                  int *equipaOut)
{
    char buf[128];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    int equipaTmp;

    /* Jogo normal */
    if (sscanf(buf, "JOGO_NORMAL %s", idOut) == 1) {
        if (modoOut)   *modoOut   = 1;  // modo normal
        if (equipaOut) *equipaOut = 0;
        return 1;
    }

    /* Jogo competição */
    if (sscanf(buf, "JOGO_COMPETICAO %s %d", idOut, &equipaTmp) == 2) {
        if (modoOut)   *modoOut   = 2;
        if (equipaOut) *equipaOut = equipaTmp;
        return 1;
    }

    return -1;
}

/* ============================================================
   ID ATRIBUÍDO
   ------------------------------------------------------------
   Servidor → Cliente:
     ID_ATRIBUIDO <idInterno>
   ============================================================ */

int enviarIdAtribuidoServidor(int sock, int id)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "ID_ATRIBUIDO %d\n", id);
    return enviarMensagem(sock, msg);
}

int receberIdAtribuidoCliente(int sock, int *id)
{
    char buf[128];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    if (sscanf(buf, "ID_ATRIBUIDO %d", id) == 1)
        return 1;
    return -1;
}

/* ============================================================
   JOGO (tabuleiro inicial)
   ------------------------------------------------------------
   Servidor → Cliente:
     JOGO <idJogo> <tabuleiro81chars>
   ============================================================ */

int enviarJogoServidor(int sock, int idJogo, const char *tab)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "JOGO %d %s\n", idJogo, tab);
    return enviarMensagem(sock, msg);
}

int receberJogo(int sock, int *idJogo, char *tab)
{
    char buf[256];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    if (sscanf(buf, "JOGO %d %81s", idJogo, tab) == 2)
        return 1;

    return -1;
}

/* ============================================================
   SOLUÇÃO
   ------------------------------------------------------------
   Cliente → Servidor:
     SOLUCAO <idJogo> <string81>
   ============================================================ */

int enviarSolucao(int sock, int idJogo, const char *sol)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "SOLUCAO %d %s\n", idJogo, sol);
    return enviarMensagem(sock, msg);
}

int receberSolucaoServidor(int sock, int *idJogo, char *sol)
{
    char buf[256];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    if (sscanf(buf, "SOLUCAO %d %81s", idJogo, sol) == 2)
        return 1;

    return -1;
}

/* ============================================================
   SET / UPDATE
   ------------------------------------------------------------
   SET → Cliente envia jogada
   UPDATE → Servidor reenvia jogada para colegas da mesma equipa
   ============================================================ */

int enviarSET(int sock, int lin, int col, int val)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "SET %d %d %d\n", lin, col, val);
    return enviarMensagem(sock, msg);
}

int receberSET(int sock, int *lin, int *col, int *val)
{
    char buf[128];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    if (sscanf(buf, "SET %d %d %d", lin, col, val) == 3)
        return 1;
    return 0;
}

int enviarUPDATE(int sock, int lin, int col, int val)
{
    char msg[64];
    snprintf(msg, sizeof(msg), "UPDATE %d %d %d\n", lin, col, val);
    return enviarMensagem(sock, msg);
}

int receberUPDATE(int sock, int *lin, int *col, int *val)
{
    char buf[128];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    if (sscanf(buf, "UPDATE %d %d %d", lin, col, val) == 3)
        return 1;
    return 0;
}

/* ============================================================
   RESULTADO
   ------------------------------------------------------------
   Formatos:
     RESULTADO <id> OK
     RESULTADO <id> ERROS <num>
   ============================================================ */

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
    char buf[128];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    // Sudoku correto
    if (strstr(buf, "OK")) {
        sscanf(buf, "RESULTADO %d OK", idJogo);
        *erros = 0;
        return 1;
    }

    // Sudoku incorreto
    if (strstr(buf, "ERROS")) {
        sscanf(buf, "RESULTADO %d ERROS %d", idJogo, erros);
        return 1;
    }

    return -1;
}

/* ============================================================
   SAIR
   ============================================================ */

int enviarSair(int sock)
{
    return enviarMensagem(sock, "SAIR\n");
}

int receberSairServidor(int sock)
{
    char buf[64];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    return (strncmp(buf, "SAIR", 4) == 0);
}

/* ============================================================
   ERRO
   ============================================================ */

int enviarErro(int sock, const char *descricao)
{
    char msg[256];
    snprintf(msg, sizeof(msg), "ERRO %s\n", descricao);
    return enviarMensagem(sock, msg);
}

int receberErro(int sock, char *descricao, int max)
{
    char buf[256];
    int n = receberLinha(sock, buf, sizeof(buf));
    if (n <= 0) return n;

    if (strncmp(buf, "ERRO", 4) != 0)
        return -1;

    // Copiar texto de erro, ignorando "ERRO "
    strncpy(descricao, buf + 5, max - 1);
    descricao[max - 1] = '\0';
    return 1;
}
