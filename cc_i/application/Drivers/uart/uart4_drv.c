#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "communication_service.h"
#include "gd5820.h"


UART_HandleTypeDef huart4;
uint8_t UART4RxChar[UART4_RX_BUFFER_SIZE] = {0};

osThreadId UART4ReceTaskHandle;
osMessageQId UART4FrameQueueHandle;
osTimerId  FrameDoneTimerOut;

/* USART1 init function */
void MX_USART4_UART_Init(void)
{

  huart4.Instance = UART4;
  huart4.Init.BaudRate = 9600;
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
void FrameDoneCallback(void const *argument)
{
    CLCP[COM4].protocolType = TYPE_B;
    CLCP[COM4].flag = CLCP_ETX;
    xQueueSend(UART4FrameQueueHandle, &CLCP[COM4], portMAX_DELAY);
    memset(CLCP[COM4].buf, 0 ,CLCP[COM4].datIndex);
    CLCP[COM4].datIndex = 0;
}

void StartUART4ReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        APP_COM_DATA ap_data = {0};
        if(!UART4FrameQueueHandle) continue;
        xQueueReceive(UART4FrameQueueHandle, &FrameBuffer, portMAX_DELAY);

        ap_data.length = FrameBuffer.datIndex;
        memcpy(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
        //printf("GD5820 rece: len:%d, data:%s\n",ap_data.length,FrameBuffer.buf);
        gd5820_send_query_result(atoi((char const *)ap_data.app_data));

    }
}
void uart4_postinit(void)
{
    HAL_UART_Receive_IT(&huart4, (uint8_t *)&UART4RxChar, UART4_RX_BUFFER_SIZE);

    osThreadDef(UART4ReceTask, StartUART4ReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*2);
    UART4ReceTaskHandle = osThreadCreate(osThread(UART4ReceTask), NULL);

    osMessageQDef(Uart4Frame, 2, _clcpCom);
    UART4FrameQueueHandle = osMessageCreate(osMessageQ(Uart4Frame), NULL);

    osTimerDef(FrameDoneTypeB, FrameDoneCallback);
    FrameDoneTimerOut = osTimerCreate(osTimer(FrameDoneTypeB), osTimerOnce, NULL);
    osTimerStart(FrameDoneTimerOut, 200);
}
void uart4_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    BaseType_t ret = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);

    ClcpRecv_B(COM4, TempChar);

    ret = xTimerResetFromISR( FrameDoneTimerOut, &xHigherPriorityTaskWoken );
    if(ret != pdPASS){
        xloge("%s reset timer error\n",__func__);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

void uart4_init(void)
{
    MX_USART4_UART_Init();
    uart4_postinit();

}

