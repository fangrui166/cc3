#include "communication_service.h"
#include "common.h"
#include "NPL.h"
#include "CLCPL.h"
ascomm com;

static osThreadId CommunicationTaskHandle;
static osMessageQId CommuMessageQueueHandle;
static osSemaphoreId CommuSemaHandle;

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
    com.comm.cmd = I2C_SET_CC_O_ADDR;
    com.comm.len = 1;
    com.comm.buf[0] = addr;
    communication_send_to_cc_c(com);

  return 0;
}
int cc_i_get_device_id(void)
{
    ascomm com;
    com.comm.cmd = I2C_GET_DEVICE_ID;
    com.comm.len = 0;
    communication_send_to_cc_c(com);
    return 0;

}
int cc_i_set_charge_port(uint8_t port)
{
    ascomm data = {0};
    data.comm.cmd = I2C_SELECT_CHARGE_PORT;
    data.comm.len = 1;
    data.comm.buf[0] = port;
    communication_send_to_cc_c(data);
    return 0;
}
int cc_i_set_charge_coin(uint8_t coin_cout, uint32_t security_code)
{
    ascomm data = {0};

    data.comm.cmd = I2C_CHARGE_OFFLINE;
    data.comm.len = 5;
    data.comm.buf[0] = coin_cout;
    memcpy(&data.comm.buf[1], &security_code, 4);
    communication_send_to_cc_c(data);
    return 0;
}
int cc_i_set_charge_card_id(uint32_t card_id)
{
    ascomm data = {0};

    data.comm.cmd = I2C_CHARGE_ONLINE;
    data.comm.len = 4;
    memcpy(data.comm.buf, &card_id, 4);
    communication_send_to_cc_c(data);
    return 0;
}

int cc_i_send_charge_rf_card_id(uint32_t card_id)
{
    ascomm data = {0};

    data.comm.cmd = I2C_SEND_RF_CARD_ID;
    data.comm.len = 4;
    memcpy(data.comm.buf, &card_id, 4);
    communication_send_to_cc_c(data);
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
        // 丢弃
        /*
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
        */
    }
    return 0;
}
static void StartCommunicationTask(void const * argument)
{
    ascomm com_send;
    for(;;){

        if((!CommuMessageQueueHandle) || (!CommuSemaHandle)) continue;
        xQueueReceive(CommuMessageQueueHandle, &com_send, portMAX_DELAY);
        osSemaphoreWait(CommuSemaHandle, osWaitForever);

        AppCommSend_A(&com_send, NPL_CC_C_ADDR, 24);

        osSemaphoreRelease(CommuSemaHandle);
    }
}
void communication_send_to_cc_c(ascomm data)
{
    xQueueSend(CommuMessageQueueHandle, &data, portMAX_DELAY);

}
void communication_init(void)
{

    osThreadDef(CommunTask, StartCommunicationTask, osPriorityNormal, 0, 512);
    CommunicationTaskHandle = osThreadCreate(osThread(CommunTask), NULL);
    if(CommunicationTaskHandle == NULL){
      xloge("%s CommunicationTaskHandle is NULL",__func__);
    }

    osMessageQDef(CommuMessage, 2, ascomm);
    CommuMessageQueueHandle = osMessageCreate(osMessageQ(CommuMessage), NULL);

    osSemaphoreDef(Commu_sema);
    CommuSemaHandle = osSemaphoreCreate(osSemaphore(Commu_sema), 1);

}

