/*
// Operações sobre contas, versao 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
*/

#include "contas.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int contasSaldos[NUM_CONTAS];

/* log.txt file to print */
extern FILE *log_f;

int contaExiste(int idConta) {
  return (idConta > 0 && idConta <= NUM_CONTAS);
}

void inicializarContas() {
  int i;
  for (i=0; i<NUM_CONTAS; i++)
    contasSaldos[i] = 0;
}

int debitar(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;
    
  mutex_lock(&contas_ctrl[idConta-1]);
  if (contasSaldos[idConta - 1] < valor) {
    mutex_unlock(&contas_ctrl[idConta-1]);
    return -1;
  }
  contasSaldos[idConta - 1] -= valor;
  fprintf(log_f, "%lu: debitar %d %d\n", pthread_self(), idConta, valor); 
  mutex_unlock(&contas_ctrl[idConta-1]);
  atrasar();
  
  return 0;
}

int debitar_trans(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;
    
  if (contasSaldos[idConta - 1] < valor) {
    return -1;
  }
  contasSaldos[idConta - 1] -= valor;
  atrasar();
  
  return 0;
}

int creditar(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;
    
  mutex_lock(&contas_ctrl[idConta-1]);
  contasSaldos[idConta - 1] += valor;
  fprintf(log_f, "%lu: creditar %d %d\n", pthread_self(), idConta, valor);
  mutex_unlock(&contas_ctrl[idConta-1]);
  
  return 0;
}

int creditar_trans(int idConta, int valor) {
  atrasar();
  if (!contaExiste(idConta))
    return -1;
  contasSaldos[idConta - 1] += valor;
  return 0;
}

int lerSaldo(int idConta) {
  int saldo;
  atrasar();
  if (!contaExiste(idConta))
    return -1;
    
  mutex_lock(&contas_ctrl[idConta-1]);
  saldo = contasSaldos[idConta - 1];
  fprintf(log_f, "%lu: lerSaldo %d\n", pthread_self(), idConta);
  mutex_unlock(&contas_ctrl[idConta-1]);
  
  return saldo;
}

int transferir(int idConta, int idContaDestino, int valor) {
  atrasar();
  if(!contaExiste(idConta) || !contaExiste(idContaDestino))
    return -1;
    
  mutex_lock_cond(contas_ctrl, idConta, idContaDestino);
  
  if(debitar_trans(idConta, valor) < 0) {
    mutex_unlock_cond(contas_ctrl, idConta, idContaDestino);
    return -1;
  }
  creditar_trans(idContaDestino, valor);
  
  fprintf(log_f, "%lu: transferir %d %d %d\n", pthread_self(), idConta, idContaDestino, valor);
  
  mutex_unlock_cond(contas_ctrl, idConta, idContaDestino);
  return 0;
}

void simular(int numAnos) {
  int i,j;

  for(i = 0; i <= numAnos ; i++) {
    printf("SIMULACAO: Ano %d\n", i);
    printf("=================\n");

    for(j = 1; j <= NUM_CONTAS; j++) {
      atrasar();
      while (printf("Conta %d, Saldo %d\n", j, contasSaldos[j - 1] ) < 0) {
        if (errno == EINTR)
          continue;
        else
          break;
      }      
      contasSaldos[j-1] = MAX(contasSaldos[j - 1]*(1 + TAXAJURO) - CUSTOMANUTENCAO, 0);
    }
    printf("\n");

    if(flag_exit == 1) {
      printf("Simulacao terminada por signal\n");
      return;
    }
  }
}
