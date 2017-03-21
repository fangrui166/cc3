#include <string.h>
#include <stdlib.h>
#include "usart_drv.h"
#include "command.h"
#include "freertos_trace.h"
#include "sim900a.h"
#include "rtc_drv.h"
#include "iap.h"
#include "common.h"
#include "mqtt.h"
#include "http_post.h"
#include "hw_self_inspection.h"


static osThreadId commandTaskHandle;
static osMessageQId CommdMsgQueueHandle;
static APP_COM_DATA app_data;
static osSemaphoreId cmd_semaphore;

const unsigned char whiteSpace[] = {' ', '\t', '=', '\r', '\n'};

#define FLASH_ADR 0x08018802
#define FLASH_DATA 0x5a5a5a5a

int flash_test(int argc, char * argv[])
{

    if(argc <= 2) return -1;
    else if(!strcmp(argv[1],"r")){
        uint32_t addr = str2hex((uint8_t *)argv[2]);
        uint8_t buf[8]={0};
        printf("addr:%#08x\n",addr);
        Read_Flash(addr, buf, 8);
        dump_hex((char*)buf, 8);
    }
    else if(!strcmp(argv[1],"w")){
        if(argc < 4){
            printf("arg is too less\n");
            return -2;
        }
        uint32_t addr = str2hex((uint8_t *)argv[2]);
        printf("addr:%#08x\n",addr);
        if(argc == 4){
            uint8_t data = atoi(argv[3]);
            write_flash_byte(addr, data);
        }
        else {
            uint8_t array[100]={0};
            memcpy(array, &argv[3], argc-3);
            write_flash_array(addr, argc-3, array);
            memset(array, 0, argc-3);
            Read_Flash(addr, array, argc-3);
        }
    }
    return 0;
}

