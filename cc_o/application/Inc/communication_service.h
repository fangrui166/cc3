#ifndef __COMMUNICATION_SERVICE_H__
#define __COMMUNICATION_SERVICE_H__

#include "cmsis_os.h"
#include "common.h"
#include "global.h"

typedef struct{
    uint8_t len;
    uint16_t para[20];
}gd5820_cmd_para_t;

uint8_t AppCommSend_A(pAscomm ps, uint8_t Addr,uint8_t Port);
int TypeA_rec_process(uint8_t * data, uint8_t len);
#endif
