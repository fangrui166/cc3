#ifndef __RTC_DRV_H__
#define __RTC_DRV_H__
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal_def.h"
#include <time.h>

typedef struct
{
    uint16_t Year;
    uint8_t Month;
    uint8_t Date;
    uint8_t Hours;
    uint8_t Minutes;
    uint8_t Seconds;
}RTC_DataTypeDef;


void MX_RTC_Init(void);
HAL_StatusTypeDef rtc_get_date_time(struct tm *t);
void GetCompileDateTime(struct tm *t);
uint8_t RTCWeekDayNum(uint32_t nYear, uint8_t nMonth, uint8_t nDay);
extern struct tm Time_ConvUnixToCalendar(time_t t);
extern time_t Time_ConvCalendarToUnix(struct tm t);
extern time_t Time_GetUnixTime(void);
extern struct tm Time_GetCalendarTime(void);
extern void Time_SetUnixTime(time_t);
extern void Time_SetCalendarTime(struct tm t);
HAL_StatusTypeDef rtc_set_date(uint32_t year, uint8_t month, uint8_t day);
HAL_StatusTypeDef rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec);

#ifdef __cplusplus
}
#endif


#endif
