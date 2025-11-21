// servidor/validacao_fifo.c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include "validacao_fifo.h"
#include "sudoku.h"

/* ============================================================
   FIFO DE VALIDAÇÃO DE SUDOKUS
   ============================================================ */

/* Declaração antecipada da função da thread */
static void *threadValidadorFunc(void *arg);

typedef struct PedidoValidacao {
    char solCliente[82];
    char solCorreta[82];
    sem_t semDone;   /* semáforo privado */
    int erros;       /* resultado */
} PedidoValidacao;

#define MAX_FILA_VALIDACAO 128

/* FIFO circular */
static PedidoValidacao *fila[MAX_FILA_VALIDACAO];
static int ini = 0;
static int fim = 0;

/* Sincronização */
static sem_t semItens;      /* nº de pedidos na fila */
static sem_t semEspaco;     /* espaços livres */
static pthread_mutex_t mutexFila = PTHREAD_MUTEX_INITIALIZER;

static int inicializado = 0;
static pthread_t tidValidador;

/* ============================================================
   INICIALIZAÇÃO
   ============================================================ */
void iniciarValidadorFIFO(void)
{
    if (inicializado) return;

    /* iniciar semáforos */
    sem_init(&semItens, 0, 0);
    sem_init(&semEspaco, 0, MAX_FILA_VALIDACAO);

    ini = 0;
    fim = 0;

    /* criar thread validador */
    if (pthread_create(&tidValidador, NULL, threadValidadorFunc, NULL) != 0) {
        perror("Erro ao criar thread validador FIFO");
    }
    pthread_detach(tidValidador);

    inicializado = 1;
}

/* ============================================================
   FUNÇÕES PRIVADAS DA FILA
   ============================================================ */

/* meter pedido na fila */
static void enfileirar(PedidoValidacao *p)
{
    sem_wait(&semEspaco);
    pthread_mutex_lock(&mutexFila);

    fila[fim] = p;
    fim = (fim + 1) % MAX_FILA_VALIDACAO;

    pthread_mutex_unlock(&mutexFila);
    sem_post(&semItens);
}

/* retirar pedido da fila */
static PedidoValidacao *desenfileirar(void)
{
    sem_wait(&semItens);
    pthread_mutex_lock(&mutexFila);

    PedidoValidacao *p = fila[ini];
    ini = (ini + 1) % MAX_FILA_VALIDACAO;

    pthread_mutex_unlock(&mutexFila);
    sem_post(&semEspaco);

    return p;
}

/* ============================================================
   THREAD VALIDADOR
   ============================================================ */

static void *threadValidadorFunc(void *arg)
{
    (void)arg;

    while (1) {
        PedidoValidacao *p = desenfileirar();
        if (!p) continue;

        /* validação do sudoku */
        p->erros = verificarSudokuStrings(p->solCliente, p->solCorreta);

        /* acordar thread do cliente */
        sem_post(&p->semDone);
    }

    return NULL;
}

/* ============================================================
   FUNÇÃO PÚBLICA PARA VALIDAR SUDOKUS
   ============================================================ */

int validarSudokuFIFO(const char *solCliente, const char *solCorreta)
{
    iniciarValidadorFIFO();

    PedidoValidacao pedido;

    /* copiar strings */
    strncpy(pedido.solCliente, solCliente, 81);
    pedido.solCliente[81] = '\0';

    strncpy(pedido.solCorreta, solCorreta, 81);
    pedido.solCorreta[81] = '\0';

    /* semáforo privado */
    sem_init(&pedido.semDone, 0, 0);
    pedido.erros = -1;

    /* colocar pedido na fila */
    enfileirar(&pedido);

    /* esperar resultado */
    sem_wait(&pedido.semDone);
    sem_destroy(&pedido.semDone);

    return pedido.erros;
}
