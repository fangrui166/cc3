#include "cmsis_os.h"
#include "key_drv.h"
#include "ttp229.h"
#include "common.h"
#include "communication_service.h"
#include "gd5820.h"
#include "totp.h"

static osThreadId KeyScanTaskHandle;
static uint8_t keypad_lock = 0;

void key_separation( uint16_t KeyValue )
{
	uint8_t per = 0xFF;
    static int last_key = -1;
    static uint32_t temp = 0;
	int key_value = KeyValue & 0xffff;
    if((key_value) && (key_value != 0xFF) && (last_key != key_value))
        printf("%#x \n", key_value);
	switch(key_value)
	{
		case 0x0001:
            per = 1;
            break;
		case 0x1f00:
        case 0x1000:
            per = 2;
            break;
		case 0x80:
            per = 3;
            break;
		case 0x2:
            per = 4;
            break;
		case 0x3f00:
        case 0x2000:
            per = 5;
            break;
		case 0x40:
            per = 6;
            break;
		case 0x4:
            per = 7;
            break;
		case 0x7f00:
        case 0x4000:
            per = 8;
            break;
		case 0x20:
            per = 9;
            break;
        case 0x8000:
            per = 0;
            break;
        case 0x8:  //取消

            if(last_key != key_value){
                last_key = key_value;
                temp = temp/10;
                led_set_show_num(temp);
            }
            break;
		case 0x10: // 确定
		    if(temp <= 0) break;
		    else if(temp/10000000){
                // 8 位验证码
                totp_send_security_code(temp);
		    }
            else if(temp < 20){
                // 选择充电端口号
                cc_i_set_charge_port(temp);
            }
            else{
                gd5820_play_one_mp3(MP3_INDEX_INPUT_ERROR);
            }
		    temp = 0;
            break;
        case 0x800:
            break;
		case 0x400:
            break;
        case 0x200:
            break;
		case 0x100:
            break;

		default:
            //printf("No KEY Press  per=%d\r\n",per);
            break ;
	}
    if(last_key == key_value){
        // long key press
    }
    else{
        // short key press
        last_key = key_value;
        if(per < 10){
            if((temp/10000000) > 0) return ;
            temp = ((temp * 10) + per);
            printf("KEY%d press, temp:%d\r\n",per, temp);
            led_set_show_num(temp);
        }
        else if(temp == 0){
            // cancel or enter key press
            led_set_display_switch(LED_DISPLAY_OFF);
        }
    }
}

void key_input_lock(void)
{
    keypad_lock = 1;

}
void key_input_unlock(void)
{
    keypad_lock = 0;

}
void StartKeyScanTask(void const * argument)
{
    for(;;){
        if(!keypad_lock){
            key_separation(TTP229_ReadOneByte());
        }
        osDelay(100);
    }
}

int key_init(void)
{
    IIC_Init();
    osThreadDef(KeyScanTask, StartKeyScanTask, osPriorityLow, 0, 512);
    KeyScanTaskHandle = osThreadCreate(osThread(KeyScanTask), NULL);
    if(!KeyScanTaskHandle){
        xloge("%s KeyScanTaskHandle is NULL\n",__func__);
    }
    return 0;
}

