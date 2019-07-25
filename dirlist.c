#include "dirlist.h"
#include <time.h>

threadpool *thpoolp = NULL;
int K_MOST_FREQUENT_WORDS_FROM_FILE = 10;

pthread_mutex_t mhcontainer_mutex;
struct maxHeapArray maxHArr;

int main(int argc, char** argv)
{
	clock_t t1 = clock();
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] enter\n", pthread_self(), __func__, __LINE__);
#endif
	int tcnt = 0;
	int opt;
	int index = 0;
	char filepath[PATH_MAX] = {0};

	pthread_mutex_init(&mhcontainer_mutex, NULL);
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] arg count: [%d]\n", pthread_self(), __func__, __LINE__,argc);
#endif
	if(argc != 4 && argc != 5)
	{
#ifdef _ENABLE_LOGS_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] ssfi usage: ./ssfi -t <num of threads> <file path> <num of top K frequents words>\n", pthread_self(), __func__, __LINE__);
#endif
		return -1;
	}

	while((opt = getopt(argc, argv, "t:")) != -1)
	{
		switch(opt)
		{
			case 't':
#ifdef _ENABLE_LOGS_INFO_
				printf("thread: [%ld] function: [%s] line: [%d] option: [%c] - \'t\' option value: [%s]\n", pthread_self(), __func__, __LINE__, opt, optarg);
#endif
				tcnt = atoi(optarg);
				break;
			case ':':
#ifdef _ENABLE_LOGS_VERBOSE_
				printf("thread: [%ld] function: [%s] line: [%d] \':\' option needs value\n", pthread_self(), __func__, __LINE__);
#endif
				break;
			case '?':
#ifdef _ENABLE_LOGS_VERBOSE_
				printf("thread: [%ld] function: [%s] line: [%d] unknown option: [%c]\n", pthread_self(), __func__, __LINE__, optopt);
#endif
				break;
		}
	}

	for(index = optind; index < argc ; index++)
	{
#ifdef _ENABLE_LOGS_INFO_
		printf("thread: [%ld] function: [%s] line: [%d] other arguments: [%s]\n", pthread_self(), __func__, __LINE__, argv[index]);
#endif
	}

	if(tcnt < 1)
	{
#ifdef _ENABLE_LOGS_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] number of threads should be minimum 1 or more\n", pthread_self(), __func__, __LINE__);
#endif
		return -1;	
	}

	int argvlen = strlen(argv[3]);
	int cnt;
	for(cnt = 0; cnt< (argvlen-1); cnt++)
	{
		filepath[cnt] = argv[3][cnt];
	}
	if((argv[3][cnt] == 0x2f) || (argv[3][cnt] == 0x5c))
	{
		filepath[cnt] = '\0';
	}
	else
	{
		filepath[cnt] = argv[3][cnt];
		filepath[cnt+1] = '\0';
	}

	if(argc == 5)
		K_MOST_FREQUENT_WORDS_FROM_FILE = atoi(argv[4]);
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] threadpool initialization\n", pthread_self(), __func__, __LINE__);
#endif
	thpoolp = threadpool_init(tcnt);
#ifdef _ENABLE_LOGS_INFO_
	printf("thread: [%ld] function: [%s] line: [%d] reading directory & files from path: [%s]\n", pthread_self(), __func__, __LINE__, filepath);
#endif
	listDir(filepath);
	usleep(1000);
	
	threadpool_destroy(thpoolp);
	usleep(10000);
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] max heap container  curr index: [%d] max heap container total size: [%d]\n", pthread_self(), __func__, __LINE__, maxHArr.curr_index, maxHArr.total_size);
#endif
	maxHeapSort(maxHArr);
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("\nthread: [%ld] function: [%s] line: [%d] Top [%d] frequent words from all the files from path: [%s]\n\n", pthread_self(), __func__, __LINE__, K_MOST_FREQUENT_WORDS_FROM_FILE, filepath);
#endif
#ifdef _ENABLE_LOGS_INFO_RESULT_
	printf("\nTop [%d] frequent words from all the files from path: [%s] ==>\n\n", K_MOST_FREQUENT_WORDS_FROM_FILE, filepath);
#endif
	displayMaxHeapArray(maxHArr, K_MOST_FREQUENT_WORDS_FROM_FILE);
	destroy_maxHeap(maxHArr);

	clock_t t2 = clock();
	double time_taken = (double)(t2 - t1)/CLOCKS_PER_SEC;
#ifdef _ENABLE_LOGS_INFO_TIME_
	printf("\nthread: [%ld] function: [%s] line: [%d] Work Completed in seconds: [%f]\n", pthread_self(), __func__, __LINE__, time_taken);
#endif
	return 0;
}


int isTxtFile(char *filename)
{
	int len = strlen(filename);
	if(len <= 4)
		return 0;
	if(strcmp(filename+len-4, ".txt") == 0)
		return 1;
	else
		return 0;
}

