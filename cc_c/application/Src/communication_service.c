#include "communication_service.h"
#include "common.h"
#include "NPL.h"
#include "CLCPL.h"
ascomm com;

#pragma inline
uint8_t AppCommSend_A(pAscomm ps, uint8_t Addr,uint8_t Port)
{
	uint8_t i, sum;

	ps->comm.len += 3;

	for (sum = 0, i = 0; i < ps->comm.len-1; i++)
		sum += ps->sbuf[i];

	ps->sbuf[ps->comm.len-1] = 0 - sum;

	return (NPL_Send_A(ps->sbuf, ps->comm.len, Addr,Port));
}
int AppCommRecv24_A(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
	uint8_t i, sum;
	uint8_t ret = 1;
	uint8_t Port;
	pAscomm ps;

    for (sum = 0, i = 0; i < len; i++){
		sum += buf[i];
	}

	ps = (pAscomm)buf;

    if (sum != 0 && ps->comm.len != len){
        // 数据包校验出错
        xloge("%s sum or len not match\n",__func__);
		return 0;
    }

	ps->comm.len -= 3;

	if (Addr ==0x00){
        // 外设数据处理
	}
	else{
        // 上位机数据处理
		com.comm.cmd = 0x80 + ps->comm.cmd;
       	switch (ps->comm.cmd){
    		case 0x11:
    		    break;
    		case 0x12:
    	        break;
    		case 0x13:
    	        break;
    		case 0x14:
    	        break;
    		case 0x15:
    	        break;
    		case 0x18:
    			break;
    		case 0x1a:
    			break;
    		default:
    			break;
       	}
	}

	Port=24;
	if (ret)
		AppCommSend_A(&com, Addr,Port);

	return 1;

}

int AppCommRecv25_A(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
	uint8_t i, sum;
	uint8_t ret = 1;
	uint8_t Port;
	pAscomm ps;

    for (sum = 0, i = 0; i < len; i++){
		sum += buf[i];
	}

	ps = (pAscomm)buf;

    if (sum != 0 && ps->comm.len != len){
        // 数据包校验出错
        xloge("%s sum or len not match\n",__func__);
		return 0;
    }

	ps->comm.len -= 3;

	if (Addr ==0x00){
        // 外设数据处理
	}
	else{
        // 上位机数据处理

		com.comm.cmd = 0x80 + ps->comm.cmd;
       	switch (ps->comm.cmd){
    		case 0x11:
    		    break;
    		case 0x12:
    	        break;
    		case 0x13:
    	        break;
    		case 0x14:
    	        break;
    		case 0x15:
    	        break;
    		case 0x18:
    			break;
    		case 0x1a:
    			break;
    		default:
    			break;
		}
	}

	Port=25;
	if (ret)
		AppCommSend_A(&com, Addr,Port);

	return 1;

}

int Set_CC_O_Addr(uint8_t addr)
{
    ascomm com;
    com.comm.cmd = SET_CC_O_ADDR;
    com.comm.len = 1;
    com.comm.buf[0] = addr;

    AppCommSend_A(&com, addr, 24);
  return 0;
}
int TypeA_rec_process(uint8_t * data, uint8_t len)
{
    uint8_t purpose_addr = data[0];  // 目标地址
    uint8_t source_addr = data[1];   // 源地址
    uint8_t port = data[2];          // 端口号
    uint8_t buf_len = data[3];       // 数据包长度
    uint8_t i,sum;
    for(i=0,sum=0; i< len; i++){
        sum += data[i];
    }
    if((buf_len != len) || (sum != 0)){
        return -1;  // 无效数据，丢弃。
    }
    if(purpose_addr == NPL_LOCAL_ADDR) {
        //  是本机地址
        if(source_addr == NPL_CC_O_ADDR){
            // 输出设备没有分配地址，重新分配
            xlogi("set cc_o addr\n");
            Set_CC_O_Addr(NPL_CC_O1_ADDR);
            Set_CC_O_Addr(NPL_CC_O2_ADDR);
            return 0;
        }
        if(port == 24){
            AppCommRecv24_A(&data[4], len-5, source_addr);
        }
        else if(port == 25){
            AppCommRecv25_A(&data[4], len-5, source_addr);
        }

    }
    else{
        // 不是本机地址，直接转发
        switch(purpose_addr){
            case NPL_CC_I_ADDR:
                ClcpSend_A(COM3, data, len);
                break;
            case NPL_CC_O1_ADDR:
                ClcpSend_A(COM4, data, len);
                break;
            case NPL_CC_O2_ADDR:
                ClcpSend_A(COM5, data, len);
                break;
            default:
                break;
        }
    }
    return 0;
}

