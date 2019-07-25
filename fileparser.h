#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include "logdef.h"

struct tNode
{
	int isEnd;
	int freq;
	int indexMH;
	struct tNode *child[36];
};

struct mhNode
{
	struct tNode* root;
	int freq;
	char *word;
};

struct mh
{
	int capacity;
	int count;
	struct mhNode* arr;
};

struct tNode* newTNode();
struct mh* createMH(int size);
void swapMHNodes(struct mhNode* a, struct mhNode* b);
void minHeapify(struct mh* minH, int index);
void buildMH(struct mh* minH);
void tolowerStr(char* str, int len);
void insertInMH(struct mh* minH, struct tNode** root, char* word);
void insertUtil(struct tNode**root, struct mh* minH, char* word, char* dupword);
void insertTrieAndMH(char* word, struct tNode **root, struct mh* minH);
void displayMH(struct mh* minH);
void removeNonAlphaNumFromStr(char* buf);

struct mh* printKMostFreq(FILE *fp, char* str, int k);
void destroy_minHeap(struct mh* minH);


