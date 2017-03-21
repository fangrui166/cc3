#ifndef __CLCPL_H__
#define __CLCPL_H__
#ifdef __cplusplus
 extern "C" {
#endif
#include "usart_drv.h"

#define COMM1_BUF_LEN	100
#define COMM2_BUF_LEN	100
#define COMM3_BUF_LEN	100
#define COMM4_BUF_LEN	100
#define COMM5_BUF_LEN	100

#define CLCP_STX	    2 // CLCP 报头标志
#define CLCP_ETX	    3 // CLCP 报尾标志


typedef struct _CLCP_COM
{
	uint8_t *buf;			// 数据缓冲区
	uint8_t flag;			// 数据报标志
	uint8_t bufLen;		    // 缓冲区长度
	uint8_t datIndex;		// 数据指针
	uint8_t protocolType;
} _clcpCom;

typedef enum {
    TYPE_RX = 1,
    TYPE_TX,
    TYPE_UNKOW
}protocolType;

extern _clcpCom CLCP[MAX_COM];

void ClcpInit(void);
uint8_t App2Clcp_A(uint8_t *ClcpBuf, const uint8_t *AppBuf, const uint8_t len);
uint8_t Clcp2App_A(uint8_t *AppBuf, const uint8_t *ClcpBuf, const uint8_t len);
UART_RET ClcpRecv_A(const COM_INDEX ComID, const uint8_t comChar);
void ClcpSend_A(const COM_INDEX ComID, uint8_t *buf, uint8_t len);
UART_RET ClcpRecv_B(const COM_INDEX ComID, const uint8_t comChar);
void ClcpSend_B(const COM_INDEX ComID, uint8_t *buf, uint8_t len);
UART_RET ClcpRecv_C(const COM_INDEX ComID, const uint8_t comChar);
uint32_t filter_useless_string(char *dest,const char *src);

#ifdef __cplusplus
}
#endif
#endif
