#include "gestor_ids.h"

#define MAX_CLIENTES 100

static int idsOcupados[MAX_CLIENTES + 1] = {0};

int atribuirIdCliente(void)
{
    for (int i = 1; i <= MAX_CLIENTES; i++) {
        if (idsOcupados[i] == 0) {
            idsOcupados[i] = 1;
            return i;
        }
    }
    return -1;
}

void libertarIdCliente(int id)
{
    if (id >= 1 && id <= MAX_CLIENTES)
        idsOcupados[id] = 0;
}
