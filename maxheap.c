#include "maxheap.h"

void displayMaxHeapArray(struct maxHeapArray maxHArr, int k)
{
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] enter\n", pthread_self(), __func__, __LINE__);
#endif
	int count;

	if(maxHArr.total_size == 0)
		return;
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("thread: [%ld] function: [%s] line: [%d] Max Heap Array:\n\n\n", pthread_self(), __func__, __LINE__);
#endif
	if(k > maxHArr.total_size)
		k = maxHArr.total_size;

	int printCount = 0;
#ifdef _ENABLE_LOGS_INFO_RESULT_
	printf("%s %d\n", maxHArr.maxH[maxHArr.total_size-1].word, maxHArr.maxH[maxHArr.total_size-1].freq);
#endif
	printCount = 1;
	for(count=maxHArr.total_size-2; count>=0; count--)
	{
		if(checkRepeatingWordMaxHeap(maxHArr, maxHArr.maxH[count].word, count+1, maxHArr.total_size-1) == 0)
		{
#ifdef _ENABLE_LOGS_INFO_RESULT_
			printf("%s %d\n", maxHArr.maxH[count].word, maxHArr.maxH[count].freq);
#endif
			printCount++;
			if(printCount == k)
				break;
		}
		
		else
		{
#ifdef _ENABLE_LOGS_INFO_RESULT_ALREADY_PRINTED_
			printf("\n\nthread: [%ld] function: [%s] line: [%d] already printed word: [%s]\n", pthread_self(), __func__, __LINE__, maxHArr.maxH[count].word);
#endif
		}		
	}
#ifdef _ENABLE_LOGS_VERBOSE_
	printf("\n\nthread: [%ld] function: [%s] line: [%d] exit\n", pthread_self(), __func__, __LINE__);
#endif
}

int checkRepeatingWordMaxHeap(struct maxHeapArray maxHArr, char* word, int start, int end)
{
	int count;
	for(count=start; count<=end; count++)
	{
		if(strcmp(maxHArr.maxH[count].word, word) == 0)
			return 1;
	}
	return 0;
}

void maxHeapSort(struct maxHeapArray maxHArr)
{
	int count;
	int size = maxHArr.total_size;

	//build heap
	for(count = size/2 -1; count>=0; count--)
	{
		maxHeapify(maxHArr, size, count);
	}

	//extract one element from top and put it end
	for(count = size -1; count>=0; count --)
	{
		swapMaxHeapNodes(maxHArr, 0, count);
		maxHeapify(maxHArr, count, 0);		
	}
}

void maxHeapify(struct maxHeapArray maxHArr, int size, int index)
{
	int largest = index;
	int l = 2*index+1;
	int r = 2*index+2;

	if( l < size && maxHArr.maxH[l].freq > maxHArr.maxH[largest].freq)
		largest = l;
	if( r < size && maxHArr.maxH[r].freq > maxHArr.maxH[largest].freq)
		largest = r;

	if(largest != index)
	{
		swapMaxHeapNodes(maxHArr, index, largest);
		maxHeapify(maxHArr, size, largest);
	}
}

void swapMaxHeapNodes(struct maxHeapArray maxHArr, int index1, int index2)
{
	struct maxHeapNode temp;
	temp = maxHArr.maxH[index1];
	maxHArr.maxH[index1] = maxHArr.maxH[index2];
	maxHArr.maxH[index2] = temp;
}

void destroy_maxHeap(struct maxHeapArray maxHArr)
{
	int count;
	for(count=0; count<maxHArr.total_size; count++)
	{
		free(maxHArr.maxH[count].word);
	}
	free(maxHArr.maxH);
}



