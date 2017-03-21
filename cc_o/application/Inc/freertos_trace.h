#ifndef __FREERTOS_TRACE_H__
#define __FREERTOS_TRACE_H__

#include "cmsis_os.h"
#include "FreeRTOSConfig.h"





void Tim1PeriodElapsedHandle(void);


unsigned int ValueTimerForRunTimeStats(void);
void vConfigureTimerForRunTimeStats(void);


int get_task_state(int argc, char *cmd_arg[]);














#endif
