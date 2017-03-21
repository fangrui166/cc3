#include "totp.h"
#include "gd5820.h"
#include "rtc_drv.h"
#include "communication_service.h"
#include <stdlib.h>
#include <string.h>
#include "ds3231.h"

static osThreadId TotpTaskHandle;
static osMessageQId TotpMessageHandle;
static osSemaphoreId TotpSemaHandle;
static osTimerId  TotpOverRunTimerHandle;

#define USE_MY_RANDOM 1


#define MIN_RANDOM  1000
#define MAX_RANDOM  9999-MIN_RANDOM

#define MAX_COIN       15
#define MAX_COUNTOFDAY 25

typedef struct {
	int securityCode[MAX_COUNTOFDAY*MAX_COIN];
	int count;
    uint32_t date_seed_bk;
}invalidSecurityCode;
invalidSecurityCode invalidCode = {0};
static uint8_t SecurityCodeRetryCount = 0;
static uint8_t SecurityCodeAnalysisEnable = TOTP_ENABLE;

#if USE_MY_RANDOM
#define RANDOM_MAX 0x7FFFFFFF
static unsigned long my_seed = 0;

/*
    这个算法保证所产生的值不会超过(2^31 - 1)
    这里(2^31 - 1)就是 0x7FFFFFFF。而 0x7FFFFFFF
    等于127773 * (7^5) + 2836,7^5 = 16807。
    整个算法是通过：t = (7^5 * t) mod (2^31 - 1)
*/
static long my_do_rand(unsigned long *value)
{

   long quotient, remainder, t;

   quotient = *value / 127773L;
   remainder = *value % 127773L;
   t = 16807L * remainder - 2836L * quotient;

   if (t <= 0)
      t += 0x7FFFFFFFL;
  return ((*value = t) % ((unsigned long)RANDOM_MAX + 1));
}
int my_rand(void)
{
   return my_do_rand(&my_seed);
}
void my_srand(unsigned int seed)
{
   my_seed = seed;
}
#endif

static uint16_t totp_get_serial_id(void)
{
    return 1234;
}

static uint32_t totp_get_date_seed(void)
{
#if 0
    struct tm time;
    uint32_t date_seed = 0;
    rtc_get_date_time(&time);
    date_seed = (((time.tm_year%100)*10000 )
                + ((time.tm_mon%100) * 100)
                + (time.tm_mday%100));
    date_seed &= 0xFFFFF;
    return date_seed;
#else
    uint32_t date_seed = 0;
    Time_Typedef TimeValue;
    DS3231_Get_Date_Time(&TimeValue);
    date_seed = (((TimeValue.year%100)*10000 )
                + ((TimeValue.month%100) * 100)
                + (TimeValue.date%100));
    date_seed &= 0xFFFFF;
    return date_seed;

#endif
}

static unsigned int totp_my_random(int start, int end)
{
	int max = end;
#if USE_MY_RANDOM
    return start+my_rand()%(max+1);
#else
	return start+rand()%(max+1);
#endif
}

static void set_srand(unsigned int seed)
{
#if USE_MY_RANDOM
    my_srand(seed);
#else
    srand(seed);
#endif
}

/*
    date --> year month day --> 0xFFFF F
    count --> 0xFF
*/
static unsigned int totp_get_random_date(uint32_t date, uint8_t count)
{

	/*     date    |  count
	 *  0x FFFF F  |  0FF
	 */
    int seed = 0;
    seed = (((date & 0xFFFFF) << 12) | (count & 0x0FF));
    set_srand(seed);
    return totp_my_random(MIN_RANDOM,MAX_RANDOM);
}
/*
    serial --> 0xFFFF
    count  --> 0xFF
    coni   --> 0xFF
*/
static unsigned int totp_get_random_id_coin(uint16_t serial_id, uint8_t count,
                                                    uint8_t coin)
{
	/*     serial  |  count  | coin
	 *    0xFFFF   |   FF    |  FF
	 */
    int seed = 0;
    seed = (serial_id << 16) | (count << 8) | coin;
    set_srand(seed);
    return totp_my_random(MIN_RANDOM,MAX_RANDOM);
}

