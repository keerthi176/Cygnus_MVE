#include "main.h"

#include "usart.h"
#include "usb_device.h"
#include "usbd_core.h"
#include "gpio.h"
#include "ltdc.h"
#include "fmc.h"
#include "sdram.h"
#include "lptim.h"
 
#include "usbd_dfu.h"
#include "usbd_dfu_flash.h"

#include "stm32f7xx_hal_iwdg.h"
 
#include "gfx.h"
 
#include <stdarg.h>
 
/* USER CODE BEGIN Includes */
 
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config( int, int );
 
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

  

/*************************************************************************/
/**  \fn      int _aeabi_assert( int q )
***  \brief   assert function
**************************************************************************/
void __aeabi_assert(const char *e, const char *f, int l )
{
	//db( "ASSERTFAIL: %s ( %s :%d )\n", e, f, l );  
}
 
 
extern USBD_DescriptorsTypeDef FS_Desc;
extern USBD_ClassTypeDef  USBD_DFU;

int __rcc_csr = 0;
int fw_size   = 0;


void db( const char* fmt, ... )
{
	if ( fmt != NULL )
	{
		char buffer[ 256];
		va_list args;
		
		va_start( args, fmt );
		vsprintf( buffer, fmt, args );
		HAL_UART_Transmit( &huart7, (uint8_t*) buffer, strlen( buffer ), 300 );
		va_end( args );
	}
}

void BootSettings( )
{
	FLASH_OBProgramInitTypeDef OBInit;
	
	/* Allow Access to Flash control registers and user Flash */
	HAL_FLASH_Unlock( );

	/* Allow Access to option bytes sector */ 
	HAL_FLASH_OB_Unlock( );

	/* next boot will be executed from bank2 */ 
	OBInit.OptionType = OPTIONBYTE_BOOTADDR_0 | OPTIONBYTE_BOOTADDR_1;
	OBInit.BootAddr0  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x08000000 );
	OBInit.BootAddr1  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x08000000 );

	if ( HAL_FLASHEx_OBProgram(&OBInit) != HAL_OK )
	{
		/*
		Error occurred while setting option bytes configuration.
		User can add here some code to deal with this error.
		To know the code error, user can call function 'HAL_FLASH_GetError()'
		*/

		db( "Option bytes programming fail.\n" );

		return;
	}

	/* Start the Option Bytes programming process */  
	if ( HAL_FLASH_OB_Launch( ) != HAL_OK )
	{
		/*
		Error occurred while reloading option bytes configuration.
		User can add here some code to deal with this error.
		To know the code error, user can call function 'HAL_FLASH_GetError()'
		*/

		db( "Option bytes programming fail.\n" );

		return;
	}

	/* Prevent Access to option bytes sector */
	HAL_FLASH_OB_Lock( );

	/* Disable the Flash option control register access (recommended to protect 
	the option Bytes against possible unwanted operations) */
	HAL_FLASH_Lock( );
	
}

#define HAL_IWDG_DEFAULT_TIMEOUT            48u

IWDG_HandleTypeDef hiwdg = { IWDG, {IWDG_PRESCALER_256, IWDG_RLR_RL, IWDG_WINDOW_DISABLE } };

HAL_StatusTypeDef HAL_IWDG_Init(IWDG_HandleTypeDef *hiwdg)
{
  uint32_t tickstart;

  /* Check the IWDG handle allocation */
  if (hiwdg == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_IWDG_ALL_INSTANCE(hiwdg->Instance));
  assert_param(IS_IWDG_PRESCALER(hiwdg->Init.Prescaler));
  assert_param(IS_IWDG_RELOAD(hiwdg->Init.Reload));
  assert_param(IS_IWDG_WINDOW(hiwdg->Init.Window));

  /* Enable IWDG. LSI is turned on automatically */
  __HAL_IWDG_START(hiwdg);

  /* Enable write access to IWDG_PR, IWDG_RLR and IWDG_WINR registers by writing
  0x5555 in KR */
  IWDG_ENABLE_WRITE_ACCESS(hiwdg);

  /* Write to IWDG registers the Prescaler & Reload values to work with */
  hiwdg->Instance->PR = hiwdg->Init.Prescaler;
  hiwdg->Instance->RLR = hiwdg->Init.Reload;

  /* Check pending flag, if previous update not done, return timeout */
  tickstart = HAL_GetTick();

  /* Wait for register to be updated */
  while (hiwdg->Instance->SR != 0x00u)
  {
    if ((HAL_GetTick() - tickstart) > HAL_IWDG_DEFAULT_TIMEOUT)
    {
      return HAL_TIMEOUT;
    }
  }

  /* If window parameter is different than current value, modify window
  register */
  if (hiwdg->Instance->WINR != hiwdg->Init.Window)
  {
    /* Write to IWDG WINR the IWDG_Window value to compare with. In any case,
    even if window feature is disabled, Watchdog will be reloaded by writing
    windows register */
    hiwdg->Instance->WINR = hiwdg->Init.Window;
  }
  else
  {
    /* Reload IWDG counter with value defined in the reload register */
    __HAL_IWDG_RELOAD_COUNTER(hiwdg);
  }

  /* Return function status */
  return HAL_OK;
}

