/*
//
// Projeto SO - exercicio 4, version 1
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
//
//    83416 - Afonso Mercier de Figueiredo
//    83550 - Pedro Pinto Santos
//
//              Grupo 84
*/
/* ---------- Header files ---------- */
/* - C/POSIX library headers */
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

/* - Local headers - */
#include "commandlinereader.h"
#include "contas.h"
#include "helpers.h"
#include "shared.h"

/* ----------- Definitions ----------- */
/* - Pre-processor macros */
#define PATH_LOGFILE_STR "log.txt"
#define PATH_STR "/tmp/i-banco-pipe"

/* - Numerical types - */
#define BUFFER_SIZE 80
#define SIMULAR_FILENAME_SIZE 40
#define MAX_CHILDS 20
#define ERROR_V -1
#define NUM_TRABALHADORAS 3
#define CMD_BUFFER_DIM (NUM_TRABALHADORAS * 2)

/*****************************************************
 * Global variables. *********************************
 *****************************************************/

/* Buffer */
comando_t cmd_buffer[CMD_BUFFER_DIM];

/* Semaphores (read) / (write)  */
sem_t read_sem, write_sem;

/* Mutexs (buffer mutex) / (no_command_ctrl) */
pthread_mutex_t buffer_ctrl, no_command_ctrl_mutex;

/* Variable condition variable */
pthread_cond_t no_command_ctrl;

/* buffer write index / buffer read index (global) */
int buff_write_idx = 0, buff_read_idx = 0;

/* number of non-processed items in the buffer */
int num_orders = 0;

/* Logging file (file pointer) */
FILE *log_f;

/*****************************************************
 * Helper functions Prototypes ***********************
 *****************************************************/
/* General Functions */
void *thread_handler(void *arg_ptr);
void AddBuffer(comando_t cmd);

/* Destroy resources */
void destroySharedResources();

/* Termination signal (SIGUSR1) handler / i-banco exit routine */
void trataSairAgora(int s) { flag_exit = 1; }

/*****************************************************
 * Main thread. **************************************
 *****************************************************/
