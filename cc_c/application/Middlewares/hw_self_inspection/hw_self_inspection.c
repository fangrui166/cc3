#include "hw_self_inspection.h"
#include "communication_service.h"
#include "common.h"
#include "usart_drv.h"
#include "sim900a.h"
extern int uart3_send_to_cc_i(ascomm com_data, uint8_t port);
extern int uart4_send_to_cc_o(ascomm com_data, uint8_t port);
extern int uart5_send_to_cc_o(ascomm com_data, uint8_t port);

_inspection hw_self_inspection_data[] =
{
    {"KEYPAD", MODULE_CC_I, C2I_HW_INSPECTION, INSPECTION_NO_START},
    {"RFID", MODULE_CC_I, C2I_HW_INSPECTION, INSPECTION_NO_START},
    {"RTC", MODULE_CC_I, C2I_HW_INSPECTION, INSPECTION_NO_START},
    {"MP3", MODULE_CC_I, C2I_HW_INSPECTION, INSPECTION_NO_START},
    {"SIM", MODULE_CC_C, C2C_HW_INSPECTION, INSPECTION_NO_START},
    {"portstate_o1", MODULE_CC_O1, C2O_HW_INSPECTION, INSPECTION_NO_START},
    {"portstate_o2", MODULE_CC_O2, C2O_HW_INSPECTION, INSPECTION_NO_START},
};
const unsigned long ulNumberOfInspection = (sizeof(hw_self_inspection_data) / sizeof(hw_self_inspection_data[0]));

int hw_self_inspection_dump(void)
{
    int i = 0;
    for(i = 0; i < ulNumberOfInspection; i++){
        xlog("%s\t --> (%d) %s\r\n",hw_self_inspection_data[i].hw_name,hw_self_inspection_data[i].result,
            (hw_self_inspection_data[i].result == INSPECTION_SUCCESS)? "OK":"FAIL");
    }
    return 0;
}
int hw_self_inspection_set_result(uint8_t * hw_name, uint8_t len)
{
    int i = 0;
    for(i = 0; i < ulNumberOfInspection; i++){
        if(!strncmp((char *)hw_name, (char *)hw_self_inspection_data[i].hw_name, strlen((char *)hw_self_inspection_data[i].hw_name))){
            //xlogd("%s result:\r\n",hw_self_inspection_data[i].hw_name);
            if(hw_name[len-1] == 1){
                hw_self_inspection_data[i].result = INSPECTION_SUCCESS;
            }
            else if(hw_name[len-1] == 0){
                hw_self_inspection_data[i].result = INSPECTION_FAILED;
            }
            else{
                hw_self_inspection_data[i].result = INSPECTION_UNKOWN;
            }
        }
    }
    return 0;
}

int hw_self_inspection_start(void)
{
    int i = 0, count = 0;
    for(i = 0; i < ulNumberOfInspection; i++){
        if(hw_self_inspection_data[i].result == INSPECTION_NO_START){
            count ++;
            switch(hw_self_inspection_data[i].module){
                case MODULE_CC_I:
                    uart3_hw_inspection(hw_self_inspection_data[i].hw_name);
                    break;
                case MODULE_CC_C:
                    if(!strncmp((char *)hw_self_inspection_data[i].hw_name, "SIM", 3)){
                        if(sim900a_is_exist()){
                            hw_self_inspection_data[i].result = INSPECTION_SUCCESS;
                        }
                    }
                    break;
                case MODULE_CC_O1:{
                    uart4_hw_inspection(hw_self_inspection_data[i].hw_name);
                }
                    break;
                case MODULE_CC_O2:
                    uart5_hw_inspection(hw_self_inspection_data[i].hw_name);
                    break;
            }
            osDelay(500);
        }
    }
    if(count == 0){
        xlogd("%s success\n",__func__);
    }
    return count;
}

int hw_self_inspection_init(void)
{
    int i = 0;
    for(i = 0; i < ulNumberOfInspection; i++){
        hw_self_inspection_data[i].result = INSPECTION_NO_START;
    }
    return 0;
}

