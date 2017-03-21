/***************************************************************************************
*
*   Timeout
*   Soft version:   V1.00
*   File Name:      iap.c
*   Author   :      zzw (zhangzw_3@163.com)
*   creation date:  2011-05-22
*	module description: FALSH��д���ƺ�����
*   Copyright (C) reserve
*   ʹ��flash��д����ǰ���迪��flash--stm32f10x_conf.h( #define _FLASH_PROG //����flash)
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
//*�������ƣ�Read_Flash()
//*������������flash����
//*��ڲ�������ַ������ָ�룬���ݸ���
//*���ڲ�������
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
    if(begin_addr%2 != 0)  //���ַʱ
    {
    	begin_addr -= 1;
    	Count-=1;
    	cnt = Count/2;
    	data = (*(vu16*) begin_addr);//��ָ����ַ���ݣ����ø�λ��ǰ���뻺����
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
�������ƣ�write_flash_byte()
������������flashд��һ�ֽ�����
��ڲ�������ַ������
���ڲ�������

 ���ڶ� FLASH �洢������������,STM32 �ڵ�FLASH ����ֻ����1���0,���Ҫ��0
 ���1,����Ҫ����ˢ������,��һ��ҳ��ˢ����.�������Ҳ��д����ֻ��д��0
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

	page_addr = begin_addr&(0xffffffff-(FLASH_PAGE_SIZE-1));	  //ȡҳ��ַ
	page_begin_addr = begin_addr&(FLASH_PAGE_SIZE-1);   //��ǰ������ҳ�ڵ�ַ
	//����ͬһҳ�в���Ҫ�Ķ�������
	Read_Flash(page_addr,flash_buff,FLASH_PAGE_SIZE);	 //��flash���ݱ���

	flash_buff[page_begin_addr] = temp_data;	//�����ݴ��뱸�ݻ���������Ӧλ��

    EraseInfo.PageAddress = page_addr;
    ret = HAL_FLASHEx_Erase(&EraseInfo, &EraseErrorPage); // ����ҳ������
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
        ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, (uint32_t)(page_addr + (i<<1)), (uint64_t)(Data&0xFFFF));//д��������ݣ�16λ
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
�������ƣ�write_flash_array()
������������flashд��������
��ڲ�������ַ�����ݸ�������д�������
���ڲ�������

дN�����ݵ�flash��,�����ı�ͬһҳ��(1K)����ı������
 N = 1 ----2045
֧�ֿ�һ��ҳ
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

	page_begin_addr	=	begin_addr&(FLASH_PAGE_SIZE-1);	//��ʼ��ַ��ҳ�е�λ��
	old_page_begin_addr = page_begin_addr;

	for(j=0;j<((old_page_begin_addr+counter-1)/FLASH_PAGE_SIZE)+1;j++)
	{
		page_begin_addr	=	page_begin_addr&(FLASH_PAGE_SIZE-1);	//��ʼ��ַ��ҳ�е�λ��
		page_addr	= j*FLASH_PAGE_SIZE + (begin_addr&(0xffffffff-(FLASH_PAGE_SIZE-1))); //ȡ��ҳ��ַ

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
        ret = HAL_FLASHEx_Erase(&EraseInfo, &EraseErrorPage); // ����ҳ������
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
            ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_addr + (i<<1), Data);//д��������ݣ�16λ
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



