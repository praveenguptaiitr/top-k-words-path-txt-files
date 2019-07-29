#include "threadpool.h"

static volatile int threads_keepalive;
static volatile int threads_working;
static volatile int jobqueue_not_empty;

threadpool* threadpool_init(int num_threads)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	threadpool *thpool;
	threads_keepalive =1;

	thpool = (threadpool*)malloc(sizeof(threadpool));

	if(thpool == NULL)
	{
#ifdef _ENABLE_LOGS_THREAD_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] thread pool null value\n", pthread_self(), __func__, __LINE__);
#endif
		return NULL;	
	}

	thpool->num_thread_working = 0;

	jobqueue_init(&(thpool->jobq));

	thpool->pthread = (pthread_t*)malloc(sizeof(pthread_t)*num_threads);

	if(thpool->pthread == NULL)
	{
#ifdef _ENABLE_LOGS_THREAD_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] thread pool pthread ptr null value\n", pthread_self(), __func__, __LINE__);
#endif
		return NULL;	
	}
	pthread_mutex_init(&(thpool->thpool_mutex), NULL);

	int count;
	for(count=0;count<num_threads;count++)
	{
		thread_init(thpool, &(thpool->pthread[count]), count);
	}
	usleep(10000);
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	return thpool;
}

void thread_init(threadpool* thpool, pthread_t *pthread, int count)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	pthread_create(pthread, NULL, (void*)thread_do, (void*)thpool);
	pthread_detach(*pthread);
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
}

void* thread_do(void * ptr)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	threadpool* thpool = (threadpool*)ptr;

	if(thpool == NULL)
	{
#ifdef _ENABLE_LOGS_THREAD_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] thread pool null value\n", pthread_self(), __func__, __LINE__);
#endif
		return NULL;	
	}
#ifdef _ENABLE_LOGS_THREAD_INFO_
	printf("thread: [%ld] function: [%s] line: [%d] thread alive now\n", pthread_self(), __func__,__LINE__);
#endif
	while(threads_keepalive)
	{
		if(jobqueue_not_empty/* && thpool->jobq.has_jobs*/)
		{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
			printf("thread: [%ld] function: [%s] line: [%d] jobqueue_not_empty: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqueue_not_empty, thpool->jobq.len);
#endif
			pthread_mutex_lock(&(thpool->thpool_mutex));
			thpool->num_thread_working += 1;
			threads_working = thpool->num_thread_working;
			pthread_mutex_unlock(&(thpool->thpool_mutex));
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
			printf("thread: [%ld] function: [%s] line: [%d] working started\n", pthread_self(), __func__, __LINE__);
			printf("thread: [%ld] function: [%s] line: [%d] total number of working threads: [%d]\n", pthread_self(), __func__, __LINE__, thpool->num_thread_working);
#endif
			void *arg;
			void (*func)(void*);
			job* jobp = jobqueue_pull(&(thpool->jobq));
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
			printf("thread: [%ld] function: [%s] line: [%d] jobp: [%p] \n", pthread_self(), __func__, __LINE__, jobp);
#endif
			if(jobp != NULL)
			{
				func = jobp->func;
				arg = jobp->arg;
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
				printf("thread: [%ld] function: [%s] line: [%d] func: [%p], arg: [%s]\n", pthread_self(), __func__, __LINE__, func, (char*)arg);
				printf("thread: [%ld] function: [%s] line: [%d] working started for file: [%s]\n", pthread_self(), __func__, __LINE__, (char*)arg);
#endif
				func(arg);
				free(jobp->arg);
				free(jobp);
			}
			pthread_mutex_lock(&(thpool->thpool_mutex));
			thpool->num_thread_working -= 1;
			threads_working = thpool->num_thread_working;
			pthread_mutex_unlock(&(thpool->thpool_mutex));
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
			printf("thread: [%ld] function: [%s] line: [%d] working stopped\n", pthread_self(), __func__, __LINE__);
			printf("thread: [%ld] function: [%s] line: [%d] total number of working threads: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, thpool->num_thread_working, thpool->jobq.len);
#endif
		}
	}
	pthread_mutex_lock(&(thpool->thpool_mutex));
	pthread_mutex_unlock(&(thpool->thpool_mutex));
#ifdef _ENABLE_LOGS_THREAD_INFO_
	printf("thread: [%ld] function: [%s] line: [%d] thread exiting now\n", pthread_self(), __func__, __LINE__);
#endif
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	return NULL;
}

void threadpool_add_work(threadpool* thpool, void (*funcp)(void*), void* arg)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	job *jobp;

	if(funcp == NULL || arg == NULL)
	{
#ifdef _ENABLE_LOGS_THREAD_ERR_
		printf("thread [%ld] function: [%s] line: [%d] funcp or arg null value\n", pthread_self(), __func__, __LINE__);
#endif
		return;
	}

	jobp = (job*)malloc(sizeof(job));

	if(jobp == NULL)
	{
#ifdef _ENABLE_LOGS_THREAD_ERR_
		printf("thread [%ld] function: [%s] line: [%d] jobp null value\n", pthread_self(), __func__, __LINE__);
#endif
		return;
	}
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	int sz = PATH_MAX;
	jobp->arg = malloc(sizeof(char) * sz);
	jobp->func = funcp;
	strcpy((char*)(jobp->arg), arg);
	jobp->next = jobp->prev = NULL;

#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread [%ld] function: [%s] line: [%d] jobp: [%p] funcp: [%p] arg: [%p] filepath: [%s]\n", pthread_self(), __func__, __LINE__, jobp, funcp, arg, (char*)(jobp->arg));
#endif
	jobqueue_push(&(thpool->jobq), jobp);
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_	
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
}

