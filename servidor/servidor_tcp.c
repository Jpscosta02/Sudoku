#include "servidor_tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

int criarSocketServidor(int porta)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // 1) Criar socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro a criar socket TCP");
        exit(1);
    }

    // Reusar porta
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // 2) Configurar endereço
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(porta);

    // 3) bind
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro no bind()");
        exit(1);
    }

    // 4) listen
    if (listen(sockfd, 10) < 0) {
        perror("Erro no listen()");
        exit(1);
    }

    printf("Servidor TCP à escuta na porta %d...\n", porta);
    return sockfd;
}

int aceitarCliente(int sockfd)
{
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) {
        perror("Erro no accept()");
        return -1;
    }

    return newsockfd;
}
