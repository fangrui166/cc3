#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"


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
void StartUART3ReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        APP_COM_DATA ap_data = {0};
        if(!UART3FrameQueueHandle) continue;
        xQueueReceive(UART3FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
        
        ap_data.length = Clcp2App_A(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
        dump_hex((char*)ap_data.app_data, ap_data.length);
        TypeA_rec_process(ap_data.app_data, ap_data.length);
        
    }
}
void uart3_postinit(void)
{
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&USART3RxChar, USART3_RX_BUFFER_SIZE);

    osThreadDef(UART3ReceTask, StartUART3ReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
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

