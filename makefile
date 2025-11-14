# ==========================
#  COMPILADOR E FLAGS
# ==========================

CC = gcc
CFLAGS = -g -Wall -std=c99 -pthread

# Includes das várias pastas
INCLUDES = -Iservidor -Icliente -Iprotocolo -Icomum

# ==========================
#  FICHEIROS COMUNS
# ==========================

COMMON_SRCS = comum/util.c comum/logs.c comum/configuracao.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

# ==========================
#  PROTOCOLO
# ==========================

PROTO_SRCS = protocolo/protocolo.c
PROTO_OBJS = $(PROTO_SRCS:.c=.o)

# ==========================
#  SERVIDOR
# ==========================

SERVER_SRCS = \
    servidor/servidor.c \
    servidor/servidor_tcp.c \
    servidor/tratar_cliente.c \
    servidor/jogos.c \
    servidor/gestor_ids.c



SERVER_OBJS = $(SERVER_SRCS:.c=.o)

# ==========================
#  CLIENTE
# ==========================

CLIENT_SRCS = \
	cliente/cliente.c \
	cliente/cliente_tcp.c

CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)

# ==========================
#  TARGETS PRINCIPAIS
# ==========================

all: servidor cliente

# ---- BINÁRIO DO SERVIDOR ----
servidor: $(SERVER_OBJS) $(COMMON_OBJS) $(PROTO_OBJS)
	$(CC) $(CFLAGS) -o servidorApp $(SERVER_OBJS) $(COMMON_OBJS) $(PROTO_OBJS)

# ---- BINÁRIO DO CLIENTE ----
cliente: $(CLIENT_OBJS) $(COMMON_OBJS) $(PROTO_OBJS)
	$(CC) $(CFLAGS) -o clienteApp $(CLIENT_OBJS) $(COMMON_OBJS) $(PROTO_OBJS)

# ==========================
#  REGRA GERAL PARA .c -> .o
# ==========================

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# ==========================
#  LIMPEZA
# ==========================

clean:
	rm -f servidor/*.o cliente/*.o comum/*.o protocolo/*.o
	rm -f servidorApp clienteApp
	rm -f logs/*.log
