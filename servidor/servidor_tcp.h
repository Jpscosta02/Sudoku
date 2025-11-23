#ifndef SERVIDOR_TCP_H
#define SERVIDOR_TCP_H

int criarSocketServidor(int porta);
int aceitarCliente(int sockfd);

#endif