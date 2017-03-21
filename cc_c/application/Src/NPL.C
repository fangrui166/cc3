#include "NPL.h"
#include "common.h"
#include "CLCPL.h"
int NPL_Send_A(const uint8_t * buf, const uint8_t len, const uint8_t Addr, const uint8_t Port)
{
    uint8_t NPLBuf[200]={0};
    uint8_t NPLLen = 0;
    uint8_t sum = 0,i = 0;
    COM_INDEX ComId = Addr2ComID(Addr);
    if(len > 200) return -1;


	NPLLen = len + 4;
	NPLBuf[0] = Addr&0x0F;			// 目标地址
	NPLBuf[1] = NPL_LOCAL_ADDR; // 源地址
    NPLBuf[2] = Port;           // 端口号
	NPLBuf[3] = NPLLen + 1;		// NPL包长度
	sum = NPLBuf[0] + NPLBuf[1] + NPLBuf[2]+ NPLBuf[3];

	for (i = 0; i < len; i++){
		NPLBuf[i+4] = buf[i];
		sum += buf[i];
	}

	sum = 0 - sum;
	NPLBuf[NPLLen++] = sum;

	ClcpSend_A(ComId, NPLBuf, NPLLen); // 发送NPL数据包
    return 0;
}


