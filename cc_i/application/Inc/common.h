#ifndef __COMMON_H__
#define __COMMON_H__
#ifdef __cplusplus
 extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "usart_drv.h"
#include "cmsis_os.h"
#include "led.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define xlog      printf
#define xlogi     printf
#define xlogd     printf
#define xlogw     printf
#define xloge     printf

#define LED2_ON()  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET)
#define LED2_OFF() HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)
extern uint32_t Device_ID ;
extern uint8_t is_device_id_synced;

void dbgHexDump(unsigned char *buffer, unsigned short length);
void dump_hex(char * buf,int len);
uint32_t str2hex(uint8_t *str);
COM_INDEX Addr2ComID(uint8_t Addr);
void delay_us(uint16_t nus);
void Get_Serial_ID(void) ;


#define NPL_CC_I_ADDR   0x01
#define NPL_CC_C_ADDR   0x02
#define NPL_CC_O_ADDR   0x03
#define NPL_CC_O1_ADDR  0x13
#define NPL_CC_O2_ADDR  0x23

#define NPL_LOCAL_ADDR NPL_CC_I_ADDR
#define AS_SEND_BUF_MAX	200

typedef struct _ASS_COMM
{
	uint8_t cmd;
	uint8_t len;
	uint8_t buf[AS_SEND_BUF_MAX+1];
} _ascomm;

typedef struct{
    uint8_t project[4];
    uint8_t version[5];
    struct tm build_time;
}sw_version_t;

typedef union _ASU_COMM
{
	uint8_t sbuf[AS_SEND_BUF_MAX+3];
	_ascomm comm;
} ascomm, *pAscomm;


#ifdef __cplusplus
}
#endif
#endif

