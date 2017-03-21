#include "cmsis_os.h"
#include "sim900a.h"
#include "usart_drv.h"
#include "string.h"
#include "CLCPL.h"
#include "http_post.h"

static osThreadId Sim900aTaskHandle;
static osThreadId Sim900aRecTaskHandle;
static osMessageQId Sim900aMsagQueueHandle;
static SemaphoreHandle_t xSemaphoreCheckAck;
static volatile uint8_t WaitAck = pdFALSE;
static APP_COM_DATA app_data = {0};
static int is_sim900a_init = 0;
static char is_sim900a_exist = 0;

static sim900a_cmd tcp_connect_init[]={
    {"AT+CPIN?","OK",0},
    {"AT+CSQ","OK",0},                         // 信号质量查询
    {"AT+CREG?","OK",0},
    {"AT+CGATT?","OK",0},
    {"AT+CSTT=\"CMNET\"","OK",0},                          // GPRS启动
    //{"AT+CREG=0","OK",0},
    //{"AT+CIPCLOSE=1","CLOSE OK",0},              //关闭连接
    //{"AT+CIPSHUT","SHUT OK",0},                  //关闭移动场景
    {"AT+CIICR","OK",0},                         // 移动场景开启,激活
    {"AT+CIFSR","",0},                           // 获取本地IP
    //{"AT+CGCLASS=\"B\"","OK",0},                //设置GPRS移动台类别为B,支持包交换和数据交换
    //{"AT+CGDCONT=1,\"IP\",\"CMNET\"","OK",0},   //设置PDP上下文,互联网接协议,接入点等信息
    //{"AT+CGATT=1","OK",0},                       //附着GPRS业务 ,激活PDP
    //{"AT+CIPCSGP=1,\"CMNET\"","OK",0},           //设置为GPRS连接模式
    //{"AT+CIPHEAD=1","OK",0},                     //设置接收数据显示IP头(方便判断数据来源)
    //{"AT+CLPORT=\"TCP\",\"2022\"","OK",0},       //发送指令指定本地端口
    {"AT+CIPSTART=\"TCP\",\"121.41.105.74\",\"8018\"","CONNECT",0},

};
const unsigned int ulNumberOfInit = (sizeof(tcp_connect_init) / sizeof(tcp_connect_init[0]));

int sim900a_is_exist(void)
{
    return is_sim900a_exist;
}
void sim900a_send(uint8_t * buf)
{
    char str[SIM900A_SEND_MAX];
    if((uint32_t)buf<=0XFF){
        //ClcpSend_B(COM2, buf, 1);
		while((USART2->SR&0X40)==0);//等待上一次数据发送完成
        USART2->DR=(uint32_t)buf;
    }
    else{
        snprintf(str,sizeof(str),"%s\r\n",buf);
        ClcpSend_B(COM2, (uint8_t *)str, strlen(str));
    }

}
uint8_t *sim900a_check_ack(uint8_t * str, uint8_t * rec_ack)
{
	char *strx = 0;
    app_data.app_data[app_data.length] = 0;
    strx=strstr((const char*)app_data.app_data,(const char*)str);
    memcpy((char *)rec_ack, (char *)app_data.app_data, app_data.length);
    memset(&app_data, 0, sizeof(app_data));
	return (uint8_t*)strx;

}

SIM900A_RET sim900a_send_cmd(uint8_t *cmd, uint8_t *ack)
{
    SIM900A_RET ret = SIM900A_OK;
    uint8_t ack_[APP_DATA_LEN] = {0};
    uint8_t retry = 5;
    while(retry){
        sim900a_send(cmd);
        if(xSemaphoreCheckAck && ack){
            WaitAck = pdTRUE;
            if(xSemaphoreTake(xSemaphoreCheckAck, ( TickType_t ) ( SIM900A_WAIT_ACK_TIMEOUT) ) == pdTRUE ){
                if(sim900a_check_ack(ack, ack_)){
                    ret = SIM900A_OK;
                    break;
                }
                else{
                    retry --;
                    ret = SIM900A_ACK_FAIL;
                }
            }
            else{
                retry --;
                ret = SIM900A_TIMEOUT;
            }
            if(retry) osDelay(1000);
            //WaitAck = pdFAIL;
        }
        else{
            break;
        }

    }
    if(ret==SIM900A_ACK_FAIL){
        printf("%s:%s ack fail rec:%s\n",__func__,cmd,ack_);
    }
    else if(ret == SIM900A_TIMEOUT){
        printf("%s:%s waite ack timeout\n",__func__,cmd);
    }
    return ret;
}

SIM900A_RET sim900a_send_data(uint8_t *data, uint8_t len)
{
    SIM900A_RET ret = SIM900A_OK;
    char send_cmd_buf[13];
    //sprintf(send_cmd_buf,"AT+CIPSEND=%d",len);
    sprintf(send_cmd_buf,"AT+CIPSEND");
    if(sim900a_send_cmd((uint8_t *)send_cmd_buf,">") == SIM900A_OK){
        sim900a_send(data);
        if(sim900a_send_cmd((uint8_t*)0x1A,"SEND OK") != SIM900A_OK){
            ret = SIM900A_FAIL;
        }
    }
    else{
        sim900a_send_cmd((uint8_t*)0X1B,0); // ESC,取消发送
        ret = SIM900A_FAIL;
    }
    return ret;
}

