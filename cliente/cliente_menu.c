// cliente/cliente_menu.c
// =======================================================================
// MÓDULO DO MENU DO CLIENTE
// Gere toda a interação do utilizador com o Sudoku:
//   - Inserir valores
//   - Apagar valores
//   - Validar localmente
//   - Enviar solução
//   - Receber updates em competição (UPDATE, RANKING, FIM_COMPETICAO)
// =======================================================================

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <ctype.h>

#include "../comum/logs.h"
#include "../protocolo/protocolo.h"
#include "cliente_menu.h"
#include "cliente_tabuleiro.h"
#include "cliente_ui.h"

/* =======================================================
   LER LINHA (input seguro do utilizador)
   Remove o '\n' final e evita overflow.
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
   Protótipo de função usada para ler inteiros com suporte
   para tratamento de UPDATE/RANKING/FIM enquanto lê inputs.
   ======================================================= */
static int lerIntComUpdates(const char *prompt,
                            int min, int max,
                            int *ok,
                            int sock,
                            int modoCompeticao,
                            int tab[9][9],
                            char tabuleiroStr[82],
                            int idAtribuido,
                            const char *nomeUtilizador,
                            const char *ficheiroLog);

/* Buffers globais para acumular mensagens UPDATE/RANKING */
static char bufUpdate[2048];
static int  lenBuf = 0;

/* Flags globais */
static int fimCompeticaoFlag = 0;  // indica se servidor enviou FIM_COMPETICAO
static int posRanking = 1;         // posição atual no ranking

/* =======================================================
   aplicarUpdatesPendentes
   -------------------------------------------------------
   Lê do socket em modo não-bloqueante (MSG_DONTWAIT) para:
     • UPDATE lin col val
     • RANKING N
     • linhas de ranking
     • FIM_COMPETICAO
   ======================================================= */
