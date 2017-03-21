#ifndef __LED_H__
#define __LED_H__
#include "common.h"

#define LED_DYPA_GPIO   GPIO_PIN_0
#define LED_DYPB_GPIO   GPIO_PIN_1
#define LED_DYPC_GPIO   GPIO_PIN_2
#define LED_DYPD_GPIO   GPIO_PIN_3
#define LED_DYPE_GPIO   GPIO_PIN_4
#define LED_DYPF_GPIO   GPIO_PIN_5
#define LED_DYPG_GPIO   GPIO_PIN_8
#define LED_DYPDP_GPIO  GPIO_PIN_9
#define LED_DYP_PORT    GPIOB

/*
*           ---A--
*           |    |
*           F    B
*           |    |
*           ---G--
*           |    |
*           E    C
*           |    |
*           ---D--¡d
*
*
*/

#define LED_DYP_ALL     (LED_DYPA_GPIO|LED_DYPB_GPIO|LED_DYPC_GPIO|LED_DYPD_GPIO\
                        |LED_DYPE_GPIO|LED_DYPF_GPIO|LED_DYPG_GPIO|LED_DYPDP_GPIO)
#define LED_DYP_NUM0    (LED_DYPA_GPIO|LED_DYPB_GPIO|LED_DYPC_GPIO|LED_DYPD_GPIO \
						|LED_DYPE_GPIO|LED_DYPF_GPIO)
#define LED_DYP_NUM1    (LED_DYPB_GPIO|LED_DYPC_GPIO)
#define LED_DYP_NUM2    (LED_DYPA_GPIO|LED_DYPB_GPIO|LED_DYPD_GPIO|LED_DYPE_GPIO\
						|LED_DYPG_GPIO)
#define LED_DYP_NUM3    (LED_DYPA_GPIO|LED_DYPB_GPIO|LED_DYPC_GPIO|LED_DYPD_GPIO\
						|LED_DYPG_GPIO)
#define LED_DYP_NUM4    (LED_DYPB_GPIO|LED_DYPC_GPIO|LED_DYPF_GPIO|LED_DYPG_GPIO)
#define LED_DYP_NUM5    (LED_DYPA_GPIO|LED_DYPC_GPIO|LED_DYPD_GPIO|LED_DYPF_GPIO\
						|LED_DYPG_GPIO)
#define LED_DYP_NUM6    (LED_DYPA_GPIO|LED_DYPC_GPIO|LED_DYPD_GPIO|LED_DYPE_GPIO\
						|LED_DYPF_GPIO|LED_DYPG_GPIO)
#define LED_DYP_NUM7    (LED_DYPA_GPIO|LED_DYPB_GPIO|LED_DYPC_GPIO)
#define LED_DYP_NUM8    (LED_DYPA_GPIO|LED_DYPB_GPIO|LED_DYPC_GPIO|LED_DYPD_GPIO\
						|LED_DYPE_GPIO|LED_DYPF_GPIO|LED_DYPG_GPIO)
#define LED_DYP_NUM9    (LED_DYPA_GPIO|LED_DYPB_GPIO|LED_DYPC_GPIO|LED_DYPF_GPIO\
                        |LED_DYPG_GPIO)









#define LED_GID1_GPIO   GPIO_PIN_0
#define LED_GID2_GPIO   GPIO_PIN_1
#define LED_GID3_GPIO   GPIO_PIN_2
#define LED_GID4_GPIO   GPIO_PIN_3
#define LED_GID5_GPIO   GPIO_PIN_4
#define LED_GID6_GPIO   GPIO_PIN_5
#define LED_GID7_GPIO   GPIO_PIN_6
#define LED_GID8_GPIO   GPIO_PIN_7
#define LED_GID_PORT    GPIOC
#define LED_GID_ALL     (LED_GID1_GPIO|LED_GID2_GPIO|LED_GID3_GPIO|LED_GID4_GPIO\
						|LED_GID5_GPIO|LED_GID6_GPIO|LED_GID7_GPIO|LED_GID8_GPIO)

typedef enum {
	LED_DYP_ON,
	LED_DYP_OFF
}LED_DYP_SEG;
typedef enum {
	LED_GID_ON,
	LED_GID_OFF
}LED_GID_SEG;

typedef enum {
	LED_DYP_DP_ON,
    LED_DYP_DP_OFF,
}LED_DYP_DP;

typedef enum {
	LED_DISPLAY_OFF = 0,
	LED_DISPLAY_ON = 1,
}LED_DISPLAY_SWITCH;

typedef struct _LED_MESSAGE
{
	uint8_t cmd;
	void * data;
} _led_message;

void led_set_show_num(uint32_t num);
uint32_t led_get_show_num(void);
void led_set_display_switch(uint8_t _switch);
void led_init(void);

#endif
