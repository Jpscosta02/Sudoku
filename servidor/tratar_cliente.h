// servidor/tratar_cliente.h
#ifndef TRATAR_CLIENTE_H
#define TRATAR_CLIENTE_H

/* Thread para tratar um cliente */
void *tratarCliente(void *arg);

/* Função que aceita clientes em loop infinito */
void aceitarClientes(int sockListen);

#endif
