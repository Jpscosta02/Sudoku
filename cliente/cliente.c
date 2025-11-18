// cliente/cliente.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../comum/configuracao.h"
#include "../comum/logs.h"
#include "../protocolo/protocolo.h"

#include "cliente_tcp.h"
#include "cliente_menu.h"
#include "cliente_ui.h"

#define FICHEIRO_JOGOS "jogos.txt"

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

static int lerOpcaoMenu(void)
{
    int op;
    char buf[32];

    while (1) {
        printf("\n===== MENU PRINCIPAL =====\n");
        printf("1) Jogar sozinho\n");
        printf("2) Jogar em competição\n");
        printf("3) Sair\n");
        printf("Opção: ");

        if (!fgets(buf, sizeof(buf), stdin)) return 3;

        if (sscanf(buf, "%d", &op) == 1 && op >= 1 && op <= 3)
            return op;

        printf("Opção inválida.\n");
    }
}

int main(int argc, char *argv[])
{
    ConfigCliente cfg;
    int sock, idAtribuido, idJogo;
    char tabuleiroStr[82];
    char tabuleiroAtual[82];
    char solucaoCorreta[82];
    char ficheiroLog[128];

    if (argc < 2) {
        printf("Uso: %s <ficheiro-config>\n", argv[0]);
        return 1;
    }

    if (!carregarConfiguracaoCliente(argv[1], &cfg)) {
        printf("Erro na configuração.\n");
        return 1;
    }

    snprintf(ficheiroLog, sizeof(ficheiroLog),
             "logs/cliente_%s.log", cfg.idCliente);

    registarEvento(ficheiroLog, "Cliente iniciado.");

    int modo = lerOpcaoMenu();
    if (modo == 3) return 0;

    sock = ligarServidor(cfg.ipServidor, cfg.porta);

    if (modo == 1)
        pedirJogoNormal(sock, cfg.idCliente);
    else
        pedirJogoCompeticao(sock, cfg.idCliente);

    if (receberIdAtribuidoCliente(sock, &idAtribuido) <= 0) {
        close(sock);
        return 1;
    }

    if (receberJogo(sock, &idJogo, tabuleiroStr) <= 0) {
        close(sock);
        return 1;
    }

    printf("\nFoi recebido o jogo com ID %d.\n", idJogo);

    memcpy(tabuleiroAtual, tabuleiroStr, 82);

    if (!obterSolucaoPorId(FICHEIRO_JOGOS, idJogo, solucaoCorreta)) {
        memset(solucaoCorreta, '0', 81);
        solucaoCorreta[81] = '\0';
    }

    /* =============================
       LOOP DO SUDOKU
       ============================= */
    while (1)
    {
        char solucaoOut[82] = {0};

        int enviar = menuSudoku(solucaoOut,
                                tabuleiroAtual,    // tabuleiro persistente
                                solucaoCorreta,    // para autocomplete
                                ficheiroLog,
                                idAtribuido);

        if (!enviar) {
            enviarSair(sock);
            close(sock);
            return 0;
        }

        if (enviarSolucao(sock, idJogo, solucaoOut) <= 0) {
            printf("Erro ao enviar SOLUCAO.\n");
            enviarSair(sock);
            close(sock);
            return 1;
        }

        int erros;
        if (receberResultado(sock, &idJogo, &erros) <= 0) {
            printf("Erro ao receber RESULTADO.\n");
            enviarSair(sock);
            close(sock);
            return 1;
        }

        if (erros == 0) {
            printf("\n✅ Sudoku correto! Nenhum erro.\n");

            /* Se for competição, tenta receber ranking */
            if (modo == 2) {
                int nEntradas;
                if (receberRankingHeader(sock, &nEntradas) == 1 && nEntradas > 0) {
                    printf("\n===== RESULTADOS DA COMPETIÇÃO =====\n");
                    for (int i = 0; i < nEntradas; i++) {
                        int idC;
                        double tempo;
                        if (receberRankingLinha(sock, &idC, &tempo) > 0) {
                            int minutos = (int)(tempo / 60);
                            double segundos = tempo - minutos * 60;

                            if (minutos > 0)
                                printf("%dº lugar: Jogador %d – %d min %.2f s\n",
                                       i + 1, idC, minutos, segundos);
                            else
                                printf("%dº lugar: Jogador %d – %.2f s\n",
                                       i + 1, idC, segundos);
                        }
                    }
                    printf("====================================\n\n");
                }
            }

            enviarSair(sock);
            close(sock);
            return 0;
        }
        else {
            printf("\n❌ Sudoku com %d erro(s).\n", erros);
            printf("Corrige o tabuleiro e envia novamente.\n\n");
            // tabuleiroAtual já foi atualizado dentro do menu
        }
    }

    return 0;
}
