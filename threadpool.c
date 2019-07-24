#include "threadpool.h"

static volatile int threads_keepalive;
static volatile int threads_working;
static volatile int jobqueue_not_empty;

threadpool* threadpool_init(int num_threads)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	threadpool *thpool;
	threads_keepalive =1;

	thpool = (threadpool*)malloc(sizeof(threadpool));

	if(thpool == NULL)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] thread pool null value\n", pthread_self(), __func__, __LINE__);
		return NULL;	
	}

	thpool->num_thread_working = 0;
	thpool->num_thread_alive = 0;

	jobqueue_init(&(thpool->jobq));

	thpool->pthread = (pthread_t*)malloc(sizeof(pthread_t)*num_threads);

	if(thpool->pthread == NULL)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] thread pool pthread ptr null value\n", pthread_self(), __func__, __LINE__);
		return NULL;	
	}
	pthread_mutex_init(&(thpool->thpool_mutex), NULL);

	int count;
	for(count=0;count<num_threads;count++)
	{
		thread_init(thpool, &(thpool->pthread[count]), count);
	}
	usleep(10000);
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	return thpool;
}

void thread_init(threadpool* thpool, pthread_t *pthread, int count)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	pthread_create(pthread, NULL, (void*)thread_do, (void*)thpool);
	pthread_detach(*pthread);
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
}

void* thread_do(void * ptr)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	threadpool* thpool = (threadpool*)ptr;

	if(thpool == NULL)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] thread pool null value\n", pthread_self(), __func__, __LINE__);
		return NULL;	
	}

	printf("thread: [%ld] function: [%s] line: [%d] alive now\n", pthread_self(), __func__,__LINE__);

	pthread_mutex_lock(&(thpool->thpool_mutex));
	thpool->num_thread_alive +=1;
	pthread_mutex_unlock(&(thpool->thpool_mutex));

	while(threads_keepalive)
	{
		if(jobqueue_not_empty && thpool->jobq.has_jobs)
		{
			//printf("thread: [%ld] function: [%s] line: [%d] jobqueue_not_empty: [%d] jobqueue has_jobs: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqueue_not_empty, thpool->jobq.has_jobs, thpool->jobq.len);
			pthread_mutex_lock(&(thpool->thpool_mutex));
			thpool->num_thread_working += 1;
			threads_working = thpool->num_thread_working;
			pthread_mutex_unlock(&(thpool->thpool_mutex));
			//printf("thread: [%ld] function: [%s] line: [%d] working started\n", pthread_self(), __func__, __LINE__);
			//printf("thread: [%ld] function: [%s] line: [%d] total number of working threads: [%d]\n", pthread_self(), __func__, __LINE__, thpool->num_thread_working);
			void *arg;
			void (*func)(void*);
			job* jobp = jobqueue_pull(&(thpool->jobq));
			//printf("thread: [%ld] function: [%s] line: [%d] jobp: [%p] \n", pthread_self(), __func__, __LINE__, jobp);
			if(jobp != NULL)
			{
				func = jobp->func;
				arg = jobp->arg;
				//printf("thread: [%ld] function: [%s] line: [%d] func: [%p], arg: [%s]\n", pthread_self(), __func__, __LINE__, func, (char*)arg);
			//printf("thread: [%ld] function: [%s] line: [%d] working started for file: [%s]\n", pthread_self(), __func__, __LINE__, (char*)arg);
				func(arg);
				free(jobp->arg);
				free(jobp);
			}
			pthread_mutex_lock(&(thpool->thpool_mutex));
			thpool->num_thread_working -= 1;
			threads_working = thpool->num_thread_working;
			pthread_mutex_unlock(&(thpool->thpool_mutex));
			//printf("thread: [%ld] function: [%s] line: [%d] working stopped\n", pthread_self(), __func__, __LINE__);
			//printf("thread: [%ld] function: [%s] line: [%d] total number of working threads: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, thpool->num_thread_working, thpool->jobq.len);
		}
	}
	pthread_mutex_lock(&(thpool->thpool_mutex));
	thpool->num_thread_alive -=1;
	pthread_mutex_unlock(&(thpool->thpool_mutex));
	printf("thread: [%ld] function: [%s] line: [%d] exiting now\n", pthread_self(), __func__, __LINE__);
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	return NULL;
}

void threadpool_add_work(threadpool* thpool, void (*funcp)(void*), void* arg)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	job *jobp;

	if(funcp == NULL || arg == NULL)
	{
		//printf("thread [%ld] function: [%s] line: [%d] funcp or arg null value\n", pthread_self(), __func__, __LINE__);
		return;
	}

	jobp = (job*)malloc(sizeof(job));

	if(jobp == NULL)
	{
		//printf("thread [%ld] function: [%s] line: [%d] jobp null value\n", pthread_self(), __func__, __LINE__);
		return;
	}
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	int sz = PATH_MAX;
	jobp->arg = malloc(sizeof(char) * sz);
	jobp->func = funcp;
	strcpy((char*)(jobp->arg), arg);
	jobp->next = jobp->prev = NULL;

	//printf("thread [%ld] function: [%s] line: [%d] jobp: [%p] funcp: [%p] arg: [%p] filepath: [%s]\n", pthread_self(), __func__, __LINE__, jobp, funcp, arg, (char*)(jobp->arg));

	jobqueue_push(&(thpool->jobq), jobp);
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
}

