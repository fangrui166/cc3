#ifndef __GD5820_H__
#define __GD5820_H__
#include "common.h"
#include "global.h"

#define GD5820_AUDIO_PA_GPIO                GPIO_PIN_15
#define GD5820_BUSY_GPIO                    GPIO_PIN_12

#define GD5820_CMD_IRQ                           0xAA

#define GD5820_CMD_PLAY                          0x01
#define GD5820_CMD_PAUSE                         0x02
#define GD5820_CMD_NEXT                          0x03
#define GD5820_CMD_PREVIOUS                      0x04
#define GD5820_CMD_VOLUME_UP                     0x05
#define GD5820_CMD_VOLUME_DOWN                   0x06
#define GD5820_CMD_FAST_FORWARD                  0x0A
#define GD5820_CMD_FAST_BACKWARD                 0x0B
#define GD5820_CMD_SELECT_ITEM                   0x41

#define GD5820_SET_VOLUME                        0x31  //0-30（7E 03 31 00 EF）
#define GD5820_SET_EQ                            0x32  //0-5(NO\POP\ROCK\JAZZ\CLASSIC\BASS) (断电记忆)（7E 03 32 00 EF）
#define GD5820_SET_LOOP_MODE                     0x33  //0-3(ALL\FOLDER\ONE\RANDOM)（7E 03 33 00 EF）
#define GD5820_SET_PLAY_MODE                     0X35  // 00 U盘 03不自动播放第一首，04 自动播放第一首

typedef enum{
    GD5820_QUERY_PLAY_STATE=0x10,       //0(STOP)1(PLAY)2(PAUS)3(FF)4(FR)
    GD5820_QUERY_VOLUME=0x11,           //0-30
    GD5820_QUERY_PLAY_MODE=0x13,        //0-4(ALL\FOLDER\ONE\RANDOM)
    GD5820_QUERY_U_FILE_NUM=0x16,       //1-65535
    GD5820_QUERY_FLASH_FILE_NUM=0x17,   //1-65535
    GD5820_QUERY_PLAY_DEVICE=0x18,      //0:USB 1:SD 2:SPI
    GD5820_QUERY_U_INDEX=0x1A,          //1-65535
    GD5820_QUERY_FLASH_INDEX=0x1B,      //1-200
    GD5820_QUERY_FILE_NAME = 0x1E,
    GD5820_QUERY_CURRENT_PLAY_NUM=0x1F,  //0-65536
}GD5820_QUERY_CMD;

typedef enum{

    GD5820_PLAY_MODED_U = 0,
    GD5820_PLAY_MODED_SPI = 3,
    GD5820_PLAY_MODED_SPI_AUTO = 4,
}GD5820_PLAY_MODED_TYPE;

typedef enum{

    GD5820_PLAY_LOOP_ALL = 0,
    GD5820_PLAY_LOOP_FOLDER,
    GD5820_PLAY_LOOP_ONE,
    GD5820_PLAY_LOOP_RANDOM,
    GD5820_PLAY_LOOP_SINGLE,
}GD5820_PLAY_LOOP_TYPE;

typedef enum {
    EQ_NO,
    EQ_POP,
    EQ_ROCK,
    EQ_JAZZ,
    EQ_CLASSIC,
    EQ_BASS
}GD5820_EQ;
/*
typedef enum{
    LOOP_ALL,
    LOOP_FOLDER,
    LOOP_ONE,
    LOOP_RANDOM
}GD5820_LOOP_MODE;
*/
typedef struct{
    uint8_t len;
    uint16_t para[20];
}gd5820_cmd_para_t;

typedef struct{
    uint8_t cmd;
    gd5820_cmd_para_t data;
}gd5820_message_t;

uint8_t gd5820_query_result(GD5820_QUERY_CMD query_cmd);
osStatus gd5820_send_query_result(uint32_t result);
void gd5820_send_message_queue(gd5820_message_t cmd_message);
void gd5820_send_ctrl_cmd(uint8_t ctrl_cmd);
void gd5820_play_one_mp3(uint32_t mp3_index);
void gd5820_play_multiple_mp3(uint16_t *mp3_array, uint8_t num);
void gd5820_busy_isr(void);
void gd5820_init(void);

#endif
