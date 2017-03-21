#include "common.h"
#include "fm1702_drv.h"
#include "FM1702.h"
#include "communication_service.h"
#include "gd5820.h"
#include "key_drv.h"

SPI_HandleTypeDef hspi1;
static osThreadId fm1702TaskHandle;
static uint8_t fm1702_card_id_response = 0;
static uint8_t fm1702_lock = 0;
/* SPI1 init function */
void MX_SPI1_Init(void)
{
#if USE_STM_HW_SPI
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    HAL_SPI_Init(&hspi1);
#endif
}
void rfid_fm1702_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
#if !USE_STM_HW_SPI
    GPIO_InitStruct.Pin = FM1702_MISO_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FM1702_MOSI_GPIO | FM1702_SCK_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

#endif
    GPIO_InitStruct.Pin = FM1702_CS_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FM1702_IRQ;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = FM1702_RESET;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

}
#if 0
static void rfid_fm1702_hw_reset(uint8_t reset)
{
    HAL_GPIO_WritePin(GPIOA, FM1702_RESET, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOA, FM1702_RESET, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(GPIOA, FM1702_RESET, GPIO_PIN_RESET);
    HAL_Delay(10);
}
#endif

int fm1702_rfid_response(uint8_t response)
{
    switch(response){
        case RFID_CARD_UNKOWN:
            gd5820_play_one_mp3(MP3_INDEX_RFID_CARD_UNKOWN);
            break;
        case RFID_CARD_OK:
            break;
        case RFID_CARD_NO_BALANCE:
            gd5820_play_one_mp3(MP3_INDEX_RFID_CARD_NO_BALANCE);
            break;
        case RFID_CARD_NO_REGISTER:
            gd5820_play_one_mp3(MP3_INDEX_RFID_CARD_NO_REGISTER);
            break;
        default:
            xloge("%s unkown response\r\n",__func__);
            break;
    }
    fm1702_card_id_response = response;
    return 0;

}
int fm1702_card_read_lock(void)
{
    fm1702_lock = 1;
    return 0;
}
int fm1702_card_read_unlock(void)
{
    fm1702_lock = 0;
    return 0;
}
void StartFm1702Task(void const * argument)
{
    uint32_t card_id = 0,last_card_id = 0;
    uint32_t wait_response_count = 0;
    uint8_t wait_response = 0;
    for(;;){
		//ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
		if(!fm1702_lock){
            card_id = BrushCard();
		}
        if(card_id == 0){
            /* 没有卡靠近 */
        }
        else{
            /* 一张新卡靠近 */
            xlogd("%s id:%#x\n",__func__, card_id);
            gd5820_play_one_mp3(MP3_INDEX_BRUSHCARD);
            led_set_show_num(card_id);
            key_input_lock();
            fm1702_card_read_lock();
            cc_i_send_charge_rf_card_id(card_id);
            wait_response_count = 0;
            wait_response = 1;
            last_card_id = card_id;
            card_id = 0;
        }

        if(wait_response){
            wait_response_count ++;
            if(FM1702_WAITE_RESPONSE_TIMER_OUT <=
                (wait_response_count*FM1702_SCANNING_CARD_TIME_GAP)){
                // 等待超时
                gd5820_play_one_mp3(MP3_INDEX_RFID_WAITE_RESPONSE_TIMEOUT);
                led_set_show_num(0);
                key_input_unlock();
                fm1702_card_read_unlock();
                wait_response = 0;
            }
            if(fm1702_card_id_response == RFID_CARD_OK){
                // rfid 卡验证通过
                //gd5820_play_one_mp3(MP3_INDEX_INPUT_CHARGER_PORT);
                led_set_show_num(0);
                key_input_unlock();
                cc_i_set_charge_card_id(last_card_id);
                wait_response = 0;
            }
            else if(fm1702_card_id_response != 0){
                // rfid 卡验证失败
                led_set_show_num(0);
                key_input_unlock();
                fm1702_card_read_unlock();
                wait_response = 0;
            }
        }

        osDelay(FM1702_SCANNING_CARD_TIME_GAP);
    }
}
void rfid_fm1702_isr(void)
{
    BaseType_t xSensorTaskWoken = pdFALSE;
    xlogd("%s\n",__func__);
    vTaskNotifyGiveFromISR(fm1702TaskHandle, &xSensorTaskWoken);
    portYIELD_FROM_ISR(xSensorTaskWoken);

}
void rfid_fm1702_init(void)
{
    MX_SPI1_Init();
    rfid_fm1702_gpio_init();
    //rfid_fm1702_hw_reset();
    FM1702Reset();
	osThreadDef(fm1702Task, StartFm1702Task, osPriorityLow, 0, 256);
	fm1702TaskHandle = osThreadCreate(osThread(fm1702Task), NULL);
    if(fm1702TaskHandle == NULL){
        xloge("%s osThreadCreate error !!!\n",__func__);
    }

}
