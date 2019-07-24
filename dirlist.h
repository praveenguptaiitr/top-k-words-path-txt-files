#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "threadpool.h"
#include "fileparser.h"
#include "maxheap.h"

//#define K_MOST_FREQUENT_WORDS_FROM_FILE 10

void listDir(char *filepath);
int isTxtFile(char *filename);

void parse_file(void* argp);


