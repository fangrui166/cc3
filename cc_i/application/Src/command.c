#include <string.h>
#include <stdlib.h>
#include "usart_drv.h"
#include "command.h"
#include "freertos_trace.h"
#include "rtc_drv.h"
#include "iap.h"
#include "common.h"
#include "led.h"
#include "gd5820.h"
#include "ds3231.h"
#include "totp.h"
#include "fm1702_drv.h"
#include "FM1702.h"
#include "communication_service.h"


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
int RTC_DS3231_CMD(int argc, char * argv[])
{
    Time_Typedef TimeValue;
    if(argc == 1){
        DS3231_Get_Date_Time(&TimeValue);
        xlogd("%d-%d-%d %d:%d:%d\nWeek:%d\n",TimeValue.year,TimeValue.month,TimeValue.date,
            TimeValue.hour,TimeValue.minute,TimeValue.second,TimeValue.week);
    }
    else if(!strcmp(argv[1], "date")){
        if(argc < 5){
            printf("RTC useage:\n\t rtc date year month day \n\t rtc 2016 07 03 \n");
        }
        else{
            DS3231_Set_Date(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        }
    }
    else if(!strcmp(argv[1], "time")){
        if(argc < 5){
            printf("RTC useage:\n\t rtc time hour min sec \n\t rtc 14 22 00 \n");
        }
        else{
            DS3231_Set_Time(atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
        }
    }
    else if(argc < 7){
        printf("RTC useage:\n\t rtc year month day hour min sec\n\t rtc 2016 07 03 14 22 00\n");
    }
    else{
        TimeValue.year = atoi(argv[1]);
        TimeValue.month = atoi(argv[2]);
        TimeValue.date = atoi(argv[3]);
        TimeValue.hour = atoi(argv[4]);
        TimeValue.minute = atoi(argv[5]);
        TimeValue.second = atoi(argv[6]);
        TimeValue.week = RTCWeekDayNum(TimeValue.year,
                            TimeValue.month, TimeValue.date);
        if(TimeValue.week == 0){
            TimeValue.week = 7;
        }
        DS3231_Time_Init(&TimeValue);
    }
    return 0;
}

int led_cmd(int argc, char * argv[])
{
    int i = 0;

    printf("argc:%d\n",argc);
    for(i=0;i< argc ; i ++){
        printf("argv[%d]=%s\n",i,argv[i]);
    }

    if(argc < 2){
		led_get_show_num();
    }
    else if(!strcmp(argv[1],"off")){
		led_set_display_switch(LED_DISPLAY_OFF);
    }
	else{
		uint32_t num = atoi(argv[1]);
		led_set_show_num(num);
	}
	return 0;
}

int audio_test(int argc, char * argv[])
{
    if(argc < 2){
        printf("audio useage:\n\taudio get\n\taudio set\n");
        return 0;
    }
    if(!strcmp(argv[1], "get")){
        if(argc <= 2) return -1;
        if(!strcmp(argv[2], "volume")){
            gd5820_query_result(GD5820_QUERY_VOLUME);
        }
        else if(!strcmp(argv[2], "fnum")){
            gd5820_query_result(GD5820_QUERY_FLASH_FILE_NUM);
        }
        else if(!strcmp(argv[2], "fname")){
            gd5820_query_result(GD5820_QUERY_FILE_NAME);
        }
        else if(!strcmp(argv[2], "pmode")){
            gd5820_query_result(GD5820_QUERY_PLAY_DEVICE);
        }
    }
    else if(!strcmp(argv[1], "set")){
        if(argc <= 2) return -1;
        if(!strcmp(argv[2], "v+")){
            gd5820_send_ctrl_cmd(GD5820_CMD_VOLUME_UP);
        }
        else if(!strcmp(argv[2], "v-")){
            gd5820_send_ctrl_cmd(GD5820_CMD_VOLUME_DOWN);
        }
        else if(!strcmp(argv[2], "play")){
            if(argc >= 4 ){
                gd5820_play_one_mp3(atoi(argv[3]));
            }
            else{
                gd5820_send_ctrl_cmd(GD5820_CMD_PLAY);
            }
        }
        else if(!strcmp(argv[2], "pmode")){

            if(argc >= 4 ){
                gd5820_message_t message_mp3;
                message_mp3.cmd = GD5820_SET_PLAY_MODE;
                message_mp3.data.len = 1;
                message_mp3.data.para[0] = (atoi(argv[3]));
                gd5820_send_message_queue(message_mp3);
            }
        }
    }
    return 0;
}
int get_temperature(int argc, char * argv[])
{
    uint8_t temp[8] = {0};
    DS3231_Read_Temp(temp);
    xlogd("Current temperature:%s ¡æ\n",(char *)temp);
    return 0;
}
int totp_test(int argc, char * argv[])
{
    if(argc < 2){
        printf("totp useage:\n\ttotp security_code\n\ttotp 12345678\n");
        return 0;
    }
    totp_send_security_code(atoi(argv[1]));
    return 0;
}
int fm1702_test(int argc, char * argv[])
{
    if(argc < 2){
        FM1702Reset();
    }
    else{
        if(!strcmp(argv[1], "read")){
            BrushCard();
        }
    }

    return 0;

}
int charge_test(int argc, char * argv[])
{
    uint8_t coin,port;
    uint32_t security_code;
    if(argc < 4){
        xloge("argc < 4\r\n");
        return -1;
    }
    if(!strcmp(argv[1], "offline")){
        coin = atoi(argv[2]);
        port = atoi(argv[3]);
        security_code = 1234;
        cc_i_set_charge_coin(coin, security_code);
        osDelay(100);
        cc_i_set_charge_port(port);

    }

    return 0;

}
int get_device_info(int argc, char * argv[])
{
    if(argc < 2){
        return -1;
    }

    if(!strcmp(argv[1], "device_id")){
        xlog("device id:%#x\n",Device_ID);
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
    {"rtc",      rtc_cmd},
    {"RTC",      RTC_DS3231_CMD},
    {"flash",    flash_test},
    {"led",      led_cmd},
    {"audio",    audio_test},
    {"temp",     get_temperature},
    {"totp",     totp_test},
    {"rfid",     fm1702_test},
    {"charge",   charge_test},
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
    osThreadDef(commandTask, StartCommandTask, osPriorityNormal, 0, 512);
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



















