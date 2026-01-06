#include "cliente_tcp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int ligarServidor(const char *ip, int porta)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // Criar socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Erro ao criar socket TCP");
        exit(1);
    }

    // Preparar endereço
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(porta);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        perror("IP inválido");
        exit(1);
    }

    // Tentar conectar
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Erro no connect()");
        exit(1);
    }

    printf("Ligado ao servidor %s:%d\n", ip, porta);
    return sockfd;
}
