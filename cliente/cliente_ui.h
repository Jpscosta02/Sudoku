#ifndef CLIENTE_UI_H
#define CLIENTE_UI_H

/* Cores ANSI para o terminal */
#define COR_RESET   "\033[0m"
#define COR_FIXO    "\033[1;36m"  // azul claro para números do puzzle original
#define COR_USER    "\033[1;32m"  // verde para números inseridos pelo utilizador
#define COR_MENU    "\033[1;33m"  // amarelo para o menu
#define COR_ERRO    "\033[1;31m"  // vermelho
#define COR_INFO    "\033[1;34m"  // azul

/* Desenha o tabuleiro com cores e bordas */
void mostrarTabuleiroColorido(int tab[9][9], int original[9][9]);

#endif
