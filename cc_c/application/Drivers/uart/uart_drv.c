#include <string.h>
#include "cmsis_os.h"
#include "usart_drv.h"
#include "CLCPL.h"
#include "common.h"
#include "command.h"
#include "sim900a.h"
#include "communication_service.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;

extern uint8_t USART1RxChar[USART1_RX_BUFFER_SIZE];
extern uint8_t USART2RxChar[USART2_RX_BUFFER_SIZE];
extern uint8_t USART3RxChar[USART3_RX_BUFFER_SIZE];
extern uint8_t UART4RxChar[UART4_RX_BUFFER_SIZE];
extern uint8_t UART5RxChar[UART5_RX_BUFFER_SIZE];

void uart_init(void)
{
    uart1_init();
    uart2_init();
    uart3_init();
    uart4_init();
    uart5_init();
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
    return NULL;
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

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1){
        HAL_UART_Receive_IT(huart, (uint8_t *)&USART1RxChar, USART1_RX_BUFFER_SIZE);
    }
    else if(huart->Instance == USART2){
        HAL_UART_Receive_IT(huart, (uint8_t *)&USART2RxChar, USART2_RX_BUFFER_SIZE);
    }
    else if(huart->Instance == USART3){
        HAL_UART_Receive_IT(huart, (uint8_t *)&USART3RxChar, USART3_RX_BUFFER_SIZE);
    }
    else if(huart->Instance == UART4){
        HAL_UART_Receive_IT(huart, (uint8_t *)&UART4RxChar, UART4_RX_BUFFER_SIZE);
    }
    else if(huart->Instance == UART5){
        HAL_UART_Receive_IT(huart, (uint8_t *)&UART5RxChar, UART5_RX_BUFFER_SIZE);
    }
}