void jobqueue_init(jobqueue* jobqp)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	jobqp->len = 0;
	jobqp->front = NULL;
	jobqp->rear = NULL;

	pthread_mutex_init(&(jobqp->queue_mutex), NULL);
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
}

void jobqueue_push(jobqueue* jobqp, job* jobp)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	pthread_mutex_lock(&(jobqp->queue_mutex));

	if(jobqp == NULL || jobp == NULL)
	{
#ifdef _ENABLE_LOGS_THREAD_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] jobqp: [%p] jobp: [%p]\n", pthread_self(), __func__, __LINE__, jobqp, jobp);
#endif
		return;
	}

	if(jobqp->len == 0)
	{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] empty job queue\n", pthread_self(), __func__, __LINE__);
#endif
		jobqp->front = jobp;
		jobqp->rear = jobp;
	}
	else
	{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
#endif
		jobqp->rear->next = jobp;
		jobp->prev = jobqp->rear;
		jobqp->rear = jobp;
	}
	jobqp->len += 1;
	jobqueue_not_empty = 1;

	pthread_mutex_unlock(&(jobqp->queue_mutex));
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] jobqueue_not_empty: [%d] jobp filepath: [%s]\n", pthread_self(), __func__, __LINE__, jobqueue_not_empty, (char*)(jobp->arg));
	printf("thread: [%ld] function: [%s] line: [%d] job added to queue, current queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);	
#endif
}

job* jobqueue_pull(jobqueue* jobqp)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	pthread_mutex_lock(&(jobqp->queue_mutex));

	job* jobp = NULL;
	jobp = jobqp->front;
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] jobqp: [%p] jobp: [%p]\n", pthread_self(), __func__, __LINE__, jobqp, jobp);
#endif
	if(jobqp == NULL || jobp == NULL)
	{
#ifdef _ENABLE_LOGS_THREAD_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] jobqp: [%p] jobp: [%p]\n", pthread_self(), __func__, __LINE__, jobqp, jobp);
#endif
		pthread_mutex_unlock(&(jobqp->queue_mutex));
		return NULL;
	}

	if(jobqp->len == 0)
	{
#ifdef _ENABLE_LOGS_THREAD_INFO_
		printf("thread: [%ld] function: [%s] line: [%d] empty job queue\n", pthread_self(), __func__, __LINE__);
#endif
		jobqueue_not_empty = 0;
	}
	else if(jobqp->len == 1)
	{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue before  pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
#endif
		jobqp->front = NULL;
		jobqp->rear = NULL;
		jobqp->len = 0;
		jobqueue_not_empty = 0;
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue after pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
#endif
	}
	else
	{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue before pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
#endif
		jobqp->front = jobqp->front->next;
		jobqp->front->prev = NULL;
		jobqp->len -=1;
		jobqueue_not_empty = 1;
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue after pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
#endif
	}
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] jobp filepath: [%s]\n", pthread_self(), __func__, __LINE__, (char*)(jobp->arg));
#endif
	pthread_mutex_unlock(&(jobqp->queue_mutex));
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);

	printf("thread: [%ld] function: [%s] line: [%d] job removed from queue, current queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
#endif
	return jobp;

}

void jobqueue_destroy(jobqueue * jobqp)
{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] jobqueue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
#endif
	while(jobqp->len>0)
	{
		job* jobp = jobqueue_pull(jobqp);
		free(jobp->arg);
		free(jobp);
	}
	jobqp->front = jobqp->rear = NULL;
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
}

void threadpool_destroy(threadpool* thpool)
{
#ifdef _ENABLE_LOGS_THREAD_INFO_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	if(thpool == NULL)
		return;

	while((threads_working > 0) || (jobqueue_not_empty == 1))
	{
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] threads_keepalive: [%d] jobqueue_not_empty: [%d] threads_working: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, threads_keepalive, jobqueue_not_empty, threads_working, thpool->jobq.len);
#endif
		usleep(100);
	}
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] threads_keepalive: [%d] jobqueue_not_empty: [%d] threads_working: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, threads_keepalive, jobqueue_not_empty, threads_working, thpool->jobq.len);
#endif
	threads_keepalive = 0;
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] threads_keepalive: [%d] jobqueue_not_empty: [%d] threads_working: [%d]\n", pthread_self(), __func__, __LINE__, threads_keepalive, jobqueue_not_empty, threads_working);
#endif
	jobqueue_destroy(&(thpool->jobq));

	free(thpool->pthread);
	free(thpool);
#ifdef _ENABLE_LOGS_THREAD_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
}



