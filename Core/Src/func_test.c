//#include "func_test.h"
//#include "stdlib.h"
//#include "string.h"
//#include "upgrade.h"

//#define VERSION_ADDR (Upgrade_AppBaseAddress + 0x200)
//void fatfs_test() //ok
//{
//	static unsigned sta = 0;
//	const char write_buf[100] = "test\r\n";
//	switch (sta) {
//		case 0: 
//			retSD = f_mount(&SDFatFS, SDPath, 1);
//			if (!retSD) {
//				printf("SD mount sucess!\r\n");
//				sta ++;
//			} else {
//				printf("SD mount failed! code : %d\r\n", retSD);
//			}
//			break;
//		case 1: 
//			retSD = f_mkdir("sd_test");
//			if (retSD == 0 || retSD == 8) {
//				printf("SD mkdir sucess!\r\n");
//				sta ++;
//			} else {
//				printf("SD mkdir failed! code : %d\r\n", retSD);
//			}
//			break;
//		case 2: 
//			if (f_open(&SDFile, "sd_test/log.txt", FA_CREATE_ALWAYS |
//				FA_WRITE | FA_READ) == FR_OK) {
//				printf("SD open sucess!\r\n");
//				sta ++;
//			} else {
//				printf("SD open failed!\r\n");
//			}
//			break;
//		case 3: 
//			for (int i = 0; i< 100; ++i) {
//				if (f_write(&SDFile, write_buf, strlen(write_buf),NULL) == FR_OK) {
//					printf("SD write count: %d!\r\n", i+1);
//				} else {
//					printf("SD write failed!\r\n");
//				}
//			}
//			sta ++;
//			f_sync(&SDFile);
//			break;
//		case 4: 
//			if (f_close(&SDFile)) {
//				printf("SD close sucess!\r\n");
//				sta ++;
//			} else {
//				printf("SD close failed!\r\n");
//			}
//			break;
//		default: 
//			printf("task end!\r\n");
//			break;
//	}
//}

//void flash_test() //ok
//{
//	const unsigned LEN = 10;
//	int i;
//	uint32_t PageError = 0;
//	FLASH_EraseInitTypeDef FlashSet;
//	HAL_StatusTypeDef status;
//	
//	uint32_t addr = 0x0800C100;
//	uint32_t data_buf[LEN];
//	

//	memcpy(data_buf, (uint32_t*)addr, sizeof(uint32_t)*LEN);
//	printf("read before erase:\r\n\t");
//	for(i = 0;i < LEN;i++)
//	{
//		printf("0x%08x ", data_buf[i]);
//	}
//	printf("\r\n");
//	

//	FlashSet.TypeErase = FLASH_TYPEERASE_SECTORS;
//	//FlashSet.Banks = FLASH_BANK_1;
//	FlashSet.Sector = FLASH_SECTOR_3;
//	FlashSet.NbSectors = 1;

//	HAL_FLASH_Unlock();
//	HAL_Delay(10);
//	status = HAL_FLASHEx_Erase(&FlashSet, &PageError);
//	HAL_Delay(10);
//	HAL_FLASH_Lock();
//	if(status != HAL_OK)
//	{
//		printf("erase fail, PageError = %d\r\n", PageError);
//	}
//	printf("erase success\r\n");
//	
//	
//	memcpy(data_buf, (uint32_t*)addr, sizeof(uint32_t)*LEN);
//	printf("read after erase:\r\n\t");
//	for(i = 0;i < LEN;i++)
//	{
//		printf("0x%08x ", data_buf[i]);
//	}
//	printf("\r\n");

//	HAL_FLASH_Unlock();
//	HAL_Delay(10);
//	for (i = 0; i < LEN * sizeof(uint32_t); i+=4)
//	{
//			status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, (uint64_t)i);
//			HAL_Delay(2);
//			if(status != HAL_OK)
//			{
//				break;
//			}
//	}
//	HAL_Delay(10);
//	HAL_FLASH_Lock();
//	if(i < LEN)
//	{
//		printf("write fail, i = %d\r\n", i);
//	}
//	else
//	{
//		printf("write success\r\n");
//	}

