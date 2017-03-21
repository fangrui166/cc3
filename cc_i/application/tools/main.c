#include<stdio.h>
#include<stdlib.h>
int main(int argc, char * argv[])
{
	if(argc < 3){
		printf("input arg error\n");
		return -1;
	}
	char * file_name = argv[1];
	int file_size = atoi(argv[2]);
	//printf("file name:%s, size:%d\n",file_name, file_size);	
	FILE *f = fopen(file_name,"w");
	fseek(f, file_size-1, SEEK_CUR);
	fputc(0,f);
	return 0;
}
