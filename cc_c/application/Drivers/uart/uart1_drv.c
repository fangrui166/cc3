#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"


UART_HandleTypeDef huart1;
uint8_t USART1RxChar[USART1_RX_BUFFER_SIZE] = {0};

static osThreadId UART1ReceTaskHandle;
static osMessageQId UART1FrameQueueHandle;
static APP_COM_DATA ap_data = {0};

/* USART1 init function */
void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.State = HAL_UART_STATE_RESET;
  HAL_UART_Init(&huart1);
  HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);

}
void StartUART1ReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        if(!UART1FrameQueueHandle) continue;
        xQueueReceive(UART1FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
        memset(&ap_data, 0, sizeof(ap_data));
        ap_data.length= filter_useless_string((char *)ap_data.app_data, (char *)FrameBuffer.buf);
        if( get_queue_commd() != 0 ){
            if( xQueueSend(get_queue_commd(), &ap_data, portMAX_DELAY) !=pdPASS ){
                xloge("StartUART1ReceTask xQueueSend error!\n");
            }
        }
    }
}
void uart1_postinit(void)
{
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&USART1RxChar, USART1_RX_BUFFER_SIZE);

    osThreadDef(UART1ReceTask, StartUART1ReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*2);
    UART1ReceTaskHandle = osThreadCreate(osThread(UART1ReceTask), NULL);

    osMessageQDef(Uart1Frame, 1, _clcpCom);
    UART1FrameQueueHandle = osMessageCreate(osMessageQ(Uart1Frame), NULL);
}
void uart1_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;

    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_C(COM1, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UART1FrameQueueHandle){
            xQueueSendFromISR( UART1FrameQueueHandle, &CLCP[COM1], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void uart1_init(void)
{
    MX_USART1_UART_Init();
    uart1_postinit();

}

