#ifndef __COMMUNICATION_SERVICE_H__
#define __COMMUNICATION_SERVICE_H__

#include "cmsis_os.h"
#include "common.h"
#include "global.h"

uint8_t AppCommSend_A(pAscomm ps, uint8_t Addr,uint8_t Port);

int TypeA_rec_process(uint8_t * data, uint8_t len);
void communication_send_to_cc_c(ascomm data);
void communication_init(void);
int cc_i_get_device_id(void);
int cc_i_set_charge_port(uint8_t port);
int cc_i_set_charge_coin(uint8_t coin_cout, uint32_t security_code);
int cc_i_send_charge_rf_card_id(uint32_t card_id);
int cc_i_set_charge_card_id(uint32_t card_id);

#endif
