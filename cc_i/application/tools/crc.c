#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define Poly 0xEDB88320L//CRC32标准
static uint32_t crc_tab32[256];//CRC查询表

static void init_crc32_tab(void);//生成CRC查询表
uint32_t get_crc32(uint32_t crcinit, uint8_t * bs, uint32_t bssize);//获得CRC
uint32_t GetFileCRC(FILE *fd, unsigned int *file_lenght);//获得文件CRC

static void init_crc32_tab( void ) 
{
	int i, j;
	uint32_t crc;
	
	for (i=0; i<256; i++)
	{
		crc = (unsigned long)i;
		for (j=0; j<8; j++) 
		{
			if ( crc & 0x00000001L )
				crc = ( crc >> 1 ) ^ Poly;
			else
				crc = crc >> 1;
		}
		crc_tab32[i] = crc;
	}
}

uint32_t get_crc32(uint32_t crcinit, uint8_t * bs, uint32_t bssize)
{
	uint32_t crc = crcinit^0xffffffff;
	
	init_crc32_tab();
	while(bssize--)
		crc=(crc >> 8)^crc_tab32[(crc & 0xff) ^ *bs++];
	
	return crc ^ 0xffffffff;
}

#define  size (16 * 1024)

uint32_t GetFileCRC(FILE *fd, unsigned int *file_lenght)
{
	//uint32_t size = 16 * 1024;
	uint8_t crcbuf[size] = {0};
	uint32_t rdlen = 0;
	uint32_t crc = 0;//CRC初始值为0
	
	while((rdlen = fread(crcbuf, sizeof(uint8_t), size, fd)) > 0){
		crc = get_crc32(crc, crcbuf, rdlen);
		memset(crcbuf, 0, sizeof(crcbuf));
		*file_lenght += rdlen;
	}
	
	return crc;
}

int main(int argc,char **argv)
{
	FILE *fd = NULL;
	FILE *crc_fd = NULL;
	char *crc_file_name = NULL;
	unsigned int crc_value = 0;
	unsigned int file_lenght = 0;
	unsigned int count = 0;
	unsigned char buf[1024] = {0}; 
	unsigned char crc_head[32] = {0};
	
	if(argc<2)
	{
		printf("Usage: %s file",(argv[0]));
		return (-1);
	}
	
	crc_file_name = (char*)malloc(strlen(argv[1])+5);
	if(crc_file_name == NULL){
		printf("malloc failed\n");
		return -2;
	}
	snprintf(crc_file_name, strlen(argv[1])+5,"%s.crc",argv[1]);
	
	if((fd=fopen(argv[1],"rb"))==NULL)
	{
		perror("Error:");
		return (-1);
	}
	if((crc_fd=fopen(crc_file_name,"wb"))==NULL)
	{
		perror("crc_fd Error:");
		return (-1);
	}
	crc_value = GetFileCRC(fd, &file_lenght);
	printf("CRC: %#X\nFile lenght:%#X\n",crc_value,file_lenght);
	
	if(crc_fd){
		memcpy(crc_head, &crc_value, sizeof(crc_value));
		memcpy(crc_head + 16, &file_lenght, sizeof(file_lenght));
		fwrite(crc_head, 1, 32, crc_fd);//write the crc_head into the file
		fseek(fd, 0, SEEK_SET);  
		while(!feof(fd))  
		{  
			memset(buf, 0, sizeof(buf));  
			count = fread(buf, 1, sizeof(buf), fd);  
			fwrite(buf, 1, count, crc_fd);  
		} 
		fclose(crc_fd);
	}
	
	fclose(fd);
	free(crc_file_name);
	return 0;
}
