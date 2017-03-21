#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"
#include "charger_manager.h"
#include "hw_self_inspection.h"
#include "http_post.h"


UART_HandleTypeDef huart4;
uint8_t UART4RxChar[UART4_RX_BUFFER_SIZE] = {0};

static osThreadId UART4ReceTaskHandle;
static osMessageQId UART4FrameQueueHandle;
static _clcpCom FrameBuffer = {0};
static ascomm com_data = {0};

/* USART1 init function */
void MX_USART4_UART_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart4);
  HAL_NVIC_SetPriority(UART4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(UART4_IRQn);

}

int uart4_send_to_cc_o(ascomm com_data, uint8_t port)
{
    FrameBuffer.flag = port;
    FrameBuffer.buf = com_data.sbuf;
    FrameBuffer.datIndex = com_data.comm.len + 2;
    FrameBuffer.protocolType = TYPE_TX;
    //dump_hex(FrameBuffer.buf, FrameBuffer.datIndex);
    xQueueSend(UART4FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
    return 0;
}

int uart4_send_charger_data_to_cc_o1(chargerMessage_t message)
{
    xlog("Send start charge to cc_o1, port:%d, type:%d\n", message.charger_port,message.charger_type);
    memset(&com_data, 0, sizeof(com_data));
    com_data.comm.cmd = C2O_SEND_CHARGER_DATA;
    com_data.comm.len = sizeof(chargerMessage_t);
    memcpy(com_data.comm.buf, &message, com_data.comm.len);
    return uart4_send_to_cc_o(com_data, 24);

}
int uart4_hw_inspection(uint8_t* name)
{
    memset(&com_data, 0, sizeof(com_data));
    com_data.comm.cmd = C2O_HW_INSPECTION;
    com_data.comm.len = strlen((char *)name);
    memcpy(com_data.comm.buf, name, com_data.comm.len);
    return uart4_send_to_cc_o(com_data, 24);

}
int uart4_comm24_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
    uint8_t i, sum;
    uint8_t ret = 0;
    uint8_t Port;
    pAscomm ps;
    memset(&com_data, 0, sizeof(com_data));
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
            case O2C_PLAY_MP3:{

                gd5820_cmd_para_t mp3_data = {0};
                uint16_t temp = 0, port1,port2;
                uint8_t port = ps->comm.buf[0];
                port1 = port%10;
                port2 = port/10;
                memcpy(&mp3_data, &ps->comm.buf[1], ps->comm.len-1);
                temp = mp3_data.para[0];
                mp3_data.para[0] = MP3_INDEX_CHARGE_PORT;
                mp3_data.para[1] = port2;
                mp3_data.para[2] = port1;
                mp3_data.para[3] = temp;
                mp3_data.len = 4;
                uart3_play_multi_mp3(mp3_data.para, mp3_data.len);

            }
                break;
            case O2C_SEND_CHARGER_PORT_DATA:{
                portMessage_t message;
                memcpy(&message, ps->comm.buf, ps->comm.len);
                http_post_charge_data();
            }
                break;
            case O2C_SET_PORT_CHARGE_STATE:{
                uint8_t port = ps->comm.buf[0];
                uint8_t state = ps->comm.buf[1];
                charger_set_port_status(port, state);
            }
                break;
            case O2C_SYNC_PORT_STATE:{
                uint32_t o1_state = 0;
                ret = 1;

                memcpy(&o1_state, ps->comm.buf, ps->comm.len);

                com_data.comm.cmd = ps->comm.cmd;
                com_data.comm.len = ps->comm.len;
                memcpy(com_data.comm.buf, ps->comm.buf, ps->comm.len);
                charger_set_o1_state(o1_state);
            }
                break;
            case C2O_HW_INSPECTION:
                hw_self_inspection_set_result(ps->comm.buf, ps->comm.len);
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
        uart4_send_to_cc_o(com_data, Port);

    return 1;

}

int uart4_comm25_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
    uint8_t i, sum;
    uint8_t ret = 0;
    uint8_t Port;
    pAscomm ps;
    memset(&com_data, 0, sizeof(com_data));

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
        uart4_send_to_cc_o(com_data, Port);

    return 1;

}

int uart4_rec_process(uint8_t * data, uint8_t len)
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
            uart4_comm24_recv(&data[4], len-5, source_addr);
        }
        else if(port == 25){
            uart4_comm25_recv(&data[4], len-5, source_addr);
        }
    }
    return 0;
}

void StartUART4ReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        APP_COM_DATA ap_data = {0};
        ascomm com_send = {0};
        if(!UART4FrameQueueHandle) continue;
        xQueueReceive(UART4FrameQueueHandle, &FrameBuffer, portMAX_DELAY);

        if(FrameBuffer.protocolType == TYPE_RX){
            ap_data.length = Clcp2App_A(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
            //dump_hex((char*)ap_data.app_data, ap_data.length);
            uart4_rec_process(ap_data.app_data, ap_data.length);
        }
        else if(FrameBuffer.protocolType == TYPE_TX){
            uint8_t port;
            memcpy(com_send.sbuf, FrameBuffer.buf, FrameBuffer.datIndex);
            port = FrameBuffer.flag;
            //dump_hex((char*)com_send.comm.buf, com_send.comm.len);
            AppCommSend_A(&com_send, NPL_CC_O1_ADDR, port);
        }
    }
}
void uart4_postinit(void)
{
    HAL_UART_Receive_IT(&huart4, (uint8_t *)&UART4RxChar, UART4_RX_BUFFER_SIZE);

    osThreadDef(UART4ReceTask, StartUART4ReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*3);
    UART4ReceTaskHandle = osThreadCreate(osThread(UART4ReceTask), NULL);
    if(!UART4ReceTaskHandle){
        xloge("UART4ReceTaskHandle is NULL\n");
    }

    osMessageQDef(Uart4Frame, 1, _clcpCom);
    UART4FrameQueueHandle = osMessageCreate(osMessageQ(Uart4Frame), NULL);
}
void uart4_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;

    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_A(COM4, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UART4FrameQueueHandle){
            xQueueSendFromISR( UART4FrameQueueHandle, &CLCP[COM4], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void uart4_init(void)
{
    MX_USART4_UART_Init();
    uart4_postinit();

}

