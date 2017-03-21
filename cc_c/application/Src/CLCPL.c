/*******************************************************************************
*
*   DTS AS
*   Soft version:
*   File Name:      CLCPL.c
*   Author   :      fr (fangrui2005@126.com)
*   creation date:  2016-06-11
*   module description:DTS ����ͨ����·����Э���
*   Copyright (C)
*
********************************************************************************/
#include "cmsis_os.h"
#include "CLCPL.h"
#include "common.h"

uint8_t COMM1RecvBuf[COMM1_BUF_LEN];
uint8_t COMM2RecvBuf[COMM2_BUF_LEN];
uint8_t COMM3RecvBuf[COMM3_BUF_LEN];
uint8_t COMM4RecvBuf[COMM4_BUF_LEN];
uint8_t COMM5RecvBuf[COMM5_BUF_LEN];

_clcpCom CLCP[MAX_COM]= {0};

static const uint16_t CLCPBufferSize[MAX_COM] = {COMM1_BUF_LEN,
                                            COMM2_BUF_LEN,
                                            COMM3_BUF_LEN,
                                            COMM4_BUF_LEN,
                                            COMM5_BUF_LEN
                                            };


void ClcpInit(void)
{
	uint8_t i;

	for (i = 0; i < MAX_COM; i++)
	{
		CLCP[i].flag = CLCP_ETX;
		CLCP[i].bufLen = 0;
		CLCP[i].datIndex = 0;
	}

	CLCP[COM1].buf = COMM1RecvBuf;
	CLCP[COM2].buf = COMM2RecvBuf;
	CLCP[COM3].buf = COMM3RecvBuf;
	CLCP[COM4].buf = COMM4RecvBuf;
	CLCP[COM5].buf = COMM5RecvBuf;
}

uint8_t App2Clcp_A(uint8_t *ClcpBuf, const uint8_t *AppBuf, const uint8_t len)
{
	uint8_t rLen;
	uint8_t i;

	rLen = 0;

	if (len > 200)
		return (rLen);

	for (i = 0; i < len; i++){
		if (AppBuf[i] == 0x7e){
			ClcpBuf[rLen++] = 0x5d;
			ClcpBuf[rLen++] = 0x5e;
		}
		else if (AppBuf[i] == 0x5d){
			ClcpBuf[rLen++] = 0x5d;
			ClcpBuf[rLen++] = 0x5d;
		}
		else
			ClcpBuf[rLen++] = AppBuf[i];

	}

	return (rLen);
}
uint8_t Clcp2App_A(uint8_t *AppBuf, const uint8_t *ClcpBuf, const uint8_t len)
{
	uint8_t rLen;
	uint8_t i;

	rLen = 0;

	if (len < 2)
		return (rLen);

	for (i = 0; i < len - 1; i++){
		if (ClcpBuf[i] == 0x5d){
			if (ClcpBuf[i+1] == 0x5e){
				AppBuf[rLen++] = 0x7e;
			}
			else if (ClcpBuf[i+1] == 0x5d){
				AppBuf[rLen++] = 0x5d;
			}

			i++;
		}
		else{
			AppBuf[rLen++] = ClcpBuf[i];
		}
	}

	if (i < len)
		AppBuf[rLen++] = ClcpBuf[i];

	return (rLen);
}

/* ͨ��Э��1 ����0x7E��ͷ��0x7E��β����һ֡����
*  ���һ֡����
*              0x7E  . . . 0x7E
*/
UART_RET ClcpRecv_A(const COM_INDEX ComID, const uint8_t comChar)
{

	if (ComID >= MAX_COM)
		return UART_REC_FAIL;
    if(CLCP[ComID].datIndex >= CLCPBufferSize[ComID])
        return UART_BUF_OVERFLOW;

	if (CLCP_ETX == CLCP[ComID].flag && comChar == 0x7e){
		CLCP[ComID].flag = CLCP_STX;
		CLCP[ComID].datIndex = 0;
	}
	else if (CLCP_STX == CLCP[ComID].flag && comChar == 0x7e){
        if (CLCP[ComID].datIndex != 0){ // �ų�������������0x7e
            CLCP[ComID].flag = CLCP_ETX;
        }
        CLCP[ComID].protocolType = TYPE_RX;

		return UART_FRAM_DONE;
	}
	else if (CLCP_STX == CLCP[ComID].flag && comChar != 0x7e){
		CLCP[ComID].buf[CLCP[ComID].datIndex++] = comChar;
	}

	return UART_FRAM_RECING;
}

