#ifndef __FUNC_TEST_H__
#define __FUNC_TEST_H__

#include "upgrade.h"
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "rtc.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"

extern void fatfs_test(void);
extern void flash_test(void);
extern void backup_dormain_test(void);
extern void read_bin_version_test(void);
extern int8_t compare_version(const char* ver1, const char* ver2);
extern bool updated_version(void);
#endif
