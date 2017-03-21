#ifndef __TOTP_H__
#define __TOTP_H__
#include "cmsis_os.h"
#include "common.h"




typedef enum{
    COIN_TYPE_OK,
    COIN_TYPE_USED,
    COIN_TYPE_FAKE,
}coin_analysis_type;
#define TOTP_ENABLE          1
#define SECURITY_CODE_RETRY_MAX   5
#define SECURITY_CODE_RETRY_PAUSE_TIME     (10*60*1000)   // 10 min

void totp_set_security_code_used(uint32_t security_code);

void totp_send_security_code(uint32_t security_code);
void totp_init(void);

#endif