static void aplicarUpdatesPendentes(int sock,
                                    int tab[9][9],
                                    char tabuleiroStr[82],
                                    int idAtribuido,
                                    const char *nomeUtilizador,
                                    const char *ficheiroLog)
{
    char tmp[256];

    while (1) {
        // Receção não bloqueante
        ssize_t n = recv(sock, tmp, sizeof(tmp), MSG_DONTWAIT);

        if (n < 0) {
            // Sem dados disponíveis → sair
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            break;
        }

        if (n == 0)
            break;

        // Acumula no buffer circular
        if (lenBuf + n >= (int)sizeof(bufUpdate))
            lenBuf = 0;

        memcpy(bufUpdate + lenBuf, tmp, n);
        lenBuf += (int)n;

        // Processar linhas completas terminadas em '\n'
        int start = 0;
        while (start < lenBuf) {

            int i;
            for (i = start; i < lenBuf && bufUpdate[i] != '\n'; i++);

            if (i == lenBuf)
                break; // linha incompleta

            int lineLen = i - start;
            if (lineLen > 0) {

                char line[256];
                int copyLen = (lineLen < 255 ? lineLen : 255);
                memcpy(line, bufUpdate + start, copyLen);
                line[copyLen] = '\0';

                /* ---------------------------
                   UPDATE lin col val
                   --------------------------- */
                int lin, col, val;
                if (sscanf(line, "UPDATE %d %d %d", &lin, &col, &val) == 3) {
                    if (lin >= 0 && lin < 9 &&
                        col >= 0 && col < 9 &&
                        val >= 0 && val <= 9)
                    {
                        tab[lin][col] = val;

                        char desc[80];
                        snprintf(desc, sizeof(desc),
                                 "UPDATE recebido: (%d,%d)=%d",
                                 lin, col, val);

                        LOG_CLI(idAtribuido, nomeUtilizador,
                                "UPDATE_REMOTO", desc, ficheiroLog);
                    }
                }

                /* ---------------------------
                   RANKING N
                   --------------------------- */
                else if (strncmp(line, "RANKING", 7) == 0) {
                    int total;
                    if (sscanf(line, "RANKING %d", &total) == 1) {
                        printf("\n===== 🏆 RANKING FINAL (%d equipas) =====\n", total);
                        posRanking = 1;

                        LOG_CLI(idAtribuido, nomeUtilizador,
                                "RANKING_INICIO",
                                "Recebido início de ranking", ficheiroLog);
                    }
                }

                /* ---------------------------
                   linhas do ranking: "equipa tempo"
                   --------------------------- */
                else if (isdigit((unsigned char)line[0])) {
                    int eq;
                    double t;

                    if (sscanf(line, "%d %lf", &eq, &t) == 2) {

                        if (posRanking == 1)
                            printf("\033[1;32m%2dº ► Equipa %d — %.2fs\033[0m\n",
                                   posRanking, eq, t);

                        else if (posRanking == 2)
                            printf("\033[1;33m%2dº ► Equipa %d — %.2fs\033[0m\n",
                                   posRanking, eq, t);

                        else if (posRanking == 3)
                            printf("\033[1;31m%2dº ► Equipa %d — %.2fs\033[0m\n",
                                   posRanking, eq, t);

                        else
                            printf("%2dº ► Equipa %d — %.2fs\n",
                                   posRanking, eq, t);

                        char desc[80];
                        snprintf(desc, sizeof(desc),
                                 "Linha ranking: pos=%d equipa=%d tempo=%.2fs",
                                 posRanking, eq, t);

                        LOG_CLI(idAtribuido, nomeUtilizador,
                                "RANKING_LINHA", desc, ficheiroLog);

                        posRanking++;
                    }
                }

                /* ---------------------------
                   FIM_COMPETICAO
                   --------------------------- */
                else if (strncmp(line, "FIM_COMPETICAO", 14) == 0) {

                    printf("\n🏁 Competição terminada para a tua equipa!\n");
                    fimCompeticaoFlag = 1;

                    LOG_CLI(idAtribuido, nomeUtilizador,
                            "FIM_COMPETICAO",
                            "Servidor indicou fim da competição", ficheiroLog);
                }
            }

            start = i + 1;
        }

        // Reorganizar buffer (remover linhas já processadas)
        if (start > 0 && start < lenBuf) {
            memmove(bufUpdate, bufUpdate + start, lenBuf - start);
            lenBuf -= start;
        } else if (start >= lenBuf) {
            lenBuf = 0;
        }
    }

    // Atualiza string do tabuleiro para sincronizar UI
    matrizParaString(tab, tabuleiroStr);
}

/* =======================================================
   Função auxiliar para ler inteiros durante o jogo,
   tratando também mensagens UPDATE/RANKING/FIM_COMPETICAO.
   ======================================================= */
