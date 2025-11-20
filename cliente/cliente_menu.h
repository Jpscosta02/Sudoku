#ifndef CLIENTE_MENU_H
#define CLIENTE_MENU_H

int menuSudoku(char solucaoOut[82],
               char tabuleiroStr[82],
               const char *solucaoCorreta,
               const char *ficheiroLog,
               int idAtribuido,
               int sock,
               int modoCompeticao);

#endif
