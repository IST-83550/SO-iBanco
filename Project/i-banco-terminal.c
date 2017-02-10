/*
// i-Banco Shell Terminal
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
//              Grupo 84
*/
/* - C/POSIX library headers */
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

/* - Local headers */
#include "shared.h"
#include "helpers.h"
#include "commandlinereader.h"

/* - Definitions */
#define MAXARGS 4
#define BUFFER_SIZE 100
#define TERMINAL_PIPE_NAME_SIZE 35
#define RESPOSTA_SIZE 80
#define LINE_COMMAND_MAX 128 /* Int: Command line's buffer size */

/* -- Global Variables -- */
/* Pipe name */
const char* pipe_name;

/* terminal Pipe name */
char terminal_pipe_name[TERMINAL_PIPE_NAME_SIZE];

/* -- Prototypes -- */
/* Comunication with i-banco */
void AddRequest(int operacao, int idConta, int idContaDestino, int valor);

/*****************************************************
 * Main. *********************************************
 *****************************************************/
int main (int argc, char** argv) {
    
    char *args[MAXARGS + 1];
    char buffer[LINE_COMMAND_MAX];

    if(argc != 2) {
        fprintf(stderr, "Incorrect arguments: i-banco-terminal <pipe name>\n");
        exit(EXIT_FAILURE);
    }
    
    /* Assigns signal handler (broken pipe) */
    if(signal (SIGPIPE, handle_SIGPIPE) == SIG_ERR) {
        handle_error("Signal");
    }
    
    pipe_name = argv[1];

    snprintf(terminal_pipe_name, TERMINAL_PIPE_NAME_SIZE, "%s %d\n", "/tmp/i-banco-terminal", getpid());
    
    /* cria pipe: i-banco -> terminal */
    unlink(terminal_pipe_name);
    new_pipe(terminal_pipe_name, S_IRWXU | S_IRWXG | S_IRWXO);

    while (1) {
        int numargs;

        numargs = readLineArguments(args, MAXARGS + 1, buffer, LINE_COMMAND_MAX);

        if (args[0] == NULL)
            continue;

        /* Comando Sair do terminal */
        else if (strcmp(args[0], COMANDO_SAIR_TERMINAL) == 0) {
            exit(EXIT_SUCCESS);
        }

        /* Comando Sair */
        else if (numargs < 0 ||
	        (numargs > 0 && (strcmp(args[0], COMANDO_SAIR) == 0))) {
            if (numargs == 2 && strcmp(args[1], COMANDO_SAIR_AGORA) == 0) {
                AddRequest(KEY_SAIR, 0, 0, KEY_SAIR_AGORA);
            }
            else {
                AddRequest(KEY_SAIR, 0, 0, 0);
            }
        }

        /* Comando Simular */
        else if (strcmp(args[0], COMANDO_SIMULAR) == 0) {
            int numAnos;

            if (numargs < 2) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
                continue;
            }

            numAnos = atoi(args[1]);

            AddRequest(KEY_SIMULAR, 0, 0, numAnos);
        }

        /* Comando Debitar */
        else if (strcmp(args[0], COMANDO_DEBITAR) == 0) {
            int idConta, valor;
            if (numargs < 3) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_DEBITAR);
    	       continue;
            }
    
            idConta = atoi(args[1]);
            valor = atoi(args[2]);

            AddRequest(KEY_DEBITAR, idConta, 0, valor);
        }

        /* Comando Creditar */
        else if (strcmp(args[0], COMANDO_CREDITAR) == 0) {
            int idConta, valor;
            if (numargs < 3) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_CREDITAR);
                continue;
            }

            idConta = atoi(args[1]);
            valor = atoi(args[2]);
           
            AddRequest(KEY_CREDITAR, idConta, 0, valor);
        }

        /* Comando Ler Saldo */
        else if (strcmp(args[0], COMANDO_LER_SALDO) == 0) {
            int idConta;
            if (numargs < 2) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_LER_SALDO);
                continue;
            }

            idConta = atoi(args[1]);

            AddRequest(KEY_LER_SALDO, idConta, 0, 0);
        }

        /* Comando Transferir */
        else if (strcmp(args[0], COMANDO_TRANSFERIR) == 0) {
            int idConta, idContaDestino, valor;
            if (numargs < 4) {
                printf("%s: Sintaxe inválida, tente de novo.\n", COMANDO_TRANSFERIR);
                continue;
            }

            idConta = atoi(args[1]);
            idContaDestino = atoi(args[2]);
            valor = atoi(args[3]);

            AddRequest(KEY_TRANSFERIR, idConta, idContaDestino, valor);
        }
        else {
          printf("Comando desconhecido. Tente de novo.\n");
        }
    }
}

/*****************************************************
 * Function implementations. *************************
 *****************************************************/
 /* Comunication with i-banco */
void AddRequest(int operacao, int idConta, int idContaDestino, int valor) {
    time_t start_t, end_t;
    comando_t cmd;
    char resposta[RESPOSTA_SIZE];
    int pipe_out, terminal_in;
    
    cmd.operacao = operacao;
    cmd.idConta = idConta;
    cmd.idContaDestino = idContaDestino;
    cmd.valor = valor;
    cmd.pid = getpid();
    
    /* Opens terminal pipe with write permissions: terminal -> i-banco */
    pipe_out = fd_open(pipe_name, O_WRONLY, S_IRUSR | S_IWUSR);

    fd_write(pipe_out, &cmd, sizeof(comando_t));
    fd_close(pipe_out);
    
    if(operacao != KEY_SAIR && operacao != KEY_SAIR_AGORA && operacao != KEY_SIMULAR){
        time(&start_t);

        /* Opens terminal pipe with write permissions: terminal -> i-banco */
        terminal_in = fd_open(terminal_pipe_name, O_RDONLY, S_IRUSR | S_IWUSR);
        fd_read(terminal_in, &resposta, sizeof(char) * RESPOSTA_SIZE);

        time(&end_t);
        fd_close(terminal_in);
        printf("%sTempo de execucao = %.2f segundos\n", resposta, difftime(end_t, start_t));
    }
}