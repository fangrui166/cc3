#include "gd5820.h"
#include "CLCPL.h"
#include <string.h>

static osMessageQId QueryResultQueueHandle;
static osMessageQId Gd5820CmdMessageQueueHandle;
static osThreadId GD5820TaskHandle;
static osSemaphoreId Gd5820SemaHandle;

static void gd5820send_command(uint8_t len, uint8_t cmd, uint16_t parameter)
{
    uint8_t buf[6] = {0};
    buf[0] = 0x7E;
    buf[1] = len;
    buf[2] = cmd;
    //xlogd("%s cmd:%#x\n",__func__,cmd);
    switch(len){
        case 2:
            buf[3] = 0xEF;
            break;
        case 3:
            buf[3] = (parameter & 0xFF );
            buf[4] = 0xEF;
            break;
        case 4:
            buf[3] = ((parameter >> 8) & 0xFF);
            buf[4] = (parameter & 0xFF );
            buf[5] = 0xEF;
            break;
        default:
            xloge("%s error len:%d,cmd:%#x\n",__func__,len,cmd);
            break;
    }
    ClcpSend_B(COM4, buf, len+2);

}

static void gd5820_start_play(void)
{
    gd5820send_command(2, GD5820_CMD_PLAY, 0 );
}
static void gd5820_pause_play(void)
{
    gd5820send_command(2, GD5820_CMD_PAUSE, 0 );
}
static void gd5820_next_play(void)
{
    gd5820send_command(2, GD5820_CMD_NEXT, 0 );
}
static void gd5820_previous_play(void)
{
    gd5820send_command(2, GD5820_CMD_PREVIOUS, 0 );
}
static void gd5820_volume_up(void)
{
    gd5820send_command(2, GD5820_CMD_VOLUME_UP, 0 );
}
static void gd5820_volume_down(void)
{
    gd5820send_command(2, GD5820_CMD_VOLUME_DOWN, 0 );
}
static void gd5820_item_play(uint16_t item)
{
    gd5820send_command(4, GD5820_CMD_SELECT_ITEM, item );
}

static void gd5820_set_volume(uint8_t volume)
{
    if(volume > 30) volume = 30;
    gd5820send_command(3, GD5820_SET_VOLUME, volume );
}
static void gd5820_set_EQ(GD5820_EQ EQ)
{
    gd5820send_command(3, GD5820_SET_EQ, EQ );

}
static void gd5820_set_play_mode(uint8_t mode)
{
    gd5820send_command(3, GD5820_SET_PLAY_MODE, mode );

}

static void gd5820_set_loop_mode(GD5820_PLAY_LOOP_TYPE mode)
{
    gd5820send_command(3, GD5820_SET_LOOP_MODE, mode );

}
uint8_t gd5820_query_result(GD5820_QUERY_CMD query_cmd)
{
    osEvent event;
    gd5820send_command(2, (uint8_t)query_cmd, 0 );
    event = osMessageGet(QueryResultQueueHandle, 200);
    return (uint8_t)event.value.signals;
}
osStatus gd5820_send_query_result(uint32_t result)
{
   return osMessagePut(QueryResultQueueHandle, result, 0);
}
void gd5820_busy_isr(void)
{
    BaseType_t xSensorTaskWoken = pdFALSE;
    gd5820_message_t message_irq = {0};
    message_irq.cmd = GD5820_CMD_IRQ;


    if(!Gd5820CmdMessageQueueHandle) return;
    xQueueSendFromISR( Gd5820CmdMessageQueueHandle, &message_irq, &xSensorTaskWoken );
    portYIELD_FROM_ISR(xSensorTaskWoken);

}

