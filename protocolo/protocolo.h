#ifndef PROTOCOLO_H
#define PROTOCOLO_H

int enviarMensagem(int sock, const char *msg);

/* ---- Pedir jogo ---- */
int pedirJogoNormal(int sock, const char *idCliente);
int pedirJogoCompeticao(int sock, const char *idCliente, int equipa);
int receberPedidoJogoServidorModo(int sock, char *idOut, int max, int *modoOut, int *equipaOut);

/* ---- ID atribuído ---- */
int enviarIdAtribuidoServidor(int sock, int id);
int receberIdAtribuidoCliente(int sock, int *id);

/* ---- Envio do jogo ---- */
int enviarJogoServidor(int sock, int idJogo, const char *tab);
int receberJogo(int sock, int *idJogo, char *tab);

/* ---- Solução final ---- */
int enviarSolucao(int sock, int idJogo, const char *sol);
int receberSolucaoServidor(int sock, int *idJogo, char *sol);

/* ---- SET/UPDATE antigo (ainda usamos para autocomplete opcional) ---- */
int enviarSET(int sock, int lin, int col, int val);
int receberSET(int sock, int *lin, int *col, int *val);
int enviarUPDATE(int sock, int lin, int col, int val);
int receberUPDATE(int sock, int *lin, int *col, int *val);

/* ---- Resultado ---- */
int enviarResultadoOK(int sock, int idJogo);
int enviarResultadoErros(int sock, int idJogo, int erros);
int receberResultado(int sock, int *idJogo, int *erros);

/* ---- Sair ---- */
int enviarSair(int sock);
int receberSairServidor(int sock);

/* ---- ERRO ---- */
int enviarErro(int sock, const char *descricao);
int receberErro(int sock, char *descricao, int max);

/* ============================================================
   NOVO PARA SINCRONIZAÇÃO FORTE
   ============================================================ */

/* Cliente → Servidor: envia tabuleiro completo */
int enviarSyncTabuleiro(int sock, const char tab81[82]);

/* Cliente → Servidor: pede tabuleiro oficial */
int enviarPedirTabuleiro(int sock);

/* Servidor → Cliente: envia tabuleiro oficial */
int enviarTabuleiroEquipa(int sock, const char tab81[82]);

/* Cliente → ...: recebe tabuleiro oficial */
int receberTabuleiroEquipa(int sock, char tabOut[82]);

#endif