/**
  * @brief  The application entry point.#
  */
//__attribute__((section(".bootloader"), noinline)) 
int main(void)
{
   FMC_SDRAM_CommandTypeDef command;
	
	__rcc_csr = RCC->CSR;
	/* MCU Configuration----------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
 
	// move vector table
 	SCB->VTOR = (FLASH_BASE | 0x1C0000);
 
	/* USER CODE END Init */
	__HAL_DBGMCU_FREEZE_IWDG( );
 	HAL_IWDG_Init( &hiwdg);

	/* Configure the system clock */
	SystemClock_Config( RCC_OSCILLATORTYPE_HSE, RCC_LSEDRIVE_HIGH );

	/* USER CODE BEGIN SysInit */
	
	// put correct settings back
   BootSettings( );
	
	/* USER CODE END SysInit */

	// TestJig check
	
	GPIO_InitTypeDef from;
	GPIO_InitTypeDef to;
	
	to.Mode  = GPIO_MODE_INPUT;
	to.Pull  = GPIO_NOPULL;
	to.Speed = GPIO_SPEED_FREQ_HIGH; 
	to.Pin   = GPIO_PIN_8;
	
	from.Mode  = GPIO_MODE_OUTPUT_PP;
	from.Pull  = GPIO_NOPULL;
	from.Speed = GPIO_SPEED_FREQ_HIGH; 
	from.Pin   = GPIO_PIN_6;
	
	HAL_GPIO_Init( GPIOE, &from );
	HAL_GPIO_Init( GPIOJ, &to );
	
	HAL_GPIO_WritePin( GPIOE, from.Pin, GPIO_PIN_RESET );

	if ( HAL_GPIO_ReadPin( GPIOJ, to.Pin ) == GPIO_PIN_RESET )
	{
		HAL_GPIO_WritePin( GPIOE, from.Pin, GPIO_PIN_SET );
		
		if ( HAL_GPIO_ReadPin( GPIOJ, to.Pin ) == GPIO_PIN_SET )
		{
			HAL_GPIO_WritePin( GPIOE, from.Pin, GPIO_PIN_RESET );
			
			if ( HAL_GPIO_ReadPin( GPIOJ, to.Pin ) == GPIO_PIN_RESET )
			{
				HAL_GPIO_WritePin( GPIOE, from.Pin, GPIO_PIN_SET );
				
				if ( HAL_GPIO_ReadPin( GPIOJ, to.Pin ) == GPIO_PIN_SET )
				{
					FLASH_OBProgramInitTypeDef OBInit;
		
					/* Allow Access to Flash control registers and user Flash */
					HAL_FLASH_Unlock( );

					/* Allow Access to option bytes sector */ 
					HAL_FLASH_OB_Unlock( );

					/* set both to dfu mode prog addr */ 
					OBInit.OptionType = OPTIONBYTE_BOOTADDR_0 | OPTIONBYTE_BOOTADDR_1;
					OBInit.BootAddr0  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x081E0000 );
					OBInit.BootAddr1  = __HAL_FLASH_CALC_BOOT_BASE_ADR( 0x081E0000 );

					if ( HAL_FLASHEx_OBProgram(&OBInit) != HAL_OK )
					{
						/*
						Error occurred while setting option bytes configuration.
						User can add here some code to deal with this error.
						To know the code error, user can call function 'HAL_FLASH_GetError()'
						*/
					}

					/* Start the Option Bytes programming process */  
					if ( HAL_FLASH_OB_Launch( ) != HAL_OK )
					{
						/*
						Error occurred while reloading option bytes configuration.
						User can add here some code to deal with this error.
						To know the code error, user can call function 'HAL_FLASH_GetError()'
						*/

					}

					/* Prevent Access to option bytes sector */
					HAL_FLASH_OB_Lock( );

					/* Disable the Flash option control register access (recommended to protect 
					the option Bytes against possible unwanted operations) */
					HAL_FLASH_Lock( );

					/* Initiate a system reset request to reset the MCU */
					HAL_NVIC_SystemReset( );
				}
			}
		}
	}
		
	/* Initialize all configured peripherals */
	 
	MX_GPIO_Init();
	MX_FMC_Init();  
	SDRAM_Initialization_Sequence( &hsdram1, &command );
	MX_LTDC_Init();   
	MX_LPTIM1_Init();
	HAL_LPTIM_PWM_Start( &hlptim1, 0x1A00, 0x700 );
	
	if ( !(__rcc_csr & RCC_CSR_SFTRSTF ) )
	{
		DrawFloodFill( );
	}
	
   HAL_GPIO_WritePin(GPIOA, DISPLAY_EN_Pin, GPIO_PIN_SET );
	
	MX_UART7_Init();

	HAL_UART_Transmit( &huart7, (uint8_t*) "DFU Mode...\n", 12, 300 );
	
	USBD_Init(&hUsbDeviceFS, &FS_Desc, 0);
  
	/* Add Supported Class */
	USBD_RegisterClass(&hUsbDeviceFS, &USBD_DFU );

	/* Add DFU Media interface */
	USBD_DFU_RegisterMedia( &hUsbDeviceFS, &USBD_DFU_Flash_fops );

	/* Start Device Process */
	USBD_Start(&hUsbDeviceFS);
	
	HAL_PWREx_EnableBkUpReg( );
	__HAL_RCC_BKPSRAM_CLK_ENABLE();
		
	fw_size = *((int*)BKPSRAM_BASE);
	
	db( "FW size: %d\n", fw_size );
   	
	if ( fw_size > 0x1C0000 ) fw_size = 0x1C0000;
	
	while(1)
	{
		for( volatile double x = 1; x < 999; x = x * 1.000001 )
		{
		}
		__HAL_IWDG_RELOAD_COUNTER(&hiwdg);
	}
}



	
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config( int osc, int drive )
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

	/**Configure LSE Drive Capability 
	*/
	__HAL_RCC_LSEDRIVE_CONFIG( drive );

	/**Configure the main internal regulator output voltage 
	*/
	__HAL_RCC_PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|osc;

	RCC_OscInitStruct.HSEState = osc == RCC_OSCILLATORTYPE_HSE ? RCC_HSE_ON : RCC_HSE_OFF;
	RCC_OscInitStruct.HSIState = osc == RCC_OSCILLATORTYPE_HSE ? RCC_HSI_OFF : RCC_HSI_ON; 
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = osc == RCC_OSCILLATORTYPE_HSE ? RCC_PLLSOURCE_HSE : RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = osc == RCC_OSCILLATORTYPE_HSE ? 25 : 16;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;
	RCC_OscInitStruct.PLL.PLLR = 7;

	if ( HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK )
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Activate the Over-Drive mode 
	*/
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Initializes the CPU, AHB and APB busses clocks 
	*/
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
									|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC
									|RCC_PERIPHCLK_USART1|RCC_PERIPHCLK_USART2
									|RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USART6
									|RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_UART7
									|RCC_PERIPHCLK_LPTIM1|RCC_PERIPHCLK_I2C1
									|RCC_PERIPHCLK_I2C2|RCC_PERIPHCLK_SDMMC1
									|RCC_PERIPHCLK_CLK48;
	PeriphClkInitStruct.PLLSAI.PLLSAIN = 216;
	PeriphClkInitStruct.PLLSAI.PLLSAIR = 2;
	PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
	PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
	PeriphClkInitStruct.PLLSAIDivQ = 1;
	PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
	PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
	PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
	PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Uart4ClockSelection = RCC_UART4CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Usart6ClockSelection = RCC_USART6CLKSOURCE_PCLK2;
	PeriphClkInitStruct.Uart7ClockSelection = RCC_UART7CLKSOURCE_PCLK1;
	PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
	PeriphClkInitStruct.I2c2ClockSelection = RCC_I2C2CLKSOURCE_PCLK1;
	PeriphClkInitStruct.Lptim1ClockSelection = RCC_LPTIM1CLKSOURCE_PCLK;
	PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLL;
	PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
	{
		_Error_Handler(__FILE__, __LINE__);
	}

	/**Configure the Systick interrupt time 
	*/
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/**Configure the Systick 
	*/
	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	/* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
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
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

