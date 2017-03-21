#ifndef __KEY_DRV_H__
#define __KEY_DRV_H__
#include "cmsis_os.h"


int key_init(void);

void key_separation( uint16_t KeyValue );
void key_input_lock(void);
void key_input_unlock(void);

#endif
