#ifndef __CHARGER_PORT_PROCESS_H__
#define __CHARGER_PORT_PROCESS_H__
#include "cmsis_os.h"
#include "stm32f1xx_hal.h"

extern const char *stop_charge_reason[];
#define CHARGE_PORT_THREAD_STACK_SIZE    configMINIMAL_STACK_SIZE*2

#define CHARGER_DETECT1_GPIO      GPIO_PIN_0
#define CHARGER_DETECT2_GPIO      GPIO_PIN_1
#define CHARGER_DETECT3_GPIO      GPIO_PIN_2
#define CHARGER_DETECT4_GPIO      GPIO_PIN_3
#define CHARGER_DETECT5_GPIO      GPIO_PIN_4
#define CHARGER_DETECT6_GPIO      GPIO_PIN_5
#define CHARGER_DETECT7_GPIO      GPIO_PIN_6
#define CHARGER_DETECT8_GPIO      GPIO_PIN_7
#define CHARGER_DETECT9_GPIO      GPIO_PIN_8
#define CHARGER_DETECT10_GPIO     GPIO_PIN_9


#define CHARGER_RELAY1_GPIO       GPIO_PIN_0
#define CHARGER_RELAY2_GPIO       GPIO_PIN_1
#define CHARGER_RELAY3_GPIO       GPIO_PIN_4
#define CHARGER_RELAY4_GPIO       GPIO_PIN_5
#define CHARGER_RELAY5_GPIO       GPIO_PIN_6
#define CHARGER_RELAY6_GPIO       GPIO_PIN_7
#define CHARGER_RELAY7_GPIO       GPIO_PIN_8
#define CHARGER_RELAY8_GPIO       GPIO_PIN_11
#define CHARGER_RELAY9_GPIO       GPIO_PIN_12
#define CHARGER_RELAY10_GPIO      GPIO_PIN_15





#define CHARGER_DETECT_GPIO     (CHARGER_DETECT1_GPIO | CHARGER_DETECT2_GPIO | \
                                CHARGER_DETECT3_GPIO | CHARGER_DETECT4_GPIO |\
                                CHARGER_DETECT5_GPIO | CHARGER_DETECT6_GPIO |\
                                CHARGER_DETECT7_GPIO | CHARGER_DETECT8_GPIO |\
                                CHARGER_DETECT9_GPIO | CHARGER_DETECT10_GPIO )

#define CHARGER_RELAY_GPIO      (CHARGER_RELAY1_GPIO | CHARGER_RELAY2_GPIO |\
                                CHARGER_RELAY3_GPIO | CHARGER_RELAY4_GPIO |\
                                CHARGER_RELAY5_GPIO | CHARGER_RELAY6_GPIO |\
                                CHARGER_RELAY7_GPIO | CHARGER_RELAY8_GPIO |\
                                CHARGER_RELAY9_GPIO | CHARGER_RELAY10_GPIO)

#define CHARGE_OFF_LINE_TIME_PER_COIN    (60*60*configTICK_RATE_HZ)  // 1 hour per coin
#define CHARGE_OFF_LINE_CONSUME_PER_COIN   100     // 1 一个硬币 100 个脉冲
#define CHARGE_DETECT_OVERLOAD_GAP         100    // 100 ms 电表中断频率overload
#define CHARGE_DETECT_FULL_GAP             5000   // 5000 ms 电表无中断认为电已充满。
#define CHARGE_DETECT_OVERLOAD_COUNT       10     // 检测到过载次数超过 10 次断电
#define CHARGE_DETECT_FULL_COUNT           10     // 检测到功率偏小 超过 10次 断电
#define CHARGE_ON_LINE_REPORT_CONSUME_PER_TIME  5 // 在线充电， 每检测到 5 次中断上报一次数据
typedef enum{
    CHARGER_CMD_SET_TYPE = 1,
    CHARGER_CMD_SET_PORT_START,
    CHARGER_CMD_SET_PORT_STOP,
    CHARGER_CMD_OVERLOAD,    // 充电端口过载
    CHARGER_CMD_FULL,        // 充电端口功率偏小，已充满
    CHARGER_CMD_TIMEOUT,     // 充电时间到，停止充电
    CHARGER_CMD_CONSUME,     // 充电度数已到，停止充电  consume
}chargerCMD_type;

typedef enum{
    CHARGER_PORT_STATE_UNKNOWN,
    CHARGER_PORT_STATE_BUSY,
    CHARGER_PORT_STATE_FREE,
}charger_port_status_t;
typedef enum{
    CHARGER_TYPE_ONLINE = 1,
    CHARGER_TYPE_OFFLINE,
}charger_type_t;

typedef struct{
    uint8_t coin_count;
    uint32_t security_code;
}charger_offline_prar;

typedef struct{
    uint8_t cardid[5];
}charger_online_prar;

typedef union{
    charger_online_prar online_prar;
    charger_offline_prar offline_prar;
}chargerPrar_t;

typedef struct{
    chargerCMD_type cmd;
    charger_type_t charger_type;
    chargerPrar_t charger_prar;
    uint8_t charger_port;
}chargerMessage_t;


typedef enum{
    PORT_CMD_IRQ,
    PORT_CMD_SEND_STATE,
    PORT_CMD_START_CHARGE,
    PORT_CMD_STOP_CHARGE,
}portCMD_t;
typedef enum{
    PORT_STATE_NO_CHARGE,
    PORT_STATE_CHARGING,
    PORT_STATE_CHARGED,
}portState_t;

typedef struct{
    portCMD_t cmd;
    portState_t state;
    uint32_t consume;
    chargerMessage_t data;
}portMessage_t;
int port_process_init(void);
int port1_process_init(void);
int port2_process_init(void);
int port3_process_init(void);
int port4_process_init(void);
int port5_process_init(void);
int port6_process_init(void);
int port7_process_init(void);
int port8_process_init(void);
int port9_process_init(void);
int port10_process_init(void);
int send_message_to_port1_process(portMessage_t message);
int send_message_to_port2_process(portMessage_t message);
int send_message_to_port3_process(portMessage_t message);
int send_message_to_port4_process(portMessage_t message);
int send_message_to_port5_process(portMessage_t message);
int send_message_to_port6_process(portMessage_t message);
int send_message_to_port7_process(portMessage_t message);
int send_message_to_port8_process(portMessage_t message);
int send_message_to_port9_process(portMessage_t message);
int send_message_to_port10_process(portMessage_t message);
void port1_irq_hander(void);
void port2_irq_hander(void);
void port3_irq_hander(void);
void port4_irq_hander(void);
void port5_irq_hander(void);
void port6_irq_hander(void);
void port7_irq_hander(void);
void port8_irq_hander(void);
void port9_irq_hander(void);
void port10_irq_hander(void);

#endif
