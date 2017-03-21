#include "charger_manager.h"
#include "communication_service.h"
#include "common.h"
#include <string.h>
#include "http_post.h"

struct charger_manager_data{
    osThreadId ChargerTaskHandle;
    osMessageQId ChargerQueueHandle;
    osTimerId  WaitPortSetTimerOut;
    chargerMessage_t ch_data;
    chargerMessage_t message;
    uint32_t charger_port_status;
    uint8_t wait_port;

};
static struct charger_manager_data charge_data = {0};

int charger_set_o1_state(uint32_t state)
{
    struct charger_manager_data *data = &charge_data;
    data->charger_port_status &= 0xFFC00;
    state &= 0x3FF;
    data->charger_port_status |= state;
    return 0;
}
int charger_set_o2_state(uint32_t state)
{
    struct charger_manager_data *data = &charge_data;
    data->charger_port_status &= 0x3FF;
    state &= 0x3FF;
    state = (state << 10);
    data->charger_port_status |= state;
    return 0;
}
int charger_set_port_status(uint8_t port, uint8_t state)
{
    struct charger_manager_data *data = &charge_data;
    if(state == CHARGER_PORT_STATE_BUSY){
        data->charger_port_status |= 1 << port;
    }
    else if(state == CHARGER_PORT_STATE_FREE){
        data->charger_port_status &= ~(1 << port);
    }
    return 0;
}
int send_message_to_charger_manager(chargerMessage_t *message)
{
    struct charger_manager_data *data = &charge_data;
    return xQueueSend(data->ChargerQueueHandle, message, portMAX_DELAY);

}

int charger_manager_set_type_offline(uint8_t coin_count, uint32_t security_code)
{
    struct charger_manager_data *data = &charge_data;
    chargerMessage_t *message = &data->message;
    xlog("%s coin count:%d, security_code:%#x\n",__func__,coin_count, security_code);
    message->cmd = CHARGER_CMD_SET_TYPE;
    message->charger_type = CHARGER_TYPE_OFFLINE;
    message->charger_prar.offline_prar.coin_count = coin_count;
    message->charger_prar.offline_prar.security_code = security_code;
    return send_message_to_charger_manager(message);
}

int charger_manager_set_type_online(uint32_t card_id)
{
    struct charger_manager_data *data = &charge_data;
    chargerMessage_t *message = &data->message;
    xlog("%s ,card_id:%#x\n",__func__,card_id);
    message->cmd = CHARGER_CMD_SET_TYPE;
    message->charger_type = CHARGER_TYPE_ONLINE;
    message->charger_prar.online_prar.cardid= card_id;
    return send_message_to_charger_manager(message);
}

int charger_manager_set_port(uint8_t port)
{
    struct charger_manager_data *data = &charge_data;
    chargerMessage_t *message = &data->message;
    xlogd("%s port:%d\n",__func__,port);
    message->cmd = CHARGER_CMD_SET_PORT_START;
    message->charger_port= port;
    return send_message_to_charger_manager(message);
}
static void StartChargerTask(void const * argument)
{
    struct charger_manager_data *data = (struct charger_manager_data*)argument;
    chargerMessage_t ch_message;
    for(;;){

        if(!data->ChargerQueueHandle) continue;
        xQueueReceive(data->ChargerQueueHandle, &ch_message, portMAX_DELAY);
        switch(ch_message.cmd){
            case CHARGER_CMD_SET_TYPE:
                data->wait_port = 1;
                memcpy(&data->ch_data, &ch_message, sizeof(chargerMessage_t));
                xTimerReset(data->WaitPortSetTimerOut, WAIT_PORT_SET_TIMEOUT);
                break;
            case CHARGER_CMD_SET_PORT_START:
                if(data->wait_port){

                    if(data->charger_port_status & (1 << ch_message.charger_port)){
                        xlogd("%s port %d is busy now\n",__func__, ch_message.charger_port);
                        uart3_paly_one_mp3(MP3_INDEX_CHARGER_PORT_BUSY);
                        break;
                    }
                    else{
                        data->charger_port_status |= (1 << ch_message.charger_port);
                    }

                    data->wait_port = 0;
                    data->ch_data.cmd = CHARGER_CMD_SET_PORT_START;
                    data->ch_data.charger_port = ch_message.charger_port;

                    if(data->ch_data.charger_type == CHARGER_TYPE_OFFLINE){
                        uart3_set_security_code_to_used(data->ch_data.charger_prar.offline_prar.security_code);
                    }
                    else if(data->ch_data.charger_type == CHARGER_TYPE_ONLINE){
                        http_post_charge_start(data->ch_data.charger_prar.online_prar.cardid,
                            data->ch_data.charger_port);
                    }

                    if(((data->ch_data.charger_port-1)/10)==0){
                        data->ch_data.charger_port = (((data->ch_data.charger_port-1)%10) + 1);
                        uart4_send_charger_data_to_cc_o1(data->ch_data);
                    }
                    else if(((data->ch_data.charger_port-1)/10)==1){
                        data->ch_data.charger_port = (((data->ch_data.charger_port-1)%10) + 1);
                        uart5_send_charger_data_to_cc_o2(data->ch_data);
                    }
                    else{
                        uart3_paly_one_mp3(MP3_INDEX_INPUT_ERROR);
                    }

                    memset(&data->ch_data, 0, sizeof(chargerMessage_t));
                }
                else{
                    uart3_paly_one_mp3(MP3_INDEX_INPUT_TIPS);
                }
                break;
            case CHARGER_CMD_SET_PORT_STOP:
                break;
            default:
                break;
        }
    }

}
static void WaitPortSetCallback(void const *argument)
{
    struct charger_manager_data *data = (struct charger_manager_data*)argument;
    data->wait_port = 0;
    memset(&data->ch_data, 0, sizeof(chargerMessage_t));
}
int charger_init(void)
{
    struct charger_manager_data *data = &charge_data;

    osThreadDef(ChargerTask, StartChargerTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*2);
    data->ChargerTaskHandle = osThreadCreate(osThread(ChargerTask), (void*)data);
    if(data->ChargerTaskHandle == NULL){
      xloge("%s ChargerTaskHandle is NULL\n",__func__);
    }

    osMessageQDef(ChargerMessa, 2, chargerMessage_t);
    data->ChargerQueueHandle = osMessageCreate(osMessageQ(ChargerMessa), NULL);

    osTimerDef(wait_port_set, WaitPortSetCallback);
    data->WaitPortSetTimerOut = osTimerCreate(osTimer(wait_port_set), osTimerOnce,(void*)data);
    osTimerStart(data->WaitPortSetTimerOut, WAIT_PORT_SET_TIMEOUT);
    return 0;
}
