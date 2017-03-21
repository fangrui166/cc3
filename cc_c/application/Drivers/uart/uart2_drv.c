#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"

UART_HandleTypeDef huart2;
uint8_t USART2RxChar[USART2_RX_BUFFER_SIZE] = {0};

static osThreadId UART2ReceTaskHandle;
static osMessageQId UART2FrameQueueHandle;
static osTimerId  FrameDoneTimerOut;
static APP_COM_DATA ap_data = {0};

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

void FrameDoneCallback(void const *argument)
{
    CLCP[COM2].protocolType = TYPE_RX;
    CLCP[COM2].flag = CLCP_ETX;
    xQueueSend(UART2FrameQueueHandle, &CLCP[COM2], portMAX_DELAY);
    memset(CLCP[COM2].buf, 0 ,CLCP[COM2].datIndex);
    CLCP[COM2].datIndex = 0;
}
void Start2UARTReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        if(!UART2FrameQueueHandle) continue;
        xQueueReceive(UART2FrameQueueHandle, &FrameBuffer, portMAX_DELAY);
        memset(&ap_data, 0, sizeof(ap_data));
        ap_data.length = FrameBuffer.datIndex;
        memcpy(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
        if(get_sim900a_msag_handle()){
            if(xQueueSend(get_sim900a_msag_handle(), &ap_data, portMAX_DELAY) != pdPASS){
                xloge("Start2UARTReceTask xQueueSend sim900a error!\n");
            }
        }
    }
}

void uart2_postinit(void)
{
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&USART2RxChar, USART2_RX_BUFFER_SIZE);

    osThreadDef(UART2ReceTask, Start2UARTReceTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*2);
    UART2ReceTaskHandle = osThreadCreate(osThread(UART2ReceTask), NULL);

    osMessageQDef(UartFrame, 1, _clcpCom);
    UART2FrameQueueHandle = osMessageCreate(osMessageQ(UartFrame), NULL);

    osTimerDef(FrameDoneTypeB, FrameDoneCallback);
    FrameDoneTimerOut = osTimerCreate(osTimer(FrameDoneTypeB), osTimerOnce, NULL);
    osTimerStart(FrameDoneTimerOut, 200);

}
void uart2_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    BaseType_t ret = pdFAIL;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);

    ClcpRecv_B(COM2, TempChar);

    ret = xTimerResetFromISR( FrameDoneTimerOut, &xHigherPriorityTaskWoken );
    if(ret != pdPASS){
        //xloge("%s reset timer error\n",__func__);
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

}

void uart2_init(void)
{
    MX_USART2_UART_Init();
    uart2_postinit();

}

