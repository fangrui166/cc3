#ifndef __SIM900A_H__
#define __SIM900A_H__
#ifdef __cplusplus
 extern "C" {
#endif
#include "cmsis_os.h"

#define SIM900A_SEND_MAX   128

#define SIM900A_POWERKEY_GPIO GPIO_PIN_0
#define SIM900A_POWER_GPIO GPIO_PIN_1
#define SIM900A_WAIT_ACK_TIMEOUT    2000   // 2S

typedef enum {
    SIM900A_OK = 0,
    SIM900A_FAIL,
    SIM900A_TIMEOUT,
    SIM900A_ACK_FAIL,
    SIM900A_UNKOW
}SIM900A_RET;

typedef struct{
    uint8_t * cmd;
    uint8_t * ack;
    uint16_t delay;
}sim900a_cmd;

















void sim900a_init(void);
void sim900a_drv_init(void);
void sim900a_send(uint8_t * buf);
osMessageQId get_sim900a_msag_handle(void);
int sim900aTest(int i);
SIM900A_RET sim900a_send_cmd(uint8_t *cmd, uint8_t *ack);
SIM900A_RET sim900a_send_heartbeat(void);
SIM900A_RET sim900a_send_data(uint8_t *data, uint8_t len);
SIM900A_RET sim900a_tcp_init(void);
int sim900a_is_exist(void);



#ifdef __cplusplus
}
#endif
#endif

