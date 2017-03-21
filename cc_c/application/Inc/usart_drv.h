#ifndef __USART_DRV_H__
#define __USART_DRV_H__
#ifdef __cplusplus
 extern "C" {
#endif
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"
#include "charger_manager.h"

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
    COM2,      // for SIM900A
    COM3,      // for CC_I
    COM4,      // for CC_O1
    COM5,      // for CC_O2
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
void uart3_rx_handler(UART_HandleTypeDef * huart);
void uart4_rx_handler(UART_HandleTypeDef * huart);
void uart5_rx_handler(UART_HandleTypeDef * huart);
void uart1_init(void);
void uart2_init(void);
void uart3_init(void);
void uart4_init(void);
void uart5_init(void);
int uart3_paly_one_mp3(uint32_t mp3_index);
int uart3_play_multi_mp3(uint16_t *mp3_array, uint8_t num);
int uart3_set_security_code_to_used(uint32_t security_code);
int uart4_send_charger_data_to_cc_o1(chargerMessage_t message);
int uart5_send_charger_data_to_cc_o2(chargerMessage_t message);
int uart3_sync_device_id(uint32_t device_id);
int uart4_hw_inspection(uint8_t* name);
int uart5_hw_inspection(uint8_t* name);
int uart3_hw_inspection(uint8_t * name);

#ifdef __cplusplus
}
#endif
#endif /*__USART_DRV_H__*/

