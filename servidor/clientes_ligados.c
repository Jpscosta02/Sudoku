// servidor/clientes_ligados.c
#include "clientes_ligados.h"
#include "../protocolo/protocolo.h"
#include <pthread.h>

#define MAX_LIGADOS 32

typedef struct {
    int ativo;
    int idCliente;
    int equipa;
    int sock;
} ClienteLigado;

static ClienteLigado lista[MAX_LIGADOS];
static pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

/* ============================================================
   Inicializar
   ============================================================ */
void inicializarClientesLigados()
{
    pthread_mutex_lock(&mx);
    for (int i = 0; i < MAX_LIGADOS; i++)
        lista[i].ativo = 0;
    pthread_mutex_unlock(&mx);
}

/* ============================================================
   Registar cliente quando entra
   ============================================================ */
void registarClienteLigado(int idCliente, int equipa, int sock)
{
    pthread_mutex_lock(&mx);

    for (int i = 0; i < MAX_LIGADOS; i++) {
        if (!lista[i].ativo) {
            lista[i].ativo = 1;
            lista[i].idCliente = idCliente;
            lista[i].equipa = equipa;
            lista[i].sock = sock;
            break;
        }
    }

    pthread_mutex_unlock(&mx);
}

/* ============================================================
   Remover cliente quando sai
   ============================================================ */
void removerClienteLigado(int idCliente)
{
    pthread_mutex_lock(&mx);

    for (int i = 0; i < MAX_LIGADOS; i++) {
        if (lista[i].ativo && lista[i].idCliente == idCliente) {
            lista[i].ativo = 0;
            break;
        }
    }

    pthread_mutex_unlock(&mx);
}

/* ============================================================
   Enviar UPDATE para todos da equipe excepto o próprio
   ============================================================ */
void enviarUpdateEquipa(int equipa, int idClienteOrigem,
                        int lin, int col, int val)
{
    pthread_mutex_lock(&mx);

    for (int i = 0; i < MAX_LIGADOS; i++) {
        if (lista[i].ativo &&
            lista[i].equipa == equipa &&
            lista[i].idCliente != idClienteOrigem)
        {
            enviarUPDATE(lista[i].sock, lin, col, val);
        }
    }

    pthread_mutex_unlock(&mx);
}