void jobqueue_init(jobqueue* jobqp)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	jobqp->len = 0;
	jobqp->front = NULL;
	jobqp->rear = NULL;
	jobqp->has_jobs = 0;

	pthread_mutex_init(&(jobqp->queue_mutex), NULL);
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);

}

void jobqueue_push(jobqueue* jobqp, job* jobp)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	pthread_mutex_lock(&(jobqp->queue_mutex));

	if(jobqp == NULL || jobp == NULL)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] jobqp: [%p] jobp: [%p]\n", pthread_self(), __func__, __LINE__, jobqp, jobp);
		return;
	}

	if(jobqp->len == 0)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] empty job queue\n", pthread_self(), __func__, __LINE__);
		jobqp->front = jobp;
		jobqp->rear = jobp;
	}
	else
	{
		//printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
		jobqp->rear->next = jobp;
		jobp->prev = jobqp->rear;
		jobqp->rear = jobp;
	}
	jobqp->len += 1;
	jobqp->has_jobs = 1;
	jobqueue_not_empty = 1;

	pthread_mutex_unlock(&(jobqp->queue_mutex));
	//printf("thread: [%ld] function: [%s] line: [%d] current job queue length: [%d] jobqueue_not_empty: [%d] jobp filepath: [%s]\n", pthread_self(), __func__, __LINE__, jobqp->len, jobqueue_not_empty, (char*)(jobp->arg));
	//printf("thread: [%ld] function: [%s] line: [%d] job added to queue, current queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);	

}

job* jobqueue_pull(jobqueue* jobqp)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	pthread_mutex_lock(&(jobqp->queue_mutex));

	job* jobp = NULL;
	jobp = jobqp->front;
	//printf("thread: [%ld] function: [%s] line: [%d] jobqp: [%p] jobp: [%p]\n", pthread_self(), __func__, __LINE__, jobqp, jobp);

	if(jobqp == NULL || jobp == NULL)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] jobqp: [%p] jobp: [%p]\n", pthread_self(), __func__, __LINE__, jobqp, jobp);
		pthread_mutex_unlock(&(jobqp->queue_mutex));
		return NULL;
	}

	if(jobqp->len == 0)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] empty job queue\n", pthread_self(), __func__, __LINE__);
		jobqueue_not_empty = 0;
	}
	else if(jobqp->len == 1)
	{
		//printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue before  pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
		jobqp->front = NULL;
		jobqp->rear = NULL;
		jobqp->len = 0;
		jobqp->has_jobs = 0;
		jobqueue_not_empty = 0;
		//printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue after pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
	}
	else
	{
		//printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue before pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
		jobqp->front = jobqp->front->next;
		jobqp->front->prev = NULL;
		jobqp->len -=1;
		jobqueue_not_empty = 1;
		//printf("thread: [%ld] function: [%s] line: [%d] non-empty job queue after pull length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);

	}
	//printf("thread: [%ld] function: [%s] line: [%d] jobp filepath: [%s]\n", pthread_self(), __func__, __LINE__, (char*)(jobp->arg));

	pthread_mutex_unlock(&(jobqp->queue_mutex));
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);

	//printf("thread: [%ld] function: [%s] line: [%d] job removed from queue, current queue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);

	return jobp;

}

void jobqueue_destroy(jobqueue * jobqp)
{
	//printf("thread: [%ld] function: [%s] line: [%d] jobqueue length: [%d]\n", pthread_self(), __func__, __LINE__, jobqp->len);
	while(jobqp->len>0)
	{
		job* jobp = jobqueue_pull(jobqp);
		free(jobp->arg);
		free(jobp);
	}
	jobqp->front = jobqp->rear = NULL;
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
}

void threadpool_destroy(threadpool* thpool)
{
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
	if(thpool == NULL)
		return;

	while((threads_working > 0) || (jobqueue_not_empty == 1))
	{
		//printf("thread: [%ld] function: [%s] line: [%d] threads_keepalive: [%d] jobqueue_not_empty: [%d] threads_working: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, threads_keepalive, jobqueue_not_empty, threads_working, thpool->jobq.len);
		usleep(100);
	}

	//printf("thread: [%ld] function: [%s] line: [%d] threads_keepalive: [%d] jobqueue_not_empty: [%d] threads_working: [%d] job queue length: [%d]\n", pthread_self(), __func__, __LINE__, threads_keepalive, jobqueue_not_empty, threads_working, thpool->jobq.len);

	threads_keepalive = 0;
	//printf("thread: [%ld] function: [%s] line: [%d] threads_keepalive: [%d] jobqueue_not_empty: [%d] threads_working: [%d]\n", pthread_self(), __func__, __LINE__, threads_keepalive, jobqueue_not_empty, threads_working);

	jobqueue_destroy(&(thpool->jobq));

	free(thpool->pthread);
	free(thpool);
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
}