SIM900A_RET sim900a_send_heartbeat(void)
{
    SIM900A_RET ret = SIM900A_OK;
    if(sim900a_send_cmd("AT+CIPSEND",">") == SIM900A_OK){
        sim900a_send_cmd((uint8_t*)0X00,0);
        osDelay(20);
        sim900a_send_cmd((uint8_t*)0X1A,0);
    }
    else{
        sim900a_send_cmd((uint8_t*)0X1B,0); // ESC,取消发送
        ret = SIM900A_FAIL;
    }
    return ret;
}
int sim900aTest(int i)
{
    printf("%s(%d)\n",__func__,i);
   return sim900a_send_cmd(tcp_connect_init[i].cmd, tcp_connect_init[i].ack);

}

SIM900A_RET sim900a_tcp_init(void)
{
    int i=0;
    SIM900A_RET ret;
    for(i = 0; i < ulNumberOfInit; i++){
        if(tcp_connect_init[i].delay){
            osDelay(tcp_connect_init[i].delay);
        }
        else{
            //osDelay(300);
        }
        ret = sim900a_send_cmd(tcp_connect_init[i].cmd, tcp_connect_init[i].ack);
        if(ret != SIM900A_OK){
            //break;
        }
    }
    return ret;

}
static void sim900a_init(void)
{
    int i = 0;
    SIM900A_RET ret;
    printf("%s\n",__func__);
    while(sim900a_send_cmd("AT","OK") != SIM900A_OK){
        if(i++ >= 50){
            printf("Not found SIM900A\r\n");
            return ;
        }
    }
    if(i < 50){
        printf("Dectected SIM900A\r\n");
        is_sim900a_exist = 1;
        http_post_init();
    }
    sim900a_send_cmd("ATE0","OK");//不回显
    ret = sim900a_tcp_init();
    if(ret == SIM900A_OK)
        is_sim900a_init = 1;
}
void StartSim900aRecTask(void const * argument)
{

    for(;;){
        if(!Sim900aMsagQueueHandle) continue;
        xQueueReceive( Sim900aMsagQueueHandle, &app_data,  portMAX_DELAY );
        memset(app_data.app_data+app_data.length,'\0',APP_DATA_LEN-app_data.length);
        if(WaitAck){
            WaitAck = pdFAIL;
            printf("rece ack:%s\r\n",app_data.app_data);
            if(xSemaphoreCheckAck){
                xSemaphoreGive( xSemaphoreCheckAck );
            }
        }
        else{
            printf("rece data:%s\r\n",app_data.app_data);
        }
    }
}
void StartSim900aTask(void const * argument)
{
    sim900a_init();
    for(;;){
        osDelay(1000);
        if(is_sim900a_init){
            //sim900a_send_heartbeat();
        }
    }
}
void sim900a_gpio_init(void)
{
        GPIO_InitTypeDef GPIO_InitStruct;

        GPIO_InitStruct.Pin = SIM900A_POWER_GPIO | SIM900A_POWERKEY_GPIO;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}
void sim900a_power(uint8_t on)
{
    if(on){
        HAL_GPIO_WritePin(GPIOA, SIM900A_POWER_GPIO, GPIO_PIN_SET);
    }
    else{

        HAL_GPIO_WritePin(GPIOA, SIM900A_POWER_GPIO, GPIO_PIN_RESET);
    }

    HAL_GPIO_WritePin(GPIOA, SIM900A_POWERKEY_GPIO, GPIO_PIN_RESET);
    HAL_Delay(1000);
    HAL_GPIO_WritePin(GPIOA, SIM900A_POWERKEY_GPIO, GPIO_PIN_SET);
}
void sim900a_drv_init(void)
{
    printf("%s\n",__func__);
    sim900a_gpio_init();
    osThreadDef(Sim900aRecTask, StartSim900aRecTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*2);
    Sim900aRecTaskHandle = osThreadCreate(osThread(Sim900aRecTask), NULL);
    if(Sim900aRecTaskHandle == NULL){
        printf("Sim900aRecTaskHandle fail\n");
    }
    osThreadDef(Sim900aTask, StartSim900aTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*3);
    Sim900aTaskHandle = osThreadCreate(osThread(Sim900aTask), NULL);
    if(Sim900aTaskHandle == NULL){
        printf("Sim900aTaskHandle fail\n");
    }
    osMessageQDef(Sim900aMsag, 3, APP_COM_DATA);
    Sim900aMsagQueueHandle = osMessageCreate(osMessageQ(Sim900aMsag), NULL);
    if(Sim900aMsagQueueHandle == NULL){
        printf("create Message failed\n");
    }
    xSemaphoreCheckAck = xSemaphoreCreateBinary();
    if(xSemaphoreCheckAck == NULL){
        printf("create semaphore failed\n");
    }
    sim900a_power(1);

}
osMessageQId get_sim900a_msag_handle(void)
{
    return Sim900aMsagQueueHandle;
}

