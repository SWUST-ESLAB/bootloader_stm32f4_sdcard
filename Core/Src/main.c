/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "rtc.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"


#include "upgrade.h"
#include "func_test.h"

#include "stdlib.h"

typedef void (*pFunction)(void);
static const uint32_t Flag_RebootAfterUpgrade = 0xDEADBEEF;
const char ST_BOOT[] =
"  _____ _______     ____   ____   ____ _______ \r\n" 
" / ____|__   __|   |  _ \\ / __ \\ / __ \\__   __|\r\n" 
"| (___    | |______| |_) | |  | | |  | | | |   \r\n" 
" \\___ \\   | |______|  _ <| |  | | |  | | | |   \r\n" 
" ____) |  | |      | |_) | |__| | |__| | | |   \r\n" 
"|_____/   |_|      |____/ \\____/ \\____/  |_|   \r\n" ;

void SystemClock_Config(void);

/*! \brief Reboot the Micro
 *
 *  Reboot the micro and set one of the backup registers to indicate
 *  that the reboot was triggered by the bootloader.  This will
 *  cause the bootloader to start the app without initializing
 *  any internal drivers
 */
void hardware_rebootToApp(void) {
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, Flag_RebootAfterUpgrade);
	printf("reboot...\r\n");
  NVIC_SystemReset();
}

/*! \brief Clear Flag
 *
 *  Clear the backup register.  This will allow subsequent resets
 *  to behave normally
  */
void hardware_clearRebootFlag(void) {
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0);
	}

/*! \brief Determine if the reboot was triggered by the bootloader
 */
uint8_t hardware_isRebootToApp(void) {
  return (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != Flag_RebootAfterUpgrade);
}

void app_launch(void) {
  uint32_t JumpAddress;
  pFunction Jump_To_Application;

  /* Test if user code is programmed starting from address "APPLICATION_ADDRESS" */
  if (((*(__IO uint32_t*)Upgrade_AppBaseAddress) & 0x2FFE0000 ) == 0x20000000)
  {
    //NVIC_SetVectorTable(Upgrade_AppBaseAddress, 0);
		
    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (Upgrade_AppBaseAddress + 4);
    Jump_To_Application = (pFunction) JumpAddress;
    /* Initialize user application's Stack Pointer */
		
    __set_MSP(*(__IO uint32_t*) Upgrade_AppBaseAddress);
		SCB->VTOR = Upgrade_AppBaseAddress;
    /* Jump to application */
		//__disable_irq();
		printf("Jump_To_Application...\r\n");
    Jump_To_Application();
  }
	printf("Crash...\r\n");
  // Block indefinitely on failure: user is required to power cycle to reattempt upgrade
  while (1);
}

int main(void)
{

  HAL_Init();

  SystemClock_Config();

	HAL_PWR_EnableBkUpAccess();
	HAL_PWREx_EnableBkUpReg();
	__HAL_RCC_BKPSRAM_CLK_ENABLE();
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_USART2_UART_Init();
  MX_FATFS_Init();
  MX_RTC_Init();
	//updated_version();
	printf("%s", ST_BOOT);
	if (hardware_isRebootToApp() && upgrade_pin_pushed()) {
		// detect SD Card
    if (HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3) == GPIO_PIN_RESET) {
			printf("Detect SD Card...\r\n");
      // Run the upgrade driver.  This will check to see if a firmware upgrade has
      // been requested
      upgrade();

      hardware_rebootToApp();
     }
  }
	  // Clear the reboot flag
  hardware_clearRebootFlag();

  // Start the application
	printf("App launch...\r\n");
  app_launch();
	
  while (1);

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 10;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
