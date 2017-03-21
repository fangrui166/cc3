#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "communication_service.h"
#include "gd5820.h"
#include "totp.h"
#include "fm1702_drv.h"

/*
   connect to CC-C
   0X7E  ... 0X7E
*/
UART_HandleTypeDef huart2;
uint8_t USART2RxChar[USART2_RX_BUFFER_SIZE] = {0};

osThreadId UART2ReceTaskHandle;
osMessageQId UART2FrameQueueHandle;
ascomm uart2_com;

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
int uart2_comm24_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
    uint8_t i, sum;
    uint8_t ret = 0;
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
        uart2_com.comm.cmd = 0x80 + ps->comm.cmd;
           switch (ps->comm.cmd){
            case C2I_PLAY_MP3:{
                gd5820_cmd_para_t mp3_data = {0};
                memcpy(&mp3_data, ps->comm.buf, ps->comm.len);
                if(mp3_data.len == 1){
                    gd5820_play_one_mp3(mp3_data.para[0]);
                }
                else{
                    gd5820_play_multiple_mp3(mp3_data.para, mp3_data.len);
                }
            }
                break;
            case C2I_SET_SECURITY_CODE_USED:{
                uint32_t security_code = 0;
                memcpy(&security_code, ps->comm.buf, sizeof(security_code));
                totp_set_security_code_used(security_code);
            }
                break;
            case C2I_HW_INSPECTION:{
                uint8_t result = 0;
                ret = 1;
                if(!strncmp((char*)ps->comm.buf, "KEYPAD", 6)){
                    result = 1;
                }
                else if(!strncmp((char*)ps->comm.buf, "RFID", 4)){
                    result = 2;
                }
                else if(!strncmp((char*)ps->comm.buf, "RTC", 3)){
                    result = 3;
                }
                else if(!strncmp((char*)ps->comm.buf, "MP3", 3)){
                    result = 4;
                }
                xlogd("C2I_HW_INSPECTION :%#x\n", result);

                uart2_com.comm.cmd = ps->comm.cmd;
                uart2_com.comm.len = ps->comm.len+1;
                memcpy(uart2_com.comm.buf, ps->comm.buf, ps->comm.len);
                uart2_com.comm.buf[ps->comm.len] = result; // 1 result = seccuss
            }
                break;
            case C2I_SYNC_DEVICE_ID:
                ret = 1;
                memcpy(&Device_ID, ps->comm.buf, ps->comm.len);
                xlogd("C2I_SYNC_DEVICE_ID :%#x\n", Device_ID);
                uart2_com.comm.cmd = ps->comm.cmd;
                uart2_com.comm.len = ps->comm.len;
                memcpy(uart2_com.comm.buf, ps->comm.buf, ps->comm.len);
                break;
            case I2C_GET_DEVICE_ID:
                memcpy(&Device_ID, ps->comm.buf, ps->comm.len);
                xlogd("I2C_GET_DEVICE_ID :%#x\n", Device_ID);
                is_device_id_synced = 1;
                break;
            case C2I_RF_CARD_ID_RESP:{
                uint8_t rf_card_response = 0;
                rf_card_response = ps->comm.buf[0];
                fm1702_rfid_response(rf_card_response);
            }

            default:
                break;
           }
    }

    if (ret)
        communication_send_to_cc_c(uart2_com);

    return 1;

}

int uart2_comm25_recv(const uint8_t *buf, uint8_t len, uint8_t Addr)
{
    uint8_t i, sum;
    uint8_t ret = 0;
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
        uart2_com.comm.cmd = 0x80 + ps->comm.cmd;
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
        AppCommSend_A(&uart2_com, Addr,Port);

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

        ap_data.length = Clcp2App_A(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
        //dump_hex((char*)ap_data.app_data, ap_data.length);
        uart2_rec_process(ap_data.app_data, ap_data.length);
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

