#include "led.h"

static osThreadId LedDisplayTaskHandle;
static osMessageQId DisplaySwitchMesQueueHandle;
static volatile uint32_t LedDisplayFrameBuf = 0;
static volatile uint8_t LedDisplaySwitch = LED_DISPLAY_OFF;
static _led_message LedSwitchMessage = {0};
static osTimerId  UpdateDisplayTimerOut;
static osMutexId LedDisplayMutexHandle;

#define LED_DISPALY_FPS  20  // ms

/*
*           ---A--
*           |    |
*           F    B
*           |    |
*           ---G--
*           |    |
*           E    C
*           |    |
*           ---D--¡d
*
*
*/
static void led_unit_num_on(uint8_t num, uint8_t is_dp)
{
	HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_ALL,(GPIO_PinState)LED_DYP_OFF);
	switch(num){
		case 0:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM0,(GPIO_PinState)LED_DYP_ON);
			break;
		case 1:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM1,(GPIO_PinState)LED_DYP_ON);
			break;
		case 2:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM2,(GPIO_PinState)LED_DYP_ON);
			break;
		case 3:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM3,(GPIO_PinState)LED_DYP_ON);
			break;
		case 4:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM4,(GPIO_PinState)LED_DYP_ON);
			break;
		case 5:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM5,(GPIO_PinState)LED_DYP_ON);
			break;
		case 6:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM6,(GPIO_PinState)LED_DYP_ON);
			break;
		case 7:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM7,(GPIO_PinState)LED_DYP_ON);
			break;
		case 8:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM8,(GPIO_PinState)LED_DYP_ON);
			break;
		case 9:
			HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_NUM9,(GPIO_PinState)LED_DYP_ON);
			break;
		case 0x0a:
			break;
		case 0x0b:
			break;
		case 0x0c:
			break;
		case 0x0d:
			break;
		case 0x0e:
			break;
		case 0x0f:
			break;
		default:
			break;

	}
	if(is_dp){
		HAL_GPIO_WritePin(LED_DYP_PORT,(LED_DYPDP_GPIO),(GPIO_PinState)LED_DYP_ON);
	}
}
static void led_unit_selection(uint8_t unit)
{
	switch(unit){
		case 1:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID1_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		case 2:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID2_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		case 3:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID3_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		case 4:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID4_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		case 5:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID5_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		case 6:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID6_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		case 7:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID7_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		case 8:
			HAL_GPIO_WritePin(LED_GID_PORT, LED_GID8_GPIO, (GPIO_PinState)LED_GID_ON);
			break;
		default :
			break;

	}
    osDelay(2);
	HAL_GPIO_WritePin(LED_GID_PORT, LED_GID_ALL, (GPIO_PinState)LED_GID_OFF);
}

static void led_display_off(void)
{
	HAL_GPIO_WritePin(LED_DYP_PORT, LED_DYP_ALL, (GPIO_PinState)LED_DYP_OFF);
	HAL_GPIO_WritePin(LED_GID_PORT, LED_GID_ALL, (GPIO_PinState)LED_GID_OFF);
}

static void led_show_num(uint32_t _num)
{
	uint8_t i = 0, unit = 0;
	uint32_t num = _num;
    osMutexWait(LedDisplayMutexHandle, osWaitForever);
    if(num == 0){
        led_display_off();
        LedDisplaySwitch = LED_DISPLAY_OFF;
        osMutexRelease(LedDisplayMutexHandle);
        return;
    }
	for(i = 0; i < 8; i++){
		unit = num%10;
		led_unit_num_on(unit, LED_DYP_DP_ON);
		led_unit_selection(i+1);
		num = num/10;
		if(num == 0){
			break;
		}
	}
    osMutexRelease(LedDisplayMutexHandle);
}
static void led_gpio_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = (LED_DYPA_GPIO | LED_DYPB_GPIO |
						LED_DYPC_GPIO | LED_DYPD_GPIO |
						LED_DYPE_GPIO | LED_DYPF_GPIO |
						LED_DYPG_GPIO | LED_DYPDP_GPIO);
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LED_DYP_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = (LED_GID1_GPIO | LED_GID2_GPIO |
						LED_GID3_GPIO | LED_GID4_GPIO |
						LED_GID5_GPIO | LED_GID6_GPIO |
						LED_GID7_GPIO | LED_GID8_GPIO);
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(LED_GID_PORT, &GPIO_InitStruct);

}
static void StartLedDisplayTask(void const * argument)
{
	led_set_display_switch(LED_DISPLAY_OFF);
	for(;;){
        _led_message LedMessage = {0};
        if(!DisplaySwitchMesQueueHandle) continue;
        xQueueReceive(DisplaySwitchMesQueueHandle, &LedMessage, portMAX_DELAY);
        xlogd("%s turn %s display\n",__func__,LedMessage.cmd ? "on" : "off");
        if(LedMessage.cmd == LED_DISPLAY_ON){
            LedDisplaySwitch = LED_DISPLAY_ON;
            //led_show_num(LedDisplayFrameBuf);
            osTimerStart(UpdateDisplayTimerOut, LED_DISPALY_FPS);
        }
        else if(LedMessage.cmd == LED_DISPLAY_OFF){
            LedDisplayFrameBuf = 0;
            LedDisplaySwitch = LED_DISPLAY_OFF;
            led_display_off();
            osTimerStop(UpdateDisplayTimerOut);
        }

	}
}

void UpdateDisplayCallback(void const *argument)
{
    led_show_num(LedDisplayFrameBuf);

}

void led_set_display_switch(uint8_t _switch)
{
    BaseType_t ret;
    LedSwitchMessage.cmd = _switch;
    if(DisplaySwitchMesQueueHandle){
        ret = xQueueSend(DisplaySwitchMesQueueHandle, &LedSwitchMessage, portMAX_DELAY);
        if(ret != pdPASS){
            xloge("%s xQueueSend error\n",__func__);
        }
    }
}
void led_set_show_num(uint32_t num)
{
	LedDisplayFrameBuf = num;
	if(LedDisplaySwitch == LED_DISPLAY_OFF){
		led_set_display_switch(LED_DISPLAY_ON);
	}
}

uint32_t led_get_show_num(void)
{
	xlogd("%s :%d, Switch:%d\n",__func__, LedDisplayFrameBuf,LedDisplaySwitch);
	return LedDisplayFrameBuf;
}
void led_init(void)
{
	led_gpio_init();
	osThreadDef(LedDisplayTask, StartLedDisplayTask, osPriorityNormal, 0, 256);
	LedDisplayTaskHandle = osThreadCreate(osThread(LedDisplayTask), NULL);
    if(LedDisplayTaskHandle == NULL){
        xloge("%s osThreadCreate error !!!\n",__func__);
    }
    osMessageQDef(DisplaySwitchMes, 2, _led_message);
    DisplaySwitchMesQueueHandle = osMessageCreate(osMessageQ(DisplaySwitchMes), NULL);
    if(!DisplaySwitchMesQueueHandle){
        xloge("osMessageCreate DisplaySwitchMesQueueHandle error\n");
    }

    osTimerDef(update_display, UpdateDisplayCallback);
    UpdateDisplayTimerOut = osTimerCreate(osTimer(update_display), osTimerPeriodic, NULL);

    osMutexDef(led_display_mutex);
    LedDisplayMutexHandle = osMutexCreate(osMutex(led_display_mutex));
}

