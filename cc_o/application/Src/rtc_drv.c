#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "rtc_drv.h"
#include <string.h>

#define MONTH_PER_YEAR 12
static RTC_HandleTypeDef hrtc;
const char MonthStr[MONTH_PER_YEAR][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov","Dec"
    };
static uint32_t RTCReadTimeCounter(RTC_HandleTypeDef* hrtc);
static HAL_StatusTypeDef RTCWriteTimeCounter(RTC_HandleTypeDef* hrtc, uint32_t TimeCounter);
static HAL_StatusTypeDef RTCEnterInitMode(RTC_HandleTypeDef* hrtc);
static HAL_StatusTypeDef RTCExitInitMode(RTC_HandleTypeDef* hrtc);

static void GetCompileDateTime(struct tm *t)
{
    char szTmpDate[40]={0};
    char szTmpTime[20]={0};
    char szMonth[4]={0};
    int iYear,iMonth,iDay,iHour,iMin,iSec;

    //��ȡ�������ڡ�ʱ��
    sprintf(szTmpDate,"%s",__DATE__);    //"Sep 18 2010"
    sprintf(szTmpTime,"%s",__TIME__);    //"10:59:19"

    sscanf(szTmpDate,"%s %d %d",szMonth,&iDay,&iYear);
    sscanf(szTmpTime,"%d:%d:%d",&iHour,&iMin,&iSec);

    for(int i=0;i< MONTH_PER_YEAR;i++){
        if(strncmp(szMonth,MonthStr[i],3)==0){
            iMonth=i+1;
            break;
        }
    }

    t->tm_year = iYear;
    t->tm_mon = iMonth;
    t->tm_mday = iDay;
    t->tm_hour = iHour;
    t->tm_min = iMin;
    t->tm_sec = iSec;
    printf("%d-%d-%d %d:%d:%d\n",t->tm_year,t->tm_mon,t->tm_mday,
        t->tm_hour,t->tm_min,t->tm_sec);
}
/*
static void GetBuildDateTime(RTC_DataTypeDef *date_time)
{
    unsigned char temp_str[4] = {0, 0, 0, 0}, i = 0;

    sscanf(__DATE__, "%s - M", temp_str, &(date_time->Date), &(date_time->Year));
    sscanf(__TIME__, "-:-:-", &(date_time->Hours), &(date_time->Minutes), &(date_time->Seconds));
    for (i = 0; i < 12; i++){
        if (temp_str[0] == MonthStr[i][0] &&
            temp_str[1] == MonthStr[i][1] &&
            temp_str[2] == MonthStr[i][2]){
            date_time->Month = i + 1;
            break;
        }
    }
    printf("%d-%d-%d %d:%d:%d\n",date_time->Year,date_time->Month,date_time->Date,
        date_time->Hours,date_time->Minutes,date_time->Seconds);
}
*/
uint8_t RTCByteToBcd2(uint8_t Value)
{
  uint32_t bcdhigh = 0;

  while(Value >= 10)
  {
    bcdhigh++;
    Value -= 10;
  }

  return  ((uint8_t)(bcdhigh << 4) | Value);
}
uint8_t RTCWeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay)
{
  uint32_t year = 0, weekday = 0;

  year = 2000 + nYear;

  if(nMonth < 3)
  {
    /*D = { [(23 x month)/9] + day + 4 + year + [(year-1)/4] - [(year-1)/100] + [(year-1)/400] } mod 7*/
    weekday = (((23 * nMonth)/9) + nDay + 4 + year + ((year-1)/4) - ((year-1)/100) + ((year-1)/400)) % 7;
  }
  else
  {
    /*D = { [(23 x month)/9] + day + 4 + year + [year/4] - [year/100] + [year/400] - 2 } mod 7*/
    weekday = (((23 * nMonth)/9) + nDay + 4 + year + (year/4) - (year/100) + (year/400) - 2 ) % 7;
  }

  return (uint8_t)weekday;
}

