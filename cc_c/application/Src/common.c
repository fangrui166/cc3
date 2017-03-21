#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "common.h"
#include <stdio.h>
#include <string.h>

uint32_t Device_ID = 0;
uint8_t Full_Device_ID[12] = {0};
uint8_t is_device_id_synced = 0;
void dbgHexDump(unsigned char *buffer, unsigned short length)
{
    unsigned short i, rest;
    char tmp[150];
    rest = length % 8;

    if (length >= 8){
        for (i = 0; i < length/8; i++){
            snprintf(tmp,sizeof(tmp),"0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
                   buffer[i*8], buffer[i*8+1], buffer[i*8+2], buffer[i*8+3],
                   buffer[i*8+4], buffer[i*8+5], buffer[i*8+6], buffer[i*8+7]);
        }
    }
    if (rest > 0){
        for (i = length/8 * 8; i < length; i++){
            snprintf(tmp,sizeof(tmp),"0x%02x ", buffer[i]);
        }
        snprintf(tmp,sizeof(tmp),"\n");
    }
    printf("%s",tmp);
}
void dump_hex(char * buf,int len)
{
    int i=0;
    printf("%s",__func__);
    for(i=0;i<len;i++){
        if(!(i%8))
            printf("\n");
        printf("%#02x ",buf[i]);
    }
    printf("\n");
}
//将1个字符转换为16进制数字
//chr:字符,0~9/A~F/a~F
//返回值:chr对应的16进制数值
static uint8_t chr2hex(uint8_t chr)
{
	if(chr>='0'&&chr<='9')return chr-'0';
	if(chr>='A'&&chr<='F')return (chr-'A'+10);
	if(chr>='a'&&chr<='f')return (chr-'a'+10);
	return 0;
}
//将1个16进制数字转换为字符
//hex:16进制数字,0~15;
//返回值:字符
uint8_t hex2chr(uint8_t hex)
{
	if(hex<=9)return hex+'0';
	if(hex>=10&&hex<=15)return (hex-10+'A');
	return '0';
}
uint32_t str2hex(uint8_t *str)
{
    int i=0;
    uint8_t buf[4]={0};
    uint32_t data = 0;
    uint8_t isodd = 0;
    uint8_t len = (strlen((char *)str) > 8) ? 8 : strlen((char *)str);
    if(len%2){
        len = len/2 + 1;
        isodd = 1;
    }
    else{
        len = len/2;
    }
    for( i = 0; i < len; i++ ){
        buf[i] = chr2hex(*str++)<<4;
        buf[i] += chr2hex(*str++);
        data = ((data << 8) | buf[i]);
    }
    if(isodd){
        data >>= 4;
    }
    return data;
}

COM_INDEX Addr2ComID(uint8_t Addr)
{
    COM_INDEX ComId;
    switch(Addr){
        case NPL_CC_I_ADDR:
            ComId = COM3;
            break;
        case NPL_CC_C_ADDR:
            ComId = MAX_COM;
            xloge("%s error\n",__func__);
            break;
        case NPL_CC_O1_ADDR:
            ComId = COM4;
            break;
        case NPL_CC_O2_ADDR:
            ComId = COM5;
            break;
        default:
            break;
    }
    return ComId;
}
uint32_t Get_Serial_ID(uint8_t *id)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;
  Device_Serial0 = *(volatile uint32_t*)(0x1FFFF7E8);
  Device_Serial1 = *(volatile uint32_t*)(0x1FFFF7EC);
  Device_Serial2 = *(volatile uint32_t*)(0x1FFFF7F0);
  xlogd("serial id:%#x %x %x\n", Device_Serial0, Device_Serial1, Device_Serial2);
  memcpy(id,   (uint8_t*)&Device_Serial0, 4);
  memcpy(id+4, (uint8_t*)&Device_Serial1, 4);
  memcpy(id+8, (uint8_t*)&Device_Serial2, 4);
  return Device_Serial2;
}

