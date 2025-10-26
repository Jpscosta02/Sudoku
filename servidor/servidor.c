#include <stdio.h>
#include "configuracao.h"
#include "logs.h"
#include "sudoku.h"
// Se estas a ler isto, eu consegui. Palavra-passe: semilhas.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <ficheiro_config>\n", argv[0]);
        return 1;
    }

    // --- Ler configuração do servidor ---
    ConfigServidor cfg;
    if (!carregarConfiguracaoServidor(argv[1], &cfg)) {
        return 1;
    }

    // --- Registar evento inicial ---
    registarEvento("logs/servidor.log", "Servidor iniciado");
    printf("Servidor configurado na porta %d\n", cfg.porta);

    // --- TESTE DE VERIFICAÇÃO DE SUDOKU ---
    int jogo[TAM][TAM];
    int solucao[TAM][TAM];


    // Sudoku do enunciado
    const char *jogoStr =
        "530070000600195000098000060800060003400803001700020006060000280000419005000080079";
    const char *solucaoStr =
        "534678912672195348198342567859761423426853791713924856961537284287419635345286179";

    // Converter as strings para matrizes 9x9
    lerSudokuDeString(jogoStr, jogo);
    lerSudokuDeString(solucaoStr, solucao);

    printf("\nTabuleiro de Sudoku:\n");
    mostrarSudoku(jogo);

    printf("\nSolucao:\n");
    mostrarSudoku(solucao);

    
    // Verificar se o Sudoku está correto
    int tamanho = verificarValidezTamanho(jogoStr);
    if (tamanho == 0) {
        printf("Sudoku verificado: Jogo inválido; tamanho errado.\n.");
        registarEvento("logs/servidor.log", "Verificação: Sudoku inválido; tamanho errado.");
    }
    else {
        int erros = verificarSudoku(jogo, solucao);
        if (erros == 0) {
            printf(" Sudoku verificado: sem erros!\n");
            registarEvento("logs/servidor.log", "Verificação: Sudoku correto");
        }
        else {
            printf(" Sudoku incorreto: %d erros encontrados.\n", erros);
            registarEvento("logs/servidor.log", "Verificação: Sudoku incorreto");
        }
    }
    return 0;
}
