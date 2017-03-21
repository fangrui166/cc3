#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"


UART_HandleTypeDef huart5;
uint8_t UART5RxChar[UART5_RX_BUFFER_SIZE] = {0};

osThreadId UART5ReceTaskHandle;
osMessageQId UART5FrameQueueHandle;

/* USART1 init function */
void MX_USART5_UART_Init(void)
{

  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&huart5);
  HAL_NVIC_SetPriority(UART5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(UART5_IRQn);

}
void StartUART5ReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        APP_COM_DATA ap_data = {0};
        if(!UART5FrameQueueHandle) continue;
        xQueueReceive(UART5FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
        
        ap_data.length = Clcp2App_A(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
        dump_hex((char*)ap_data.app_data, ap_data.length);
        TypeA_rec_process(ap_data.app_data, ap_data.length);
        
    }
}
void uart5_postinit(void)
{
    HAL_UART_Receive_IT(&huart5, (uint8_t *)&UART5RxChar, UART5_RX_BUFFER_SIZE);

    osThreadDef(UART5ReceTask, StartUART5ReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    UART5ReceTaskHandle = osThreadCreate(osThread(UART5ReceTask), NULL);

    osMessageQDef(Uart5Frame, 2, _clcpCom);
    UART5FrameQueueHandle = osMessageCreate(osMessageQ(Uart5Frame), NULL);
}
void uart5_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;

    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_A(COM5, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UART5FrameQueueHandle){
            xQueueSendFromISR( UART5FrameQueueHandle, &CLCP[COM5], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void uart5_init(void)
{
    MX_USART5_UART_Init();
    uart5_postinit();

}

