/***************************************************************************************
*
*   Timeout
*   Soft version:   V1.00
*   File Name:      iap.c
*   Author   :      zzw (zhangzw_3@163.com)
*   creation date:  2011-05-22
*	module description: FALSH读写控制函数。
*   Copyright (C) reserve
*   使用flash擦写函数前，需开放flash--stm32f10x_conf.h( #define _FLASH_PROG //开放flash)
****************************************************************************************/
#include "iap.h"
#include "cmsis_os.h"
#include <string.h>
#include "common.h"

//#define FLASH_PAGE_SIZE  ((uint16_t)0x400)
static osMutexId Flash_Mutex;

static uint8_t	flash_buff[FLASH_PAGE_SIZE];

void Enable_IAP(void)
{
	HAL_FLASH_Unlock();

  	/* Clear All pending flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP | FLASH_FLAG_OPTVERR);

}

void Close_IAP(void)
{
	/* Clear All pending flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_BSY | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR | FLASH_FLAG_EOP | FLASH_FLAG_OPTVERR);

  	HAL_FLASH_Lock();
}

//*********************************************************************
//*函数名称：Read_Flash()
//*功能描述：读flash数据
//*入口参数：地址，数据指针，数据个数
//*出口参数：无
//**********************************************************************

void Read_Flash(uint32_t begin_addr,uint8_t *pReadBuf,uint16_t Count)
{
	uint16_t  i,cnt;
	uint16_t  data;

	if((begin_addr > IAP_MAX_ADR)||(begin_addr<IAP_MIN_ADR))
		return;
	if(Count==0)
	    return;
    #if 0
    if(begin_addr%2 != 0)  //奇地址时
    {
    	begin_addr -= 1;
    	Count-=1;
    	cnt = Count/2;
    	data = (*(vu16*) begin_addr);//读指定地址数据，采用高位在前存入缓冲区
    	pReadBuf[0] = data&0xff;
    	begin_addr+=2;

    	for(i=0;i<cnt;i++)
		{
			data = (*(vu16*) (begin_addr+ (i<<1)));
            //printf("%s 2W addr:%#x=%#x\n",__func__,(begin_addr+ (i<<1)),data);

			pReadBuf[(i<<1)+1] = (data>>8)&0xff;
			pReadBuf[(i<<1)+2] = data&0xff;
		}

		if(Count % 2 != 0)
		{
			data = (*(vu16*) (begin_addr+ (i<<1)));
			pReadBuf[(i<<1)+1] = (data>>8)&0xff;
		}
    }
    else
    {
    	cnt = Count/2;
    	for(i=0;i<cnt;i++)
		{
			data = (*(vu16*) (begin_addr+ (i<<1)));
            //printf("%s addr:%#x=%#x\n",__func__,(begin_addr+ (i<<1)),data);
			pReadBuf[(i<<1)] = (data>>8)&0xff;
			pReadBuf[(i<<1)+1] = data&0xff;
		}

		if(Count%2 != 0)
		{
			data = (*(vu16*) (begin_addr+ (i<<1)));
			pReadBuf[(i<<1)] = (data>>8)&0xff;
		}
    }
    #else

    cnt = Count/2;
    for(i=0;i<cnt;i++)
    {
        data = (*(vu16*) (begin_addr+ (i<<1)));
        //printf("%s addr:%#x=%#x\n",__func__,(begin_addr+ (i<<1)),data);
        pReadBuf[(i<<1)] = (data>>8)&0xff;
        pReadBuf[(i<<1)+1] = data&0xff;
    }

    if(Count%2 != 0)
    {
        data = (*(vu16*) (begin_addr+ (i<<1)));
        pReadBuf[(i<<1)] = (data>>8)&0xff;
    }
    #endif

}
/**************************************************************************
函数名称：write_flash_byte()
功能描述：向flash写入一字节数据
入口参数：地址，数据
出口参数：无

 和众多 FLASH 存储器的特性类似,STM32 内的FLASH 数据只能由1变成0,如果要由0
 变成1,则需要调用刷除函数,把一个页都刷除掉.如果不擦也能写但是只能写上0
**************************************************************************/
int write_flash_byte(uint32_t begin_addr, uint8_t temp_data)
{
	uint16_t  i;
    uint64_t Data;
	uint32_t page_begin_addr,page_addr;
    HAL_StatusTypeDef ret;
    FLASH_EraseInitTypeDef EraseInfo;
    uint32_t EraseErrorPage = 0;
    EraseInfo.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInfo.Banks = FLASH_BANK_1;
    EraseInfo.NbPages = 1;


	if((begin_addr > IAP_MAX_ADR)||(begin_addr<IAP_MIN_ADR))
		return -1;
    osMutexWait(Flash_Mutex, osWaitForever);
	Enable_IAP();

	page_addr = begin_addr&(0xffffffff-(FLASH_PAGE_SIZE-1));	  //取页地址
	page_begin_addr = begin_addr&(FLASH_PAGE_SIZE-1);   //当前数据在页内地址
	//保护同一页中不需要改动的数据
	Read_Flash(page_addr,flash_buff,FLASH_PAGE_SIZE);	 //读flash数据备份

	flash_buff[page_begin_addr] = temp_data;	//将数据存入备份缓存区的相应位置

    EraseInfo.PageAddress = page_addr;
    ret = HAL_FLASHEx_Erase(&EraseInfo, &EraseErrorPage); // 擦除页内数据
    if(ret != HAL_OK){
        printf("flash Erase %#x error :%d\n",page_addr,ret);
        Close_IAP();
        osMutexRelease(Flash_Mutex);
        return -2;
    }

	for(i=0;i<FLASH_PAGE_SIZE/2;i++)
	{
		Data = flash_buff[i<<1];
		Data <<= 8;
		Data |= flash_buff[(i<<1)+1];
        //printf("addr:%#x,data=%#x\n",page_addr + (i<<1), Data);
        ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)(page_addr + (i<<1)), (uint64_t)(Data&0xFFFF));//写入半字数据，16位
        if(ret != HAL_OK){
            printf("flash program %#x, error :%d\n",page_addr + (i<<1),ret);
            Close_IAP();
            osMutexRelease(Flash_Mutex);
            return -3;
        }
	}

	Close_IAP();

    osMutexRelease(Flash_Mutex);
    printf("flash program done\n");
	return 0;

}