int main (int argc, char** argv) {

    /* Status, number of child processes and child process id */
    int status, child_processes = 0 , i = 0, fd, operacao;
    
    /* Comando Struct*/
    comando_t cmd;
    
    pid_t pid;
    /* Threads ids vector */
    pthread_t tid[NUM_TRABALHADORAS];

    flag_exit = 0;

    /* Opens log file with write permissions */
    log_f = f_open(PATH_LOGFILE_STR, "w");

    /* Signal handler / Exit routine */
    if(signal (SIGUSR1, trataSairAgora) == SIG_ERR) {
        handle_error("Signal");
    }
    
    /* Assigns signal handler (broken pipe) */
    if(signal (SIGPIPE, handle_SIGPIPE) == SIG_ERR) {
        handle_error("Signal");
    }

    /* Initializes read semaphore */
    semaphore_init(&read_sem, 0 , 0);

    /* Initializes write semaphore */
    semaphore_init(&write_sem, 0 , CMD_BUFFER_DIM);

    /* Initializes condition variable */
    cond_init(&no_command_ctrl);

    /* Initializes mutex (data_ctrl) */
    mutex_init(&buffer_ctrl);

    /* Initializes mutex (no_command_ctrl_mutex) */
    mutex_init(&no_command_ctrl_mutex);

    /* Initializes NUM_CONTAS mutexs */
    for (i = 0; i < NUM_CONTAS ; i++) {
        mutex_init(&contas_ctrl[i]);
    }

    /* Creates threads pool */
    for (i = 0; i < NUM_TRABALHADORAS; i++) {
        pcreate(&tid[i], thread_handler, NULL);
    }
    
    inicializarContas();

    /* creates pipe: terminal -> i-banco */
    unlink(PATH_STR);
    new_pipe(PATH_STR, S_IRWXU | S_IRWXG | S_IRWXO);
    
    printf("Bem-vinda/o ao i-banco\n\n");

    while (1) {
        
        /* opens pipe to read */
        fd = fd_open(PATH_STR, O_RDONLY, S_IRUSR | S_IWUSR);
        fd_read(fd, &cmd, sizeof(comando_t));
        fd_close(fd);

        operacao = cmd.operacao;

        /* EOF (end of file) or command "sair" */
        if (operacao == KEY_SAIR) {
            printf("i-banco vai terminar.\n--\n");
            kill(0, SIGUSR1);

            /* comando sair agora */
            if (cmd.valor == KEY_SAIR_AGORA)
                kill(0, SIGUSR1);

            /* sends the exit command to all threads */
            for(i = 0; i < NUM_TRABALHADORAS; i++) {
                cmd.operacao = KEY_SAIR;
                AddBuffer(cmd);
            }

            /* Waits for threads */
            for(i = 0; i < NUM_TRABALHADORAS; i++) {
                if( pthread_join(tid[i], NULL) != 0 ){
                    fprintf(stderr, "pthread_join");
                    exit(EXIT_FAILURE);
                }
            }

            while (child_processes > 0) {
                pid = wait(&status);
                if (pid == ERROR_V){
                    if (errno == EINTR)
                        continue;
                    else {
                        fprintf(stderr, "Wait");
                        exit(EXIT_FAILURE);
                    }
                }
                else {
                    /* Prints PID and status */
                    if (WIFEXITED(status))
                        fprintf(stderr, "FILHO TERMINADO (PID=%d; terminou normalmente)\n", pid);
                    else
                        fprintf(stderr, "FILHO TERMINADO (PID=%d; terminou abruptamente)\n", pid);
                }
                child_processes--;
            }
            printf("--\ni-banco terminou.\n");

            fflush(log_f);
            f_close(log_f);

            destroySharedResources();

            exit(EXIT_SUCCESS);
        }
        /* comando simular */
        else if (operacao == KEY_SIMULAR) {
            int simular_file;
            char filename[SIMULAR_FILENAME_SIZE];

            /* waits for the conclusion of all buffer's commands */
            mutex_lock(&no_command_ctrl_mutex);
                while( num_orders != 0)
                    cond_wait(&no_command_ctrl, &no_command_ctrl_mutex);
            mutex_unlock(&no_command_ctrl_mutex);

            fflush(log_f);

            pid = fork();
            if (pid == ERROR_V)                /* Error */
                perror("fork");
            else if (pid == 0) {               /* Child process */

                snprintf(filename,SIMULAR_FILENAME_SIZE, "i-banco-sim-%d", getpid());
                simular_file = fd_open(filename, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);

                /* Redirects child output */
                fd_close(1);
                fd_dup(simular_file);

                simular(cmd.valor);

                fprintf(log_f, "%lu: simular %d\n", pthread_self(), cmd.valor);
                exit(EXIT_SUCCESS);
            }
            else {                             /* Father process */
                child_processes++;
                continue;
            }
        }
        else {
            AddBuffer(cmd);
        }
    }
}

/* Sends a message through pipe (receives pipe PID indentifier) */
void SendsThroughPipe(char* buffer, int terminal_pid){
    char terminal_pipe_name[35];
    int fifo_out;

    snprintf(terminal_pipe_name, 35, "%s %d\n","/tmp/i-banco-terminal" , terminal_pid);

    fifo_out = fd_open(terminal_pipe_name, O_WRONLY, S_IRUSR | S_IWUSR);

    fd_write(fifo_out, buffer, (strlen(buffer) + 1));
    fd_close(fifo_out);
}

/*****************************************************
 * Thread Handler function. **************************
 *****************************************************/
