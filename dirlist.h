#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

#include "threadpool.h"
#include "fileparser.h"
#include "maxheap.h"
#include "logdef.h"

void listDir(char *filepath);
int isTxtFile(char *filename);

void parse_file(void* argp);


