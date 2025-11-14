// cliente/cliente.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../comum/configuracao.h"
#include "../comum/logs.h"
#include "../protocolo/protocolo.h"
#include "cliente_tcp.h"

int main(int argc, char *argv[])
{
    ConfigCliente cfg;
    int sock, idAtribuido, idJogo;
    char tabuleiro[128];
    char ficheiroLog[128];

    if (argc < 2) {
        printf("Uso: %s <ficheiro_config>\n", argv[0]);
        return 1;
    }

    if (!carregarConfiguracaoCliente(argv[1], &cfg)) {
        return 1;
    }

    /* Log inicial antes do ID */
    snprintf(ficheiroLog, sizeof(ficheiroLog), "logs/%s.log", cfg.idCliente);
    registarEvento(ficheiroLog, "Cliente iniciado");

    printf("Cliente %s vai ligar ao servidor %s:%d\n",
           cfg.idCliente, cfg.ipServidor, cfg.porta);

    /* 1. Ligar ao servidor */
    sock = ligarServidor(cfg.ipServidor, cfg.porta);
    registarEvento(ficheiroLog, "Ligado ao servidor");

    /* 2. Pedir jogo */
    pedirJogo(sock, cfg.idCliente);
    registarEvento(ficheiroLog, "PEDIR_JOGO enviado");

    /* 3. Receber ID atribuído */
    if (receberIdAtribuidoCliente(sock, &idAtribuido) <= 0) {
        registarEvento(ficheiroLog, "Erro ao receber ID atribuído");
        close(sock);
        return 1;
    }

    printf("Servidor atribuiu ID interno: %d\n", idAtribuido);

    /* Troca para log correto deste cliente */
    snprintf(ficheiroLog, sizeof(ficheiroLog), "logs/cliente_%d.log", idAtribuido);
    registarEventoID(ficheiroLog, idAtribuido, "ID atribuído recebido");

    /* 4. Receber jogo */
    if (receberJogo(sock, &idJogo, tabuleiro) <= 0) {
        registarEventoID(ficheiroLog, idAtribuido, "Erro ao receber jogo");
        close(sock);
        return 1;
    }

    printf("\n--- Jogo Recebido ---\n");
    printf("ID Jogo: %d\n", idJogo);
    printf("%s\n", tabuleiro);

    registarEventoID(ficheiroLog, idAtribuido, "Jogo recebido com sucesso");

    /* 5. Impedir o cliente de terminar automaticamente */
    printf("\nPressiona ENTER para terminar o cliente...\n");
    getchar();

    /* Cliente irá fechar a ligação → servidor detecta e liberta ID */
    registarEventoID(ficheiroLog, idAtribuido, "Cliente terminou");
    close(sock);

    return 0;
}
