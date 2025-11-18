#ifndef PROTOCOLO_H
#define PROTOCOLO_H

/* ======== BASE ======== */

int enviarMensagem(int sock, const char *msg);
int receberMensagem(int sock, char *buffer, int max);

/* ======== PEDIR JOGO / MODOS ======== */

/* Versão antiga mantida para compatibilidade – trata como modo normal */
int pedirJogo(int sock, const char *idClienteBase);

/* Versões novas explícitas */
int pedirJogoNormal(int sock, const char *idClienteBase);
int pedirJogoCompeticao(int sock, const char *idClienteBase);

/* Versão antiga (servidor) – ainda existe, mas sem informação de modo */
int receberPedidoJogoServidor(int sock, char *idClienteBase, int max);

/* Versão nova – devolve modo:
   modo = 0 → normal
   modo = 1 → competição
*/
int receberPedidoJogoServidorModo(int sock, char *idClienteBase, int max, int *modo);

/* ======== ID_ATRIBUIDO ======== */

int enviarIdAtribuidoServidor(int sock, int idNovo);
int receberIdAtribuidoCliente(int sock, int *idNovo);

/* ======== JOGO ======== */

int enviarJogoServidor(int sock, int idJogo, const char *tab);
int receberJogo(int sock, int *idJogo, char *tab);

/* ======== SOLUCAO ======== */

int enviarSolucao(int sock, int idJogo, const char *sol);
int receberSolucaoServidor(int sock, int *idJogo, char *sol);

/* ======== RESULTADO ======== */

int enviarResultadoOK(int sock, int idJogo);
int enviarResultadoErros(int sock, int idJogo, int erros);
int receberResultado(int sock, int *idJogo, int *erros);

/* ======== SAIR ======== */

int enviarSair(int sock);
int receberSairServidor(int sock);

/* ======== ERRO ======== */

int enviarErro(int sock, const char *descricao);
int receberErro(int sock, char *descricao, int maxLen);

/* ================== RANKING (Competição) ================== */

/* Lê a linha "RANKING N" */
int receberRankingHeader(int sock, int *nEntradas);

/* Lê linhas "id tempo" */
int receberRankingLinha(int sock, int *idCliente, double *tempo);

#endif