void gd5820_send_message_queue(gd5820_message_t cmd_message)
{
    if(!Gd5820CmdMessageQueueHandle) return;
    xQueueSend(Gd5820CmdMessageQueueHandle, &cmd_message, portMAX_DELAY);

}
void gd5820_play_one_mp3(uint32_t mp3_index)
{
    gd5820_message_t message_mp3;
    xlogd("%s(%d)\n",__func__,mp3_index);
    message_mp3.cmd = GD5820_CMD_SELECT_ITEM;
    message_mp3.data.len = 1;
    message_mp3.data.para[0] = mp3_index;
    gd5820_send_message_queue(message_mp3);

}
void gd5820_play_multiple_mp3(uint16_t *mp3_array, uint8_t num)
{
    gd5820_message_t message_mp3;
    message_mp3.cmd = GD5820_CMD_SELECT_ITEM;
    message_mp3.data.len = num;
    memcpy(message_mp3.data.para, mp3_array, num);
    gd5820_send_message_queue(message_mp3);

}
void gd5820_send_ctrl_cmd(uint8_t ctrl_cmd)
{
    gd5820_message_t message_mp3;
    message_mp3.cmd = ctrl_cmd;
    message_mp3.data.len = 0;
    gd5820_send_message_queue(message_mp3);
}
static void gd5820_gpio_init(void)
{

    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_InitStruct.Pin = GD5820_AUDIO_PA_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GD5820_BUSY_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


    HAL_GPIO_WritePin(GPIOA, GD5820_AUDIO_PA_GPIO, GPIO_PIN_RESET);

}
void StartGD5820Task(void const * argument)
{
    gd5820_message_t message;
    for(;;){
        if((!Gd5820CmdMessageQueueHandle) || (!Gd5820SemaHandle)) continue;
        xQueueReceive(Gd5820CmdMessageQueueHandle, &message, portMAX_DELAY);
        osSemaphoreWait(Gd5820SemaHandle, osWaitForever);

        //xlogd("%s cmd:%#x\n",__func__, message.cmd);
        switch(message.cmd){
            case GD5820_CMD_IRQ:
                if(HAL_GPIO_ReadPin(GPIOA, GD5820_BUSY_GPIO) == GPIO_PIN_SET){
                    HAL_GPIO_WritePin(GPIOA, GD5820_AUDIO_PA_GPIO, GPIO_PIN_RESET);
                }
                else{
                    HAL_GPIO_WritePin(GPIOA, GD5820_AUDIO_PA_GPIO, GPIO_PIN_SET);
                }

                break;
            case GD5820_CMD_PLAY:
                gd5820_start_play();
                break;
            case GD5820_CMD_PAUSE:
                gd5820_pause_play();
                break;
            case GD5820_CMD_NEXT:
                gd5820_next_play();
                break;
            case GD5820_CMD_PREVIOUS:
                gd5820_previous_play();
                break;
            case GD5820_CMD_VOLUME_UP:
                gd5820_volume_up();
                break;
            case GD5820_CMD_VOLUME_DOWN:
                gd5820_volume_down();
                break;
            case GD5820_CMD_FAST_FORWARD:
                break;
            case GD5820_CMD_FAST_BACKWARD:
                break;
            case GD5820_CMD_SELECT_ITEM:
                {
                    int i=0;
                    for(i=0;i < message.data.len; i++){
                        gd5820_item_play(message.data.para[i]);
                    }
                }
                break;
            case GD5820_SET_VOLUME:
                if(message.data.len == 1){
                    gd5820_set_volume(message.data.para[0]);
                }
                break;
            case GD5820_SET_EQ:
                if(message.data.len == 1){
                    gd5820_set_EQ((GD5820_EQ)message.data.para[0]);
                }
                break;
            case GD5820_SET_LOOP_MODE:
                if(message.data.len == 1){
                    gd5820_set_loop_mode((GD5820_PLAY_LOOP_TYPE)message.data.para[0]);
                }
                break;

            case GD5820_SET_PLAY_MODE:
                if(message.data.len == 1){
                    gd5820_set_play_mode(message.data.para[0]);
                }
                break;
            default:
                xloge("Command(%#x) not support\n",message.cmd);
                break;
        }

		osSemaphoreRelease(Gd5820SemaHandle);
    }
}
void gd5820_system_setting(void)
{
    gd5820_set_volume(30);
    gd5820_set_EQ(EQ_POP);
    gd5820_set_play_mode(GD5820_PLAY_MODED_SPI);
    gd5820_set_loop_mode(GD5820_PLAY_LOOP_SINGLE);

}
void gd5820_init(void)
{
    gd5820_gpio_init();
    gd5820_system_setting();

    osMessageQDef(Gd5820QueryResult, 2, uint32_t);
    QueryResultQueueHandle = osMessageCreate(osMessageQ(Gd5820QueryResult), NULL);

    osMessageQDef(Gd5820CmdMessage, 2, gd5820_message_t);
    Gd5820CmdMessageQueueHandle = osMessageCreate(osMessageQ(Gd5820CmdMessage), NULL);

	osThreadDef(gd5820Task, StartGD5820Task, osPriorityNormal, 0, 256);
	GD5820TaskHandle = osThreadCreate(osThread(gd5820Task), NULL);
    if(GD5820TaskHandle == NULL){
        xloge("%s GD5820TaskHandle is NULL\n",__func__);
    }

	osSemaphoreDef(gd5820_sema);
	Gd5820SemaHandle = osSemaphoreCreate(osSemaphore(gd5820_sema), 1);
}

