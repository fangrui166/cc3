#ifndef __SPI_SD_DRV_H__
#define __SPI_SD_DRV_H__
#include "cmsis_os.h"
#include "common.h"
#include "stm32f1xx_hal.h"

#define SPI_SD_CS_GPIO    GPIO_PIN_4
#define SPI_SD_DET_GPIO   GPIO_PIN_0

#define SPI_SD_CS_H                   HAL_GPIO_WritePin(GPIOA, SPI_SD_CS_GPIO, GPIO_PIN_SET)
#define SPI_SD_CS_L                   HAL_GPIO_WritePin(GPIOA, SPI_SD_CS_GPIO, GPIO_PIN_RESET)


#define SPI_SD_DET()                  !HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) //检测有卡

#define SPI1_TIMEOUT_MAX   1000

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/* SPI总线速度设置*/
#define SPI_SPEED_LOW   0
#define SPI_SPEED_HIGH  1

/* SD卡类型定义 */
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4

/* SD传输数据结束后是否释放总线宏定义 */
#define NO_RELEASE      0
#define RELEASE         1

/* SD卡指令表 */
#define CMD0    0       //卡复位
#define CMD9    9       //命令9 ，读CSD数据
#define CMD10   10      //命令10，读CID数据
#define CMD12   12      //命令12，停止数据传输
#define CMD16   16      //命令16，设置SectorSize 应返回0x00
#define CMD17   17      //命令17，读sector
#define CMD18   18      //命令18，读Multi sector
#define ACMD23  23      //命令23，设置多sector写入前预先擦除N个block
#define CMD24   24      //命令24，写sector
#define CMD25   25      //命令25，写Multi sector
#define ACMD41  41      //命令41，应返回0x00
#define CMD55   55      //命令55，应返回0x01
#define CMD58   58      //命令58，读OCR信息
#define CMD59   59      //命令59，使能/禁止CRC，应返回0x00

/* SD DET 状态*/
#define SD_DET_PULL_IN    0
#define SD_DET_PULL_OUT   1

#define SD_CMD_IRQ  1
typedef struct{
    uint8_t cmd;
    uint8_t state;
}SDMessage_t;
int spi_sd_thread_init(void);
void spi_sd_det_isr(void);
int SD_Init(void);
uint8_t SD_ReadSingleBlock(uint32_t sector, uint8_t *buffer);
uint8_t SD_ReadMultiBlock(uint32_t sector, uint8_t *buffer, uint8_t count);
uint8_t SD_WriteSingleBlock(uint32_t sector, const uint8_t *data);
uint8_t SD_WriteMultiBlock(uint32_t sector, const uint8_t *data, uint8_t count);
uint8_t SD_WaitReady(void);
uint32_t SD_GetCapacity(void);

#endif
