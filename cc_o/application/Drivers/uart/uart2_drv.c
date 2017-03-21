#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "communication_service.h"
#include "charger_port_process.h"

UART_HandleTypeDef huart2;
uint8_t USART2RxChar[USART2_RX_BUFFER_SIZE] = {0};

osThreadId UART2ReceTaskHandle;
osMessageQId UART2FrameQueueHandle;
osTimerId  FrameDoneTimerOut;

/* USART2 init function */
void MX_USART2_UART_Init(void)
{

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart2);
  HAL_NVIC_SetPriority(USART2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);

}

int uart2_send_to_cc_c(ascomm com_data, uint8_t port)
{
    _clcpCom FrameBuffer = {0};
    FrameBuffer.flag = port;
    FrameBuffer.buf = com_data.sbuf;
    FrameBuffer.datIndex = com_data.comm.len + 2;
    FrameBuffer.protocolType = TYPE_TX;
    //dump_hex(FrameBuffer.buf, FrameBuffer.datIndex);
    xQueueSend(UART2FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
    return 0;
}
int uart2_play_one_mp3(uint32_t mp3_index,uint8_t port)
{
    gd5820_cmd_para_t mp3_data = {0};
    ascomm com_data = {0};
    mp3_data.len = 1;
    mp3_data.para[0] = mp3_index;
    com_data.comm.cmd = O2C_PLAY_MP3;
    com_data.comm.len = (mp3_data.len*sizeof(mp3_data.para[0])) + 2;
    com_data.comm.buf[0] = port;
    memcpy(&com_data.comm.buf[1], &mp3_data, com_data.comm.len-1);
    return uart2_send_to_cc_c(com_data, 24);

}
int uart2_send_charge_data_to_cc_c(portMessage_t message)
{
    ascomm com_data = {0};
    com_data.comm.cmd = O2C_SEND_CHARGER_PORT_DATA;
    com_data.comm.len = sizeof(portMessage_t);
    memcpy(com_data.comm.buf, &message, com_data.comm.len);
    return uart2_send_to_cc_c(com_data, 24);

}
int uart2_set_port_charge_state(uint8_t port, uint8_t state)
{
    ascomm com_data = {0};
    com_data.comm.cmd = O2C_SET_PORT_CHARGE_STATE;
    com_data.comm.len = 2;
    com_data.comm.buf[0] = port;
    com_data.comm.buf[1] = state;
    return uart2_send_to_cc_c(com_data, 24);

}

int uart2_sync_port_state(uint32_t state)
{
    ascomm com_data = {0};
    com_data.comm.cmd = O2C_SYNC_PORT_STATE;
    com_data.comm.len = sizeof(uint32_t);
    memcpy(com_data.comm.buf, &state, com_data.comm.len);
    return uart2_send_to_cc_c(com_data, 24);

}
int uart2_send_message_to_port_process(portMessage_t portMessage)
{
    switch(portMessage.data.charger_port){
        case 1:
            send_message_to_port1_process(portMessage);
            break;
        case 2:
            send_message_to_port2_process(portMessage);
            break;
        case 3:
            send_message_to_port3_process(portMessage);
            break;
        case 4:
            send_message_to_port4_process(portMessage);
            break;
        case 5:
            send_message_to_port5_process(portMessage);
            break;
        case 6:
            send_message_to_port6_process(portMessage);
            break;
        case 7:
            send_message_to_port7_process(portMessage);
            break;
        case 8:
            send_message_to_port8_process(portMessage);
            break;
        case 9:
            send_message_to_port9_process(portMessage);
            break;
        case 10:
            send_message_to_port10_process(portMessage);
            break;
    }
    return 0;
}
int uart2_comm24_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
    uint8_t i, sum;
    uint8_t ret = 0;
    uint8_t Port;
    pAscomm ps;
    ascomm com_data;
    //chargerMessage_t message;

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
            case C2O_SEND_CHARGER_DATA:{
                portMessage_t portMessage;
                memcpy(&portMessage.data, ps->comm.buf, ps->comm.len);
                if(portMessage.data.cmd == CHARGER_CMD_SET_PORT_START){
                    portMessage.cmd = PORT_CMD_START_CHARGE;
                    portMessage.state = PORT_STATE_NO_CHARGE;
                    portMessage.consume = 0;
                }
                else if(portMessage.data.cmd == CHARGER_CMD_SET_PORT_STOP){
                    portMessage.cmd = PORT_CMD_STOP_CHARGE;
                }
                uart2_send_message_to_port_process(portMessage);
            }
                break;
            case O2C_SYNC_PORT_STATE:{
                is_sync_port_state = 1;
            }
                break;
            case C2O_HW_INSPECTION:{
                uint8_t result = 0;
                ret = 1;
                if(!strncmp((char*)ps->comm.buf, "portstate_o", 11)){
                    result = 1;
                }
                //xlogd("C2O_HW_INSPECTION %s, len:%d\r\n",ps->comm.buf, ps->comm.len);
                com_data.comm.cmd = ps->comm.cmd;
                com_data.comm.len = ps->comm.len+1;
                memcpy(com_data.comm.buf, ps->comm.buf, ps->comm.len);
                com_data.comm.buf[ps->comm.len] = result;

            }

                break;
            case 0x1a:
                break;
            default:
                break;
           }
    }

    Port=24;
    if (ret)
        uart2_send_to_cc_c(com_data ,Port);

    return 1;

}

int uart2_comm25_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
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
        uart2_send_to_cc_c(com_data ,Port);

    return 1;

}

int uart2_rec_process(uint8_t * data, uint8_t len)
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
            uart2_comm24_recv(&data[4], len-5, source_addr);
        }
        else if(port == 25){
            uart2_comm25_recv(&data[4], len-5, source_addr);
        }
    }
    return 0;
}
void Start2UARTReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        APP_COM_DATA ap_data = {0};
        if(!UART2FrameQueueHandle) continue;
        xQueueReceive(UART2FrameQueueHandle, &FrameBuffer, portMAX_DELAY);

        if(FrameBuffer.protocolType == TYPE_RX){
            ap_data.length = Clcp2App_A(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
            //dump_hex((char*)ap_data.app_data, ap_data.length);
            uart2_rec_process(ap_data.app_data, ap_data.length);
        }
        else if(FrameBuffer.protocolType == TYPE_TX){
            uint8_t port;
            ascomm com_send;
            memcpy(com_send.sbuf, FrameBuffer.buf, FrameBuffer.datIndex);
            port = FrameBuffer.flag;
            AppCommSend_A(&com_send, NPL_CC_C_ADDR, port);
        }

    }
}

void uart2_postinit(void)
{
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&USART2RxChar, USART2_RX_BUFFER_SIZE);

    osThreadDef(UART2ReceTask, Start2UARTReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*3);
    UART2ReceTaskHandle = osThreadCreate(osThread(UART2ReceTask), NULL);

    osMessageQDef(UartFrame, 2, _clcpCom);
    UART2FrameQueueHandle = osMessageCreate(osMessageQ(UartFrame), NULL);

}
void uart2_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;

    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_A(COM2, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UART2FrameQueueHandle){
            xQueueSendFromISR( UART2FrameQueueHandle, &CLCP[COM2], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

}

void uart2_init(void)
{
    MX_USART2_UART_Init();
    uart2_postinit();

}

