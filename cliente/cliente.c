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
static void receberRankingCompeticao(int sock,
                                     int idAtribuido,
                                     const char *nomeUtilizador,
                                     const char *ficheiroLog)
{
    printf("\n⏳ A aguardar ranking final da competição...\n\n");
    LOG_CLI(idAtribuido, nomeUtilizador,
            "RANKING_ESPERA", "A aguardar ranking final da competição", ficheiroLog);

    char buf[256];
    static int pos = 1;

    while (1) {
        int n = recv(sock, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            printf("Erro ao receber ranking.\n");
            LOG_CLI(idAtribuido, nomeUtilizador,
                    "RANKING_ERRO", "Erro ao receber ranking do servidor", ficheiroLog);
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

                    char desc[80];
                    snprintf(desc, sizeof(desc),
                             "Início de ranking final (%d equipas)", total);
                    LOG_CLI(idAtribuido, nomeUtilizador,
                            "RANKING_INICIO", desc, ficheiroLog);
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

                    char desc[80];
                    snprintf(desc, sizeof(desc),
                             "Ranking: pos=%d equipa=%d tempo=%.2fs",
                             pos, eq, t);
                    LOG_CLI(idAtribuido, nomeUtilizador,
                            "RANKING_LINHA", desc, ficheiroLog);

                    pos++;
                }
            }
            else if (strncmp(linha, "FIM_COMPETICAO", 14) == 0) {
                printf("\n🏁 Competição terminada!\n");
                LOG_CLI(idAtribuido, nomeUtilizador,
                        "FIM_COMPETICAO", "Fim de competição recebido no cliente", ficheiroLog);
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

    /* log inicial */
    LOG_CLI(0, cfg.idCliente,
            "INICIO_CLIENTE", "Cliente iniciado", ficheiroLog);

    /* menu inicial */
    int modo = lerOpcaoMenu();
    if (modo == 3) {
        LOG_CLI(0, cfg.idCliente,
                "SAIR_MENU", "Utilizador saiu no menu principal", ficheiroLog);
        return 0;
    }

    /* ligar ao servidor */
    sock = ligarServidor(cfg.ipServidor, cfg.porta);
    if (sock < 0) {
        printf("Erro ao ligar ao servidor.\n");
        LOG_CLI(0, cfg.idCliente,
                "ERRO_LIGACAO", "Falha ao ligar ao servidor", ficheiroLog);
        return 1;
    }
    LOG_CLI(0, cfg.idCliente,
            "LIGACAO_OK", "Ligado ao servidor com sucesso", ficheiroLog);

    /* enviar pedido de jogo */
    int r;
    if (modo == 1) {
        LOG_CLI(0, cfg.idCliente,
                "PEDIR_JOGO_SOLO", "Pediu jogo em modo individual", ficheiroLog);
        r = pedirJogoNormal(sock, cfg.idCliente);
    } else {
        LOG_CLI(0, cfg.idCliente,
                "PEDIR_JOGO_COMP", "Pediu jogo em modo competição", ficheiroLog);
        r = pedirJogoCompeticao(sock, cfg.idCliente, cfg.equipa);
    }

    if (r <= 0) {
        printf("Falha ao enviar pedido de jogo.\n");
        LOG_CLI(0, cfg.idCliente,
                "ERRO_PEDIR_JOGO", "Falha ao enviar pedido de jogo ao servidor", ficheiroLog);
        close(sock);
        return 1;
    }

    /* receber ID interno */
    if (receberIdAtribuidoCliente(sock, &idAtribuido) <= 0) {
        printf("Erro: não foi possível receber ID interno.\n");
        LOG_CLI(0, cfg.idCliente,
                "ERRO_ID_ATRIBUIDO", "Falha ao receber ID atribuído pelo servidor", ficheiroLog);
        close(sock);
        return 1;
    }

    LOG_CLI(idAtribuido, cfg.idCliente,
            "ID_ATRIBUIDO", "ID interno atribuído pelo servidor", ficheiroLog);

    /* receber tabuleiro */
    if (receberJogo(sock, &idJogo, tabuleiroStr) <= 0) {
        printf("Erro: não foi possível receber tabuleiro.\n");
        LOG_CLI(idAtribuido, cfg.idCliente,
                "ERRO_RECEBER_JOGO", "Falha ao receber tabuleiro de jogo", ficheiroLog);
        close(sock);
        return 1;
    }

    {
        char desc[80];
        snprintf(desc, sizeof(desc), "Recebido jogo com ID=%d", idJogo);
        LOG_CLI(idAtribuido, cfg.idCliente,
                "RECEBER_JOGO", desc, ficheiroLog);
    }

    /* ler solução correta */
    if (!obterSolucaoPorId(FICHEIRO_JOGOS, idJogo, solucaoCorreta)) {
        memset(solucaoCorreta, '0', 82);
        LOG_CLI(idAtribuido, cfg.idCliente,
                "ERRO_SOLUCAO_CORRETA", "Não foi possível obter solução correta do ficheiro", ficheiroLog);
    }

    /* LOOP SUDOKU */
    while (1) {
        char solucaoOut[82] = {0};

        int enviar = menuSudoku(solucaoOut,
                                tabuleiroStr,
                                solucaoCorreta,
                                ficheiroLog,
                                idAtribuido,
                                cfg.idCliente,
                                sock,
                                (modo == 2));

        /* CASO 1 — fim de competição (return -1) */
        if (enviar == -1) {
            LOG_CLI(idAtribuido, cfg.idCliente,
                    "SAIR_FIM_COMP", "Cliente a sair após fim de competição", ficheiroLog);
            enviarSair(sock);
            close(sock);
            return 0;
        }

        /* CASO 2 — sair sem enviar solução (return 0) */
        if (enviar == 0) {
            LOG_CLI(idAtribuido, cfg.idCliente,
                    "SAIR_SEM_ENVIAR", "Cliente saiu do jogo sem enviar solução", ficheiroLog);
            enviarSair(sock);
            close(sock);
            return 0;
        }

        /* CASO 3 — enviar solução (return 1) */

        if (enviarSolucao(sock, idJogo, solucaoOut) <= 0) {
            printf("Erro: falhou enviar solução.\n");
            LOG_CLI(idAtribuido, cfg.idCliente,
                    "ERRO_ENVIAR_SOL", "Falha ao enviar solução para o servidor", ficheiroLog);
            enviarSair(sock);
            close(sock);
            return 1;
        }

        printf("[DEBUG CLIENTE] Solução enviada. Vou esperar resultado...\n");
        LOG_CLI(idAtribuido, cfg.idCliente,
                "SOLUCAO_ENVIADA", "Solução enviada, a aguardar resultado", ficheiroLog);

        /* esperar resultado */
        int erros;
        if (receberResultado(sock, &idJogo, &erros) <= 0) {
            printf("Erro: falhou receber resultado.\n");
            LOG_CLI(idAtribuido, cfg.idCliente,
                    "ERRO_RECEBER_RESULT", "Falha ao receber resultado do servidor", ficheiroLog);
            enviarSair(sock);
            close(sock);
            return 1;
        }

        if (erros == 0) {
            printf("\n✔ Sudoku correto!\n");
            LOG_CLI(idAtribuido, cfg.idCliente,
                    "RESULTADO_OK", "Sudoku correto (0 erros)", ficheiroLog);

            if (modo == 2) {
                receberRankingCompeticao(sock, idAtribuido, cfg.idCliente, ficheiroLog);
            }

            enviarSair(sock);
            close(sock);
            return 0;
        }

        printf("\n✘ Sudoku com %d erro(s). Corrige e tenta de novo.\n", erros);

        {
            char desc[80];
            snprintf(desc, sizeof(desc),
                     "Sudoku com %d erro(s) devolvido pelo servidor", erros);
            LOG_CLI(idAtribuido, cfg.idCliente,
                    "RESULTADO_KO", desc, ficheiroLog);
        }
    }

    return 0;
}
