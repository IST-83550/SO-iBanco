/*
// Operações sobre contas, versao 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#ifndef CONTAS_H
#define CONTAS_H

#include <pthread.h>

#define NUM_CONTAS 10
#define TAXAJURO 0.1
#define CUSTOMANUTENCAO 1

#define ATRASO 1

#define atrasar() sleep(ATRASO)
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

void inicializarContas();
int contaExiste(int idConta);
int debitar(int idConta, int valor);
int creditar(int idConta, int valor);
int debitar_trans(int idConta, int valor);
int creditar_trans(int idConta, int valor);
int lerSaldo(int idConta);
int transferir(int idconta, int idContaDestino, int valor);
void simular(int numAnos);

/* Mutexs */
void mutex_lock(pthread_mutex_t *mutex);
void mutex_unlock(pthread_mutex_t *mutex);
void mutex_lock_cond(pthread_mutex_t *mutex, int id1, int id2);
void mutex_unlock_cond(pthread_mutex_t *mutex, int id1, int id2);
/* vetor de mutexs para contas */
pthread_mutex_t contas_ctrl[NUM_CONTAS];

/* exit flag (activated in signal handler) */
int flag_exit;

#endif /* CONTAS_H */