struct tm getnow;
/*
void Rtc_Init(void)
{
    RCC->APB1ENR |= 1<<28; //ʹ��PWRʱ��
    RCC->APB1ENR |= 1<<27;  //ʹ��BKPʱ��,RTCУ׼��BKP��ؼĴ�����
    PWR->CR |= 1<<8;       //ȡ��BKP��ؼĴ���д����
    //RCC->BDCR |= 1<<16;  //����������λ
    //RCC->BDCR |= ~(1<<16);   //����������λ����
    RCC->BDCR |= 1<<0;     //�ⲿ����ʱ�ӣ�LSE��ʹ��
    while(!(RCC->BDCR & 0x02));  //�ȴ��ⲿʱ�Ӿ���
    RCC->BDCR |= 1<<8;         //LSE��ΪRTCʱ��
    RCC->BDCR |= 1<<15;            //RTCʱ��ʹ��
    while(!(RTC->CRL & (1<<5)));   //�ȴ�RTC�Ĵ������һ�β������
    while(!(RTC->CRL & (1<<3)));   //�ȴ�RTC�Ĵ���ͬ�����
    RTC->CRH |= 0x07;                //��������ж�[2]�������ж�[1]�����ж�[0],CRH�Ĵ�������λ��Ч
    while(!(RTC->CRL & (1<<5)));   //�ȴ�RTC�Ĵ������һ�β������
    RTC->CRL |=  1<<4;             //��������ģʽ
    RTC->PRLH = 0x0000;
    RTC->PRLL = 32767;               //�趨��Ƶֵ
    //Rtc_TIME_AutoSet();               //����ǰ����ʱ��д��Ĵ���
    //Rtc_TIME_Set(2012,7,7,20,50,0);   //�꣬�£��գ�ʱ���֣���
    RTC->CRL &= ~(1<<4);           //�˳�����ģʽ����ʼ����RTC�Ĵ���
    while(!(RTC->CRL & (1<<5)));   //�ȴ�RTC�Ĵ������һ�β������

}
*/
/* RTC init function */
void MX_RTC_Init(void)
{
  struct tm now;

  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
  HAL_RTC_Init(&hrtc);
  //Rtc_Init();

  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4) != 0xA5A5){
      HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, 0xA5A5);
      printf("config RTC by first time\n");
      GetCompileDateTime(&now);
      Time_SetCalendarTime(now); //����RTCʱ��
  }
  else{
    printf("RTC was config\n");
  }

}

HAL_StatusTypeDef rtc_get_date_time(struct tm *t)
{
    HAL_StatusTypeDef ret = HAL_OK;
    uint8_t WeekDay=0;
    *t = Time_GetCalendarTime();
    WeekDay = RTCWeekDayNum(t->tm_year%100,t->tm_mon,t->tm_mday);

    printf("week:%d\n",WeekDay);

    printf("%d-%d-%d %d:%d:%d\n",t->tm_year,t->tm_mon,t->tm_mday,
        t->tm_hour,t->tm_min,t->tm_sec);
    return ret;
}
HAL_StatusTypeDef rtc_set_date(uint32_t year, uint8_t month, uint8_t day)
{
    HAL_StatusTypeDef ret = HAL_OK;
    struct tm t;
    t = Time_GetCalendarTime();
    t.tm_year = year;
    t.tm_mon = month;
    t.tm_mday = day;
    printf("Set RTC:%d-%d-%d %d:%d:%d\n",t.tm_year,t.tm_mon,t.tm_mday,
        t.tm_hour,t.tm_min,t.tm_sec);
    Time_SetCalendarTime(t);
    return ret;
}
HAL_StatusTypeDef rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
    HAL_StatusTypeDef ret = HAL_OK;
    struct tm t;
    t = Time_GetCalendarTime();
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    printf("Set RTC:%d-%d-%d %d:%d:%d\n",t.tm_year,t.tm_mon,t.tm_mday,
        t.tm_hour,t.tm_min,t.tm_sec);
    Time_SetCalendarTime(t);
    return ret;
}

/*******************************************************************************
* Function Name  : Time_ConvUnixToCalendar(time_t t)
* Description    : ת��UNIXʱ���Ϊ����ʱ��
* Input 		 : u32 t  ��ǰʱ���UNIXʱ���
* Output		 : None
* Return		 : struct tm
*******************************************************************************/
struct tm Time_ConvUnixToCalendar(time_t t)
{
	struct tm *t_tm;
	t_tm = localtime(&t);
	t_tm->tm_year += 1900;	//localtimeת�������tm_year�����ֵ����Ҫת�ɾ���ֵ
	return *t_tm;
}

/*******************************************************************************
* Function Name  : Time_ConvCalendarToUnix(struct tm t)
* Description    : д��RTCʱ�ӵ�ǰʱ��
* Input 		 : struct tm t
* Output		 : None
* Return		 : time_t
*******************************************************************************/
time_t Time_ConvCalendarToUnix(struct tm t)
{
	t.tm_year -= 1900;  //�ⲿtm�ṹ��洢�����Ϊ2008��ʽ
						//��time.h�ж������ݸ�ʽΪ1900�꿪ʼ�����
						//���ԣ�������ת��ʱҪ���ǵ�������ء�
	return mktime(&t);
}