static int lerIntComUpdates(const char *prompt,
                            int min, int max,
                            int *ok,
                            int sock,
                            int modoCompeticao,
                            int tab[9][9],
                            char tabuleiroStr[82],
                            int idAtribuido,
                            const char *nomeUtilizador,
                            const char *ficheiroLog)
{
    char linha[64];
    int v;

    while (1) {

        // Se em competição, verificar mensagens pendentes
        if (modoCompeticao)
            aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                    idAtribuido, nomeUtilizador, ficheiroLog);

        // Se já terminou, abortar leitura
        if (modoCompeticao && fimCompeticaoFlag) {
            *ok = 0;
            return 0;
        }

        printf("%s", prompt);
        lerLinha(linha, sizeof(linha));

        if (linha[0] == '\0') {
            *ok = 0;
            return 0;
        }

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
   MENU DO SUDOKU
   -------------------------------------------------------
   Mostra tabuleiro e opções:
     1 Inserir valor
     2 Apagar valor
     3 Validar localmente
     4 Enviar solução
     5 Sair sem enviar
     6 Preencher automaticamente (TESTE)
   ======================================================= */
int menuSudoku(char solucaoOut[82],
               char tabuleiroStr[82],
               const char *solucaoCorreta,
               const char *ficheiroLog,
               int idAtribuido,
               const char *nomeUtilizador,
               int sock,
               int modoCompeticao)
{
    int tab[9][9];
    int original[9][9];

    // Inicializar tabuleiro interno
    inicializarTabuleiro(tabuleiroStr, tab, original);

    fimCompeticaoFlag = 0;

    LOG_CLI(idAtribuido, nomeUtilizador,
            "ENTRAR_JOGO",
            "Cliente entrou no menu Sudoku", ficheiroLog);

    while (1) {

        // Atualiza estado em competição
        if (modoCompeticao)
            aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                    idAtribuido, nomeUtilizador, ficheiroLog);

        if (modoCompeticao && fimCompeticaoFlag) {
            printf("\n[INFO] A competição terminou. Prima ENTER para continuar...\n");
            char lixo[8];
            fgets(lixo, sizeof(lixo), stdin);

            solucaoOut[0] = '\0';

            LOG_CLI(idAtribuido, nomeUtilizador,
                    "SAIR_JOGO_FIM_COMP",
                    "Saiu do Sudoku por fim de competição", ficheiroLog);

            return -1;
        }

        // Mostrar tabuleiro
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
                                  sock, modoCompeticao, tab, tabuleiroStr,
                                  idAtribuido, nomeUtilizador, ficheiroLog);

        if (!ok) {
            if (modoCompeticao && fimCompeticaoFlag) {
                solucaoOut[0] = '\0';

                LOG_CLI(idAtribuido, nomeUtilizador,
                        "SAIR_JOGO_FIM_COMP",
                        "Saiu do Sudoku por fim de competição", ficheiroLog);

                return -1;
            }
            continue;
        }

        /* =======================================================
           OPÇÃO 1 — INSERIR VALOR
           ======================================================= */
        if (op == 1) {
            int lin = lerIntComUpdates("Linha (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab,
                                       tabuleiroStr,
                                       idAtribuido, nomeUtilizador,
                                       ficheiroLog);
            if (!ok) continue;

            int col = lerIntComUpdates("Coluna (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab,
                                       tabuleiroStr,
                                       idAtribuido, nomeUtilizador,
                                       ficheiroLog);
            if (!ok) continue;

            int val = lerIntComUpdates("Valor (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao, tab,
                                       tabuleiroStr,
                                       idAtribuido, nomeUtilizador,
                                       ficheiroLog);
            if (!ok) continue;

            lin--; col--;

            if (inserirValor(tab, original, lin, col, val) == 0) {

                matrizParaString(tab, tabuleiroStr);

                char desc[80];
                snprintf(desc, sizeof(desc),
                         "Inseriu %d em (%d,%d)",
                         val, lin + 1, col + 1);

                LOG_CLI(idAtribuido, nomeUtilizador,
                        "INSERIR", desc, ficheiroLog);

                if (modoCompeticao) {
                    enviarSET(sock, lin, col, val);

                    aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                            idAtribuido, nomeUtilizador,
                                            ficheiroLog);
                }

            } else {
                printf("Não podes inserir nessa célula.\n");
            }

            continue;
        }

        /* =======================================================
           OPÇÃO 2 — APAGAR VALOR
           ======================================================= */
        if (op == 2) {
            int lin = lerIntComUpdates("Linha (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao,
                                       tab, tabuleiroStr,
                                       idAtribuido, nomeUtilizador,
                                       ficheiroLog);
            if (!ok) continue;

            int col = lerIntComUpdates("Coluna (1-9): ", 1, 9, &ok,
                                       sock, modoCompeticao,
                                       tab, tabuleiroStr,
                                       idAtribuido, nomeUtilizador,
                                       ficheiroLog);
            if (!ok) continue;

            lin--; col--;

            if (apagarValor(tab, original, lin, col) == 0) {

                matrizParaString(tab, tabuleiroStr);

                char desc[80];
                snprintf(desc, sizeof(desc),
                         "Apagou valor em (%d,%d)",
                         lin + 1, col + 1);

                LOG_CLI(idAtribuido, nomeUtilizador,
                        "APAGAR", desc, ficheiroLog);

                if (modoCompeticao) {
                    enviarSET(sock, lin, col, 0);

                    aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                            idAtribuido, nomeUtilizador,
                                            ficheiroLog);
                }

            } else {
                printf("Não podes apagar essa célula.\n");
            }

            continue;
        }

        /* =======================================================
           OPÇÃO 3 — VALIDAR LOCALMENTE
           ======================================================= */
        if (op == 3) {

            if (modoCompeticao)
                aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                        idAtribuido, nomeUtilizador,
                                        ficheiroLog);

            int e1 = validarLinhas(tab);
            int e2 = validarColunas(tab);
            int e3 = validarQuadrados(tab);

            if (e1 == 0 && e2 == 0 && e3 == 0) {

                printf("Sem erros locais.\n");

                LOG_CLI(idAtribuido, nomeUtilizador,
                        "VALIDAR_LOCAL",
                        "Validação local sem erros", ficheiroLog);

            } else {

                printf("Foram detetados erros locais.\n");

                char desc[80];
                snprintf(desc, sizeof(desc),
                         "Validação local com erros (lin=%d col=%d quad=%d)",
                         e1, e2, e3);

                LOG_CLI(idAtribuido, nomeUtilizador,
                        "VALIDAR_LOCAL", desc, ficheiroLog);
            }

            continue;
        }

        /* =======================================================
           OPÇÃO 4 — ENVIAR SOLUÇÃO
           ======================================================= */
        if (op == 4) {

            if (modoCompeticao) {

                aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                        idAtribuido, nomeUtilizador,
                                        ficheiroLog);

                if (fimCompeticaoFlag) {
                    printf("\n[INFO] A competição já terminou enquanto tentavas enviar a solução.\n");

                    solucaoOut[0] = '\0';

                    LOG_CLI(idAtribuido, nomeUtilizador,
                            "ENVIAR_SOLUCAO_FALHA",
                            "Tentou enviar solução mas competição já tinha terminado",
                            ficheiroLog);

                    return -1;
                }
            }

            matrizParaString(tab, solucaoOut);
            matrizParaString(tab, tabuleiroStr);

            LOG_CLI(idAtribuido, nomeUtilizador,
                    "ENVIAR_SOLUCAO",
                    "Solução enviada para validação", ficheiroLog);

            return 1;
        }

        /* =======================================================
           OPÇÃO 5 — SAIR SEM ENVIAR
           ======================================================= */
        if (op == 5) {
            solucaoOut[0] = '\0';

            LOG_CLI(idAtribuido, nomeUtilizador,
                    "SAIR_SEM_ENVIAR",
                    "Jogador saiu sem enviar solução", ficheiroLog);

            return 0;
        }

        /* =======================================================
           OPÇÃO 6 — AUTO COMPLETE (TESTE)
           ======================================================= */
        if (op == 6) {

            if (!solucaoCorreta || strlen(solucaoCorreta) < 81) {
                printf("Solução indisponível.\n");
                continue;
            }

            if (modoCompeticao)
                aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                        idAtribuido, nomeUtilizador,
                                        ficheiroLog);

            // Copia diretamente a solução correta para o tabuleiro
            for (int i = 0; i < 9; i++)
                for (int j = 0; j < 9; j++)
                    tab[i][j] = solucaoCorreta[i*9 + j] - '0';

            matrizParaString(tab, tabuleiroStr);

            LOG_CLI(idAtribuido, nomeUtilizador,
                    "AUTO_COMPLETE",
                    "Tabuleiro preenchido automaticamente", ficheiroLog);

            if (modoCompeticao) {
                for (int i = 0; i < 9; i++)
                    for (int j = 0; j < 9; j++)
                        enviarSET(sock, i, j, tab[i][j]);

                aplicarUpdatesPendentes(sock, tab, tabuleiroStr,
                                        idAtribuido, nomeUtilizador,
                                        ficheiroLog);
            }

            continue;
        }
    }

    return 0;
}
