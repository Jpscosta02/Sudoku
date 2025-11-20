// cliente/cliente_menu.c
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include "../comum/logs.h"
#include "../protocolo/protocolo.h"
#include "cliente_menu.h"
#include "cliente_tabuleiro.h"
#include "cliente_ui.h"

/* =======================================================
   LER LINHA
   ======================================================= */
static void lerLinha(char *buf, int max)
{
    if (fgets(buf, max, stdin) == NULL) {
        buf[0] = '\0';
        return;
    }
    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n')
        buf[len - 1] = '\0';
}

/* =======================================================
   LER INTEIRO (com aplicação de updates antes de cada input)
   ======================================================= */
static int lerIntComUpdates(const char *prompt,
                            int min, int max,
                            int *ok,
                            int sock,
                            int modoCompeticao,
                            int tab[9][9],
                            char tabuleiroStr[82]);

/* =======================================================
   buffer persistente para UPDATEs
   ======================================================= */
static char bufUpdate[2048];
static int lenBuf = 0;

/* =======================================================
   aplicar updates NÃO bloqueante
   ======================================================= */
static void aplicarUpdatesPendentes(int sock,
                                    int tab[9][9],
                                    char tabuleiroStr[82])
{
    char tmp[256];

    while (1) {
        ssize_t n = recv(sock, tmp, sizeof(tmp), MSG_DONTWAIT);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            break;
        }
        if (n == 0)
            break;

        if (lenBuf + n >= sizeof(bufUpdate))
            lenBuf = 0;

        memcpy(bufUpdate + lenBuf, tmp, n);
        lenBuf += n;

        int start = 0;
        while (start < lenBuf) {
            int i;
            for (i = start; i < lenBuf && bufUpdate[i] != '\n'; i++);

            if (i == lenBuf)
                break;

            int lineLen = i - start;
            if (lineLen > 0) {
                char line[256];
                int copyLen = (lineLen < 255 ? lineLen : 255);
                memcpy(line, bufUpdate + start, copyLen);
                line[copyLen] = '\0';

                int lin, col, val;
                if (sscanf(line, "UPDATE %d %d %d", &lin, &col, &val) == 3) {
                    if (lin >= 0 && lin < 9 &&
                        col >= 0 && col < 9 &&
                        val >= 0 && val <= 9)
                    {
                        tab[lin][col] = val;
                    }
                }
            }
            start = i + 1;
        }

        if (start > 0 && start < lenBuf) {
            memmove(bufUpdate, bufUpdate + start, lenBuf - start);
            lenBuf -= start;
        } else if (start >= lenBuf)
            lenBuf = 0;
    }

    matrizParaString(tab, tabuleiroStr);
}

/* =======================================================
   LER INTEIRO → versão final
   ======================================================= */
static int lerIntComUpdates(const char *prompt,
                            int min, int max,
                            int *ok,
                            int sock,
                            int modoCompeticao,
                            int tab[9][9],
                            char tabuleiroStr[82])
{
    char linha[64];
    int v;

    while (1) {

        if (modoCompeticao)
            aplicarUpdatesPendentes(sock, tab, tabuleiroStr);

        printf("%s", prompt);
        lerLinha(linha, sizeof(linha));

        if (linha[0] == '\0') { *ok = 0; return 0; }

        if (sscanf(linha, "%d", &v) != 1) {
            printf("Valor inválido.\n");
            continue;
        }

        if (v < min || v > max) {
            printf("Valor fora dos limites (%d-%d)\n", min, max);
            continue;
        }

        *ok = 1;
        return v;
    }
}

/* =======================================================
   MENU SUDOKU — FINAL
   ======================================================= */
