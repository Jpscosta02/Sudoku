// cliente/cliente_menu.c
#include <stdio.h>
#include <string.h>

#include "../comum/logs.h"
#include "cliente_menu.h"
#include "cliente_tabuleiro.h"
#include "cliente_ui.h"

/* ================================================
   Auxiliares
   ================================================ */

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

static int lerInt(const char *prompt, int min, int max, int *ok)
{
    char linha[128];
    int v;

    while (1) {
        printf("%s", prompt);
        lerLinha(linha, sizeof(linha));

        if (linha[0] == '\0') {
            *ok = 0;
            return 0;
        }

        if (sscanf(linha, "%d", &v) != 1) {
            printf(COR_ERRO "Valor inválido.\n" COR_RESET);
            continue;
        }

        if (v < min || v > max) {
            printf(COR_ERRO "Valor fora dos limites (%d-%d).\n" COR_RESET, min, max);
            continue;
        }

        *ok = 1;
        return v;
    }
}

static void validarLocal(int tab[9][9])
{
    int e1 = validarLinhas(tab);
    int e2 = validarColunas(tab);
    int e3 = validarQuadrados(tab);

    if (e1 == 0 && e2 == 0 && e3 == 0) {
        printf(COR_INFO "Validação local: não foram detetados conflitos.\n" COR_RESET);
    } else {
        printf(COR_ERRO "Conflitos detetados no Sudoku.\n" COR_RESET);
    }
}

/* =================================================
   MENU SUDOKU — função principal do cliente
   ================================================= */

int menuSudoku(char solucaoOut[82],
               char tabuleiroStr[82],        // tabuleiro persistente
               const char *solucaoCorreta,   // para autocomplete (opção 6)
               const char *ficheiroLog,
               int idAtribuido)
{
    int tab[9][9];
    int original[9][9];

    // Construir matriz a partir da string atual
    inicializarTabuleiro(tabuleiroStr, tab, original);

    while (1) {

        mostrarTabuleiroColorido(tab, original);

        printf(COR_MENU "===== MENU SUDOKU =====\n" COR_RESET);
        printf(COR_MENU "1. Inserir valor\n" COR_RESET);
        printf(COR_MENU "2. Apagar valor\n" COR_RESET);
        printf(COR_MENU "3. Validar localmente\n" COR_RESET);
        printf(COR_MENU "4. Enviar solução\n" COR_RESET);
        printf(COR_MENU "5. Sair sem enviar\n" COR_RESET);
        printf(COR_MENU "6. Preencher solução automaticamente (TESTE)\n" COR_RESET);
        printf(COR_MENU "=======================\n" COR_RESET);

        int ok;
        int op = lerInt("Escolha uma opção (1-6): ", 1, 6, &ok);
        if (!ok) {
            printf(COR_ERRO "Entrada inválida.\n" COR_RESET);
            continue;
        }

        switch (op) {

        /* ===========================
           1. Inserir valor
           =========================== */
        case 1: {
            int lin = lerInt("Linha (1-9): ", 1, 9, &ok); if (!ok) break;
            int col = lerInt("Coluna (1-9): ", 1, 9, &ok); if (!ok) break;
            int val = lerInt("Valor (1-9): ", 1, 9, &ok); if (!ok) break;

            lin--; col--;

            if (inserirValor(tab, original, lin, col, val) == 0) {
                matrizParaString(tab, tabuleiroStr); // atualizar persistente
                printf(COR_INFO "Valor inserido com sucesso.\n" COR_RESET);
                registarEventoID(ficheiroLog, idAtribuido, "Valor inserido");
            } else {
                printf(COR_ERRO "Não podes inserir nesta célula.\n" COR_RESET);
                registarEventoID(ficheiroLog, idAtribuido, "Falha inserir");
            }
            break;
        }

        /* ===========================
           2. Apagar valor
           =========================== */
        case 2: {
            int lin = lerInt("Linha (1-9): ", 1, 9, &ok); if (!ok) break;
            int col = lerInt("Coluna (1-9): ", 1, 9, &ok); if (!ok) break;

            lin--; col--;

            if (apagarValor(tab, original, lin, col) == 0) {
                matrizParaString(tab, tabuleiroStr); // atualizar persistente
                printf(COR_INFO "Célula apagada.\n" COR_RESET);
                registarEventoID(ficheiroLog, idAtribuido, "Valor apagado");
            } else {
                printf(COR_ERRO "Não podes apagar esta célula.\n" COR_RESET);
                registarEventoID(ficheiroLog, idAtribuido, "Falha apagar");
            }
            break;
        }

        /* ===========================
           3. Validação local
           =========================== */
        case 3:
            validarLocal(tab);
            registarEventoID(ficheiroLog, idAtribuido, "Validação local");
            printf("Pressiona ENTER para continuar...");
            {
                char tmp[16];
                lerLinha(tmp, sizeof(tmp));
            }
            break;

        /* ===========================
           4. Enviar solução
           =========================== */
        case 4: {

            matrizParaString(tab, solucaoOut);   // preparar solução final
            matrizParaString(tab, tabuleiroStr); // atualizar persistente

            int vazias = 0;
            for (int i = 0; i < 81; i++)
                if (solucaoOut[i] == '0')
                    vazias++;

            if (vazias > 0) {
                printf(COR_ERRO "Existem %d células vazias.\n" COR_RESET, vazias);
                printf("Enviar mesmo assim? (s/n): ");

                char resp[16];
                lerLinha(resp, sizeof(resp));
                if (resp[0] != 's' && resp[0] != 'S')
                    break;
            }

            registarEventoID(ficheiroLog, idAtribuido, "Solução enviada");
            return 1; // enviar ao servidor
        }

        /* ===========================
           5. Sair sem enviar
           =========================== */
        case 5:
            registarEventoID(ficheiroLog, idAtribuido, "Sair sem enviar");
            return 0;

        /* ===========================
           6. Preencher solução automaticamente (TESTE)
           =========================== */
        case 6: {
            if (!solucaoCorreta || strlen(solucaoCorreta) < 81) {
                printf(COR_ERRO "Solução correta indisponível para autocomplete.\n" COR_RESET);
                break;
            }

            // Preencher tabuleiro com solucaoCorreta (81 chars)
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    char c = solucaoCorreta[i * 9 + j];
                    if (c >= '1' && c <= '9')
                        tab[i][j] = c - '0';
                    else
                        tab[i][j] = 0;
                }
            }

            matrizParaString(tab, tabuleiroStr); // atualizar persistente
            printf(COR_INFO "Tabuleiro preenchido automaticamente com a solução (TESTE).\n" COR_RESET);
            registarEventoID(ficheiroLog, idAtribuido, "Autocomplete com solução");
            break;
        }

        } // switch
    } // while

    return 0;
}
