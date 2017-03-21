#ifndef __FM1702_H__
#define __FM1702_H__
#include "stm32f1xx_hal.h"

#define USE_STM_HW_SPI    0

#define FM1702_IRQ        GPIO_PIN_0
#define FM1702_RESET      GPIO_PIN_8

#define FM1702_MOSI_GPIO    GPIO_PIN_7
#define FM1702_MISO_GPIO    GPIO_PIN_6
#define FM1702_SCK_GPIO     GPIO_PIN_5
#define FM1702_CS_GPIO      GPIO_PIN_4


#define FM1702_Reset_H                HAL_GPIO_WritePin(GPIOA, FM1702_RESET, GPIO_PIN_SET)
#define FM1702_Reset_L                HAL_GPIO_WritePin(GPIOA, FM1702_RESET, GPIO_PIN_RESET)
#define FM1702_CS_H                   HAL_GPIO_WritePin(GPIOA, FM1702_CS_GPIO, GPIO_PIN_SET)
#define FM1702_CS_L                   HAL_GPIO_WritePin(GPIOA, FM1702_CS_GPIO, GPIO_PIN_RESET)
#define FM1702_MOSI_H                 HAL_GPIO_WritePin(GPIOA, FM1702_MOSI_GPIO, GPIO_PIN_SET)
#define FM1702_MOSI_L                 HAL_GPIO_WritePin(GPIOA, FM1702_MOSI_GPIO, GPIO_PIN_RESET)
#define FM1702_SCK_H                  HAL_GPIO_WritePin(GPIOA, FM1702_SCK_GPIO, GPIO_PIN_SET)
#define FM1702_SCK_L                  HAL_GPIO_WritePin(GPIOA, FM1702_SCK_GPIO, GPIO_PIN_RESET)
#define FM1702_MISO                   HAL_GPIO_ReadPin(GPIOA, FM1702_MISO_GPIO)

#define FM1702_WAITE_RESPONSE_TIMER_OUT   40*configTICK_RATE_HZ    // 40 S
#define FM1702_SCANNING_CARD_TIME_GAP     500

void rfid_fm1702_isr(void);
void rfid_fm1702_init(void);
unsigned char FM1702Reset(void);
int fm1702_rfid_response(uint8_t response);

#endif
