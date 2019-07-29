#include "threadpool.h"
#include <time.h>

void thread_func(void* argp);

int main(int argc, char** argv)
{
	clock_t t1 = clock();
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] thread pool testing started\n", pthread_self(), __func__, __LINE__);
#endif

	if(argc != 2)
	{
#ifdef _ENABLE_LOGS_ERR_
	printf("thread: [%ld] function: [%s] line: [%d] threadpool usage: ./main <num of threads>\n", pthread_self(), __func__, __LINE__);
#endif
		return -1;
	}
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] arguments passed: [%s] [%s]\n", pthread_self(), __func__, __LINE__, argv[0], argv[1]);
#endif
	int num_thread = atoi(argv[1]);
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] number of threads: [%d]\n", pthread_self(), __func__, __LINE__, num_thread);
#endif

	if(num_thread < 1)
	{
#ifdef _ENABLE_LOGS_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] number of threads should be minimum 1 or more\n", pthread_self(), __func__, __LINE__);
#endif
		return -1;	
	}

	threadpool *thpoolp = NULL;

	thpoolp = threadpool_init(num_thread);

	threadpool_add_work(thpoolp, thread_func, "./1.txt");
	threadpool_add_work(thpoolp, thread_func, "./2.txt");
	threadpool_add_work(thpoolp, thread_func, "./3.txt");
	threadpool_add_work(thpoolp, thread_func, "./4.txt");

	threadpool_destroy(thpoolp);
	usleep(10000);
	clock_t t2 = clock();
	double time_taken = (double)(t2 - t1)/CLOCKS_PER_SEC;
#ifdef _ENABLE_LOGS_INFO_TIME_
	printf("\nthread: [%ld] function: [%s] line: [%d] Work Completed in seconds: [%f]\n", pthread_self(), __func__, __LINE__, time_taken);
#endif
	return 0;
}

void thread_func(void *argp)
{
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	FILE* fp;
	char* str = (char*)argp;
	if(str == NULL)
		return;
#ifdef _ENABLE_LOGS_INFO_
	printf("\n\nthread: [%ld] function: [%s] line: [%d] reading file: [%s] started\n\n", pthread_self(), __func__, __LINE__, str);
#endif
	fp = fopen(str, "r");
	if(fp == NULL)
	{
#ifdef _ENABLE_LOGS_ERR_
	printf("\n\nthread: [%ld] function: [%s] line: [%d] file: [%s] does not exist\n\n", pthread_self(), __func__, __LINE__, str);
#endif
		return;
	}
	char buf[256];
	while(fscanf(fp, "%s", buf) != EOF)
	{
		printf("[%s] ", buf);	
	}

	fclose(fp);
#ifdef _ENABLE_LOGS_INFO_
	printf("\n\nthread: [%ld] function: [%s] line: [%d] reading file: [%s] completed\n\n", pthread_self(), __func__, __LINE__, str);
#endif
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	return;
}


