#ifndef PROTOCOLO_H
#define PROTOCOLO_H

/* Funções base */
int enviarMensagem(int sock, const char *msg);
int receberMensagem(int sock, char *buffer, int max);

/* Pedido de jogo */
int pedirJogo(int sock, const char *idClienteBase);
int receberPedidoJogoServidor(int sock, char *idClienteBase, int maxLen);

/* ID Atribuído */
int enviarIdAtribuidoServidor(int sock, int idNovo);
int receberIdAtribuidoCliente(int sock, int *idNovo);

/* Envio/Receção de jogo */
int enviarJogoServidor(int sock, int idJogo, const char *tabuleiro81);
int receberJogo(int sock, int *idJogo, char *tabuleiro81);

/* Solução */
int enviarSolucao(int sock, int idJogo, const char *sol81);
int receberSolucaoServidor(int sock, int *idJogo, char *sol81);

/* Resultado */
int enviarResultadoOK(int sock, int idJogo);
int enviarResultadoErros(int sock, int idJogo, int erros);
int receberResultado(int sock, int *idJogo, int *erros);

#endif