void ClcpSend_A(const COM_INDEX ComID, uint8_t *buf, uint8_t len)
{
	uint8_t ClcpBuf[200];
	uint8_t ClcpLen;
    UART_HandleTypeDef * huart;

	if (ComID >= MAX_COM)
		return;

	ClcpLen = App2Clcp_A(&ClcpBuf[1], buf, len);

	ClcpLen++;
	ClcpBuf[0] = 0x7e;
	ClcpBuf[ClcpLen++] = 0x7e;
    huart = GetUARTHandle(ComID);
    //dump_hex(ClcpBuf, ClcpLen);
    HAL_UART_Transmit(huart, ClcpBuf, ClcpLen, portMAX_DELAY);

}

/* ͨ��Э��2 ��
*  ��� 100ms ��û���ٽ��յ����ݣ���Ϊһ֡������
*
*/
UART_RET ClcpRecv_B(const COM_INDEX ComID, const uint8_t comChar)
{
	if (ComID >= MAX_COM)
		return UART_REC_FAIL;
    if(CLCP[ComID].datIndex >= CLCPBufferSize[ComID]){
        CLCP[ComID].datIndex = 0;
        return UART_BUF_OVERFLOW;
    }
    if(CLCP[ComID].flag == CLCP_ETX){
        CLCP[ComID].flag = CLCP_STX;
        CLCP[ComID].datIndex = 0;
    }
    CLCP[ComID].buf[CLCP[ComID].datIndex] = comChar;
    CLCP[ComID].datIndex++;


    return UART_FRAM_RECING;
}

void ClcpSend_B(const COM_INDEX ComID, uint8_t *buf, uint8_t len)
{
    UART_HandleTypeDef *huart;

	if (ComID >= MAX_COM)
		return ;

    huart = GetUARTHandle(ComID);

    HAL_UART_Transmit(huart, buf, len, portMAX_DELAY);

}

/*for log and shell */
UART_RET ClcpRecv_C(const COM_INDEX ComID, const uint8_t comChar)
{
    UART_HandleTypeDef *huart;
	if (ComID >= MAX_COM)
		return UART_REC_FAIL;
    if(CLCP[ComID].datIndex >= CLCPBufferSize[ComID]){
        CLCP[ComID].datIndex = 0;
        return UART_BUF_OVERFLOW;
    }
    huart = GetUARTHandle(ComID);
    huart->Instance->DR = comChar; // ����

    CLCP[ComID].buf[CLCP[ComID].datIndex] = comChar;
    if((0x0D==CLCP[ComID].buf[CLCP[ComID].datIndex-1])&&
        (0x0A==CLCP[ComID].buf[CLCP[ComID].datIndex])){

        CLCP[ComID].protocolType = TYPE_RX;
        CLCP[ComID].datIndex = 0;
        return UART_FRAM_DONE;

    }
    CLCP[ComID].datIndex ++;
    return UART_FRAM_RECING;

}

/**
* ʹ��SecureCRT�����շ�����,�ڷ��͵��ַ����п��ܴ��в���Ҫ���ַ��Լ������ַ�,
* �����˸��,�����ƶ����ȵ�,��ʹ�������й��߽����ַ���֮ǰ,��Ҫ����Щ�����ַ���
* �������ַ�ȥ����.
* ֧�ֵĿ����ַ���:
*   ����:1B 5B 41
*   ����:1B 5B 42
*   ����:1B 5B 43
*   ����:1B 5B 44
*   �س�����:0D 0A
*  Backspace:08
*  Delete:7F
*/
uint32_t filter_useless_string(char *dest,const char *src)
{
    uint32_t dest_count=0;
    uint32_t src_count=0;
    while(src[src_count]!=0x0D && src[src_count+1]!=0x0A){
        //if(isprint(src[src_count])){
        if(src[src_count] >= 0x20 && src[src_count] <= 0x7E){
            dest[dest_count++]=src[src_count++];
        }
        else{
            switch(src[src_count]){
                case    0x08:                          //�˸����ֵ
                {
                    if(dest_count>0){
                        dest_count --;
                    }
                    src_count ++;
                }
                break;
                case    0x1B:
                {
                    if(src[src_count+1]==0x5B){
                        if(src[src_count+2]==0x41 || src[src_count+2]==0x42){
                            src_count +=3;              //���ƺ����Ƽ���ֵ
                        }
                        else if(src[src_count+2]==0x43){
                            dest_count++;               //���Ƽ���ֵ
                            src_count+=3;
                        }
                        else if(src[src_count+2]==0x44){
                            if(dest_count >0)           //���Ƽ���ֵ
                            {
                                dest_count --;
                            }
                            src_count +=3;
                        }
                        else{
                            src_count +=3;
                        }
                    }
                    else{
                        src_count ++;
                    }
                }
                break;
                default:
                {
                    src_count++;
                }
                break;
            }
        }
    }

    dest[dest_count++]=src[src_count++];
    dest[dest_count++]=src[src_count++];

    return dest_count;
}



