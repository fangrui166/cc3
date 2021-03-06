#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"
#include "charger_manager.h"

/*
  0x7E ... ... 0x7E
*/

UART_HandleTypeDef huart3;
uint8_t USART3RxChar[USART3_RX_BUFFER_SIZE] = {0};

osThreadId UART3ReceTaskHandle;
osMessageQId UART3FrameQueueHandle;


/* USART1 init function */
void MX_USART3_UART_Init(void)
{

  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.State = HAL_UART_STATE_RESET;
  HAL_UART_Init(&huart3);
  HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);

}

int uart3_send_to_cc_i(ascomm com_data, uint8_t port)
{
    _clcpCom FrameBuffer = {0};
    FrameBuffer.flag = port;
    FrameBuffer.buf = com_data.sbuf;
    FrameBuffer.datIndex = com_data.comm.len + 2;
    FrameBuffer.protocolType = TYPE_TX;
    //dump_hex(FrameBuffer.buf, FrameBuffer.datIndex);
    xQueueSend(UART3FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
    return 0;
}

int uart3_paly_one_mp3(uint32_t mp3_index)
{
    gd5820_cmd_para_t mp3_data = {0};
    ascomm com_data = {0};
    mp3_data.len = 1;
    mp3_data.para[0] = mp3_index;
    com_data.comm.cmd = C2I_PLAY_MP3;
    com_data.comm.len = (mp3_data.len*sizeof(mp3_data.para[0])) + 1;
    memcpy(com_data.comm.buf, &mp3_data, com_data.comm.len);
    return uart3_send_to_cc_i(com_data, 24);
}
int uart3_set_security_code_to_used(uint32_t security_code)
{
    ascomm com_data = {0};
    com_data.comm.cmd = C2I_SET_SECURITY_CODE_USED;
    com_data.comm.len = sizeof(security_code);
    memcpy(com_data.comm.buf, &security_code, sizeof(security_code));
    return uart3_send_to_cc_i(com_data, 24);

}
int uart3_comm24_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
    uint8_t i, sum;
    uint8_t ret = 0;
    uint8_t Port;
    pAscomm ps;
    ascomm com_data;
    chargerMessage_t message;

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
        com_data.comm.cmd = 0x80 + ps->comm.cmd;
           switch (ps->comm.cmd){
            case I2C_SET_CC_O_ADDR:
                break;
            case I2C_CHARGE_OFFLINE:
                xlog("%s coin count:%d\n",__func__,ps->comm.buf[0]);
                message.cmd = CHARGER_CMD_SET_TYPE;
                message.charger_type = CHARGER_TYPE_OFFLINE;
                message.charger_prar.offline_prar.coin_count = ps->comm.buf[0];
                memcpy(&message.charger_prar.offline_prar.security_code, &ps->comm.buf[1], 4);
                send_message_to_charger_manager(message);
                uart3_paly_one_mp3(MP3_INDEX_INPUT_CHARGER_PORT);
                break;
            case I2C_SELECT_CHARGE_PORT:
                xlog("%s set port:%d\n",__func__,ps->comm.buf[0]);
                message.cmd = CHARGER_CMD_SET_PORT;
                message.charger_port= ps->comm.buf[0];
                send_message_to_charger_manager(message);

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
        uart3_send_to_cc_i(com_data, Port);

    return 1;

}

int uart3_comm25_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
    uint8_t i, sum;
    uint8_t ret = 0;
    uint8_t Port;
    pAscomm ps;
    ascomm com_data;

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
        com_data.comm.cmd = 0x80 + ps->comm.cmd;
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
        uart3_send_to_cc_i(com_data, Port);

    return 1;

}

int uart3_rec_process(uint8_t * data, uint8_t len)
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
        if(port == 24){
            uart3_comm24_recv(&data[4], len-5, source_addr);
        }
        else if(port == 25){
            uart3_comm25_recv(&data[4], len-5, source_addr);
        }
    }
    return 0;
}
void StartUART3ReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        APP_COM_DATA ap_data = {0};
        if(!UART3FrameQueueHandle) continue;
        xQueueReceive(UART3FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
        if(FrameBuffer.protocolType == TYPE_RX){
            ap_data.length = Clcp2App_A(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
            //dump_hex((char*)ap_data.app_data, ap_data.length);
            uart3_rec_process(ap_data.app_data, ap_data.length);
        }
        else if(FrameBuffer.protocolType == TYPE_TX){
            uint8_t port;
            ascomm com_send;
            memcpy(com_send.sbuf, FrameBuffer.buf, FrameBuffer.datIndex);
            port = FrameBuffer.flag;
            AppCommSend_A(&com_send, NPL_CC_I_ADDR, port);
        }

    }
}
void uart3_postinit(void)
{
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&USART3RxChar, USART3_RX_BUFFER_SIZE);

    osThreadDef(UART3ReceTask, StartUART3ReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*3);
    UART3ReceTaskHandle = osThreadCreate(osThread(UART3ReceTask), NULL);

    osMessageQDef(Uart3Frame, 2, _clcpCom);
    UART3FrameQueueHandle = osMessageCreate(osMessageQ(Uart3Frame), NULL);
}
void uart3_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;

    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_A(COM3, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UART3FrameQueueHandle){
            xQueueSendFromISR( UART3FrameQueueHandle, &CLCP[COM3], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void uart3_init(void)
{
    MX_USART3_UART_Init();
    uart3_postinit();

}

