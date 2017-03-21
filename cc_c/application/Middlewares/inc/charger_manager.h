#ifndef __CHARGER_MANAGER_H__
#define __CHARGER_MANAGER_H__
#include "cmsis_os.h"

#define WAIT_PORT_SET_TIMEOUT  1*60*1000    // 1分钟

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
    uint8_t resolved;
    uint32_t cardid;
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

int charger_set_port_status(uint8_t port, uint8_t state);
int charger_init(void);
int charger_set_o1_state(uint32_t state);
int charger_set_o2_state(uint32_t state);
int charger_manager_set_type_offline(uint8_t coin_count, uint32_t security_code);
int charger_manager_set_type_online(uint32_t card_id);
int charger_manager_set_port(uint8_t port);

#endif