void *thread_handler(void *arg_ptr) {
    int operacao, idConta, valor, idContaDestino, saldo, terminal_pid;
    char buffer[BUFFER_SIZE];

    while(1) {

        semaphore_wait(&read_sem); /* waits for the next addition to the buffer */

        mutex_lock(&buffer_ctrl);

        operacao = cmd_buffer[buff_read_idx].operacao;
        idConta = cmd_buffer[buff_read_idx].idConta;
        idContaDestino = cmd_buffer[buff_read_idx].idContaDestino;
        valor = cmd_buffer[buff_read_idx].valor;
        terminal_pid = cmd_buffer[buff_read_idx].pid;

        buff_read_idx = (buff_read_idx + 1) % CMD_BUFFER_DIM;

        mutex_unlock(&buffer_ctrl);

        semaphore_post(&write_sem); /* releases buffer position */

        /* comando Creditar */
        if (operacao == KEY_CREDITAR){
            if (creditar(idConta, valor) < 0)
                snprintf(buffer, BUFFER_SIZE, "%s(%d, %d): Erro\n\n", COMANDO_CREDITAR, idConta, valor);                
            else
                snprintf(buffer, BUFFER_SIZE, "%s(%d, %d): OK\n\n", COMANDO_CREDITAR, idConta, valor);
        }
        /* comando Debitar */
        else if (operacao == KEY_DEBITAR){
            if (debitar(idConta, valor) )
                snprintf(buffer, BUFFER_SIZE, "%s(%d, %d): Erro\n\n", COMANDO_DEBITAR, idConta, valor);
            else
                snprintf(buffer, BUFFER_SIZE, "%s(%d, %d): OK\n\n", COMANDO_DEBITAR, idConta, valor);
        }
        /* comando lerSaldo */
        else if (operacao == KEY_LER_SALDO){
            saldo = lerSaldo(idConta);
            if (saldo < 0)
                snprintf(buffer, BUFFER_SIZE, "%s(%d): Erro.\n\n", COMANDO_LER_SALDO, idConta);
            else
                snprintf(buffer, BUFFER_SIZE, "%s(%d): O saldo da conta é %d.\n\n", COMANDO_LER_SALDO, idConta, saldo);
        }
        /* comando transferir */
        else if (operacao == KEY_TRANSFERIR){
            if (transferir(idConta, idContaDestino, valor))
                snprintf(buffer, BUFFER_SIZE, "Erro ao transferir valor da conta %d para a conta %d\n", idConta, idContaDestino);
            else
                snprintf(buffer, BUFFER_SIZE, "%s(%d, %d, %d): OK\n\n", COMANDO_TRANSFERIR, idConta, idContaDestino, valor);
        }
        /* comando Sair */
        else if (operacao == KEY_SAIR) {
            pthread_exit(NULL);
        }

        /* sends answer back to correct pipe */
        SendsThroughPipe(buffer, terminal_pid);

        /* Signal for condition variable (comando simular) */
        mutex_lock(&no_command_ctrl_mutex);
        num_orders--;
        cond_signal(&no_command_ctrl);
        mutex_unlock(&no_command_ctrl_mutex);
    }
}

/* Add to buffer */
void AddBuffer(comando_t cmd) {
    semaphore_wait(&write_sem); /* Waits for space to write in the buffer */

    cmd_buffer[buff_write_idx].operacao = cmd.operacao;
    cmd_buffer[buff_write_idx].idConta = cmd.idConta;
    cmd_buffer[buff_write_idx].idContaDestino = cmd.idContaDestino;
    cmd_buffer[buff_write_idx].valor = cmd.valor;
    cmd_buffer[buff_write_idx].pid = cmd.pid;    
    buff_write_idx = (buff_write_idx + 1) % CMD_BUFFER_DIM;

    mutex_lock(&no_command_ctrl_mutex);
    num_orders++;
    mutex_unlock(&no_command_ctrl_mutex);

    semaphore_post(&read_sem); /* Points out buffer write */
}

/* ----- Destroy Resources ----- */
void destroySharedResources(){
    int i = 0;

    mutex_destroy(&buffer_ctrl);
    mutex_destroy(&no_command_ctrl_mutex);

    for(i = 0; i < NUM_CONTAS; i++)
        mutex_destroy(&contas_ctrl[i]);

    semaphore_destroy(&write_sem);
    semaphore_destroy(&read_sem);

    cond_destroy(&no_command_ctrl);
}