static coin_analysis_type totp_coin_count_analysis(uint32_t security_code,
                                            uint8_t * coin_count )
{
    int index_coin,index_count,index_invalid;
    uint32_t date_seed;
    uint16_t serial_id;
    coin_analysis_type ret = COIN_TYPE_OK;
    serial_id = totp_get_serial_id();
    date_seed = totp_get_date_seed();
    * coin_count = 0;
    if(date_seed != invalidCode.date_seed_bk){
        memset(&invalidCode, 0, sizeof(invalidSecurityCode));
        invalidCode.date_seed_bk = date_seed;
    }
    //printf("serial_id:%d, date_seed:%d\nsecurity_code:%d\n",serial_id, date_seed,security_code);
    for(index_coin = 1; index_coin < MAX_COIN; index_coin ++){
        for(index_count = 0; index_count < MAX_COUNTOFDAY; index_count ++){
            uint16_t tempcode1,tempcode2;
            tempcode1 = totp_get_random_date(date_seed, index_count);
            tempcode2 = totp_get_random_id_coin(serial_id, index_count, index_coin);
            //printf("array[%d,%d]%d\n",index_coin, index_count, tempcode1*10000+tempcode2);
            if((tempcode1*10000+tempcode2) == security_code){
                for(index_invalid = 0; index_invalid < invalidCode.count;
                index_invalid ++){
                    if(security_code ==
                        invalidCode.securityCode[index_invalid]){
                        xloge("The code was used!\n");
                        ret = COIN_TYPE_USED;
                        break;
                    }
                }
                if(index_invalid >= invalidCode.count){
                    xlogd("coin count: %d\n",index_coin);
                    * coin_count = index_coin;
                    //invalidCode.securityCode[invalidCode.count] = security_code;
                    //invalidCode.count ++;
                    ret = COIN_TYPE_OK;
                }
                break;
            }
        }
        if(index_count < MAX_COUNTOFDAY) break;
    }
    if(index_coin >= MAX_COIN){
        xlogw("The code is Invalid\n");
        ret = COIN_TYPE_FAKE;
    }
    return ret;
}

void TotpOverRunTimerCallback(void const *argument)
{
    SecurityCodeAnalysisEnable = 1;
    SecurityCodeRetryCount = 0;

}
void StartTotpTask(void const * argument)
{
    osEvent event_data;
    uint32_t security_code;
    uint8_t coin_cout = 0;
    coin_analysis_type ret;
    for(;;){
        if((!TotpMessageHandle) || (!TotpSemaHandle)) continue;
        event_data = osMessageGet(TotpMessageHandle, osWaitForever);
        osSemaphoreWait(TotpSemaHandle, osWaitForever);
        security_code = (uint32_t)event_data.value.signals;
        ret = totp_coin_count_analysis(security_code, &coin_cout);

        switch (ret){
            case COIN_TYPE_USED:{
                gd5820_play_one_mp3(MP3_INDEX_SECURITY_CODE_USED);
            }
            case COIN_TYPE_FAKE:
                SecurityCodeRetryCount ++;
                if(SecurityCodeRetryCount > SECURITY_CODE_RETRY_MAX){
                    gd5820_play_one_mp3(MP3_INDEX_SECURITY_CODE_RETRY_OVERRUN);
                    //SecurityCodeAnalysisEnable = 0;
                    osTimerStart(TotpOverRunTimerHandle, SECURITY_CODE_RETRY_PAUSE_TIME);
                }
                break;
            case COIN_TYPE_OK:{
                SecurityCodeRetryCount = 0;
                cc_i_set_charge_coin(coin_cout, security_code);
            }
                break;
            default:
                break;
        }

		osSemaphoreRelease(TotpSemaHandle);
    }

}

void totp_set_security_code_used(uint32_t security_code)
{
    osSemaphoreWait(TotpSemaHandle, osWaitForever);
    invalidCode.securityCode[invalidCode.count] = security_code;
    invalidCode.count ++;
    osSemaphoreRelease(TotpSemaHandle);

}
void totp_send_security_code(uint32_t security_code)
{
    if(SecurityCodeAnalysisEnable ){
        if(SecurityCodeRetryCount <= SECURITY_CODE_RETRY_MAX){
	        osMessagePut(TotpMessageHandle, security_code, portMAX_DELAY);
        }
        else{
            gd5820_play_one_mp3(MP3_INDEX_SECURITY_CODE_RETRY_TIPS);
        }
    }
    else{
        gd5820_play_one_mp3(MP3_INDEX_SECURITY_CODE_DISABLE);
    }

}
void totp_init(void)
{
	osThreadDef(TotpTask, StartTotpTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE*2);
	TotpTaskHandle = osThreadCreate(osThread(TotpTask), NULL);
    if(TotpTaskHandle == NULL){
        xloge("%s TotpTaskHandle is NULL\n",__func__);
    }

    osMessageQDef(totp_queue_t, 8, uint32_t);
    TotpMessageHandle = osMessageCreate(osMessageQ(totp_queue_t),NULL);

	osSemaphoreDef(totp_sema);
	TotpSemaHandle = osSemaphoreCreate(osSemaphore(totp_sema), 1);

    osTimerDef(TotpOverRunTimer, TotpOverRunTimerCallback);
    TotpOverRunTimerHandle = osTimerCreate(osTimer(TotpOverRunTimer), osTimerOnce, NULL);

}
