#include "fileparser.h"

struct tNode* newTNode()
{
	struct tNode *trNode = (struct tNode*)malloc(sizeof(struct tNode));
	trNode->isEnd = 0;
	trNode->freq = 0;
	trNode->indexMH = -1;
	for(int cnt = 0; cnt < 36; cnt++)
		trNode->child[cnt] = NULL;

	return trNode;
}

struct mh* createMH(int size)
{
	struct mh* minH = (struct mh*)malloc(sizeof(struct mh));
	minH->capacity = size;
	minH->count = 0;
	minH->arr = (struct mhNode*)malloc(sizeof(struct mhNode)*size);
	return minH;
}

void swapMHNodes(struct mhNode* a, struct mhNode* b)
{
	struct mhNode temp = *a;
	*a = *b;
	*b = temp;
}

void minHeapify(struct mh* minH, int index)
{
	int left, right, smallest;
	left = 2*index +1;
	right = 2*index +2;
	smallest = index;

	if(left < minH->count &&
			minH->arr[left].freq < minH->arr[smallest].freq)
		smallest = left;
	if(right <minH->count &&
			minH->arr[right].freq < minH->arr[smallest].freq)
		smallest = right;
	if(smallest != index)
	{
		minH->arr[smallest].root->indexMH = index;
		minH->arr[index].root->indexMH = smallest;
		swapMHNodes(&minH->arr[smallest], &minH->arr[index]);

		minHeapify(minH, smallest);
	}
}

void buildMH(struct mh* minH)
{
	int len, cnt;
	len = minH->count -1;
	for(cnt = (len-1)/2; cnt>=0; cnt--)
		minHeapify(minH, cnt);
}

void tolowerStr(char* str, int len)
{
	int count;
	for(count=0;count<len;count++)
	{
		if(isalpha(str[count]))
			str[count] = tolower(str[count]);
	}
}

void insertInMH(struct mh* minH, struct tNode** root, char* word)
{
	if((*root)->indexMH != -1)
	{
		(minH->arr[(*root)->indexMH].freq)++;
		minHeapify(minH, (*root)->indexMH);
	}

	else if (minH->count < minH->capacity)
	{
		int count = minH->count;
		minH->arr[count].freq = (*root)->freq;
		minH->arr[count].word = (char*)malloc((strlen(word)+1)*sizeof(char));
		tolowerStr(word,strlen(word));
		strcpy(minH->arr[count].word,word);

		minH->arr[count].root = *root;
		(*root)->indexMH = minH->count;

		(minH->count)++;
		buildMH(minH);
	}

	else if((*root)->freq > minH->arr[0].freq)
	{
		minH->arr[0].root->indexMH = -1;
		minH->arr[0].root = *root;
		minH->arr[0].root->indexMH = 0;
		minH->arr[0].freq = (*root)->freq;

		free(minH->arr[0].word);
		minH->arr[0].word = (char*)malloc((strlen(word)+1)*sizeof(char));
		tolowerStr(word,strlen(word));
		strcpy(minH->arr[0].word, word);

		minHeapify(minH, 0);
	}
}	

void insertUtil(struct tNode**root, struct mh* minH, char* word, char* dupword)
{
	if(*root == NULL)
		*root = newTNode();

	if(*word != '\0')
		if(*word >= '0' && *word <='9')
			insertUtil(&((*root)->child[*word - '0' + 26]), minH, word+1, dupword);
		else
			insertUtil(&((*root)->child[tolower(*word) - 'a']), minH, word+1, dupword);
	else
	{
		if((*root)->isEnd)
			((*root)->freq)++;
		else
		{
			(*root)->isEnd =1;
			(*root)->freq = 1;
		}

		insertInMH(minH, root, dupword);
	}
}

void insertTrieAndMH(char* word, struct tNode **root, struct mh* minH)
{
	insertUtil(root, minH, word, word);
}

void displayMH(struct mh* minH)
{
#ifdef _ENABLE_LOGS_INFO_RESULT_FILE_PARSER_
	int count;
	for(count=0; count<minH->count; count++)
	{
		printf("%s %d\n", minH->arr[count].word, minH->arr[count].freq);
	}
	printf("\n");
#endif
}

void removeNonAlphaNumFromStr(char* buf)
{
	int len = strlen(buf);
	char temp[PATH_MAX] = {0};
	int temp_count = 0;
	int count;
#ifdef _ENABLE_LOGS_VERBOSE_FILE_PARSER_REMOVE_NON_ALPHA_NUMERIC_
	printf("non-modified string is: [%s]\n", buf);
#endif
	for(count=0; count<len; count++)
	{
		if(isalpha(buf[count]) || isdigit(buf[count]))
			temp[temp_count++] = buf[count];
	}
	temp[temp_count] = '\0';
	strcpy(buf, temp);
#ifdef _ENABLE_LOGS_VERBOSE_FILE_PARSER_REMOVE_NON_ALPHA_NUMERIC_
	printf("modified string is: [%s]\n", buf);
#endif
}

struct mh* printKMostFreq(FILE *fp, char* str, int k)
{
	struct mh* minH = createMH(k);

	struct tNode* root = NULL;
	char buf[PATH_MAX];

#ifdef _ENABLE_LOGS_VERBOSE_FILE_PARSER_
		printf("\n\ncontents of file [%s] =>\n\n", str);
#endif
	while(fscanf(fp, "%s", buf) != EOF)
	{
#ifdef _ENABLE_LOGS_VERBOSE_FILE_PARSER_
		printf("[%s] ", buf);
#endif
		removeNonAlphaNumFromStr(buf);
		if(buf[0] != '\0')
			insertTrieAndMH(buf, &root, minH);
	}
#ifdef _ENABLE_LOGS_VERBOSE_FILE_PARSER_
	printf("\n");
#endif
#ifdef _ENABLE_LOGS_INFO_RESULT_FILE_PARSER_
	printf("\nTop [%d] frequent words from file [%s] =>\n", k, str);
#endif
	displayMH(minH);

	return minH;
}

void destroy_minHeap(struct mh* minH)
{
	int count = 0;
	for(count=0; count< minH->count; count++)
		free(minH->arr[count].word);
	free(minH->arr);
	free(minH);
}



