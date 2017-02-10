/*
// Auxiliar functions
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
//                Grupo 84
*/
/*****************************************************
 * Header files. *************************************
 *****************************************************/
/* - C/POSIX library headers */
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/stat.h>

/* - Local headers - */
#include "helpers.h"
#include "shared.h"

/*****************************************************
 * Helper functions **********************************
 *****************************************************/

 /* ----- Pthread ----- */

/* Pthread Create */
void pcreate(pthread_t *thread, void* (*start_routine)(void*), void *arg) {
    if(pthread_create(thread, NULL, start_routine, arg) != 0)
        handle_error("pcreate: pthread_create");
}

/* ----- Mutexs ----- */

/* Mutex init */
void mutex_init(pthread_mutex_t *mutex) {
    if(pthread_mutex_init(mutex, NULL) != 0)
        handle_error("mutex_init: pthread_mutex_init");
}

/* Mutex lock */
void mutex_lock(pthread_mutex_t *mutex) {
    if(pthread_mutex_lock(mutex) != 0){
        fprintf(stderr, "Error in pthread_mutex_lock()\n");
        exit(EXIT_FAILURE);
    }
}

/* Mutex unlock */
void mutex_unlock(pthread_mutex_t *mutex) {
    if(pthread_mutex_unlock(mutex) != 0){
        fprintf(stderr, "Error in pthread_mutex_unlock()\n");
        exit(EXIT_FAILURE);
    }
}

/* Mutex lock with condition*/
void mutex_lock_cond(pthread_mutex_t *mutex, int id1, int id2) {
    if(id1 < id2){
        mutex_lock(&mutex[id1-1]);
        mutex_lock(&mutex[id2-1]);
    }
    else{
        mutex_lock(&mutex[id2-1]);
        mutex_lock(&mutex[id1-1]);
    }
}

/* Mutex unlock with condition*/
void mutex_unlock_cond(pthread_mutex_t *mutex, int id1, int id2) {
    if(id1 < id2){
        mutex_unlock(&mutex[id1-1]);
        mutex_unlock(&mutex[id2-1]);
    }
    else{
        mutex_unlock(&mutex[id2-1]);
        mutex_unlock(&mutex[id1-1]);
    }
}

/* Mutex Destroy */
void mutex_destroy(pthread_mutex_t *mutex) {
    if( pthread_mutex_destroy(mutex) != 0){
        fprintf(stderr, "Error in pthread_mutex_destroy()\n");
        exit(EXIT_FAILURE);
    }
}

/* ----- Semaphores ----- */

/* Semaphore init */
void semaphore_init(sem_t *sem, int pshared, unsigned int value){
    if ( sem_init(sem, pshared , value ) < 0 )
        handle_error("Semaphore");
}

/* Semaphore wait */
void semaphore_wait(sem_t *sem) {
    if(sem_wait(sem) != 0)
        handle_error("SemaphoreWait");
}

/* Semaphore post */
void semaphore_post(sem_t *sem) {
    if(sem_post(sem) != 0)
        handle_error("SemaphorePost");
}

/* Semaphore Destroy */
void semaphore_destroy(sem_t *sem){
    if(sem_destroy(sem) != 0)
        handle_error("SemaphoreDestroy");
}

/* ----- Condition Variables ----- */

/* Condition variable init */
void cond_init(pthread_cond_t *cv) {
    if(pthread_cond_init(cv, NULL) != 0)
        handle_error("cond_init: pthread_cond_init");
}

/* Condition variable wait */
void cond_wait(pthread_cond_t* condition, pthread_mutex_t* mutex) {
    if (pthread_cond_wait(condition, mutex) != 0)
        handle_error("cond_wait: pthread_cond_wait");
}

/* Condition variable signal */
void cond_signal(pthread_cond_t* condition) {
    if (pthread_cond_signal(condition) != 0)
        handle_error("cond_signal: pthread_cond_signal");
}

/* Condition variable destroy */
void cond_destroy(pthread_cond_t* condition){
    if(pthread_cond_destroy(condition) != 0)
        handle_error("cond_destroy: pthread_cond_destroy");
}

/* ----- Pipes ----- */
void new_pipe(const char *pathname, mode_t mode){
    if(mkfifo(pathname, mode) < 0)
        handle_error("new_pipe: mkfifo");
}

/* ----- I/O & File system ----- */

/* Opens file */
FILE *f_open(const char *path, const char *mode){
    FILE *fp;
    if((fp = fopen(path, mode)) == NULL)
        handle_error("f_open: open");
    return fp;
}

/* Opens file (version 2) receives file descriptor */
int fd_open(const char *pathname, int flags, mode_t mode){
    int fd;
    if((fd = open(pathname, flags, mode)) < 0)
        handle_error("fd_open: open");
    return fd;
}

/* Closes file */
void f_close(FILE *stream){
    if(fclose(stream) < 0)
        handle_error("f_close: close");
}

/* Closes file (file descriptor) */
void fd_close(int fd){
    if(close(fd) < 0)
        handle_error("fd_close: close");
}

/* Write to file (file descriptor) */
void fd_write(int fd, const void *buf, size_t count){
    if(write(fd, buf, count) < 0)
        handle_error("fd_write: write");
}

/* Reads file (file descriptor) */
void fd_read(int fd, void *buf, size_t count){
    if(read(fd, buf, count) < 0)
        handle_error("fd_read: read");    
}

void fd_dup(int oldfd){
    if(dup(oldfd) < 0)
        handle_error("fd_read: read");
}

/* EXTRAS */
/* Handles a signal sent when the terminal writes to a broken pipe */
void handle_SIGPIPE(int signum) {
  fprintf(stderr, "Received SIGPIPE: Broken fifo. Exiting..\n");
  exit(signum);
}