#ifndef SUDOKU_H
#define SUDOKU_H

/* 
 * Compara a resposta do cliente com a solução correta e
 * verifica as regras do Sudoku.
 *
 * Retorna o número TOTAL de erros:
 *  - posições com dígitos inválidos (não '1'..'9')
 *  - posições incompletas (menos de 81 casas válidas)
 *  - posições diferentes da solução correta
 *  - violações das regras em linhas/colunas/quadrados 3x3
 *
 * Em caso de erro grave (ponteiros NULL), retorna -1.
 */
int verificarSudokuStrings(const char *resposta, const char *solucaoCorreta);

#endif
