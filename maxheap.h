#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "logdef.h"

struct maxHeapNode
{
	int freq;
	char *word;
};

struct maxHeapArray
{
	struct maxHeapNode *maxH;
	int curr_index;
	int total_size;
};


void displayMaxHeapArray(struct maxHeapArray maxHArr, int k);
int checkRepeatingWordMaxHeap(struct maxHeapArray maxHArr, char* word, int start, int end);
void maxHeapSort(struct maxHeapArray maxHArr);
void maxHeapify(struct maxHeapArray maxHArr, int size, int index);
void swapMaxHeapNodes(struct maxHeapArray maxHArr, int index1, int index2);

void destroy_maxHeap(struct maxHeapArray maxHArr);

