#ifndef __FLASH_MEMORY_DEFINE_H__
#define __FLASH_MEMORY_DEFINE_H__

/* Vector Table Base ---------------------------------------------------------*/
#define NVIC_VectTab_RAM             ((unsigned int)0x20000000)
#define NVIC_VectTab_FLASH           ((unsigned int)0x08000000)

#define IS_NVIC_VECTTAB(VECTTAB) (((VECTTAB) == NVIC_VectTab_RAM) || \
                                  ((VECTTAB) == NVIC_VectTab_FLASH))
#define IS_NVIC_OFFSET(OFFSET)  ((OFFSET) < 0x0007FFFF)

#define APP_START_OFFSET     0x4000
#define APP_START_ADDRESS    (NVIC_VectTab_FLASH+APP_START_OFFSET)

#define APP_VERSION_LOCATION_F  (APP_START_ADDRESS+0x200)

#endif
