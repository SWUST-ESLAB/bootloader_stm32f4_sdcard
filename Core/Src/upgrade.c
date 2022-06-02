/*! \file upgrade.c
 *  \brief
 *
 *  Details
 *
 *  Copyright (c) 2015 Traxxas, All Rights Reserved
 */

// -----------------------------------------------------------------------------
// Includes
#include <upgrade.h>
#include <ff.h>
#include "dma.h"
#include "fatfs.h"
#include "rtc.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"
#include "main.h"
#include <string.h>
#include <stdbool.h>
#include "stdlib.h"
#include <crc.h>
#include "string.h"


#define Upgrade_WriteBufferSize 512 /*!< bytes, matches microSD Sector Size */
const uint32_t Upgrade_AppBaseAddress = 0x08008000;
#define  VERSION_ADDR 0x08008200
#define  FIRMWARE_OFFSET_ADDR 0x260
#define  FIRMWARE_KEY_ADDR (Upgrade_AppBaseAddress + FIRMWARE_OFFSET_ADDR)

const char FIRMWARE_KEY[] = "Golzm6XhdNLoF82K";


/*! \brief Boot States
 *
 *  This enumeration is used to communicate the current bootloader state to the IO microcontroller.  This
 *  is used by the IO microcontroller to set the LED pattern.
 */
typedef enum {
  BootState_Normal,             /*!< Normal Application Boot */
  BootState_UpgradeInProgress,  /*!< Upgrade In Progress */
  BootState_BootErr,            /*!< Boot/Upgrade Error */

  BootState_Max                 /*!< Number of BootState, must be final entry */
} BootState;

/*! \brief Upgrade Context Data
 */
typedef struct {
  FATFS fatFs;              /*!< Fat File System Context */
  FIL fp;                   /*!< Upgrade File Handle */
  union {
    uint8_t byteArray[Upgrade_WriteBufferSize];
    uint32_t wordArray[Upgrade_WriteBufferSize / 4];
  } writeBuffer;
  uint32_t writeAddress;    /*!< Flash Write Address */
  uint32_t bytesRead;       /*!< Number of bytes read into buffer */
} UpgradeContext;

// -----------------------------------------------------------------------------
// Forward Declarations

static bool upgrade_mount(void);
static bool upgrade_openUpgradeFile(void);
static void upgrade_closeUpgradeFile(void);
static bool upgrade_erase(void);
static void upgrade_program(void);
static bool updated_version(void);
static void upgrade_txBootState(BootState state);

// -----------------------------------------------------------------------------
// Globals

static const char* Upgrade_FileName = "UPGRADE.bin";      /*!< Firmware upgrade file */
static UpgradeContext upgradeContext;

// -----------------------------------------------------------------------------
// Functions

/*! \brief Upgrade via MicroSD
 *
 *  This function will upgrade the application firmware if it is requested. This
 *  function:
 *    1. Mounts the SD card
 *    2. Looks for the existance of the firmware upgrade file
 *    3. Erases flash memory
 *    4. Loads the firmware upgrade file into flash
 *
 */
void upgrade(void) {
	printf("Start upgrade...\r\n");
  uint8_t err = 0;
  volatile uint32_t uartDelay = 0;

  uartDelay = 0x1FFFFF;
  while(uartDelay--);
  upgrade_txBootState(BootState_UpgradeInProgress);
	
  if (upgrade_mount()) {	
    if (upgrade_openUpgradeFile()) {
      err = 1; // default to error
			if(!check_firmware_key()) {
				printf("invild firmware\r\n");
				return;
			}
			if (!updated_version()) {
				printf("Old firmware version\r\n");
				return;
			}
      HAL_FLASH_Unlock();
      if (upgrade_erase()) {
				printf("upgrade_erase...\r\n");
        upgrade_txBootState(BootState_UpgradeInProgress);
        upgrade_program();
        err = 0;
      }

      HAL_FLASH_Lock();
      upgrade_closeUpgradeFile();
    }
  }

  if (err) {
    upgrade_txBootState(BootState_BootErr);
  } else {
    upgrade_txBootState(BootState_Normal);
  }

  uartDelay = 0xFFFF;
  while(uartDelay--);
}

/*! \brief Mount the MicroSD Card
 *
 *  This function checks for the presence of the microSD card.  If it
 *  is detected, it will attempt to mount the card.
 *
 *  \return bool, true if the microSD card was successfully mounted
 */
bool upgrade_mount(void) {

  bool mountSuccess = false;

  if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3) == GPIO_PIN_RESET) {
    if (f_mount(&upgradeContext.fatFs, SDPath, 1) == FR_OK) {
      mountSuccess = true;
			printf("SD Card mount success...\r\n");
    }
  }
  return mountSuccess;
}

/*! \brief Open Upgrade File
 *
 *  Open the firmware upgrade file.  This file is at a well known name and
 *  location.  If the file exists, an upgrade has been requested and the
 *  file will be opened for reading.
 *
 *  If the file does not exist, this function will return 'false' indicating
 *  that an upgrade file was not found or could not be opened.
 */
