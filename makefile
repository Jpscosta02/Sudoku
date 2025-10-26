# Compilação do Servidor Sudoku e do Cliente Sudoku

CC = gcc
CFLAGS = -g -std=c99 -Wall

# ---- COMPILA TUDO ----
all: servidor cliente

# ---- SERVIDOR ----
SERVER_OBJECTS = servidor/servidor.o servidor/configuracao.o servidor/logs.o servidor/sudoku.o

servidor: $(SERVER_OBJECTS)
	$(CC) -o servidorApp $(SERVER_OBJECTS)

servidor/servidor.o : servidor/servidor.c servidor/configuracao.h servidor/logs.h servidor/sudoku.h
	$(CC) -c $(CFLAGS) servidor/servidor.c -o servidor/servidor.o

servidor/configuracao.o : servidor/configuracao.c servidor/configuracao.h
	$(CC) -c $(CFLAGS) servidor/configuracao.c -o servidor/configuracao.o

servidor/logs.o : servidor/logs.c servidor/logs.h
	$(CC) -c $(CFLAGS) servidor/logs.c -o servidor/logs.o

servidor/sudoku.o : servidor/sudoku.c servidor/sudoku.h
	$(CC) -c $(CFLAGS) servidor/sudoku.c -o servidor/sudoku.o


# ---- CLIENTE ----
CLIENT_OBJECTS = cliente/cliente.o cliente/configuracao.o cliente/logs.o cliente/sudoku.o

cliente: $(CLIENT_OBJECTS)
	$(CC) -o clienteApp $(CLIENT_OBJECTS)

cliente/cliente.o : cliente/cliente.c cliente/configuracao.h cliente/logs.h cliente/sudoku.h
	$(CC) -c $(CFLAGS) cliente/cliente.c -o cliente/cliente.o

cliente/configuracao.o : cliente/configuracao.c cliente/configuracao.h
	$(CC) -c $(CFLAGS) cliente/configuracao.c -o cliente/configuracao.o

cliente/logs.o : cliente/logs.c cliente/logs.h
	$(CC) -c $(CFLAGS) cliente/logs.c -o cliente/logs.o

cliente/sudoku.o : cliente/sudoku.c cliente/sudoku.h
	$(CC) -c $(CFLAGS) cliente/sudoku.c -o cliente/sudoku.o


# ---- LIMPEZA ----
clean:
	rm -f servidor/*.o cliente/*.o
	rm -f servidorApp clienteApp
	rm -f logs/*.log

