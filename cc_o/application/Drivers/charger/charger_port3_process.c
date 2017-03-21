#include <string.h>
#include "usart_drv.h"
#include "charger_port_process.h"
#include "common.h"

static osThreadId Port3ProcessTaskHandle;
static osMessageQId Port3MessaQueueHandle;
static osTimerId  Port3TimerHandle;

static portMessage_t port3_charge_data;

int send_message_to_port3_process(portMessage_t message)
{
    return xQueueSend(Port3MessaQueueHandle, &message, portMAX_DELAY);

}
void port3_set_charge_busy(uint8_t busy)
{
    uart2_set_port_charge_state(3,busy);
}
void port3_charge_enable(uint8_t enable)
{
    static uint32_t start_charge_ticks = 0;
    if(enable){
        HAL_GPIO_WritePin(GPIOA, CHARGER_RELAY3_GPIO, GPIO_PIN_SET);
        start_charge_ticks = HAL_GetTick();
        port3_set_charge_busy(CHARGER_PORT_STATE_BUSY);
    }
    else{
        HAL_GPIO_WritePin(GPIOA, CHARGER_RELAY3_GPIO, GPIO_PIN_RESET);

        xlog("Stop charge,\tconsume:%d, \n\treason=%s, \n\ttimer:%d\n",port3_charge_data.consume,
                                                stop_charge_reason[port3_charge_data.data.cmd],
                                                (HAL_GetTick()-start_charge_ticks));
        if(port3_charge_data.data.charger_type == CHARGER_TYPE_ONLINE){
            // 在线充电，停止充电后上报充电信息。
            port3_charge_data.state = PORT_STATE_CHARGED;
            uart2_send_charge_data_to_cc_c(port3_charge_data);
        }
        memset(&port3_charge_data, 0xFF, sizeof(portMessage_t));
        port3_set_charge_busy(CHARGER_PORT_STATE_FREE);
    }
}
void Port3TimerOutCallback(void const *argument)
{
    if(port3_charge_data.data.charger_type == CHARGER_TYPE_OFFLINE){
        port3_charge_data.data.cmd = CHARGER_CMD_TIMEOUT;
        xlog("%s \n",__func__);
        port3_charge_enable(0);
    }
}

void port3_irq_hander(void)
{
    static uint32_t last_timestamp = 0;
    static uint8_t overload_count = 0, full_count = 0;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    port3_charge_data.consume ++;
    port3_charge_data.cmd = PORT_CMD_IRQ;

    if((HAL_GetTick() - last_timestamp) < CHARGE_DETECT_OVERLOAD_GAP){
        // 充电过载
        overload_count ++;
        if(overload_count > CHARGE_DETECT_OVERLOAD_COUNT){
            port3_charge_data.data.cmd = CHARGER_CMD_OVERLOAD;
            overload_count = 0;
        }
    }
    else if((HAL_GetTick() - last_timestamp) > CHARGE_DETECT_FULL_GAP){
        // 充电功率小于阈值， 电已充满
        full_count ++;
        if(full_count > CHARGE_DETECT_FULL_COUNT){
            port3_charge_data.data.cmd = CHARGER_CMD_FULL;
            full_count = 0;
        }
    }
    else{
        // 正常充电
        overload_count = 0;
        full_count = 0;
    }
    last_timestamp = HAL_GetTick();
    if(Port3MessaQueueHandle == NULL) return;
    xQueueSendFromISR(Port3MessaQueueHandle, &port3_charge_data, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
void StartPort3ProcessTask(void const * argument)
{
    for(;;){
        portMessage_t message;

        if(!Port3MessaQueueHandle) continue;
        xQueueReceive(Port3MessaQueueHandle, &message, portMAX_DELAY);
        switch(message.cmd){
            case PORT_CMD_IRQ:
                if(message.data.charger_type == CHARGER_TYPE_OFFLINE){
                    uint8_t coin = message.data.charger_prar.offline_prar.coin_count;
                    if(message.consume >= coin * CHARGE_OFF_LINE_CONSUME_PER_COIN){
                        // 充电度数已到，停止充电

                        port3_charge_data.data.cmd = CHARGER_CMD_CONSUME;
                        port3_charge_enable(0);
                    }
                }
                else if(message.data.charger_type == CHARGER_TYPE_ONLINE){
                    static uint8_t consume_count = 0;
                    consume_count ++;
                    if(consume_count >= CHARGE_ON_LINE_REPORT_CONSUME_PER_TIME){
                        consume_count = 0;
                        uart2_send_charge_data_to_cc_c(message);
                    }
                }

                if(message.data.cmd == CHARGER_CMD_OVERLOAD){
                    // 充电端口过载 停止充电
                    //port3_charge_enable(0);
                }
                else if(message.data.cmd == CHARGER_CMD_FULL){
                    // 充电端口功率偏小，已充满 停止充电
                    //port3_charge_enable(0);
                }
                xlog("%s consume=%d\n",__func__,message.consume);
                break;
            case PORT_CMD_SEND_STATE:
                break;
            case PORT_CMD_STOP_CHARGE:
                port3_charge_data.data.cmd = CHARGER_CMD_SET_PORT_STOP;
                port3_charge_enable(0);
                break;
            case PORT_CMD_START_CHARGE:
                message.state = PORT_STATE_CHARGING;
                memcpy(&port3_charge_data, &message, sizeof(portMessage_t));
                if(message.data.charger_type == CHARGER_TYPE_OFFLINE){
                    xlog("start charge,port:%d, type:offline, coin:%d\n",
                        message.data.charger_port, message.data.charger_prar.offline_prar.coin_count);
                    uint8_t coin = message.data.charger_prar.offline_prar.coin_count;
                    osTimerStart(Port3TimerHandle, coin*CHARGE_OFF_LINE_TIME_PER_COIN);
                }
                else if(message.data.charger_type == CHARGER_TYPE_ONLINE){
                    uint32_t security_code = 0;
                    memcpy(&security_code, message.data.charger_prar.online_prar.cardid, 4);
                    xlog("start charge,port:%d, type:online, cardID:%d\n",message.data.charger_port,security_code);
                }
                port3_charge_enable(1);
                break;
            default:
                break;
        }
    }
}
int port3_process_init(void)
{
    osThreadDef(Port3Task, StartPort3ProcessTask, osPriorityNormal, 0, CHARGE_PORT_THREAD_STACK_SIZE);
    Port3ProcessTaskHandle = osThreadCreate(osThread(Port3Task), NULL);
    if(Port3ProcessTaskHandle == NULL){
        xloge("%s Port3ProcessTaskHandle is NULL\n",__func__);
    }
    osMessageQDef(Port3Messa, 2, portMessage_t);
    Port3MessaQueueHandle = osMessageCreate(osMessageQ(Port3Messa), NULL);

    osTimerDef(Port3Timer, Port3TimerOutCallback);
    Port3TimerHandle = osTimerCreate(osTimer(Port3Timer), osTimerOnce, NULL);
    return 0;
}
