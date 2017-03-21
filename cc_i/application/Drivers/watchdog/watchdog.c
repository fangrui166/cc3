#include "watchdog.h"
#include "stm32f1xx_hal.h"
#include "cmsis_os.h"
#include "common.h"

#ifdef HAL_IWDG_MODULE_ENABLED
IWDG_HandleTypeDef hiwdg;
#endif
#ifdef HAL_WWDG_MODULE_ENABLED
WWDG_HandleTypeDef hwwdg;
#endif



static osThreadId watchdog_th;

void watchdog_refresh(void)
{
#if defined(HAL_IWDG_MODULE_ENABLED)
    if(NULL == hiwdg.Instance)
            return;
    HAL_IWDG_Refresh(&hiwdg);
#elif defined(HAL_WWDG_MODULE_ENABLED)
    if(NULL == hwwdg.Instance)
            return;
    HAL_WWDG_Refresh(&hwwdg, 0x7e);
#endif
}
#ifdef HAL_WWDG_MODULE_ENABLED
/* WWDG init function */
void MX_WWDG_Init(void)
{

    hwwdg.Instance = WWDG;
#if 0
    hwwdg.Init.Prescaler = WWDG_PRESCALER_1;
    hwwdg.Init.Window = 64;
    hwwdg.Init.Counter = 64;
#else
    /*8/9 * 63*/
    hwwdg.Init.Prescaler = WWDG_PRESCALER_8;
    hwwdg.Init.Window = 0x7f;
    hwwdg.Init.Counter = 0x7e;
#endif
    HAL_WWDG_Init(&hwwdg);

}
#endif

#ifdef HAL_IWDG_MODULE_ENABLED
/* IWDG init function */
void MX_IWDG_Init(void)
{
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4095;
  HAL_IWDG_Init(&hiwdg);

}
#endif

static void watchdog_task( void const * pvParameters )
{
    TickType_t xLastWakeTime;
#if defined(HAL_IWDG_MODULE_ENABLED)
    const TickType_t xFrequency = portTICK_PERIOD_MS * 2000;
    MX_IWDG_Init();
    HAL_IWDG_Start(&hiwdg);
#elif defined(HAL_WWDG_MODULE_ENABLED)
    const TickType_t xFrequency = portTICK_PERIOD_MS * 16;
    //HAL_RCC_WWDG_CTRL(1);
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET)
            __HAL_RCC_CLEAR_RESET_FLAGS();
    MX_WWDG_Init();
    HAL_WWDG_Start(&hwwdg);
#else
    const TickType_t xFrequency = portTICK_PERIOD_MS * 10000;

#endif

    (void)pvParameters;
    xlogd("%s Init Done\n", __func__);

    xLastWakeTime = xTaskGetTickCount ();
    for( ;; )
    {
#if defined(HAL_IWDG_MODULE_ENABLED)
        HAL_IWDG_Refresh(&hiwdg);
#elif defined(HAL_WWDG_MODULE_ENABLED)
        HAL_WWDG_Refresh(&hwwdg, 0x7e);
#endif
#if WWDG_TEST == 1
        {
            static int cnt;
            if (cnt%100 == 0)
                  printf("WDG : cnt=%d\n", cnt);
            if (cnt == 1000)
                    while(1)
                        ;
            cnt++;
        }
#endif
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}
void watchdog_init(void)
{
#if defined(HAL_IWDG_MODULE_ENABLED) || defined(HAL_WWDG_MODULE_ENABLED)
    osThreadDef(watchdog, watchdog_task, osPriorityHigh, 0, configMINIMAL_STACK_SIZE*2);
    watchdog_th = osThreadCreate(osThread(watchdog), NULL);
    if(!watchdog_th){
        xloge("%s watchdog_th is NULL\n",__func__);
    }
#endif
}



