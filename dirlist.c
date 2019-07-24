#include "dirlist.h"
#include <time.h>

threadpool *thpoolp = NULL;
int K_MOST_FREQUENT_WORDS_FROM_FILE = 10;

pthread_mutex_t mhcontainer_mutex;
struct maxHeapArray maxHArr;

int main(int argc, char** argv)
{
	clock_t t1 = clock();
	//printf("thread: [%ld] function: [%s] line: [%d] enter\n", pthread_self(), __func__, __LINE__);

	int tcnt = 0;
	int opt;
	int index = 0;
	char filepath[PATH_MAX] = {0};

	pthread_mutex_init(&mhcontainer_mutex, NULL);

	//printf("thread: [%ld] function: [%s] line: [%d] arg count: [%d]\n", pthread_self(), __func__, __LINE__,argc);

	if(argc != 4 && argc != 5)
	{
		printf("thread: [%ld] function: [%s] line: [%d] ssfi usage: ./ssfi -t <num of threads> <file path> <num of top K frequents words>\n", pthread_self(), __func__, __LINE__);
		return -1;
	}

	while((opt = getopt(argc, argv, "t:")) != -1)
	{
		switch(opt)
		{
			case 't':
				printf("thread: [%ld] function: [%s] line: [%d] option: [%c] - \'t\' option value: [%s]\n", pthread_self(), __func__, __LINE__, opt, optarg);

				tcnt = atoi(optarg);
				break;
			case ':':
				//printf("thread: [%ld] function: [%s] line: [%d] \':\' option needs value\n", pthread_self(), __func__, __LINE__);


				break;
			case '?':
				//printf("thread: [%ld] function: [%s] line: [%d] unknown option: [%c]\n", pthread_self(), __func__, __LINE__, optopt);

				break;
		}
	}

	for(index = optind; index < argc ; index++)
	{
		printf("thread: [%ld] function: [%s] line: [%d] other arguments: [%s]\n", pthread_self(), __func__, __LINE__, argv[index]);
	}

	if(tcnt < 1)
	{
		printf("thread: [%ld] function: [%s] line: [%d] number of threads should be minimum 1 or more\n", pthread_self(), __func__, __LINE__);
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

	//printf("thread: [%ld] function: [%s] line: [%d] threadpool initialization\n", pthread_self(), __func__, __LINE__);

	thpoolp = threadpool_init(tcnt);
	//sleep(2);

	printf("thread: [%ld] function: [%s] line: [%d] reading directory & files from path: [%s]\n", pthread_self(), __func__, __LINE__, filepath);

	listDir(filepath);
	usleep(1000);
	
	threadpool_destroy(thpoolp);
	usleep(10000);
	//printf("thread: [%ld] function: [%s] line: [%d] max heap container  curr index: [%d] max heap container total size: [%d]\n", pthread_self(), __func__, __LINE__, maxHArr.curr_index, maxHArr.total_size);

	maxHeapSort(maxHArr);
	//printf("\nthread: [%ld] function: [%s] line: [%d] Top [%d] frequent words from all the files from path: [%s]\n\n", pthread_self(), __func__, __LINE__, K_MOST_FREQUENT_WORDS_FROM_FILE, filepath);

	printf("\nTop [%d] frequent words from all the files from path: [%s] ==>\n\n", K_MOST_FREQUENT_WORDS_FROM_FILE, filepath);

	//displayMaxHeapArray(maxHArr, maxHArr.total_size);
	displayMaxHeapArray(maxHArr, K_MOST_FREQUENT_WORDS_FROM_FILE);
	destroy_maxHeap(maxHArr);

	clock_t t2 = clock();
	double time_taken = (double)(t2 - t1)/CLOCKS_PER_SEC;
	printf("\nthread: [%ld] function: [%s] line: [%d] Work Completed in seconds: [%f]\n", pthread_self(), __func__, __LINE__, time_taken);

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
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);

	struct dirent *de;
	struct stat statbuf;
	int sz = PATH_MAX;
	//char *buf, *ptr;
	//buf = (char*) malloc(sizeof(char) * sz);
	char *origpath = (char*) malloc(sizeof(char) * sz);
	//ptr = getcwd(buf, sz);
	//printf("current working dir: [%s]\n", buf);
	//printf("current file path: [%s]\n", filepath);
	DIR *dr = opendir(filepath);

	if(dr == NULL)
	{
		printf("thread: [%ld] function: [%s] line: [%d] can't open the directory: [%s]\n", pthread_self(), __func__, __LINE__, filepath);

		return;
	}

	while((de = readdir(dr)) != NULL)
	{
		if(strcmp(".", de->d_name) == 0 ||
				strcmp("..", de->d_name) == 0)
		{
			//printf("DIR: \'%s\'\n",de->d_name);
			continue;
		}
		strcpy(origpath, filepath);
		//printf("orig path: [%s]\n", origpath);
		strcat(filepath, "/");
		strcat(filepath,de->d_name);
		//printf("new file path: [%s]\n", filepath);
		lstat(filepath, &statbuf);
		if((statbuf.st_mode & S_IFMT) == S_IFDIR)
		{
			//printf("DIR: [%s], statbuf.st_mode: [%X]\n", de->d_name, statbuf.st_mode & S_IFMT);
			//printf("Go to dir and list files/dirs: [%s]\n", filepath);

			printf("thread: [%ld] function: [%s] line: [%d] DIRECTORY: [%s]\n", pthread_self(), __func__, __LINE__, filepath);

			listDir(filepath);
		}
		else
		{
			//printf("OTHER FILE: [%s], PATH: [%s], statbuf.st_mode: [%X]\n", de->d_name, filepath, statbuf.st_mode & S_IFMT);
			//printf("FILE: [%s]\n", filepath);
			if(isTxtFile(filepath))
			{
				printf("thread: [%ld] function: [%s] line: [%d] TXT FILE: [%s]\n", pthread_self(), __func__, __LINE__, filepath);


				threadpool_add_work(thpoolp, parse_file, filepath);
			}	
			else
			{
				printf("thread: [%ld] function: [%s] line: [%d] OTHER FILE: [%s]\n", pthread_self(), __func__, __LINE__, filepath);	
			}
		}
		strcpy(filepath, origpath);
	}

	//free(buf);
	free(origpath);
	closedir(dr);
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);

	return;
}


void parse_file(void *argp)
{
	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);

	FILE* fp;
	char* str = (char*)argp;
	struct mh* minH;

	if(str == NULL)
		return;

	printf("thread: [%ld] function: [%s] line: [%d] reading file: [%s] started\n", pthread_self(), __func__, __LINE__, str);

	fp = fopen(str, "r");
	if(fp == NULL)
	{
		printf("thread: [%ld] function: [%s] line: [%d] file: [%s] does not exist\n", pthread_self(), __func__, __LINE__, str);
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
		//printf("thread: [%ld] function: [%s] line: [%d] minH word: [%s] max heap word: [%s] max heap current index: [%d]\n", pthread_self(), __func__, __LINE__,minH->arr[count].word, maxHArr.maxH[maxHArr.curr_index].word, maxHArr.curr_index);

		(maxHArr.curr_index) += 1;
	}

	destroy_minHeap(minH);

	pthread_mutex_unlock(&mhcontainer_mutex);

	//sleep(1);

	fclose(fp);

	printf("thread: [%ld] function: [%s] line: [%d] reading file: [%s] completed\n", pthread_self(), __func__, __LINE__, str);

	//printf("thread: [%ld] function: [%s] line: [%d]\n", pthread_self(), __func__, __LINE__);

	return;
}