/********************************************************
函数名称：write_flash_array()
功能描述：向flash写入组数据
入口参数：地址，数据个数，待写入的数据
出口参数：无

写N个数据到flash中,而不改变同一页里(1K)不须改变的数据
 N = 1 ----2045
支持跨一次页
*********************************************************/
int write_flash_array(uint32_t begin_addr,uint16_t counter,uint8_t array[])
{
	uint32_t page_addr,page_begin_addr,old_page_begin_addr;
	uint16_t i, j;
	uint16_t counter_nextuint = 0,Data;
    HAL_StatusTypeDef ret;
    FLASH_EraseInitTypeDef EraseInfo;
    uint32_t EraseErrorPage = 0;
    EraseInfo.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInfo.Banks = FLASH_BANK_1;
    EraseInfo.NbPages = 1;

	if((begin_addr > IAP_MAX_ADR)||(begin_addr<IAP_MIN_ADR))
		return -1;
    if(counter==0)
	    return -2;

    osMutexWait(Flash_Mutex, osWaitForever);
    Enable_IAP();

	page_begin_addr	=	begin_addr&(FLASH_PAGE_SIZE-1);	//开始地址在页中的位置
	old_page_begin_addr = page_begin_addr;

	for(j=0;j<((old_page_begin_addr+counter-1)/FLASH_PAGE_SIZE)+1;j++)
	{
		page_begin_addr	=	page_begin_addr&(FLASH_PAGE_SIZE-1);	//开始地址在页中的位置
		page_addr	= j*FLASH_PAGE_SIZE + (begin_addr&(0xffffffff-(FLASH_PAGE_SIZE-1))); //取出页地址

		Read_Flash(page_addr,flash_buff,FLASH_PAGE_SIZE);

		for(i=0;i<(counter - counter_nextuint);i++)
		{
			flash_buff[page_begin_addr] = array[i + counter_nextuint];
			page_begin_addr++;
			if(page_begin_addr>=FLASH_PAGE_SIZE)
			{
				counter_nextuint = i +1;
				break;
			}
		}

        EraseInfo.PageAddress = page_addr;
        ret = HAL_FLASHEx_Erase(&EraseInfo, &EraseErrorPage); // 擦除页内数据
        if(ret != HAL_OK){
            printf("flash Erase %#x error :%d\n",page_addr,ret);
            Close_IAP();
            osMutexRelease(Flash_Mutex);
            return -3;
        }

		for(i=0;i<FLASH_PAGE_SIZE/2;i++)
		{
			Data = flash_buff[i<<1];
			Data <<= 8;
			Data |= flash_buff[(i<<1)+1];
            ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_addr + (i<<1), Data);//写入半字数据，16位
            if(ret != HAL_OK){
                printf("flash program %#x, error :%d\n",page_addr + (i<<1),ret);
                Close_IAP();
                osMutexRelease(Flash_Mutex);
                return -4;
            }

		}
	}

	Close_IAP();
    osMutexRelease(Flash_Mutex);
	return 0;
}
void iap_init(void)
{

    /* enable flash write protect */
    /* Lock the Option Bytes */
    HAL_FLASH_OB_Lock();

    /* Lock the Flash Program Erase controller */
    HAL_FLASH_Lock();

    osMutexDef(Flash_Mutex);
    Flash_Mutex = osMutexCreate(osMutex(Flash_Mutex));
}



