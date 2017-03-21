#include "version.h"
#include "flash_memory_define.h"

#pragma location = APP_VERSION_LOCATION_F
__root const char project[16] = "cc-i";

#pragma location = (APP_VERSION_LOCATION_F+0x10)
__root const char version_string[16] = MAINIMAGE_VERSION_STRING;

#pragma location = (APP_VERSION_LOCATION_F+0x20)
__root const char build_date[16] = __DATE__;

#pragma location = (APP_VERSION_LOCATION_F+0x30)
__root const char build_time[16] = __TIME__;

