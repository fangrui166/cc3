#ifndef __DS_IIC_H__
#define __DS_IIC_H__

#include "stm32f1xx_hal.h"

#define USE_HW_I2C   1

#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)

#define DS_SDA_IN()  {GPIOB->CRL&=0XFFFF0FFF;GPIOB->CRL|=8<<12;} //PB11
#define DS_SDA_OUT() {GPIOB->CRL&=0XFFFF0FFF;GPIOB->CRL|=3<<12;} //PB10


#define DS_IIC_SCL    PBout(10) //SCL 10
#define DS_IIC_SDA    PBout(11) //SDA 11
#define DS_READ_SDA   PBin(11)

void DS_IIC_Init(void);
void DS_IIC_Start(void);
void DS_IIC_Stop(void);
void DS_IIC_Write_Byte(uint8_t txd);
uint8_t DS_IIC_Read_Byte(unsigned char ack);
uint8_t DS_IIC_Wait_Ack(void);
void DS_IIC_Ack(void);
void DS_IIC_NAck(void);

void DS_IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t DS_IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);


#endif

