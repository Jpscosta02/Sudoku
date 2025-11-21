// cliente/cliente.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>

#include "../comum/configuracao.h"
#include "../comum/logs.h"
#include "../protocolo/protocolo.h"

#include "cliente_tcp.h"
#include "cliente_menu.h"
#include "cliente_ui.h"

#define FICHEIRO_JOGOS "jogos.txt"

/* ============================================================
   Ler solução correta do ficheiro
   ============================================================ */
static int obterSolucaoPorId(const char *ficheiro, int idJogo, char out[82])
{
    FILE *f = fopen(ficheiro, "r");
    if (!f) return 0;

    char linha[256];
    int id;
    char puzzle[82], sol[82];

    while (fgets(linha, sizeof(linha), f)) {
        if (sscanf(linha, "%d,%81[^,],%81s", &id, puzzle, sol) == 3) {
            if (id == idJogo) {
                strncpy(out, sol, 81);
                out[81] = '\0';
                fclose(f);
                return 1;
            }
        }
    }

    fclose(f);
    return 0;
}

/* ============================================================
   Menu Inicial
   ============================================================ */
static int lerOpcaoMenu(void)
{
    int op;
    char buf[32];

    while (1) {
        printf("\n===== MENU PRINCIPAL =====\n");
        printf("1) Jogar sozinho\n");
        printf("2) Jogar em competição (equipas)\n");
        printf("3) Sair\n");
        printf("Opção: ");

        if (!fgets(buf, sizeof(buf), stdin)) return 3;
        if (sscanf(buf, "%d", &op) == 1 && op >= 1 && op <= 3)
            return op;

        printf("Opção inválida.\n");
    }
}

/* ============================================================
   Receber ranking completo até FIM_COMPETICAO
   (para o jogador que enviou solução)
   ============================================================ */
static void receberRankingCompeticao(int sock)
{
    printf("\n⏳ A aguardar ranking final da competição...\n\n");

    char buf[256];
    static int pos = 1;

    while (1) {
        int n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            printf("Erro ao receber ranking.\n");
            return;
        }

        buf[n] = '\0';
        char *linha = strtok(buf, "\n");

        while (linha) {

            if (strncmp(linha, "RANKING", 7) == 0) {
                int total;
                if (sscanf(linha, "RANKING %d", &total) == 1) {
                    printf("===== 🏆 RANKING FINAL (%d equipas) =====\n", total);
                    pos = 1;
                }
            }
            else if (isdigit((unsigned char)linha[0])) {
                int eq;
                double t;

                if (sscanf(linha, "%d %lf", &eq, &t) == 2) {

                    if (pos == 1)
                        printf("\033[1;32m%2dº ► Equipa %d — %.2fs\033[0m\n", pos, eq, t);
                    else if (pos == 2)
                        printf("\033[1;33m%2dº ► Equipa %d — %.2fs\033[0m\n", pos, eq, t);
                    else if (pos == 3)
                        printf("\033[1;31m%2dº ► Equipa %d — %.2fs\033[0m\n", pos, eq, t);
                    else
                        printf("%2dº ► Equipa %d — %.2fs\n", pos, eq, t);

                    pos++;
                }
            }
            else if (strncmp(linha, "FIM_COMPETICAO", 14) == 0) {
                printf("\n🏁 Competição terminada!\n");
                return;
            }

            linha = strtok(NULL, "\n");
        }
    }
}

/* ============================================================
   MAIN
   ============================================================ */
int main(int argc, char *argv[])
{
    ConfigCliente cfg;
    int sock;
    int idAtribuido;
    int idJogo;
    char tabuleiroStr[82];
    char solucaoCorreta[82];
    char ficheiroLog[128];

    if (argc < 2) {
        printf("Uso: %s <config>\n", argv[0]);
        return 1;
    }

    if (!carregarConfiguracaoCliente(argv[1], &cfg)) {
        printf("Erro ao ler configuração.\n");
        return 1;
    }

    snprintf(ficheiroLog, sizeof(ficheiroLog),
            "logs/cliente_%s.log", cfg.idCliente);
    registarEvento(ficheiroLog, "Cliente iniciado.");

    /* menu inicial */
    int modo = lerOpcaoMenu();
    if (modo == 3) return 0;

    /* ligar ao servidor */
    sock = ligarServidor(cfg.ipServidor, cfg.porta);
    if (sock < 0) {
        printf("Erro ao ligar ao servidor.\n");
        return 1;
    }

    /* enviar pedido de jogo */
    int r;
    if (modo == 1)
        r = pedirJogoNormal(sock, cfg.idCliente);
    else
        r = pedirJogoCompeticao(sock, cfg.idCliente, cfg.equipa);

    if (r <= 0) {
        printf("Falha ao enviar pedido de jogo.\n");
        close(sock);
        return 1;
    }

    /* receber ID interno */
    if (receberIdAtribuidoCliente(sock, &idAtribuido) <= 0) {
        printf("Erro: não foi possível receber ID interno.\n");
        close(sock);
        return 1;
    }

    /* receber tabuleiro */
    if (receberJogo(sock, &idJogo, tabuleiroStr) <= 0) {
        printf("Erro: não foi possível receber tabuleiro.\n");
        close(sock);
        return 1;
    }

    /* ler solução correta */
    if (!obterSolucaoPorId(FICHEIRO_JOGOS, idJogo, solucaoCorreta))
        memset(solucaoCorreta, '0', 82);

    /* LOOP SUDOKU */
    while (1) {
        char solucaoOut[82] = {0};

        int enviar = menuSudoku(solucaoOut,
                                tabuleiroStr,
                                solucaoCorreta,
                                ficheiroLog,
                                idAtribuido,
                                sock,
                                (modo == 2));

        /* ======================================================
           CASO 1 — fim de competição (return -1)
           ====================================================== */
        if (enviar == -1) {
            enviarSair(sock);
            close(sock);
            return 0;
        }

        /* ======================================================
           CASO 2 — sair sem enviar solução (return 0)
           ====================================================== */
        if (enviar == 0) {
            enviarSair(sock);
            close(sock);
            return 0;
        }

        /* ======================================================
           CASO 3 — enviar solução (return 1)
           ====================================================== */

        if (enviarSolucao(sock, idJogo, solucaoOut) <= 0) {
            printf("Erro: falhou enviar solução.\n");
            enviarSair(sock);
            close(sock);
            return 1;
        }

        printf("[DEBUG CLIENTE] Solução enviada. Vou esperar resultado...\n");

        /* esperar resultado */
        int erros;
        if (receberResultado(sock, &idJogo, &erros) <= 0) {
            printf("Erro: falhou receber resultado.\n");
            enviarSair(sock);
            close(sock);
            return 1;
        }

        if (erros == 0) {
            printf("\n✔ Sudoku correto!\n");

            if (modo == 2) {
                receberRankingCompeticao(sock);
            }

            enviarSair(sock);
            close(sock);
            return 0;
        }

        printf("\n✘ Sudoku com %d erro(s). Corrige e tenta de novo.\n", erros);
    }

    return 0;
}
