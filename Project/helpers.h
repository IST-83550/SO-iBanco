/*
// Auxiliar functions
// Sistemas Operativos, DEI/IST/ULisboa 2016-17
//                Grupo 84
*/

#ifndef HELPERS_H
#define HELPERS_H

/*****************************************************
 * Header files. *************************************
 *****************************************************/
 
#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

/* Pthread */
void pcreate(pthread_t *thread, void* (*start_routine)(void*), void *arg);

/* Mutexs */
void mutex_init(pthread_mutex_t *mutex);
void mutex_lock(pthread_mutex_t *mutex);
void mutex_unlock(pthread_mutex_t *mutex);
void mutex_lock_cond(pthread_mutex_t *mutex, int id1, int id2);
void mutex_unlock_cond(pthread_mutex_t *mutex, int id1, int id2);
void mutex_destroy(pthread_mutex_t *mutex);

/* Semaphores */
void semaphore_init(sem_t *sem, int pshared, unsigned int value);
void semaphore_wait(sem_t *sem);
void semaphore_post(sem_t *sem);
void semaphore_destroy(sem_t *sem);

/* Condition Variables */
void cond_init(pthread_cond_t *cv);
void cond_wait(pthread_cond_t* condition, pthread_mutex_t* mutex);
void cond_signal(pthread_cond_t* condition);
void cond_destroy(pthread_cond_t* condition);

/* Pipes */
void new_pipe(const char *pathname, mode_t mode);

/* I/O & File system */
/* (file pointer version) */
FILE *f_open(const char *path, const char *mode);
void f_close(FILE *stream);
/* (file descriptor version) */
int fd_open(const char *pathname, int flags, mode_t mode);
void fd_close(int fd);
void fd_write(int fd, const void *buf, size_t count);
void fd_read(int fd, void *buf, size_t count);
void fd_dup(int oldfd);

/* EXTRAS */
void handle_SIGPIPE(int signum);

#endif /* __HELPERS_H__ */