int menuSudoku(char solucaoOut[82],
               char tabuleiroStr[82],
               const char *solucaoCorreta,
               const char *ficheiroLog,
               int idAtribuido,
               int sock,
               int modoCompeticao)
{
    int tab[9][9];
    int original[9][9];
    inicializarTabuleiro(tabuleiroStr, tab, original);

    while (1) {

        if (modoCompeticao)
            aplicarUpdatesPendentes(sock, tab, tabuleiroStr);

        mostrarTabuleiroColorido(tab, original);

        printf("===== MENU SUDOKU =====\n");
        printf("1. Inserir valor\n");
        printf("2. Apagar valor\n");
        printf("3. Validar localmente\n");
        printf("4. Enviar solução\n");
        printf("5. Sair sem enviar\n");
        printf("6. Preencher automaticamente (TESTE)\n");
        printf("========================\n");

        int ok;
        int op = lerIntComUpdates("Opção (1-6): ", 1, 6, &ok,
                                  sock, modoCompeticao, tab, tabuleiroStr);
        if (!ok) continue;

        /* =======================================================
           1 — INSERIR
           ======================================================= */
        if (op == 1) {
            int lin = lerIntComUpdates("Linha (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab, tabuleiroStr);
            if (!ok) continue;

            int col = lerIntComUpdates("Coluna (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab, tabuleiroStr);
            if (!ok) continue;

            int val = lerIntComUpdates("Valor (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab, tabuleiroStr);
            if (!ok) continue;

            lin--; col--;

            if (inserirValor(tab, original, lin, col, val) == 0) {
                matrizParaString(tab, tabuleiroStr);
                registarEventoID(ficheiroLog, idAtribuido, "Valor inserido");

                if (modoCompeticao) {
                    enviarSET(sock, lin, col, val);
                    aplicarUpdatesPendentes(sock, tab, tabuleiroStr);
                }

            } else {
                printf("Não podes inserir nessa célula.\n");
            }

            continue;
        }

        /* =======================================================
           2 — APAGAR
           ======================================================= */
        if (op == 2) {
            int lin = lerIntComUpdates("Linha (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab, tabuleiroStr);
            if (!ok) continue;

            int col = lerIntComUpdates("Coluna (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab, tabuleiroStr);
            if (!ok) continue;

            lin--; col--;

            if (apagarValor(tab, original, lin, col) == 0) {
                matrizParaString(tab, tabuleiroStr);
                registarEventoID(ficheiroLog, idAtribuido, "Valor apagado");

                if (modoCompeticao) {
                    enviarSET(sock, lin, col, 0);
                    aplicarUpdatesPendentes(sock, tab, tabuleiroStr);
                }

            } else {
                printf("Não podes apagar essa célula.\n");
            }

            continue;
        }

        /* =======================================================
           3 — VALIDAR LOCAL
           ======================================================= */
        if (op == 3) {
            if (modoCompeticao)
                aplicarUpdatesPendentes(sock, tab, tabuleiroStr);

            int e1 = validarLinhas(tab);
            int e2 = validarColunas(tab);
            int e3 = validarQuadrados(tab);

            if (e1 == 0 && e2 == 0 && e3 == 0)
                printf("Sem erros locais.\n");
            else
                printf("Foram detetados erros locais.\n");

            continue;
        }

        /* =======================================================
           4 — ENVIAR SOLUÇÃO
           ======================================================= */
        if (op == 4) {
            if (modoCompeticao)
                aplicarUpdatesPendentes(sock, tab, tabuleiroStr);

            matrizParaString(tab, solucaoOut);
            matrizParaString(tab, tabuleiroStr);
            registarEventoID(ficheiroLog, idAtribuido, "Solução enviada");
            return 1;
        }

        /* =======================================================
           5 — SAIR
           ======================================================= */
        if (op == 5) {
            registarEventoID(ficheiroLog, idAtribuido, "Sair sem enviar");
            return 0;
        }

        /* =======================================================
           6 — AUTO COMPLETE
           ======================================================= */
        if (op == 6) {
            if (!solucaoCorreta || strlen(solucaoCorreta) < 81) {
                printf("Solução indisponível.\n");
                continue;
            }

            if (modoCompeticao)
                aplicarUpdatesPendentes(sock, tab, tabuleiroStr);

            for (int i = 0; i < 9; i++)
                for (int j = 0; j < 9; j++)
                    tab[i][j] = solucaoCorreta[i*9 + j] - '0';

            matrizParaString(tab, tabuleiroStr);

            if (modoCompeticao) {
                for (int i = 0; i < 9; i++)
                    for (int j = 0; j < 9; j++)
                        enviarSET(sock, i, j, tab[i][j]);

                aplicarUpdatesPendentes(sock, tab, tabuleiroStr);
            }

            continue;
        }

    } // while

    return 0;
}
