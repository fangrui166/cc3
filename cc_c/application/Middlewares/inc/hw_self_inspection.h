#ifndef __HW_SELF_INSPECTION_H__
#define __HW_SELF_INSPECTION_H__
#include "cmsis_os.h"

typedef enum{
    INSPECTION_NO_START=0,
    INSPECTION_SUCCESS,
    INSPECTION_FAILED,
    INSPECTION_UNKOWN,
}inspection_result;
typedef enum{
    MODULE_CC_I,
    MODULE_CC_C,
    MODULE_CC_O1,
    MODULE_CC_O2,
} module_tyep;
typedef struct{
    uint8_t * hw_name;
    uint8_t module;
    uint8_t inspection_cmd;
    inspection_result result;
}_inspection;

int hw_self_inspection_set_result(uint8_t * hw_name, uint8_t len);
int hw_self_inspection_start(void);
int hw_self_inspection_init(void);
int hw_self_inspection_dump(void);

#endif
