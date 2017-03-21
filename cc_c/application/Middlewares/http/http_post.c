#include "http_post.h"
#include "common.h"
#include <string.h>

struct http_post_data{
    osThreadId httpPostHandle;
    osMessageQId httpMsagQueueHandle;
    osTimerId  httpHeardBeatTimerOut;
    http_post_msg HeardBeatMsg;

    uint8_t is_tcp_connect;
    uint8_t is_heard_beat_timer_start;
};
static struct http_post_data http_data = {0};

int http_heard_beat_timer_start(void)
{
    int ret = 0;
    struct http_post_data *data = &http_data;
    if(data->is_tcp_connect){
        ret = osTimerStart(data->httpHeardBeatTimerOut, HTTP_POST_HEARDBEAT_TIMER);
        if(osOK == ret){
            data->is_heard_beat_timer_start = 1;
        }
    }
    return ret;
}

int http_heard_beat_timer_stop(void)
{
    int ret = 0;
    struct http_post_data *data = &http_data;
    if(data->is_heard_beat_timer_start){
        ret = osTimerStop(data->httpHeardBeatTimerOut);
        if(osOK == ret){
            data->is_heard_beat_timer_start = 0;
        }
    }
    return ret;
}
int http_send_msg_queue(uint8_t * web_interface, uint8_t * post_para)
{
    int ret = 0;
    struct http_post_data *data = &http_data;
    http_post_msg *msg_queue = &data->HeardBeatMsg;
    memset(msg_queue, 0, sizeof(http_post_msg));
    msg_queue->cmd = HTTP_POST_CHARGDATA;
    memcpy(msg_queue->web_interface, web_interface, strlen((char*)web_interface));
    memcpy(msg_queue->post_para, post_para, strlen((char*)post_para));
    if(!data->httpMsagQueueHandle) return -2;
    if(xQueueSend(data->httpMsagQueueHandle, msg_queue, portMAX_DELAY) != pdPASS){
        ret = -1;
    }
    return ret;
}
int http_tcp_reconnect(void)
{
    int ret = 0;
    struct http_post_data *data = &http_data;
    http_post_msg *msg_queue = &data->HeardBeatMsg;
    msg_queue->cmd = HTTP_POST_TCP_RECONNESCT;
    if(!data->httpMsagQueueHandle) return -2;
    if(xQueueSend(data->httpMsagQueueHandle, msg_queue, portMAX_DELAY) != pdPASS){
        ret = -1;
    }
    return ret;
}
int http_post_poweron(uint32_t id, uint8_t* manu, uint8_t* modl, uint8_t* version)
{
    uint8_t para[SIM900A_SEND_MAX];
    snprintf((char*)para,sizeof(para),"{\"id\":%d,\"fmwManu\":\"%s\",\"fmwModl\":\"%s\",\"fmwVern\":\"%s\"}",
                    id, manu, modl, version);
    return http_send_msg_queue(HTTP_WEB_INTERFACE_POWERON, para);
}

