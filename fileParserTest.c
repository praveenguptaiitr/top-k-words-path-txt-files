#include "fileparser.h"

int main(int argc, char** argv)
{
	if(argc != 3)
	{
	printf("\nfunction: [%s] line: [%d] fileparser usage: ./main <filename.txt> <num of most freq words>\n", __func__, __LINE__);
		return -1;
	}
	printf("\nfunction: [%s] line: [%d] arguments passed: [%s] [%s] [%s]\n\n",  __func__, __LINE__, argv[0], argv[1], argv[2]);

	//int k = 10;
	//FILE *fp = fopen("./1.txt","r");
	FILE *fp = fopen(argv[1],"r");
	int k = atoi(argv[2]);
	if(fp == NULL)
	{
		printf("\nFile: [%s] doesn't exist\n", argv[1]);
		return -1;
	}
	else
		printKMostFreq(fp, argv[1], k);
	fclose(fp);
	return 0;
}

