#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "logdef.h"

typedef struct job
{
	struct job* next;
	struct job* prev;
	void (*func)(void* arg);
	void *arg;
} job;

typedef struct jobqueue
{
	job *front;
	job *rear;
	int len;
	pthread_mutex_t queue_mutex;
} jobqueue;

typedef struct threadpool
{
	jobqueue jobq;
	int num_thread_working;
	pthread_mutex_t thpool_mutex;
	pthread_t *pthread;
} threadpool;


threadpool* threadpool_init(int num_threads);
void thread_init(threadpool* thpool, pthread_t *pthread, int count);
void* thread_do(void * ptr);
void threadpool_add_work(threadpool* thpool, void (*funcp)(void*), void* arg);
void jobqueue_init(jobqueue* jobqp);
void jobqueue_push(jobqueue* jobqp, job* jobp);
job* jobqueue_pull(jobqueue* jobqp);
void jobqueue_destroy(jobqueue * jobqp);
void threadpool_destroy(threadpool* thpool);





