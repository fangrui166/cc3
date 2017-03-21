#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart5;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;
osMutexId uart_Mutex;

static uint8_t USART1RxChar[USART1_RX_BUFFER_SIZE] = {0};
static uint8_t USART2RxChar[USART2_RX_BUFFER_SIZE] = {0};
static uint8_t USART3RxChar[USART3_RX_BUFFER_SIZE] = {0};
static uint8_t UART4RxChar[UART4_RX_BUFFER_SIZE] = {0};
static uint8_t UART5RxChar[UART5_RX_BUFFER_SIZE] = {0};

static const uint16_t UARTRxBufferSize[MAX_COM] = {USART1_RX_BUFFER_SIZE,
                                            USART2_RX_BUFFER_SIZE,
                                            USART3_RX_BUFFER_SIZE,
                                            UART4_RX_BUFFER_SIZE,
                                            UART5_RX_BUFFER_SIZE
                                            };
static uint32_t RxCharIndexIn[MAX_COM] = {0};

osThreadId UARTReceTaskHandle;
osMessageQId UARTFrameQueueHandle;
osTimerId  FrameDoneTimerOut;


/* UART4 init function */
void MX_UART4_Init(void)
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

/* UART5 init function */
void MX_UART5_Init(void)
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

/* USART3 init function */
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
  HAL_UART_Init(&huart3);
  HAL_NVIC_SetPriority(USART3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART3_IRQn);

}

void uart_init(void)
{
    osMutexDef(uart_mutex);
    uart_Mutex = osMutexCreate(osMutex(uart_mutex));
    MX_UART4_Init();
    MX_UART5_Init();
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();
    MX_USART3_UART_Init();
    ClcpInit();

}

