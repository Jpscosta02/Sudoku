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

    printf("[DEBUG CLIENTE] Início do programa.\n");

    if (argc < 2) {
        printf("Uso: %s <config>\n", argv[0]);
        return 1;
    }

    printf("[DEBUG CLIENTE] A carregar configuração...\n");

    if (!carregarConfiguracaoCliente(argv[1], &cfg)) {
        printf("Erro ao ler configuração.\n");
        return 1;
    }

    printf("[DEBUG CLIENTE] Config carregada. IP=%s PORTA=%d ID=%s EQUIPA=%d\n",
           cfg.ipServidor, cfg.porta, cfg.idCliente, cfg.equipa);

    snprintf(ficheiroLog, sizeof(ficheiroLog),
            "logs/cliente_%s.log", cfg.idCliente);
    registarEvento(ficheiroLog, "Cliente iniciado.");

    /* menu inicial */
    int modo = lerOpcaoMenu();
    printf("[DEBUG CLIENTE] Modo escolhido = %d\n", modo);

    if (modo == 3) return 0;

    /* ligar ao servidor */
    printf("[DEBUG CLIENTE] A ligar ao servidor...\n");
    sock = ligarServidor(cfg.ipServidor, cfg.porta);

    if (sock < 0) {
        printf("Erro ao ligar ao servidor.\n");
        return 1;
    }

    printf("[DEBUG CLIENTE] Ligação estabelecida (socket=%d).\n", sock);

    /* ============================================================
       ENVIAR PEDIDO DE JOGO
       ============================================================ */

    printf("[DEBUG CLIENTE] Vou enviar pedido de jogo...\n");

    int r;
    if (modo == 1)
        r = pedirJogoNormal(sock, cfg.idCliente);
    else
        r = pedirJogoCompeticao(sock, cfg.idCliente, cfg.equipa);

    printf("[DEBUG CLIENTE] Resultado pedirJogo = %d\n", r);

    if (r <= 0) {
        printf("[DEBUG CLIENTE] Falha ao enviar pedido de jogo.\n");
        close(sock);
        return 1;
    }

    printf("[DEBUG CLIENTE] Pedido enviado. Vou receber ID...\n");

    /* receber ID interno */
    int res = receberIdAtribuidoCliente(sock, &idAtribuido);

    printf("[DEBUG CLIENTE] receberIdAtribuidoCliente retornou %d\n", res);

    if (res <= 0) {
        printf("Erro: não foi possível receber ID interno.\n");
        close(sock);
        return 1;
    }

    printf("[DEBUG CLIENTE] ID interno recebido = %d\n", idAtribuido);

    /* receber tabuleiro */
    printf("[DEBUG CLIENTE] Vou receber JOGO...\n");

    if (receberJogo(sock, &idJogo, tabuleiroStr) <= 0) {
        printf("Erro: não foi possível receber tabuleiro.\n");
        close(sock);
        return 1;
    }

    printf("[DEBUG CLIENTE] Recebi jogo ID=%d\n", idJogo);

    /* ler solução correta */
    if (!obterSolucaoPorId(FICHEIRO_JOGOS, idJogo, solucaoCorreta))
        memset(solucaoCorreta, '0', 82);

    /* LOOP SUDOKU */
    while (1) {
        char solucaoOut[82] = {0};

        printf("[DEBUG CLIENTE] A entrar no menu Sudoku...\n");

        int enviar = menuSudoku(solucaoOut,
                                tabuleiroStr,
                                solucaoCorreta,
                                ficheiroLog,
                                idAtribuido,
                                sock,
                                (modo == 2));

        printf("[DEBUG CLIENTE] retorno menuSudoku = %d\n", enviar);

        if (!enviar) {
            printf("[DEBUG CLIENTE] ENVIAR=0 → Vou enviar SAIR\n");
            enviarSair(sock);
            close(sock);
            return 0;
        }

        /* enviar solução */
        printf("[DEBUG CLIENTE] A enviar solução...\n");

        if (enviarSolucao(sock, idJogo, solucaoOut) <= 0) {
            printf("Erro: falhou enviar solução.\n");
            enviarSair(sock);
            close(sock);
            return 1;
        }

        printf("[DEBUG CLIENTE] Solução enviada. Vou esperar resultado...\n");

        int erros;
        if (receberResultado(sock, &idJogo, &erros) <= 0) {
            printf("Erro: falhou receber resultado.\n");
            enviarSair(sock);
            close(sock);
            return 1;
        }

        printf("[DEBUG CLIENTE] Resultado recebido. Erros=%d\n", erros);

        if (erros == 0) {
            printf("\n✔ Sudoku correto!\n");
            if (modo == 2)
                printf("(Competição por equipas)\n");

            enviarSair(sock);
            close(sock);
            return 0;
        }

        printf("\n✘ Sudoku com %d erro(s). Corrige e tenta de novo.\n", erros);
    }

    return 0;
}
