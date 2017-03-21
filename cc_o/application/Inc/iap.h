/*******************************************************************************
*
*   Timeout
*   Soft version:   V1.00
*   File Name:      iap.h
*   Author   :      zzw (zhangzw_3@163.com)
*   creation date:  2011-06-17
*	module description:
*   Copyright (C) reserve
*
********************************************************************************/
#ifndef __IAP_H__
#define __IAP_H__

#include "stm32f1xx_hal.h"

#define IAP_MIN_ADR    ((uint32_t)0x08018800)   //用于保存数据flash起始地址,98K
#define IAP_MAX_ADR    ((uint32_t)0x0803FFFF)   //用于保存数据flash结束地址，分配12K


#define W0_ADDR           0
#define W1_ADDR           200
#define W2_ADDR           400
#define W3_ADDR           600


typedef volatile unsigned short vu16;

#define IS_FLASH_CLEAR_FLAG(FLAG) ((((FLAG) & (uint32_t)0xFFFFFFCA) == 0x00000000) && ((FLAG) != 0x00000000))

extern void Read_Flash(uint32_t begin_addr,uint8_t *pReadBuf,uint16_t Count);
extern int write_flash_byte(uint32_t begin_addr, uint8_t temp_data);
extern int write_flash_array(uint32_t begin_addr,uint16_t counter,uint8_t array[]);
void iap_init(void);

#endif