/*******************************************************************************
* Function Name  : Time_GetUnixTime()
* Description    : ��RTCȡ��ǰʱ���Unixʱ���ֵ
* Input 		 : None
* Output		 : None
* Return		 : time_t t
*******************************************************************************/
time_t Time_GetUnixTime(void)
{
	return (time_t)RTCReadTimeCounter(&hrtc);
}

/*******************************************************************************
* Function Name  : Time_GetCalendarTime()
* Description    : ��RTCȡ��ǰʱ�������ʱ�䣨struct tm��
* Input 		 : None
* Output		 : None
* Return		 : time_t t
*******************************************************************************/
struct tm Time_GetCalendarTime(void)
{
	time_t t_t;
	struct tm t_tm;

	t_t = (time_t)RTCReadTimeCounter(&hrtc);
	t_tm = Time_ConvUnixToCalendar(t_t);
	return t_tm;
}

/*******************************************************************************
* Function Name  : Time_SetUnixTime()
* Description    : ��������Unixʱ���д��RTC
* Input 		 : time_t t
* Output		 : None
* Return		 : None
*******************************************************************************/
void Time_SetUnixTime(time_t t)
{
    RTCWriteTimeCounter(&hrtc, t);
	return;
}

/*******************************************************************************
* Function Name  : Time_SetCalendarTime()
* Description    : ��������Calendar��ʽʱ��ת����UNIXʱ���д��RTC
* Input 		 : struct tm t
* Output		 : None
* Return		 : None
*******************************************************************************/
void Time_SetCalendarTime(struct tm t)
{
	Time_SetUnixTime(Time_ConvCalendarToUnix(t));
	return;
}
static uint32_t RTCReadTimeCounter(RTC_HandleTypeDef* hrtc)
{
#if 0
  uint16_t high1 = 0, high2 = 0, low = 0;
  uint32_t timecounter = 0;

  high1 = READ_REG(hrtc->Instance->CNTH & RTC_CNTH_RTC_CNT);
  low   = READ_REG(hrtc->Instance->CNTL & RTC_CNTL_RTC_CNT);
  high2 = READ_REG(hrtc->Instance->CNTH & RTC_CNTH_RTC_CNT);

  if (high1 != high2)
  { /* In this case the counter roll over during reading of CNTL and CNTH registers,
       read again CNTL register then return the counter value */
    timecounter = (((uint32_t) high2 << 16 ) | READ_REG(hrtc->Instance->CNTL & RTC_CNTL_RTC_CNT));
  }
  else
  { /* No counter roll over during reading of CNTL and CNTH registers, counter
       value is equal to first value of CNTL and CNTH */
    timecounter = (((uint32_t) high1 << 16 ) | low);
  }

  return timecounter;

#else
    uint16_t tmp = 0;
    tmp = RTC->CNTL;

    return (((uint32_t)RTC->CNTH << 16 ) | tmp) ;

#endif
}
static HAL_StatusTypeDef RTCWriteTimeCounter(RTC_HandleTypeDef* hrtc, uint32_t TimeCounter)
{
  HAL_StatusTypeDef status = HAL_OK;

  /* Set Initialization mode */
  if(RTCEnterInitMode(hrtc) != HAL_OK)
  {
    status = HAL_ERROR;
  }
  else
  {
    /* Set RTC COUNTER MSB word */
    WRITE_REG(hrtc->Instance->CNTH, (TimeCounter >> 16));
    /* Set RTC COUNTER LSB word */
    WRITE_REG(hrtc->Instance->CNTL, (TimeCounter & RTC_CNTL_RTC_CNT));

    /* Wait for synchro */
    if(RTCExitInitMode(hrtc) != HAL_OK)
    {
      status = HAL_ERROR;
    }
  }

  return status;
}
static HAL_StatusTypeDef RTCEnterInitMode(RTC_HandleTypeDef* hrtc)
{
  uint32_t tickstart = 0;

  tickstart = HAL_GetTick();
  /* Wait till RTC is in INIT state and if Time out is reached exit */
  while((hrtc->Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET)
  {
    if((HAL_GetTick() - tickstart) >  RTC_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  /* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_DISABLE(hrtc);


  return HAL_OK;
}

static HAL_StatusTypeDef RTCExitInitMode(RTC_HandleTypeDef* hrtc)
{
  uint32_t tickstart = 0;

  /* Disable the write protection for RTC registers */
  __HAL_RTC_WRITEPROTECTION_ENABLE(hrtc);

  tickstart = HAL_GetTick();
  /* Wait till RTC is in INIT state and if Time out is reached exit */
  while((hrtc->Instance->CRL & RTC_CRL_RTOFF) == (uint32_t)RESET)
  {
    if((HAL_GetTick() - tickstart) >  RTC_TIMEOUT_VALUE)
    {
      return HAL_TIMEOUT;
    }
  }

  return HAL_OK;
}