void listDir(char *filepath)
{
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	struct dirent *de;
	struct stat statbuf;
	int sz = PATH_MAX;
	char *origpath = (char*) malloc(sizeof(char) * sz);
#ifdef _ENABLE_LOGS_INFO_DIR_
	char *buf, *ptr;
	buf = (char*) malloc(sizeof(char) * sz);
	ptr = getcwd(buf, sz);
	printf("current working dir: [%s]\n", buf);
	printf("current file path: [%s]\n", filepath);
#endif
	DIR *dr = opendir(filepath);

	if(dr == NULL)
	{
#ifdef _ENABLE_LOGS_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] can't open the directory: [%s]\n", pthread_self(), __func__, __LINE__, filepath);
#endif
		return;
	}

	while((de = readdir(dr)) != NULL)
	{
		if(strcmp(".", de->d_name) == 0 ||
				strcmp("..", de->d_name) == 0)
		{
#ifdef _ENABLE_LOGS_INFO_DIR_
			printf("DIR: \'%s\'\n",de->d_name);
#endif
			continue;
		}
		strcpy(origpath, filepath);
#ifdef _ENABLE_LOGS_INFO_DIR_
		printf("orig path: [%s]\n", origpath);
#endif
		strcat(filepath, "/");
		strcat(filepath,de->d_name);
#ifdef _ENABLE_LOGS_INFO_DIR_
		printf("new file path: [%s]\n", filepath);
#endif
		lstat(filepath, &statbuf);
		if((statbuf.st_mode & S_IFMT) == S_IFDIR)
		{
#ifdef _ENABLE_LOGS_INFO_DIR_
			printf("DIR: [%s], statbuf.st_mode: [%X]\n", de->d_name, statbuf.st_mode & S_IFMT);
			printf("Go to dir and list files/dirs: [%s]\n", filepath);
#endif
#ifdef _ENABLE_LOGS_INFO_
			printf("thread: [%ld] function: [%s] line: [%d] DIRECTORY: [%s]\n", pthread_self(), __func__, __LINE__, filepath);
#endif
			listDir(filepath);
		}
		else
		{
#ifdef _ENABLE_LOGS_INFO_DIR_
			printf("FILE: [%s], PATH: [%s], statbuf.st_mode: [%X]\n", de->d_name, filepath, statbuf.st_mode & S_IFMT);
			printf("FILE: [%s]\n", filepath);
#endif
			if(isTxtFile(filepath))
			{
#ifdef _ENABLE_LOGS_INFO_
				printf("thread: [%ld] function: [%s] line: [%d] TXT FILE: [%s]\n", pthread_self(), __func__, __LINE__, filepath);
#endif
				threadpool_add_work(thpoolp, parse_file, filepath);
			}	
			else
			{
#ifdef _ENABLE_LOGS_INFO_
				printf("thread: [%ld] function: [%s] line: [%d] OTHER FILE: [%s]\n", pthread_self(), __func__, __LINE__, filepath);
#endif	
			}
		}
		strcpy(filepath, origpath);
	}
#ifdef _ENABLE_LOGS_INFO_DIR_
	free(buf);
#endif
	free(origpath);
	closedir(dr);
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	return;
}


void parse_file(void *argp)
{
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	FILE* fp;
	char* str = (char*)argp;
	struct mh* minH;

	if(str == NULL)
		return;
#ifdef _ENABLE_LOGS_INFO_
	printf("thread: [%ld] function: [%s] line: [%d] reading file: [%s] started\n", pthread_self(), __func__, __LINE__, str);
#endif
	fp = fopen(str, "r");
	if(fp == NULL)
	{
#ifdef _ENABLE_LOGS_ERR_
		printf("thread: [%ld] function: [%s] line: [%d] file: [%s] does not exist\n", pthread_self(), __func__, __LINE__, str);
#endif
		return;
	}

	minH = printKMostFreq(fp, str, K_MOST_FREQUENT_WORDS_FROM_FILE);
	pthread_mutex_lock(&mhcontainer_mutex);

	maxHArr.maxH = (struct maxHeapNode*)realloc(maxHArr.maxH, sizeof(struct maxHeapNode) * (maxHArr.total_size + minH->count));
	(maxHArr.total_size) += minH->count;

	int count =0;
	for(count =0; count< minH->count; count++)
	{
		maxHArr.maxH[maxHArr.curr_index].freq = minH->arr[count].freq;
		maxHArr.maxH[maxHArr.curr_index].word = (char*)malloc(sizeof(char) * (strlen(minH->arr[count].word)+1));
		strcpy(maxHArr.maxH[maxHArr.curr_index].word, minH->arr[count].word);
#ifdef _ENABLE_LOGS_VERBOSE_
		printf("thread: [%ld] function: [%s] line: [%d] minH word: [%s] max heap word: [%s] max heap current index: [%d]\n", pthread_self(), __func__, __LINE__,minH->arr[count].word, maxHArr.maxH[maxHArr.curr_index].word, maxHArr.curr_index);
#endif
		(maxHArr.curr_index) += 1;
	}

	destroy_minHeap(minH);

	pthread_mutex_unlock(&mhcontainer_mutex);

	fclose(fp);
#ifdef _ENABLE_LOGS_INFO_
	printf("thread: [%ld] function: [%s] line: [%d] reading file: [%s] completed\n", pthread_self(), __func__, __LINE__, str);
#endif
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);
#endif
	return;
}