bool upgrade_openUpgradeFile(void) {
  bool openSuccess = false;

  FRESULT fresult = f_open(&upgradeContext.fp, Upgrade_FileName, FA_READ);
  if (fresult == FR_OK) {
    openSuccess = true;
		printf("open file sucess...\r\n");
  }

  return openSuccess;
}

/*! \brief Close Upgrade File
 */
void upgrade_closeUpgradeFile(void) {
  f_close(&upgradeContext.fp);
}

/*! \brief Erase Flash
 *
 *  This function erases the application flash sections.
 */
bool upgrade_erase(void) {
  uint32_t eraseSuccess = true;
	uint32_t SECTORError = 0;
	FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS; 
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Banks         = FLASH_BANK_1;
	EraseInitStruct.Sector        = FLASH_SECTOR_2;
	EraseInitStruct.NbSectors     = 9;

	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP| FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
											FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
  // Erase the Application Flash sectors
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
		printf("SECTORError :%d\r\n", SECTORError);
		eraseSuccess = false;
	}
	EraseInitStruct.Banks         = FLASH_BANK_2;
	EraseInitStruct.Sector        = FLASH_SECTOR_12;
	EraseInitStruct.NbSectors     = 11;
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
		printf("SECTORError :%d\r\n", SECTORError);
		eraseSuccess = false;
	}
  return eraseSuccess;
}

/*! \brief Program Flash
 *
 *  Write the contents of the flash upgrade file into the Application Flash Sectors.
 */
void upgrade_program(void) {
  upgradeContext.writeAddress = Upgrade_AppBaseAddress;

  do {
    if (f_read(&upgradeContext.fp, upgradeContext.writeBuffer.byteArray, 
				Upgrade_WriteBufferSize, &upgradeContext.bytesRead) == FR_OK) {
      if (upgradeContext.bytesRead % 4 == 0) {
        for (uint32_t i = 0; i < upgradeContext.bytesRead / 4; i++) {
					HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, upgradeContext.writeAddress, 
														upgradeContext.writeBuffer.wordArray[i]);
          // Advance to the next application address
          upgradeContext.writeAddress += 4;
        }
      }
			else {
        for (uint32_t i = 0; i < upgradeContext.bytesRead; i++) {
					
					HAL_StatusTypeDef status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, upgradeContext.writeAddress, 
														upgradeContext.writeBuffer.byteArray[i]);
          // Advance to the next application address
					if (status == HAL_ERROR) {
						printf("Write Error at :%d\r\n", upgradeContext.writeAddress);
					}
          upgradeContext.writeAddress += 1;
        }
     }

    }
  } while (upgradeContext.bytesRead == Upgrade_WriteBufferSize);
}


void upgrade_txBootState(BootState state) {
  // Prepare boot state packet
  printf("upgrade state: %d\r\n", state);
}

bool upgrade_pin_pushed()
{
	return !HAL_GPIO_ReadPin(KEY0_GPIO_Port, KEY0_Pin);
}

bool check_firmware_key()
{
	char key_buf[20]={0};
	key_buf[19] = '\0';
	uint8_t len = 16;
	uint32_t read_len = 0;
	f_lseek(&upgradeContext.fp, FIRMWARE_OFFSET_ADDR);
	FRESULT fresult = f_read(&upgradeContext.fp, key_buf, len, &read_len);
	f_lseek(&upgradeContext.fp, 0);
	if (memcmp(key_buf, FIRMWARE_KEY, 16) == 0) {
		return true;
	}
	
	return false;
}

int8_t compare_version(const char* ver1, const char* ver2)
{
	int p1 = 0 ,p2 = 0;
	int v1 = 0, v2 = 0;
	while (ver1[p1] != '\0' || ver2[p2] != '\0') {
		p1 ++, p2++;
		if ( ver1[p1] != '\0' ) {	
			if ( ver1[p1] != '.') {
				v1 = v1 * 10 + ( ver1[p1] - '0' );
			}
			p1 ++;
		}
		if ( ver2[p2] != '\0' ) {	
			if ( ver2[p2] != '.' ) {
				v2 = v2 * 10 + ( ver2[p2] - '0' );
			}
			p2 ++;
		}
	}
	if ( v1 == v2 ) {
		return 0;
	}
	return  v1 > v2 ? 1 : -1;
}

bool updated_version()
{
	char sd_buf[20]={0};
	char flash_buf[20]={0};
	uint32_t sd_num = 0;
	
	f_lseek(&upgradeContext.fp, 0x200);
	retSD = f_read(&upgradeContext.fp, sd_buf, 15, &sd_num);
	f_lseek(&upgradeContext.fp, 0);
	if (retSD) {
		printf("f_read file failed!\r\n");
		return false;
	}
	else {
		printf("SD version:%s\r\n", sd_buf);
	}
	memcpy(flash_buf, ( uint32_t* )( VERSION_ADDR ), 15);
	printf("Flash version:%s\r\n", flash_buf);
	return compare_version(sd_buf, flash_buf) > 0 ? true :false;
}
