# Compilador
CC = gcc
CFLAGS = -g -Wall -std=c99 -pthread

# Includes
INCLUDES = -Iservidor -Icliente -Icomum -Iprotocolo

# ============================================
#  Targets principais
# ============================================

all: servidor cliente

# ============================================
#  OBJETOS DO SERVIDOR
# ============================================

SERVER_OBJS = \
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
    comum/util.o \
    comum/logs.o \
    comum/configuracao.o \
    protocolo/protocolo.o

servidor: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o servidorApp $(SERVER_OBJS)

# --- novos módulos do servidor ---

servidor/sincronizacao.o: servidor/sincronizacao.c servidor/sincronizacao.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/sincronizacao.c -o servidor/sincronizacao.o

servidor/barreira.o: servidor/barreira.c servidor/barreira.h servidor/ranking.h servidor/sincronizacao.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/barreira.c -o servidor/barreira.o

servidor/ranking.o: servidor/ranking.c servidor/ranking.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/ranking.c -o servidor/ranking.o

servidor/validacao_fifo.o: servidor/validacao_fifo.c servidor/validacao_fifo.h servidor/sudoku.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/validacao_fifo.c -o servidor/validacao_fifo.o

# --- já existentes ---

servidor/servidor.o: servidor/servidor.c servidor/tratar_cliente.h servidor/sincronizacao.h servidor/barreira.h servidor/validacao_fifo.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/servidor.c -o servidor/servidor.o

servidor/servidor_tcp.o: servidor/servidor_tcp.c servidor/servidor_tcp.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/servidor_tcp.c -o servidor/servidor_tcp.o

servidor/tratar_cliente.o: servidor/tratar_cliente.c servidor/tratar_cliente.h servidor/ranking.h servidor/validacao_fifo.h servidor/barreira.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/tratar_cliente.c -o servidor/tratar_cliente.o

servidor/jogos.o: servidor/jogos.c servidor/jogos.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/jogos.c -o servidor/jogos.o

servidor/gestor_ids.o: servidor/gestor_ids.c servidor/gestor_ids.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/gestor_ids.c -o servidor/gestor_ids.o

servidor/sudoku.o: servidor/sudoku.c servidor/sudoku.h
	$(CC) $(CFLAGS) $(INCLUDES) -c servidor/sudoku.c -o servidor/sudoku.o


# ============================================
#  OBJETOS DO CLIENTE (SEM ALTERAÇÕES)
# ============================================

CLIENT_OBJS = \
    cliente/cliente.o \
    cliente/cliente_tcp.o \
    cliente/cliente_menu.o \
    cliente/cliente_tabuleiro.o \
    cliente/cliente_ui.o \
    comum/util.o \
    comum/logs.o \
    comum/configuracao.o \
    protocolo/protocolo.o

cliente: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o clienteApp $(CLIENT_OBJS)

cliente/cliente.o: cliente/cliente.c cliente/cliente_tcp.h cliente/cliente_menu.h cliente/cliente_ui.h
	$(CC) $(CFLAGS) $(INCLUDES) -c cliente/cliente.c -o cliente/cliente.o

cliente/cliente_tcp.o: cliente/cliente_tcp.c cliente/cliente_tcp.h
	$(CC) $(CFLAGS) $(INCLUDES) -c cliente/cliente_tcp.c -o cliente/cliente_tcp.o

cliente/cliente_menu.o: cliente/cliente_menu.c cliente/cliente_menu.h cliente/cliente_tabuleiro.h cliente/cliente_ui.h
	$(CC) $(CFLAGS) $(INCLUDES) -c cliente/cliente_menu.c -o cliente/cliente_menu.o

cliente/cliente_tabuleiro.o: cliente/cliente_tabuleiro.c cliente/cliente_tabuleiro.h
	$(CC) $(CFLAGS) $(INCLUDES) -c cliente/cliente_tabuleiro.c -o cliente/cliente_tabuleiro.o

cliente/cliente_ui.o: cliente/cliente_ui.c cliente/cliente_ui.h
	$(CC) $(CFLAGS) $(INCLUDES) -c cliente/cliente_ui.c -o cliente/cliente_ui.o


# ============================================
#  LIMPEZA
# ============================================

clean:
	rm -f servidor/*.o cliente/*.o comum/*.o protocolo/*.o
	rm -f servidorApp clienteApp
	rm -f logs/*.log
