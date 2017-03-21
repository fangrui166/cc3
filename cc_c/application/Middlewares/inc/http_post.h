#ifndef __HTTP_POST_H__
#define __HTTP_POST_H__

#include "cmsis_os.h"
#include "sim900a.h"

#define HTTP_POST_HOST "Host:121.41.105.74:8018"
#define HTTP_POST_CONTENT_TYPE "Content-Type:application/json"
#define HTTP_POST_CONNECTION "Connection:keep-alive"
#define HTTP_POST_ENCODING   "Transfer-Encoding: chunked"
#define HTTP_POST_CACHE      "Cache-Control: no-cache"

#define HTTP_WEB_INTERFACE_POWERON  "/dev/init/start"

#define HTTP_POST_HEARDBEAT_TIMER      60*configTICK_RATE_HZ  // 1 min

typedef enum{
    HTTP_POST_HEARDBEAT = 0,
    HTTP_POST_CHARGDATA,
    HTTP_POST_TCP_RECONNESCT,
}http_msg_cmd;

typedef struct {
    http_msg_cmd cmd;
    uint8_t web_interface[SIM900A_SEND_MAX];
    uint8_t post_para[SIM900A_SEND_MAX];
}http_post_msg;


int http_post_init(void);

int http_post_poweron(uint32_t id, uint8_t* manu, uint8_t* modl, uint8_t* version);
int http_post_verify_card_id(uint32_t card_id);
int http_post_charge_start(uint32_t card_id, uint8_t port);
int http_post_charge_data();

#endif
