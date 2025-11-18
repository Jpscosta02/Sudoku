// servidor/tratar_cliente.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "../protocolo/protocolo.h"
#include "../comum/logs.h"

#include "tratar_cliente.h"
#include "gestor_ids.h"
#include "jogos.h"
#include "sudoku.h"

#include "sincronizacao.h"
#include "barreira.h"
#include "validacao_fifo.h"
#include "ranking.h"

/* ============================================================
   FUNÇÃO PRINCIPAL DA THREAD DE CADA CLIENTE
   ============================================================ */

void *tratarCliente(void *arg)
{
    int sock = *(int *)arg;
    free(arg);

    char idBase[64];
    int idAtribuido;
    char ficheiroLogCliente[128];

    /* 1) receber pedido de jogo */
    int modo = 0;

    if (receberPedidoJogoServidorModo(sock, idBase, sizeof(idBase), &modo) <= 0) {
        close(sock);
        sem_post(&semClientes);
        return NULL;
    }

    /* 2) atribuir ID */
    idAtribuido = atribuirIdCliente();

    snprintf(ficheiroLogCliente, sizeof(ficheiroLogCliente),
             "logs/cliente_%d.log", idAtribuido);

    registarEventoID(ficheiroLogCliente, idAtribuido,
                     "Cliente conectado");

    enviarIdAtribuidoServidor(sock, idAtribuido);

    /* 3) escolher jogo */
    const Jogo *jogo = NULL;

    if (modo == 0) {
        /* JOGO NORMAL */
        jogo = obterJogoProximo();

        if (!jogo) {
            enviarErro(sock, "Sem jogos disponíveis");
            close(sock);
            libertarIdCliente(idAtribuido);
            sem_post(&semClientes);
            return NULL;
        }

        enviarJogoServidor(sock, jogo->id, jogo->jogo);

        registarEventoID(ficheiroLogCliente, idAtribuido,
                         "Jogo normal enviado ao cliente");
    }
    else {
        /* MODO COMPETIÇÃO */
        registarEventoID(ficheiroLogCliente, idAtribuido,
                         "Cliente entrou em competição");

        barreiraCompeticao();   // sincroniza clientes

        jogo = jogoCompeticao;

        if (!jogo) {
            enviarErro(sock, "Erro interno: jogoCompeticao = NULL");
            close(sock);
            libertarIdCliente(idAtribuido);
            sem_post(&semClientes);
            return NULL;
        }

        enviarJogoServidor(sock, jogo->id, jogo->jogo);

        registarEventoID(ficheiroLogCliente, idAtribuido,
                         "Jogo de competição enviado");
    }

    /* MARCAR INÍCIO DO TEMPO */
    time_t tInicio = time(NULL);

    /* 4) receber SOLUCAO do cliente */
    int idJogoSol;
    char solCliente[82];

    if (receberSolucaoServidor(sock, &idJogoSol, solCliente) <= 0) {
        registarEventoID(ficheiroLogCliente, idAtribuido,
                         "Cliente saiu antes de enviar solução");

        close(sock);
        libertarIdCliente(idAtribuido);
        sem_post(&semClientes);
        return NULL;
    }

    registarEventoID(ficheiroLogCliente, idAtribuido,
                     "SOLUCAO recebida");

    /* 5) VALIDAR via validador FIFO */
    int erros = validarSudokuFIFO(solCliente, jogo->solucao);

    /* MARCAR FIM DO TEMPO */
    time_t tFim = time(NULL);
    double tempoDecorrido = difftime(tFim, tInicio);

    /* REGISTAR RESULTADO (só o tempo) */
    registarResultadoCompeticao(idAtribuido, tempoDecorrido);

    /* 6) enviar resultado */
    if (erros == 0) {
        enviarResultadoOK(sock, jogo->id);

        registarEventoID(ficheiroLogCliente, idAtribuido,
                         "Sudoku correto (0 erros)");
    }
    else {
        enviarResultadoErros(sock, jogo->id, erros);

        char msg[128];
        snprintf(msg, sizeof(msg), "Sudoku com %d erro(s)", erros);
        registarEventoID(ficheiroLogCliente, idAtribuido, msg);
    }

    /* 7) enviar ranking — APENAS se acertou */
    if (modo == 1 && erros == 0) {
        enviarRankingCompeticao(sock);
    }

    /* 8) esperar comando SAIR */
    int sair = receberSairServidor(sock);

    if (sair == 1) {
        registarEventoID(ficheiroLogCliente, idAtribuido,
                         "Cliente enviou SAIR");
    } else {
        registarEventoID(ficheiroLogCliente, idAtribuido,
                         "Cliente desconectou sem SAIR");
    }

    /* 9) limpar */
    close(sock);
    libertarIdCliente(idAtribuido);
    sem_post(&semClientes);

    return NULL;
}
