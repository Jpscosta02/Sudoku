#ifndef CLIENTES_LIGADOS_H
#define CLIENTES_LIGADOS_H

void inicializarClientesLigados();
void registarClienteLigado(int idCliente, int equipa, int sock);
void removerClienteLigado(int idCliente);

/* Envia UPDATE a todos da equipa excepto origem */
void enviarUpdateEquipa(int equipa, int idClienteOrigem,
                        int lin, int col, int val);

#endif