COM_INDEX GetComID(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1){
        return COM1;
    }
    else if(huart->Instance == USART2){
        return COM2;
    }
    else if(huart->Instance == USART3){
        return COM3;
    }
    else if(huart->Instance == UART4){
        return COM4;
    }
    else if(huart->Instance == UART5){
        return COM5;
    }
    else
        return MAX_COM;
}
UART_HandleTypeDef * GetUARTHandle(COM_INDEX ComID)
{
    switch(ComID){
        case COM1:
            return &huart1;
        case COM2:
            return &huart2;
        case COM3:
            return &huart3;
        case COM4:
            return &huart4;
        case COM5:
            return &huart5;
    }
}
uint8_t * GetRxBuffer(COM_INDEX ComID)
{
    uint8_t * buffer;
    switch(ComID){
        case COM1:
            buffer = USART1RxChar;
            break;
        case COM2:
            buffer = USART2RxChar;
            break;
        case COM3:
            buffer = USART3RxChar;
            break;
        case COM4:
            buffer = UART4RxChar;
            break;
        case COM5:
            buffer = UART5RxChar;
            break;
        default :
            break;
    }
    return buffer;

}
osTimerId GetFrameDoneTimerID(void)
{
    return FrameDoneTimerOut;
}
void StartUARTReceTask(void const * argument)
{
    for(;;){
        _clcpCom FrameBuffer = {0};
        APP_COM_DATA ap_data = {0};
        if(!UARTFrameQueueHandle) continue;
        xQueueReceive(UARTFrameQueueHandle, &FrameBuffer, portMAX_DELAY);
        switch(FrameBuffer.protocolType){
            case TYPE_A:
                ap_data.length = Clcp2App_A(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
                dump_hex((char*)ap_data.app_data, ap_data.length);
                TypeA_rec_process(ap_data.app_data, ap_data.length);
                break;
            case TYPE_B:
                ap_data.length = FrameBuffer.datIndex;
                memcpy(ap_data.app_data, FrameBuffer.buf, FrameBuffer.datIndex);
                if(get_sim900a_msag_handle()){
                    if(xQueueSend(get_sim900a_msag_handle(), &ap_data, portMAX_DELAY) != pdPASS){
                        printf("StartUARTReceTask xQueueSend sim900a error!\n");
                    }
                }
                break;
            case TYPE_C:
                ap_data.length= filter_useless_string((char *)ap_data.app_data, (char *)FrameBuffer.buf);
                if( get_queue_commd() != 0 ){
                    if( xQueueSend(get_queue_commd(), &ap_data, 0) !=pdPASS ){
                        printf("StartUARTReceTask xQueueSend error!\n");
                    }
                }
                break;
            default:
                break;
        }
    }
}
void FrameDoneCallback(void const *argument)
{
    CLCP[COM2].protocolType = TYPE_B;
    CLCP[COM2].flag = CLCP_ETX;
    xQueueSend(UARTFrameQueueHandle, &CLCP[COM2], portMAX_DELAY);
    memset(CLCP[COM2].buf, 0 ,CLCP[COM2].datIndex);
    CLCP[COM2].datIndex = 0;
}

void uart_postinit(void)
{
    HAL_UART_Receive_IT(&huart1, (uint8_t *)&USART1RxChar, USART1_RX_BUFFER_SIZE);
    HAL_UART_Receive_IT(&huart2, (uint8_t *)&USART2RxChar, USART2_RX_BUFFER_SIZE);
    HAL_UART_Receive_IT(&huart3, (uint8_t *)&USART3RxChar, USART3_RX_BUFFER_SIZE);
    HAL_UART_Receive_IT(&huart4, (uint8_t *)&UART4RxChar, UART4_RX_BUFFER_SIZE);
    HAL_UART_Receive_IT(&huart5, (uint8_t *)&UART5RxChar, UART5_RX_BUFFER_SIZE);

    osThreadDef(UARTReceTask, StartUARTReceTask, osPriorityNormal, 0, 200);
    UARTReceTaskHandle = osThreadCreate(osThread(UARTReceTask), NULL);

    osMessageQDef(UartFrame, 5, _clcpCom);
    UARTFrameQueueHandle = osMessageCreate(osMessageQ(UartFrame), /*NULL*/UARTReceTaskHandle);

    osTimerDef(FrameDoneTypeB, FrameDoneCallback);
    FrameDoneTimerOut = osTimerCreate(osTimer(FrameDoneTypeB), osTimerOnce, NULL);

}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1){
        HAL_UART_Receive_IT(huart, (uint8_t *)&USART1RxChar, USART1_RX_BUFFER_SIZE);
        RxCharIndexIn[COM1] = 0;
    }
    else if(huart->Instance == USART2){
        HAL_UART_Receive_IT(huart, (uint8_t *)&USART2RxChar, USART2_RX_BUFFER_SIZE);
        RxCharIndexIn[COM2] = 0;
    }
    else if(huart->Instance == USART3){
        HAL_UART_Receive_IT(huart, (uint8_t *)&USART3RxChar, USART3_RX_BUFFER_SIZE);
        RxCharIndexIn[COM3] = 0;
    }
    else if(huart->Instance == UART4){
        HAL_UART_Receive_IT(huart, (uint8_t *)&UART4RxChar, UART4_RX_BUFFER_SIZE);
        RxCharIndexIn[COM4] = 0;
    }
    else if(huart->Instance == UART5){
        HAL_UART_Receive_IT(huart, (uint8_t *)&UART5RxChar, UART5_RX_BUFFER_SIZE);
        RxCharIndexIn[COM5] = 0;
    }
}
/*
void uart_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;
    COM_INDEX com;
    uint8_t * rxbuffer;
    com = GetComID(huart);
    rxbuffer = GetRxBuffer(com);
    if(RxCharIndexIn[com] < UARTRxBufferSize[com]){
        TempChar = rxbuffer[RxCharIndexIn[com]++];
    }
    else{
        HAL_UART_RxCpltCallback(huart);
        return;
    }
    switch(com){
        case COM1:
            ret = ClcpRecv_C(com, TempChar);
            break;
        case COM2:
            ret = ClcpRecv_B(com, TempChar);
            break;
        case COM3:
        case COM4:
        case COM5:
            ret = ClcpRecv_A(com, TempChar);
            break;
        default :
            break;
    }
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UARTFrameQueueHandle){
            xQueueSendFromISR( UARTFrameQueueHandle, &CLCP[com], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
*/
void uart1_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;
    COM_INDEX com;
    com = GetComID(huart);
    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_C(com, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UARTFrameQueueHandle){
            xQueueSendFromISR( UARTFrameQueueHandle, &CLCP[com], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

void uart2_rx_handler(UART_HandleTypeDef * huart)
{
    BaseType_t ret = pdFAIL;
    uint8_t TempChar;
    COM_INDEX com;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    com = GetComID(huart);
    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    
    ClcpRecv_B(com, TempChar);
    
    ret = xTimerChangePeriodFromISR(FrameDoneTimerOut, 200, &xHigherPriorityTaskWoken);
    if(ret != pdPASS){
        xloge("%s change period error\n",__func__);
    }
    ret = xTimerStartFromISR( FrameDoneTimerOut, &xHigherPriorityTaskWoken );
    if(ret != pdPASS){
        xloge("%s restart timer error\n",__func__);
    }
}
void uart3_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;
    COM_INDEX com;
    com = GetComID(huart);
    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_A(com, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UARTFrameQueueHandle){
            xQueueSendFromISR( UARTFrameQueueHandle, &CLCP[com], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
void uart4_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;
    COM_INDEX com;
    com = GetComID(huart);
    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_A(com, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UARTFrameQueueHandle){
            xQueueSendFromISR( UARTFrameQueueHandle, &CLCP[com], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}
void uart5_rx_handler(UART_HandleTypeDef * huart)
{
    uint8_t TempChar;
    UART_RET ret;
    COM_INDEX com;
    com = GetComID(huart);
    TempChar = (uint8_t)(huart->Instance->DR & (uint8_t)0x00FF);
    ret = ClcpRecv_A(com, TempChar);
    if(ret == UART_FRAM_DONE){
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if(UARTFrameQueueHandle){
            xQueueSendFromISR( UARTFrameQueueHandle, &CLCP[com], &xHigherPriorityTaskWoken );
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}




