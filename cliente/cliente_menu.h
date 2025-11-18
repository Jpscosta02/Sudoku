#ifndef CLIENTE_MENU_H
#define CLIENTE_MENU_H

/*
 * Função principal do menu Sudoku.
 * - solucaoOut: devolve a solução final em string (81 chars)
 * - tabuleiroStr: tabuleiro atual que deve ser mantido entre ciclos
 * - solucaoCorreta: solução correta (para autocomplete de teste)
 * - ficheiroLog: caminho do log do cliente
 * - idAtribuido: ID do cliente atribuído pelo servidor
 */
int menuSudoku(char solucaoOut[82],
               char tabuleiroStr[82],
               const char *solucaoCorreta,
               const char *ficheiroLog,
               int idAtribuido);

#endif
