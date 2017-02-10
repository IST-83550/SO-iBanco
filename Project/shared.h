/*
//  Shared definitions, macros & others (header file)
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
//                Grupo 84
*/

#ifndef SHARED_H
#define SHARED_H

#include <errno.h>

/* - String types */
#define COMANDO_DEBITAR "debitar"
#define COMANDO_CREDITAR "creditar"
#define COMANDO_LER_SALDO "lerSaldo"
#define COMANDO_SIMULAR "simular"
#define COMANDO_TRANSFERIR "transferir"
#define COMANDO_SAIR "sair"
#define COMANDO_SAIR_AGORA "agora"
#define COMANDO_SAIR_TERMINAL "sair-terminal"
#define TERMINAL_NAME_STR "/tmp/i-banco-terminal"

/* - Numerical types - */
#define KEY_DEBITAR 1
#define KEY_CREDITAR 2
#define KEY_LER_SALDO 3
#define KEY_TRANSFERIR 4
#define KEY_SAIR 5
#define KEY_SAIR_AGORA 6
#define KEY_SIMULAR 7

/* Error handling */
#define handle_error(msg) { perror(msg); exit(EXIT_FAILURE); }

/* Comand struct */
typedef struct {
    int operacao;
    int idConta;
    int idContaDestino; /* para o comando transferir */
    int valor;
    int pid;
} comando_t;

#endif /* SHARED_H */