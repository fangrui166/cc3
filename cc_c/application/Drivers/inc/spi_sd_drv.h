#ifndef __SPI_SD_DRV_H__
#define __SPI_SD_DRV_H__
#include "cmsis_os.h"
#include "common.h"
#include "stm32f1xx_hal.h"

#define SPI_SD_CS_GPIO    GPIO_PIN_4
#define SPI_SD_DET_GPIO   GPIO_PIN_0

#define SPI_SD_CS_H                   HAL_GPIO_WritePin(GPIOA, SPI_SD_CS_GPIO, GPIO_PIN_SET)
#define SPI_SD_CS_L                   HAL_GPIO_WritePin(GPIOA, SPI_SD_CS_GPIO, GPIO_PIN_RESET)


#define SPI_SD_DET()                  !HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0) //����п�

#define SPI1_TIMEOUT_MAX   1000

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */

/* SPI�����ٶ�����*/
#define SPI_SPEED_LOW   0
#define SPI_SPEED_HIGH  1

/* SD�����Ͷ��� */
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4

/* SD�������ݽ������Ƿ��ͷ����ߺ궨�� */
#define NO_RELEASE      0
#define RELEASE         1

/* SD��ָ��� */
#define CMD0    0       //����λ
#define CMD9    9       //����9 ����CSD����
#define CMD10   10      //����10����CID����
#define CMD12   12      //����12��ֹͣ���ݴ���
#define CMD16   16      //����16������SectorSize Ӧ����0x00
#define CMD17   17      //����17����sector
#define CMD18   18      //����18����Multi sector
#define ACMD23  23      //����23�����ö�sectorд��ǰԤ�Ȳ���N��block
#define CMD24   24      //����24��дsector
#define CMD25   25      //����25��дMulti sector
#define ACMD41  41      //����41��Ӧ����0x00
#define CMD55   55      //����55��Ӧ����0x01
#define CMD58   58      //����58����OCR��Ϣ
#define CMD59   59      //����59��ʹ��/��ֹCRC��Ӧ����0x00

/* SD DET ״̬*/
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
