// servidor/tratar_cliente.c
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../protocolo/protocolo.h"
#include "../comum/logs.h"
#include "tratar_cliente.h"
#include "jogos.h"
#include "gestor_ids.h"

void *tratarCliente(void *arg)
{
    int sock = *(int *)arg;
    free(arg);

    char idBase[64];
    int idAtribuido;
    const Jogo *jogo;
    char ficheiroLogCliente[128];

    /* 1. Receber PEDIR_JOGO */
    if (receberPedidoJogoServidor(sock, idBase, sizeof(idBase)) <= 0) {
        close(sock);
        return NULL;
    }

    printf("Cliente pediu jogo com ID base: %s\n", idBase);
    registarEvento("logs/servidor.log", "PEDIR_JOGO recebido");

    /* 2. Atribuir ID */
    idAtribuido = atribuirIdCliente();

    snprintf(ficheiroLogCliente, sizeof(ficheiroLogCliente),
             "logs/cliente_%d.log", idAtribuido);

    registarEventoID(ficheiroLogCliente, idAtribuido,
                     "Cliente conectado");

    enviarIdAtribuidoServidor(sock, idAtribuido);
    registarEventoID(ficheiroLogCliente, idAtribuido,
                     "ID enviado ao cliente");

    printf("ID atribuído ao cliente: %d\n", idAtribuido);

    /* 3. Enviar jogo */
    jogo = obterJogoProximo();
    enviarJogoServidor(sock, jogo->id, jogo->jogo);

    registarEventoID(ficheiroLogCliente, idAtribuido,
                     "Jogo enviado ao cliente");

    /* 4. Esperar o cliente fechar a ligação */
    char tmp[2];
    int n = read(sock, tmp, 1);  // bloqueia até cliente fechar

    registarEventoID(ficheiroLogCliente, idAtribuido,
                     "Cliente desconectou");

    /* 5. Libertar ID */
    close(sock);
    libertarIdCliente(idAtribuido);

    return NULL;
}