__weak int get_task_state(int argc, char * argv[])
{

    printf("%s\n",__func__);
    return 0;
}
int Sim900a_test(int argc, char * argv[])
{
    if(argc <= 1) return -1;
    else if(!strncmp(argv[1],"AT+",3)){
        sim900a_send_cmd((uint8_t *)argv[1], NULL);
    }
    else if(!strncmp(argv[1],"hard",4)){
        sim900a_send_heartbeat();
    }
    else if(!strncmp(argv[1],"data",4)){
        if(argc >= 3 ){
            sim900a_send_data((uint8_t *)argv[2],strlen(argv[2]));
        }
    }
    else if(!strncmp(argv[1],"mqtt",4)){
        char meassage[200];
        //将数据合成为JSON格式数据
        snprintf(meassage,sizeof(meassage),"{\"temperature\":%.1f,\"humidity\":%.1f,\"light\":%.1f,\"pressure\":%.1f}",2.0,3.0,4.0,5.0);
        //将数据发送出去
        mqtt_publish("pyboard_value",meassage);
    }
    else if(!strncmp(argv[1],"http_get",8)){
    }
    else if(!strncmp(argv[1],"http_post",9)){
    }
    return 0;
}
int rtc_cmd(int argc, char * argv[])
{
    struct tm date_time;
    if(argc == 1){
        rtc_get_date_time(&date_time);
    }
    else if(!strcmp(argv[1], "date")){
        if(argc < 5){
            printf("RTC useage:\n\t rtc date year month day \n\t rtc 2016 07 03 \n");
        }
        else{
            rtc_set_date(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        }
    }
    else if(!strcmp(argv[1], "time")){
        if(argc < 5){
            printf("RTC useage:\n\t rtc time hour min sec \n\t rtc 14 22 00 \n");
        }
        else{
            rtc_set_time(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        }
    }
    else if(argc < 7){
        printf("RTC useage:\n\t rtc year month day hour min sec\n\t rtc 2016 07 03 14 22 00\n");
    }
    else{
        date_time.tm_year = atoi(argv[1]);
        date_time.tm_mon = atoi(argv[2]);
        date_time.tm_mday = atoi(argv[3]);
        date_time.tm_hour = atoi(argv[4]);
        date_time.tm_min = atoi(argv[5]);
        date_time.tm_sec = atoi(argv[6]);
        Time_SetCalendarTime(date_time);
    }
    return 0;
}
int charge_test(int argc, char * argv[])
{
    if(argc < 2){
    }
    else if(!strcmp(argv[1], "offline")){

        chargerMessage_t ch_data;
        ch_data.charger_port = 1;
        ch_data.cmd = CHARGER_CMD_SET_PORT_START;
        ch_data.charger_type = CHARGER_TYPE_OFFLINE;
        ch_data.charger_prar.offline_prar.coin_count = 1;
        ch_data.charger_prar.offline_prar.security_code = 12345678;
        uart4_send_charger_data_to_cc_o1(ch_data);
    }
    return 0;
}
int http_test(int argc, char * argv[])
{
    int ret = 0;
    if(argc < 2){
        return -1;
    }
    else if(!strcmp(argv[1], "poweron")){
        ret = http_post_poweron(1001, "YYDZ", "CC_C", "0_0_1");
    }
    return ret;
}
int hw_inspection(int argc, char * argv[])
{
    if(argc < 2){
        return -1;
    }
    else if(!strcmp(argv[1], "start")){
        hw_self_inspection_init();
        hw_self_inspection_start();
    }
    else if(!strcmp(argv[1], "dump")){
        hw_self_inspection_dump();
    }
    return 0;
}
int power_test(int argc, char * argv[])
{
    if(argc < 2){
        return -1;
    }
    else if(!strcmp(argv[1], "reboot")){
        __disable_irq();
        HAL_NVIC_SystemReset();
    }
    return 0;
}
const MONITOR_COMMAND commandTable[] =
{
    {"task",     get_task_state},
    {"sim",      Sim900a_test},
    {"rtc",      rtc_cmd},
    {"flash",    flash_test},
    {"charge",   charge_test},
    {"http",     http_test},
    {"hw_test",  hw_inspection},
    {"pm",       power_test},
    {"?",        cmd_help}, //This must be the last command
};
const unsigned long ulNumberOfCommands = (sizeof(commandTable) / sizeof(commandTable[0]));
int cmd_help(int argc, char * argv[])
{
  uint8_t i;

   printf("All Command list\r\n");

  for (i=0; i<(ulNumberOfCommands-1); i++)
  {
     printf("%s\r\n", commandTable[i].command);
  }
  return 0;
}

int isaspace(unsigned char c)
{
    int     i;

    for (i = 0; i < sizeof(whiteSpace); i++)
    {
        if (c == whiteSpace[i])
        {
            return 1;
        }
    }
    return 0;
}

int ParseCommandAndData(unsigned char *pLineBuf, int *argn, unsigned char *argv[], int MaxArgs)
{
    int             n;
    int             i;
    unsigned char   quoteChar;

    n = 0;
    while (n < MaxArgs)
    {
        while (isaspace(*pLineBuf))
        {
            pLineBuf++;
        }

        if (*pLineBuf == '"' || *pLineBuf == '\'')
        {
            quoteChar = *pLineBuf;
            *pLineBuf = (unsigned char)1;
            argv[n++] = pLineBuf++;
            while (*pLineBuf && (*pLineBuf != quoteChar))
            {
                pLineBuf++;
            }
            if (*pLineBuf)
            {
                *pLineBuf = 0;
                pLineBuf++;
            }
            else
            {
                n = 0;                     // Error, no matching quote char
                break;
            }
        }
        else if (*pLineBuf)
        {
            argv[n++] = pLineBuf;
            //
            // Go to the next whiteSpace
            //
            while (*pLineBuf && !isaspace(*pLineBuf))
            {
                pLineBuf++;
            }
            if (*pLineBuf)
            {
                *pLineBuf = 0;
                pLineBuf++;
            }
            else break;
        }
        else break;
    }

    if ((n >= 1) && *argv[0] == '?' && *(argv[0] + 1))
    {
        n++;
        if (n <= MaxArgs)
        {
            for (i = 1; i < n; i++)
            {
                argv[i] = argv[i - 1];
            }
            (argv[1])++;
            argv[0] = (unsigned char*)"?";
        }
    }
    if (n > MaxArgs)
    {
        printf("Too many arguments\n");
        n = 0;
    }
    *argn = n;
    return n;
}
int DoCommand(int argn,unsigned char *argv[])
{
    unsigned int uiCount;

    //
    // The first argument should be the command
    //
    for (uiCount = 0; uiCount < ulNumberOfCommands; uiCount++)
    {
        char resut_cmp = (strcmp((char const*)argv[0], (char const*)commandTable[uiCount].command)== 0);
        if ( resut_cmp )
        {
            return(*(commandTable[uiCount].pFunc))(argn, (char **)argv);

        }
    }

     printf("Command error !!!\n");

    return 0;
}


void StartCommandTask(void const * argument)
{

    int argn;
    unsigned char *argv[MAX_ARGS];
    for(;;)
    {
        if(!CommdMsgQueueHandle) continue;
        xQueueReceive( CommdMsgQueueHandle, &app_data,  portMAX_DELAY );
        if(!cmd_semaphore) continue;
        osSemaphoreWait(cmd_semaphore, osWaitForever);
        memset(app_data.app_data+app_data.length,'\0',APP_DATA_LEN-app_data.length);
        if(app_data.length!= 0)
        {
            if(0 != ParseCommandAndData((unsigned char *)app_data.app_data, &argn, argv, (sizeof(argv) / sizeof(argv[0]))))
            {
                DoCommand(argn, argv);
            }
            else
            {
                printf("CommandManger_Thread %s \r\n", "[No Processor for Command]");
            }
        }
        else
        {
            printf("[No Command]\n");
        }
        osSemaphoreRelease(cmd_semaphore);

    }

}

void cmd_init()
{
    osThreadDef(commandTask, StartCommandTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*3);
    commandTaskHandle = osThreadCreate(osThread(commandTask), NULL);

    osMessageQDef(CommdMsgQueue, 5, APP_COM_DATA);
    CommdMsgQueueHandle = osMessageCreate(osMessageQ(CommdMsgQueue), NULL);

    osSemaphoreDef(cmd_semaphore);
    cmd_semaphore = osSemaphoreCreate(osSemaphore(cmd_semaphore),1);


}


osMessageQId get_queue_commd(void)
{
    return CommdMsgQueueHandle;
}

osThreadId get_commd_task_handle(void)
{
    return commandTaskHandle;
}



















