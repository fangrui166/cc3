#ifndef __USART_DRV_H__
#define __USART_DRV_H__
#ifdef __cplusplus
 extern "C" {
#endif
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "charger_port_process.h"

#define USART1_RX_BUFFER_SIZE     (512)
#define USART2_RX_BUFFER_SIZE     (512)
#define USART3_RX_BUFFER_SIZE     (512)
#define UART4_RX_BUFFER_SIZE      (512)
#define UART5_RX_BUFFER_SIZE      (512)
#define UART_RX_BUFFER_SIZE       (512)

#define APP_DATA_LEN              (130)


typedef struct
{
    uint8_t app_data[APP_DATA_LEN];
    uint8_t length;
} APP_COM_DATA;

typedef enum {
    MSG_QUEUE_FRAME = 1,
    MSG_QUEUE_RXCHAR_FULL,

}USART_MSG_QUEUE_TYPE;
typedef enum {
    MSG_QUEUE_SUCCES = 1,
    MSG_QUEUE_ERROR,
    MSG_QUEUE_FULL,

}MSG_QUEUE_RET;
typedef enum {
    COM1 = 0,  // for log and shell
    COM2,      // for CC_C
    //COM3,
    //COM4,
    //COM5,
    MAX_COM
}COM_INDEX;

typedef enum {
    UART_OK = 0,
    UART_FAIL,
    UART_FRAM_DONE,
    UART_FRAM_RECING,
    UART_REC_FAIL,
    UART_BUF_OVERFLOW,
    UART_UNKOW
}UART_RET;

void uart_init(void);
void uart_postinit(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
//void usart1_set_rxne_flag(void);
//void uart_rx_handler(UART_HandleTypeDef * huart);
//osTimerId GetFrameDoneTimerID(void);
UART_HandleTypeDef * GetUARTHandle(COM_INDEX ComID);

void uart1_rx_handler(UART_HandleTypeDef * huart);
void uart2_rx_handler(UART_HandleTypeDef * huart);
void uart1_init(void);
void uart2_init(void);
int uart2_play_one_mp3(uint32_t mp3_index,uint8_t port);
int uart2_send_charge_data_to_cc_c(portMessage_t message);
int uart2_set_port_charge_state(uint8_t port, uint8_t state);
int uart2_sync_port_state(uint32_t state);

#ifdef __cplusplus
}
#endif
#endif /*__USART_DRV_H__*/