int http_post_verify_card_id(uint32_t card_id)
{
    return 0;
}
int http_post_charge_start(uint32_t card_id, uint8_t port)
{
    return 0;
}
int http_post_charge_data()
{
    return 0;
}
/*

POST /dev/init/start HTTP/1.1
Host:121.41.105.74:8018
Content-Type:application/json
Content-Length:63
Connection:keep-alive

{"id":1001,"fmwManu":"YYDZ","fmwModl":"cc_c","fmwVern":"1_0_0"}


*/
static int http_send_post(uint8_t * web_interface, uint8_t * post_para)
{
    int ret = 0;
    struct http_post_data *data = &http_data;
    uint8_t protocol[SIM900A_SEND_MAX] = {0};
    uint8_t content_len[SIM900A_SEND_MAX] = {0};
    uint8_t len = strlen((char*)post_para);

    uint8_t http_buf[256] = {0};

    snprintf((char*)protocol,sizeof(protocol),"POST %s HTTP/1.1",web_interface);
    snprintf((char*)content_len,sizeof(content_len),"Content-Length:%d",len);

    snprintf((char*)http_buf,sizeof(http_buf),"%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n",protocol,HTTP_POST_HOST,
        HTTP_POST_CONTENT_TYPE, HTTP_POST_CONNECTION, HTTP_POST_CACHE, content_len);
    xlogd("%s\r\n%s\r\n", http_buf,post_para);
    if(sim900a_send_cmd("AT+CIPSEND",">") == SIM900A_OK){

        sim900a_send(protocol);
        sim900a_send(HTTP_POST_HOST);
        sim900a_send(HTTP_POST_CONTENT_TYPE);
        sim900a_send(HTTP_POST_CONNECTION);
        sim900a_send(HTTP_POST_CACHE);
        sim900a_send(content_len);
        sim900a_send("");  //send \r\n
        sim900a_send(post_para);
        ret = sim900a_send_cmd((uint8_t*)0X1A,"OK");
    }
    else {
        xloge("%s tcp disconnetc\n",__func__);
        data->is_tcp_connect = 0;
        ret = sim900a_send_cmd((uint8_t*)0X1B,0); // ESC,È¡Ïû·¢ËÍ
        http_tcp_reconnect();
    }
    return ret;
}

static void httpHeardBeatCallback(void const *argument)
{
    struct http_post_data *data = (struct http_post_data*)argument;
    http_post_msg *HeardBeat_Msg = &data->HeardBeatMsg;
    memset(HeardBeat_Msg, 0, sizeof(http_post_msg));

    if(data->is_tcp_connect){
        HeardBeat_Msg->cmd = HTTP_POST_HEARDBEAT;
    }
    else{
        HeardBeat_Msg->cmd = HTTP_POST_TCP_RECONNESCT;
    }
    memcpy(HeardBeat_Msg->web_interface, "/dev/init/polling",strlen("/dev/init/polling"));
    memcpy(HeardBeat_Msg->post_para, "{\"id\":1001}", strlen("{\"id\":1001}"));
    if(!data->httpMsagQueueHandle) return ;
    if(xQueueSend(data->httpMsagQueueHandle, HeardBeat_Msg, portMAX_DELAY) != pdPASS){
    }

}

void StartHttpPostTask(void const * argument)
{
    int ret = 0;
    struct http_post_data *data = (struct http_post_data*)argument;
    http_post_msg httpMsag;
    for(;;){
        memset(&httpMsag, 0, sizeof(http_post_msg));
        if(!data->httpMsagQueueHandle) continue;
        xQueueReceive( data->httpMsagQueueHandle, &httpMsag,  portMAX_DELAY );
        http_heard_beat_timer_stop();
        switch(httpMsag.cmd){
            case HTTP_POST_HEARDBEAT:
                http_send_post(httpMsag.web_interface, httpMsag.post_para);
                break;
            case HTTP_POST_CHARGDATA:
                http_send_post(httpMsag.web_interface, httpMsag.post_para);
                break;
            case HTTP_POST_TCP_RECONNESCT:
                ret = (int)sim900a_tcp_init();
                if(ret == SIM900A_OK){
                    data->is_tcp_connect = 1;
                }

            default:
                break;
        }
        http_heard_beat_timer_start();

    }
}

int http_post_init(void)
{
    struct http_post_data *data = &http_data;
    osThreadDef(httpPostTask, StartHttpPostTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*5);
    data->httpPostHandle = osThreadCreate(osThread(httpPostTask), data);
    if(data->httpPostHandle == NULL){
        xloge("httpPostHandle fail\n");
    }
    osMessageQDef(httpMsag, 1, http_post_msg);
    data->httpMsagQueueHandle = osMessageCreate(osMessageQ(httpMsag), NULL);
    if(data->httpMsagQueueHandle == NULL){
        xloge("create Message failed\n");
    }
    osTimerDef(httpHeardBeat, httpHeardBeatCallback);
    data->httpHeardBeatTimerOut = osTimerCreate(osTimer(httpHeardBeat), osTimerPeriodic, data);
    return 0;
}

