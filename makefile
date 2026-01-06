CC = gcc
CFLAGS = -g -Wall -std=c99 -pthread

INCLUDES = -Iservidor -Icliente -Icomum -Iprotocolo

# ========================
#  OBJETOS DO SERVIDOR
# ========================

SERVIDOR_OBJS = \
    servidor/servidor.o \
    servidor/servidor_tcp.o \
    servidor/tratar_cliente.o \
    servidor/jogos.o \
    servidor/gestor_ids.o \
    servidor/sudoku.o \
    servidor/sincronizacao.o \
    servidor/barreira.o \
    servidor/ranking.o \
    servidor/validacao_fifo.o \
    servidor/equipas.o \
    servidor/clientes_ligados.o \
    comum/util.o \
    comum/logs.o \
    comum/configuracao.o \
    protocolo/protocolo.o

# ========================
#  OBJETOS DO CLIENTE
# ========================

CLIENTE_OBJS = \
	cliente/cliente.o \
	cliente/cliente_tcp.o \
	cliente/cliente_menu.o \
	cliente/cliente_tabuleiro.o \
	cliente/cliente_ui.o \
	comum/util.o \
	comum/logs.o \
	comum/configuracao.o \
	protocolo/protocolo.o

# ========================
#  PROGRAMAS
# ========================

all: servidor cliente

servidor: $(SERVIDOR_OBJS)
	$(CC) $(CFLAGS) -o servidorApp $(SERVIDOR_OBJS)

cliente: $(CLIENTE_OBJS)
	$(CC) $(CFLAGS) -o clienteApp $(CLIENTE_OBJS)

# ========================
#  REGRA GERAL DE COMPILAÇÃO
# ========================

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ========================
#  LIMPEZA
# ========================

clean:
	rm -f servidor/*.o cliente/*.o comum/*.o protocolo/*.o servidorApp clienteApp
	rm -rf logs

.PHONY: all clean servidor cliente