//	memcpy(data_buf, (uint32_t*)addr, sizeof(uint32_t)*LEN);
//	printf("read after write:\r\n\t");
//	for(i = 0;i < LEN;i++)
//	{
//		printf("0x%08x ", data_buf[i]);
//	}
//	printf("\r\n");

//}

//void backup_dormain_test() //ok
//{
//	uint32_t data = 123456;
//	uint32_t read_data;
//	read_data = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0);
//	printf("read data before write: %d\r\n", read_data);
//	
//	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, data);
//	if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != data) {
//		printf("read error data after write: %d\r\n", data);
//		return;
//	}
//	printf("read data after write: %d\r\n", data);
//}

//void read_bin_version_test()
//{

//	char fatfs_buf[20]={0};
//	char mem_buf[20]={0};
//	int num = 10;
//	uint32_t read_num = 0;
//	retSD = f_mount(&SDFatFS, SDPath, 1);
//	if (retSD) {
//		printf("mount SD failed!\r\n");
//		return;
//	}
//	else {
//		printf("mount SD ok!\r\n");
//	}
//	retSD = f_open(&SDFile, "FLASH.bin", FA_READ);
//	if (retSD) {
//		printf("open file failed code: %d!\r\n", retSD);
//		return;
//	}
//	else {
//		printf("open file ok!\r\n");
//	}
//	
//	retSD = f_lseek(&SDFile, 0x200);
//	if (retSD) {
//		printf("f_lseek file failed!\r\n");
//		return;
//	}
//	else {
//		printf("f_lseek ok!\r\n");
//	}
//	
//	retSD = f_read(&SDFile, fatfs_buf, num, &read_num);
//		if (retSD) {
//		printf("f_read file failed!\r\n");
//		return;
//	}
//	else {
//		printf("f_read ok!\r\n");
//	}
//	
//	printf("from fatfs: %s\r\n", fatfs_buf);
//	
//	memcpy(mem_buf, (uint32_t*)VERSION_ADDR, 10);
//	printf("from mem: %s\r\n", mem_buf);
//	if (strcmp(fatfs_buf, mem_buf) == 0) {
//		printf("mem_buf == fatfs_buf\r\n");
//	}
//}

//int8_t compare_version(const char* ver1, const char* ver2)
//{
//	int p1 = 0 ,p2 = 0;
//	int v1 = 0, v2 = 0;
//	while (ver1[p1] != '\0' || ver2[p2] != '\0') {
//		p1 ++, p2++;
//		if ( ver1[p1] != '\0' ) {	
//			if ( ver1[p1] != '.') {
//				v1 = v1 * 10 + ( ver1[p1] - '0' );
//			}
//			p1 ++;
//		}
//		if ( ver2[p2] != '\0' ) {	
//			if ( ver2[p2] != '.' ) {
//				v2 = v2 * 10 + ( ver2[p2] - '0' );
//			}
//			p2 ++;
//		}
//	}
//	printf("ver1: %d, ver2: %d", v1 ,v2);
//	if ( v1 == v2 ) {
//		return 0;
//	}
//	return  v1 > v2 ? 1 : -1;
//}

//bool updated_version()
//{
//	char sd_buf[20]={0};
//	char flash_buf[20]={0};
//	uint32_t sd_num = 0, flash_num = 0;
//	
//	
//	retSD = f_mount(&SDFatFS, SDPath, 1);
//	if (retSD) {
//		printf("mount SD failed!\r\n");
//		return false;
//	}
//	else {
//		printf("mount SD ok!\r\n");
//	}
//	retSD = f_open(&SDFile, "UPGRADE.bin", FA_READ);
//	if (retSD) {
//		printf("open file failed code: %d!\r\n", retSD);
//		return false;
//	}
//	else {
//		printf("open file ok!\r\n");
//	}
//	f_lseek(&SDFile, 0x200);
//	retSD = f_read(&SDFile, sd_buf, 15, &sd_num);
//	f_close(&SDFile);
//	if (retSD) {
//		printf("f_read file failed!\r\n");
//		return false;
//	}
//	else {
//		printf("sd:%s\r\n", sd_buf);
//	}
//	
//	memcpy(flash_buf, ( uint32_t* )( VERSION_ADDR ), 15);
//	printf("flash:%s\r\n", flash_buf);
//	compare_version(sd_buf, flash_buf);

//}
