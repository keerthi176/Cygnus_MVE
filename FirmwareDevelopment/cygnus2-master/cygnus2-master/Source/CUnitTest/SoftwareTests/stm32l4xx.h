/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : stm32l0xx.h
*
*  Description  : Header file for the test version of stm32l0
*
*************************************************************************************/

#ifndef ST_STM32L4XX_TEST_H
#define ST_STM32L4XX_TEST_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/


/* Public Constants
*************************************************************************************/
#define __I volatile const
#define __O volatile
#define __IO volatile
#define __IM volatile const
#define __OM volatile
#define __IOM volatile

#define FLASH_TIMEOUT_VALUE                ((uint32_t)50000)/* 50 s */

/** @defgroup GPIO_pins GPIO pins
* @{
*/
#define GPIO_PIN_0                 ((uint16_t)0x0001)  /* Pin 0 selected    */
#define GPIO_PIN_1                 ((uint16_t)0x0002)  /* Pin 1 selected    */
#define GPIO_PIN_2                 ((uint16_t)0x0004)  /* Pin 2 selected    */
#define GPIO_PIN_3                 ((uint16_t)0x0008)  /* Pin 3 selected    */
#define GPIO_PIN_4                 ((uint16_t)0x0010)  /* Pin 4 selected    */
#define GPIO_PIN_5                 ((uint16_t)0x0020)  /* Pin 5 selected    */
#define GPIO_PIN_6                 ((uint16_t)0x0040)  /* Pin 6 selected    */
#define GPIO_PIN_7                 ((uint16_t)0x0080)  /* Pin 7 selected    */
#define GPIO_PIN_8                 ((uint16_t)0x0100)  /* Pin 8 selected    */
#define GPIO_PIN_9                 ((uint16_t)0x0200)  /* Pin 9 selected    */
#define GPIO_PIN_10                ((uint16_t)0x0400)  /* Pin 10 selected   */
#define GPIO_PIN_11                ((uint16_t)0x0800)  /* Pin 11 selected   */
#define GPIO_PIN_12                ((uint16_t)0x1000)  /* Pin 12 selected   */
#define GPIO_PIN_13                ((uint16_t)0x2000)  /* Pin 13 selected   */
#define GPIO_PIN_14                ((uint16_t)0x4000)  /* Pin 14 selected   */
#define GPIO_PIN_15                ((uint16_t)0x8000)  /* Pin 15 selected   */
#define GPIO_PIN_All               ((uint16_t)0xFFFF)  /* All pins selected */

#define  GPIO_MODE_INPUT                        ((uint32_t)0x00000000)   /*!< Input Floating Mode                   */
#define  GPIO_MODE_OUTPUT_PP                    ((uint32_t)0x00000001)   /*!< Output Push Pull Mode                 */
#define  GPIO_MODE_OUTPUT_OD                    ((uint32_t)0x00000011)   /*!< Output Open Drain Mode                */
#define  GPIO_MODE_AF_PP                        ((uint32_t)0x00000002)   /*!< Alternate Function Push Pull Mode     */
#define  GPIO_MODE_AF_OD                        ((uint32_t)0x00000012)   /*!< Alternate Function Open Drain Mode    */
#define  GPIO_MODE_ANALOG                       ((uint32_t)0x00000003)   /*!< Analog Mode  */
#define  GPIO_MODE_ANALOG_ADC_CONTROL           ((uint32_t)0x0000000B)   /*!< Analog Mode for ADC conversion */
#define  GPIO_MODE_IT_RISING                    ((uint32_t)0x10110000)   /*!< External Interrupt Mode with Rising edge trigger detection          */
#define  GPIO_MODE_IT_FALLING                   ((uint32_t)0x10210000)   /*!< External Interrupt Mode with Falling edge trigger detection         */
#define  GPIO_MODE_IT_RISING_FALLING            ((uint32_t)0x10310000)   /*!< External Interrupt Mode with Rising/Falling edge trigger detection  */
#define  GPIO_MODE_EVT_RISING                   ((uint32_t)0x10120000)   /*!< External Event Mode with Rising edge trigger detection               */
#define  GPIO_MODE_EVT_FALLING                  ((uint32_t)0x10220000)   /*!< External Event Mode with Falling edge trigger detection              */
#define  GPIO_MODE_EVT_RISING_FALLING           ((uint32_t)0x10320000)   /*!< External Event Mode with Rising/Falling edge trigger detection  

/** @defgroup GPIO_pull GPIO pull
* @brief GPIO Pull-Up or Pull-Down Activation
* @{
*/
#define  GPIO_NOPULL        ((uint32_t)0x00000000)   /*!< No Pull-up or Pull-down activation  */
#define  GPIO_PULLUP        ((uint32_t)0x00000001)   /*!< Pull-up activation                  */
#define  GPIO_PULLDOWN      ((uint32_t)0x00000002)   /*!< Pull-down activation                */

/** @defgroup GPIO_speed_define Speed definition
* @brief GPIO Output Maximum frequency
* @{
*/
#define  GPIO_SPEED_FREQ_LOW              ((uint32_t)0x00000000U)  /*!< range up to 0.4 MHz, please refer to the product datasheet */
#define  GPIO_SPEED_FREQ_MEDIUM           ((uint32_t)0x00000001U)  /*!< range 0.4 MHz to 2 MHz, please refer to the product datasheet */
#define  GPIO_SPEED_FREQ_HIGH             ((uint32_t)0x00000002U)  /*!< range   2 MHz to 10 MHz, please refer to the product datasheet */
#define  GPIO_SPEED_FREQ_VERY_HIGH        ((uint32_t)0x00000003U)  /*!< range  10 MHz to 35 MHz, please refer to the product datasheet */

/** @defgroup I2C_Error_Code_definition I2C Error Code definition
* @brief  I2C Error Code definition
* @{
*/
#define HAL_I2C_ERROR_NONE      (0x00000000U)    /*!< No error              */
#define HAL_I2C_ERROR_BERR      (0x00000001U)    /*!< BERR error            */
#define HAL_I2C_ERROR_ARLO      (0x00000002U)    /*!< ARLO error            */
#define HAL_I2C_ERROR_AF        (0x00000004U)    /*!< ACKF error            */
#define HAL_I2C_ERROR_OVR       (0x00000008U)    /*!< OVR error             */
#define HAL_I2C_ERROR_DMA       (0x00000010U)    /*!< DMA transfer error    */
#define HAL_I2C_ERROR_TIMEOUT   (0x00000020U)    /*!< Timeout error         */
#define HAL_I2C_ERROR_SIZE      (0x00000040U)    /*!< Size Management error */

/** @defgroup I2CEx_Analog_Filter I2C Extended Analog Filter
* @{
*/
#define I2C_ANALOGFILTER_ENABLE         0x00000000U
#define I2C_ANALOGFILTER_DISABLE        I2C_CR1_ANFOFF

/** @defgroup I2C_ADDRESSING_MODE I2C Addressing Mode
* @{
*/
#define I2C_ADDRESSINGMODE_7BIT         (0x00000001U)
#define I2C_ADDRESSINGMODE_10BIT        (0x00000002U)
/**
* @}
*/

/** @defgroup I2C_DUAL_ADDRESSING_MODE I2C Dual Addressing Mode
* @{
*/
#define I2C_DUALADDRESS_DISABLE         (0x00000000U)
#define I2C_DUALADDRESS_ENABLE          I2C_OAR2_OA2EN

/** @defgroup I2C_OWN_ADDRESS2_MASKS I2C Own Address2 Masks
* @{
*/
#define I2C_OA2_NOMASK                  ((uint8_t)0x00U)
#define I2C_OA2_MASK01                  ((uint8_t)0x01U)
#define I2C_OA2_MASK02                  ((uint8_t)0x02U)
#define I2C_OA2_MASK03                  ((uint8_t)0x03U)
#define I2C_OA2_MASK04                  ((uint8_t)0x04U)
#define I2C_OA2_MASK05                  ((uint8_t)0x05U)
#define I2C_OA2_MASK06                  ((uint8_t)0x06U)
#define I2C_OA2_MASK07                  ((uint8_t)0x07U)
/**
* @}
*/

/** @defgroup I2C_GENERAL_CALL_ADDRESSING_MODE I2C General Call Addressing Mode
* @{
*/
#define I2C_GENERALCALL_DISABLE         (0x00000000U)
#define I2C_GENERALCALL_ENABLE          I2C_CR1_GCEN
/**
* @}
*/

/** @defgroup I2C_NOSTRETCH_MODE I2C No-Stretch Mode
* @{
*/
#define I2C_NOSTRETCH_DISABLE           (0x00000000U)
#define I2C_NOSTRETCH_ENABLE            I2C_CR1_NOSTRETCH

/** @defgroup I2C_MEMORY_ADDRESS_SIZE I2C Memory Address Size
* @{
*/
#define I2C_MEMADD_SIZE_8BIT            (0x00000001U)
#define I2C_MEMADD_SIZE_16BIT           (0x00000002U)

/******************  Bit definition for SYSCFG_MEMRMP register  ***************/
#define SYSCFG_MEMRMP_MEM_MODE_Pos      (0U)
#define SYSCFG_MEMRMP_MEM_MODE_Msk      (0x7U << SYSCFG_MEMRMP_MEM_MODE_Pos)   /*!< 0x00000007 */
#define SYSCFG_MEMRMP_MEM_MODE          SYSCFG_MEMRMP_MEM_MODE_Msk             /*!< SYSCFG_Memory Remap Config */
#define SYSCFG_MEMRMP_MEM_MODE_0        (0x1U << SYSCFG_MEMRMP_MEM_MODE_Pos)   /*!< 0x00000001 */
#define SYSCFG_MEMRMP_MEM_MODE_1        (0x2U << SYSCFG_MEMRMP_MEM_MODE_Pos)   /*!< 0x00000002 */
#define SYSCFG_MEMRMP_MEM_MODE_2        (0x4U << SYSCFG_MEMRMP_MEM_MODE_Pos)   /*!< 0x00000004 */

#define SYSCFG_MEMRMP_FB_MODE_Pos       (8U)
#define SYSCFG_MEMRMP_FB_MODE_Msk       (0x1U << SYSCFG_MEMRMP_FB_MODE_Pos)    /*!< 0x00000100 */
#define SYSCFG_MEMRMP_FB_MODE           SYSCFG_MEMRMP_FB_MODE_Msk              /*!< Flash Bank mode selection */

/* Flags in the CSR register */
#define RCC_FLAG_LSIRDY                ((uint32_t)(0) /*!< LSI Ready flag */
#define RCC_FLAG_RMVF                  ((uint32_t)(0) /*!< Remove reset flag */
#define RCC_FLAG_FWRST                 ((uint32_t)(0) /*!< Firewall reset flag */
#define RCC_FLAG_OBLRST                ((uint32_t)(0) /*!< Option Byte Loader reset flag */
#define RCC_FLAG_PINRST                ((uint32_t)(0) /*!< PIN reset flag */
#define RCC_FLAG_BORRST                ((uint32_t)(0) /*!< BOR reset flag */
#define RCC_FLAG_SFTRST                ((uint32_t)(0) /*!< Software Reset flag */
#define RCC_FLAG_IWDGRST               ((uint32_t)(0) /*!< Independent Watchdog reset flag */
#define RCC_FLAG_WWDGRST               ((uint32_t)(0) /*!< Window watchdog reset flag */
#define RCC_FLAG_LPWRRST               ((uint32_t)(0) /*!< Low-Power reset flag */

#define FLASH_TYPEPROGRAMDATA_WORD 0

#define GPIOA 0
#define CRC 0
#define DEFAULT_POLYNOMIAL_ENABLE       ((uint8_t)0x00U)
#define DEFAULT_POLYNOMIAL_DISABLE      ((uint8_t)0x01U)
#define DEFAULT_INIT_VALUE_ENABLE      ((uint8_t)0x00U)
#define DEFAULT_INIT_VALUE_DISABLE     ((uint8_t)0x01U)
#define CRC_INPUTDATA_INVERSION_NONE              ((uint32_t)0x00000000U)
#define CRC_OUTPUTDATA_INVERSION_DISABLE         ((uint32_t)0x00000000U)
#define CRC_OUTPUTDATA_INVERSION_ENABLE         ((uint32_t)0x00000001U)
#define CRC_INPUTDATA_FORMAT_UNDEFINED     0x00000000U  /*!< Undefined input data format    */
#define CRC_INPUTDATA_FORMAT_BYTES         0x00000001U  /*!< Input data in byte format      */
#define CRC_INPUTDATA_FORMAT_HALFWORDS     0x00000002U  /*!< Input data in half-word format */
#define CRC_INPUTDATA_FORMAT_WORDS                 ((uint32_t)0x00000003U)

#define CRC_POLYLENGTH_32B                  ((uint32_t)0x00000000U)
#define CRC_POLYLENGTH_16B             (0x00000000U) /*!< Resort to a 16-bit long generating polynomial */
#define CRC_POLYLENGTH_8B              (0x00000000U) /*!< Resort to a 8-bit long generating polynomial  */
#define CRC_POLYLENGTH_7B              (0x00000000U)   /*!< Resort to a 7-bit long generating polynomial  */

#ifdef FLASH_START_BANK1
#undef FLASH_START_BANK1
#endif
#define FLASH_START_BANK1             ((uint32_t)stwin32_stub_application_image_p)

#ifdef FLASH_START_BANK2
#undef FLASH_START_BANK2
#endif
#define FLASH_START_BANK2             ((uint32_t)stwin32_stub_application_image_p)

#define FLASH_PAGE_SIZE sizeof(stwin32_stub_application_image_t)

#define UART_RXDATA_FLUSH_REQUEST 0

/* Public Enumerations
*************************************************************************************/
/*!< Interrupt Number Definition */
typedef enum
{
	/******  Cortex-M4 Processor Exceptions Numbers ****************************************************************/
	NonMaskableInt_IRQn = -14,    /*!< 2 Cortex-M4 Non Maskable Interrupt                                */
	HardFault_IRQn = -13,    /*!< 3 Cortex-M4 Hard Fault Interrupt                                  */
	MemoryManagement_IRQn = -12,    /*!< 4 Cortex-M4 Memory Management Interrupt                           */
	BusFault_IRQn = -11,    /*!< 5 Cortex-M4 Bus Fault Interrupt                                   */
	UsageFault_IRQn = -10,    /*!< 6 Cortex-M4 Usage Fault Interrupt                                 */
	SVCall_IRQn = -5,     /*!< 11 Cortex-M4 SV Call Interrupt                                    */
	DebugMonitor_IRQn = -4,     /*!< 12 Cortex-M4 Debug Monitor Interrupt                              */
	PendSV_IRQn = -2,     /*!< 14 Cortex-M4 Pend SV Interrupt                                    */
	SysTick_IRQn = -1,     /*!< 15 Cortex-M4 System Tick Interrupt                                */
	/******  STM32 specific Interrupt Numbers **********************************************************************/
	WWDG_IRQn = 0,      /*!< Window WatchDog Interrupt                                         */
	PVD_PVM_IRQn = 1,      /*!< PVD/PVM1/PVM2/PVM3/PVM4 through EXTI Line detection Interrupts    */
	TAMP_STAMP_IRQn = 2,      /*!< Tamper and TimeStamp interrupts through the EXTI line             */
	RTC_WKUP_IRQn = 3,      /*!< RTC Wakeup interrupt through the EXTI line                        */
	FLASH_IRQn = 4,      /*!< FLASH global Interrupt                                            */
	RCC_IRQn = 5,      /*!< RCC global Interrupt                                              */
	EXTI0_IRQn = 6,      /*!< EXTI Line0 Interrupt                                              */
	EXTI1_IRQn = 7,      /*!< EXTI Line1 Interrupt                                              */
	EXTI2_IRQn = 8,      /*!< EXTI Line2 Interrupt                                              */
	EXTI3_IRQn = 9,      /*!< EXTI Line3 Interrupt                                              */
	EXTI4_IRQn = 10,     /*!< EXTI Line4 Interrupt                                              */
	DMA1_Channel1_IRQn = 11,     /*!< DMA1 Channel 1 global Interrupt                                   */
	DMA1_Channel2_IRQn = 12,     /*!< DMA1 Channel 2 global Interrupt                                   */
	DMA1_Channel3_IRQn = 13,     /*!< DMA1 Channel 3 global Interrupt                                   */
	DMA1_Channel4_IRQn = 14,     /*!< DMA1 Channel 4 global Interrupt                                   */
	DMA1_Channel5_IRQn = 15,     /*!< DMA1 Channel 5 global Interrupt                                   */
	DMA1_Channel6_IRQn = 16,     /*!< DMA1 Channel 6 global Interrupt                                   */
	DMA1_Channel7_IRQn = 17,     /*!< DMA1 Channel 7 global Interrupt                                   */
	ADC1_2_IRQn = 18,     /*!< ADC1, ADC2 SAR global Interrupts                                  */
	CAN1_TX_IRQn = 19,     /*!< CAN1 TX Interrupt                                                 */
	CAN1_RX0_IRQn = 20,     /*!< CAN1 RX0 Interrupt                                                */
	CAN1_RX1_IRQn = 21,     /*!< CAN1 RX1 Interrupt                                                */
	CAN1_SCE_IRQn = 22,     /*!< CAN1 SCE Interrupt                                                */
	EXTI9_5_IRQn = 23,     /*!< External Line[9:5] Interrupts                                     */
	TIM1_BRK_TIM15_IRQn = 24,     /*!< TIM1 Break interrupt and TIM15 global interrupt                   */
	TIM1_UP_TIM16_IRQn = 25,     /*!< TIM1 Update Interrupt and TIM16 global interrupt                  */
	TIM1_TRG_COM_TIM17_IRQn = 26,     /*!< TIM1 Trigger and Commutation Interrupt and TIM17 global interrupt */
	TIM1_CC_IRQn = 27,     /*!< TIM1 Capture Compare Interrupt                                    */
	TIM2_IRQn = 28,     /*!< TIM2 global Interrupt                                             */
	TIM3_IRQn = 29,     /*!< TIM3 global Interrupt                                             */
	TIM4_IRQn = 30,     /*!< TIM4 global Interrupt                                             */
	I2C1_EV_IRQn = 31,     /*!< I2C1 Event Interrupt                                              */
	I2C1_ER_IRQn = 32,     /*!< I2C1 Error Interrupt                                              */
	I2C2_EV_IRQn = 33,     /*!< I2C2 Event Interrupt                                              */
	I2C2_ER_IRQn = 34,     /*!< I2C2 Error Interrupt                                              */
	SPI1_IRQn = 35,     /*!< SPI1 global Interrupt                                             */
	SPI2_IRQn = 36,     /*!< SPI2 global Interrupt                                             */
	USART1_IRQn = 37,     /*!< USART1 global Interrupt                                           */
	USART2_IRQn = 38,     /*!< USART2 global Interrupt                                           */
	USART3_IRQn = 39,     /*!< USART3 global Interrupt                                           */
	EXTI15_10_IRQn = 40,     /*!< External Line[15:10] Interrupts                                   */
	RTC_Alarm_IRQn = 41,     /*!< RTC Alarm (A and B) through EXTI Line Interrupt                   */
	DFSDM1_FLT3_IRQn = 42,     /*!< DFSDM1 Filter 3 global Interrupt                                  */
	TIM8_BRK_IRQn = 43,     /*!< TIM8 Break Interrupt                                              */
	TIM8_UP_IRQn = 44,     /*!< TIM8 Update Interrupt                                             */
	TIM8_TRG_COM_IRQn = 45,     /*!< TIM8 Trigger and Commutation Interrupt                            */
	TIM8_CC_IRQn = 46,     /*!< TIM8 Capture Compare Interrupt                                    */
	ADC3_IRQn = 47,     /*!< ADC3 global  Interrupt                                            */
	FMC_IRQn = 48,     /*!< FMC global Interrupt                                              */
	SDMMC1_IRQn = 49,     /*!< SDMMC1 global Interrupt                                           */
	TIM5_IRQn = 50,     /*!< TIM5 global Interrupt                                             */
	SPI3_IRQn = 51,     /*!< SPI3 global Interrupt                                             */
	UART4_IRQn = 52,     /*!< UART4 global Interrupt                                            */
	UART5_IRQn = 53,     /*!< UART5 global Interrupt                                            */
	TIM6_DAC_IRQn = 54,     /*!< TIM6 global and DAC1&2 underrun error  interrupts                 */
	TIM7_IRQn = 55,     /*!< TIM7 global interrupt                                             */
	DMA2_Channel1_IRQn = 56,     /*!< DMA2 Channel 1 global Interrupt                                   */
	DMA2_Channel2_IRQn = 57,     /*!< DMA2 Channel 2 global Interrupt                                   */
	DMA2_Channel3_IRQn = 58,     /*!< DMA2 Channel 3 global Interrupt                                   */
	DMA2_Channel4_IRQn = 59,     /*!< DMA2 Channel 4 global Interrupt                                   */
	DMA2_Channel5_IRQn = 60,     /*!< DMA2 Channel 5 global Interrupt                                   */
	DFSDM1_FLT0_IRQn = 61,     /*!< DFSDM1 Filter 0 global Interrupt                                  */
	DFSDM1_FLT1_IRQn = 62,     /*!< DFSDM1 Filter 1 global Interrupt                                  */
	DFSDM1_FLT2_IRQn = 63,     /*!< DFSDM1 Filter 2 global Interrupt                                  */
	COMP_IRQn = 64,     /*!< COMP1 and COMP2 Interrupts                                        */
	LPTIM1_IRQn = 65,     /*!< LP TIM1 interrupt                                                 */
	LPTIM2_IRQn = 66,     /*!< LP TIM2 interrupt                                                 */
	OTG_FS_IRQn = 67,     /*!< USB OTG FS global Interrupt                                       */
	DMA2_Channel6_IRQn = 68,     /*!< DMA2 Channel 6 global interrupt                                   */
	DMA2_Channel7_IRQn = 69,     /*!< DMA2 Channel 7 global interrupt                                   */
	LPUART1_IRQn = 70,     /*!< LP UART1 interrupt                                                */
	QUADSPI_IRQn = 71,     /*!< Quad SPI global interrupt                                         */
	I2C3_EV_IRQn = 72,     /*!< I2C3 event interrupt                                              */
	I2C3_ER_IRQn = 73,     /*!< I2C3 error interrupt                                              */
	SAI1_IRQn = 74,     /*!< Serial Audio Interface 1 global interrupt                         */
	SAI2_IRQn = 75,     /*!< Serial Audio Interface 2 global interrupt                         */
	SWPMI1_IRQn = 76,     /*!< Serial Wire Interface 1 global interrupt                          */
	TSC_IRQn = 77,     /*!< Touch Sense Controller global interrupt                           */
	LCD_IRQn = 78,     /*!< LCD global interrupt                                              */
	RNG_IRQn = 80,     /*!< RNG global interrupt                                              */
	FPU_IRQn = 81      /*!< FPU global interrupt                                              */
} IRQn_Type;

typedef enum
{
	RESET = 0,
	SET = !RESET
} FlagStatus, ITStatus;

/**
* @brief  HAL Lock structures definition
*/
typedef enum
{
	HAL_UNLOCKED = 0x00U,
	HAL_LOCKED = 0x01U
} HAL_LockTypeDef;

/**
* @brief  HAL SPI State structure definition
*/
typedef enum
{
	HAL_SPI_STATE_RESET = 0x00U,    /*!< Peripheral not Initialized                         */
	HAL_SPI_STATE_READY = 0x01U,    /*!< Peripheral Initialized and ready for use           */
	HAL_SPI_STATE_BUSY = 0x02U,    /*!< an internal process is ongoing                     */
	HAL_SPI_STATE_BUSY_TX = 0x03U,    /*!< Data Transmission process is ongoing               */
	HAL_SPI_STATE_BUSY_RX = 0x04U,    /*!< Data Reception process is ongoing                  */
	HAL_SPI_STATE_BUSY_TX_RX = 0x05U,    /*!< Data Transmission and Reception process is ongoing */
	HAL_SPI_STATE_ERROR = 0x06U,    /*!< SPI error state                                    */
	HAL_SPI_STATE_ABORT = 0x07U     /*!< SPI abort is ongoing                               */
} HAL_SPI_StateTypeDef;

/**
* @brief  HAL DMA State structures definition
*/
typedef enum
{
	HAL_DMA_STATE_RESET = 0x00U,  /*!< DMA not yet initialized or disabled */
	HAL_DMA_STATE_READY = 0x01U,  /*!< DMA process success and ready for use   */
	HAL_DMA_STATE_BUSY = 0x02U,  /*!< DMA process is ongoing              */
	HAL_DMA_STATE_TIMEOUT = 0x03U,  /*!< DMA timeout state                   */
	HAL_DMA_STATE_ERROR = 0x04U,  /*!< DMA error state                     */
	HAL_DMA_STATE_READY_HALF = 0x05U,  /*!< DMA Half process success            */
}HAL_DMA_StateTypeDef;

typedef enum
{
	HAL_I2C_STATE_RESET = 0x00U,   /*!< Peripheral is not yet Initialized         */
	HAL_I2C_STATE_READY = 0x20U,   /*!< Peripheral Initialized and ready for use  */
	HAL_I2C_STATE_BUSY = 0x24U,   /*!< An internal process is ongoing            */
	HAL_I2C_STATE_BUSY_TX = 0x21U,   /*!< Data Transmission process is ongoing      */
	HAL_I2C_STATE_BUSY_RX = 0x22U,   /*!< Data Reception process is ongoing         */
	HAL_I2C_STATE_LISTEN = 0x28U,   /*!< Address Listen Mode is ongoing            */
	HAL_I2C_STATE_BUSY_TX_LISTEN = 0x29U,   /*!< Address Listen Mode and Data Transmission
											process is ongoing                         */
	HAL_I2C_STATE_BUSY_RX_LISTEN = 0x2AU,   /*!< Address Listen Mode and Data Reception
											process is ongoing                         */
	HAL_I2C_STATE_ABORT = 0x60U,   /*!< Abort user request ongoing                */
	HAL_I2C_STATE_TIMEOUT = 0xA0U,   /*!< Timeout state                             */
	HAL_I2C_STATE_ERROR = 0xE0U    /*!< Error                                     */

}HAL_I2C_StateTypeDef;

typedef enum
{
	HAL_I2C_MODE_NONE = 0x00U,   /*!< No I2C communication on going             */
	HAL_I2C_MODE_MASTER = 0x10U,   /*!< I2C communication is in Master Mode       */
	HAL_I2C_MODE_SLAVE = 0x20U,   /*!< I2C communication is in Slave Mode        */
	HAL_I2C_MODE_MEM = 0x40U    /*!< I2C communication is in Memory Mode       */

}HAL_I2C_ModeTypeDef;

/**
* @brief  GPIO Bit SET and Bit RESET enumeration
*/
typedef enum
{
	GPIO_PIN_RESET = 0,
	GPIO_PIN_SET
}GPIO_PinState;


typedef enum
{
	HAL_OK = 0x00U,
	HAL_ERROR = 0x01U,
	HAL_BUSY = 0x02U,
	HAL_TIMEOUT = 0x03U
} HAL_StatusTypeDef;

/** @defgroup RNG_Exported_Types_Group1 RNG State Structure definition
* @{
*/
typedef enum
{
	HAL_RNG_STATE_RESET = 0x00U,  /*!< RNG not yet initialized or disabled */
	HAL_RNG_STATE_READY = 0x01U,  /*!< RNG initialized and ready for use   */
	HAL_RNG_STATE_BUSY = 0x02U,  /*!< RNG internal process is ongoing     */
	HAL_RNG_STATE_TIMEOUT = 0x03U,  /*!< RNG timeout state                   */
	HAL_RNG_STATE_ERROR = 0x04U   /*!< RNG error state                     */

}HAL_RNG_StateTypeDef;

typedef enum
{
	HAL_CRC_STATE_RESET = 0x00U,  /*!< CRC not yet initialized or disabled */
	HAL_CRC_STATE_READY = 0x01U,  /*!< CRC initialized and ready for use   */
	HAL_CRC_STATE_BUSY = 0x02U,  /*!< CRC internal process is ongoing     */
	HAL_CRC_STATE_TIMEOUT = 0x03U,  /*!< CRC timeout state                   */
	HAL_CRC_STATE_ERROR = 0x04U   /*!< CRC error state                     */
}HAL_CRC_StateTypeDef;

/* Public Structures
*************************************************************************************/ 

typedef struct
{
	__IO uint32_t CCR;          /*!< DMA channel x configuration register */
	__IO uint32_t CNDTR;        /*!< DMA channel x number of data register */
	__IO uint32_t CPAR;         /*!< DMA channel x peripheral address register */
	__IO uint32_t CMAR;         /*!< DMA channel x memory address register */
} DMA_Channel_TypeDef;

/**
* @brief  DMA Configuration Structure definition
*/
typedef struct
{
	uint32_t Request;                   /*!< Specifies the request selected for the specified channel.
										This parameter can be a value of @ref DMA_request */

	uint32_t Direction;                 /*!< Specifies if the data will be transferred from memory to peripheral,
										from memory to memory or from peripheral to memory.
										This parameter can be a value of @ref DMA_Data_transfer_direction */

	uint32_t PeriphInc;                 /*!< Specifies whether the Peripheral address register should be incremented or not.
										When Memory to Memory transfer is used, this is the Source Increment mode
										This parameter can be a value of @ref DMA_Peripheral_incremented_mode */

	uint32_t MemInc;                    /*!< Specifies whether the memory address register should be incremented or not.
										When Memory to Memory transfer is used, this is the Destination Increment mode
										This parameter can be a value of @ref DMA_Memory_incremented_mode */

	uint32_t PeriphDataAlignment;       /*!< Specifies the Peripheral data width.
										When Memory to Memory transfer is used, this is the Source Alignment format
										This parameter can be a value of @ref DMA_Peripheral_data_size */

	uint32_t MemDataAlignment;          /*!< Specifies the Memory data width.
										When Memory to Memory transfer is used, this is the Destination Alignment format
										This parameter can be a value of @ref DMA_Memory_data_size */

	uint32_t Mode;                      /*!< Specifies the operation mode of the DMAy Channelx (Normal or Circular).
										This parameter can be a value of @ref DMA_mode
										@note The circular buffer mode cannot be used if the memory-to-memory
										data transfer is configured on the selected Channel */

	uint32_t Priority;                   /*!< Specifies the software priority for the DMAy Channelx.
										 This parameter can be a value of @ref DMA_Priority_level */
} DMA_InitTypeDef;

/**
* @brief  DMA handle Structure definition
*/
typedef struct __DMA_HandleTypeDef
{
	DMA_Channel_TypeDef    *Instance;                                                   /*!< Register base address                  */

	DMA_InitTypeDef       Init;                                                         /*!< DMA communication parameters           */

	HAL_LockTypeDef       Lock;                                                         /*!< DMA locking object                     */

	__IO HAL_DMA_StateTypeDef  State;                                                   /*!< DMA transfer state                     */

	void                  *Parent;                                                      /*!< Parent object state                    */

	void(*XferCpltCallback)(struct __DMA_HandleTypeDef * hdma);     /*!< DMA transfer complete callback         */

	void(*XferHalfCpltCallback)(struct __DMA_HandleTypeDef * hdma); /*!< DMA Half transfer complete callback    */

	void(*XferErrorCallback)(struct __DMA_HandleTypeDef * hdma);    /*!< DMA transfer error callback            */

	void(*XferAbortCallback)(struct __DMA_HandleTypeDef * hdma);    /*!< DMA transfer abort callback          */

	__IO uint32_t          ErrorCode;                                                     /*!< DMA Error code                         */

} DMA_HandleTypeDef;

typedef struct
{
   __IO uint32_t MODER;        /*!< GPIO port mode register,                     Address offset: 0x00 */
   __IO uint32_t OTYPER;       /*!< GPIO port output type register,              Address offset: 0x04 */
   __IO uint32_t OSPEEDR;      /*!< GPIO port output speed register,             Address offset: 0x08 */
   __IO uint32_t PUPDR;        /*!< GPIO port pull-up/pull-down register,        Address offset: 0x0C */
   __IO uint32_t IDR;          /*!< GPIO port input data register,               Address offset: 0x10 */
   __IO uint32_t ODR;          /*!< GPIO port output data register,              Address offset: 0x14 */
   __IO uint32_t BSRR;         /*!< GPIO port bit set/reset registerBSRR,        Address offset: 0x18 */
   __IO uint32_t LCKR;         /*!< GPIO port configuration lock register,       Address offset: 0x1C */
   __IO uint32_t AFR[2];       /*!< GPIO alternate function register,            Address offset: 0x20-0x24 */
   __IO uint32_t BRR;          /*!< GPIO bit reset register,                     Address offset: 0x28 */
}GPIO_TypeDef;

typedef struct
{
   __IO uint32_t CR1;      /*!< I2C Control register 1,            Address offset: 0x00 */
   __IO uint32_t CR2;      /*!< I2C Control register 2,            Address offset: 0x04 */
   __IO uint32_t OAR1;     /*!< I2C Own address 1 register,        Address offset: 0x08 */
   __IO uint32_t OAR2;     /*!< I2C Own address 2 register,        Address offset: 0x0C */
   __IO uint32_t TIMINGR;  /*!< I2C Timing register,               Address offset: 0x10 */
   __IO uint32_t TIMEOUTR; /*!< I2C Timeout register,              Address offset: 0x14 */
   __IO uint32_t ISR;      /*!< I2C Interrupt and status register, Address offset: 0x18 */
   __IO uint32_t ICR;      /*!< I2C Interrupt clear register,      Address offset: 0x1C */
   __IO uint32_t PECR;     /*!< I2C PEC register,                  Address offset: 0x20 */
   __IO uint32_t RXDR;     /*!< I2C Receive data register,         Address offset: 0x24 */
   __IO uint32_t TXDR;     /*!< I2C Transmit data register,        Address offset: 0x28 */
}I2C_TypeDef;

typedef struct
{
   __IO uint32_t ISR;      /*!< LPTIM Interrupt and Status register,             Address offset: 0x00 */
   __IO uint32_t ICR;      /*!< LPTIM Interrupt Clear register,                  Address offset: 0x04 */
   __IO uint32_t IER;      /*!< LPTIM Interrupt Enable register,                 Address offset: 0x08 */
   __IO uint32_t CFGR;     /*!< LPTIM Configuration register,                    Address offset: 0x0C */
   __IO uint32_t CR;       /*!< LPTIM Control register,                          Address offset: 0x10 */
   __IO uint32_t CMP;      /*!< LPTIM Compare register,                          Address offset: 0x14 */
   __IO uint32_t ARR;      /*!< LPTIM Autoreload register,                       Address offset: 0x18 */
   __IO uint32_t CNT;      /*!< LPTIM Counter register,                          Address offset: 0x1C */
} LPTIM_TypeDef;

typedef struct
{
   __IO uint32_t TR;         /*!< RTC time register,                                         Address offset: 0x00 */
   __IO uint32_t DR;         /*!< RTC date register,                                         Address offset: 0x04 */
   __IO uint32_t CR;         /*!< RTC control register,                                      Address offset: 0x08 */
   __IO uint32_t ISR;        /*!< RTC initialization and status register,                    Address offset: 0x0C */
   __IO uint32_t PRER;       /*!< RTC prescaler register,                                    Address offset: 0x10 */
   __IO uint32_t WUTR;       /*!< RTC wakeup timer register,                                 Address offset: 0x14 */
   uint32_t RESERVED;   /*!< Reserved,                                                  Address offset: 0x18 */
   __IO uint32_t ALRMAR;     /*!< RTC alarm A register,                                      Address offset: 0x1C */
   __IO uint32_t ALRMBR;     /*!< RTC alarm B register,                                      Address offset: 0x20 */
   __IO uint32_t WPR;        /*!< RTC write protection register,                             Address offset: 0x24 */
   __IO uint32_t SSR;        /*!< RTC sub second register,                                   Address offset: 0x28 */
   __IO uint32_t SHIFTR;     /*!< RTC shift control register,                                Address offset: 0x2C */
   __IO uint32_t TSTR;       /*!< RTC time stamp time register,                              Address offset: 0x30 */
   __IO uint32_t TSDR;       /*!< RTC time stamp date register,                              Address offset: 0x34 */
   __IO uint32_t TSSSR;      /*!< RTC time-stamp sub second register,                        Address offset: 0x38 */
   __IO uint32_t CALR;       /*!< RTC calibration register,                                  Address offset: 0x3C */
   __IO uint32_t TAMPCR;     /*!< RTC tamper configuration register,                         Address offset: 0x40 */
   __IO uint32_t ALRMASSR;   /*!< RTC alarm A sub second register,                           Address offset: 0x44 */
   __IO uint32_t ALRMBSSR;   /*!< RTC alarm B sub second register,                           Address offset: 0x48 */
   __IO uint32_t OR;         /*!< RTC option register,                                       Address offset  0x4C */
   __IO uint32_t BKP0R;      /*!< RTC backup register 0,                                     Address offset: 0x50 */
   __IO uint32_t BKP1R;      /*!< RTC backup register 1,                                     Address offset: 0x54 */
   __IO uint32_t BKP2R;      /*!< RTC backup register 2,                                     Address offset: 0x58 */
   __IO uint32_t BKP3R;      /*!< RTC backup register 3,                                     Address offset: 0x5C */
   __IO uint32_t BKP4R;      /*!< RTC backup register 4,                                     Address offset: 0x60 */
} RTC_TypeDef;

/**
* @brief Reset and Clock Control
*/
typedef struct
{
   __IO uint32_t CR;            /*!< RCC clock control register,                                   Address offset: 0x00 */
   __IO uint32_t ICSCR;         /*!< RCC Internal clock sources calibration register,              Address offset: 0x04 */
   __IO uint32_t CRRCR;         /*!< RCC Clock recovery RC register,                               Address offset: 0x08 */
   __IO uint32_t CFGR;          /*!< RCC Clock configuration register,                             Address offset: 0x0C */
   __IO uint32_t CIER;          /*!< RCC Clock interrupt enable register,                          Address offset: 0x10 */
   __IO uint32_t CIFR;          /*!< RCC Clock interrupt flag register,                            Address offset: 0x14 */
   __IO uint32_t CICR;          /*!< RCC Clock interrupt clear register,                           Address offset: 0x18 */
   __IO uint32_t IOPRSTR;       /*!< RCC IO port reset register,                                   Address offset: 0x1C */
   __IO uint32_t AHBRSTR;       /*!< RCC AHB peripheral reset register,                            Address offset: 0x20 */
   __IO uint32_t APB2RSTR;      /*!< RCC APB2 peripheral reset register,                           Address offset: 0x24 */
   __IO uint32_t APB1RSTR;      /*!< RCC APB1 peripheral reset register,                           Address offset: 0x28 */
   __IO uint32_t IOPENR;        /*!< RCC Clock IO port enable register,                            Address offset: 0x2C */
   __IO uint32_t AHBENR;        /*!< RCC AHB peripheral clock enable register,                     Address offset: 0x30 */
   __IO uint32_t APB2ENR;       /*!< RCC APB2 peripheral enable register,                          Address offset: 0x34 */
   __IO uint32_t APB1ENR;       /*!< RCC APB1 peripheral enable register,                          Address offset: 0x38 */
   __IO uint32_t IOPSMENR;      /*!< RCC IO port clock enable in sleep mode register,              Address offset: 0x3C */
   __IO uint32_t AHBSMENR;      /*!< RCC AHB peripheral clock enable in sleep mode register,       Address offset: 0x40 */
   __IO uint32_t APB2SMENR;     /*!< RCC APB2 peripheral clock enable in sleep mode register,      Address offset: 0x44 */
   __IO uint32_t APB1SMENR;     /*!< RCC APB1 peripheral clock enable in sleep mode register,      Address offset: 0x48 */
   __IO uint32_t CCIPR;         /*!< RCC clock configuration register,                             Address offset: 0x4C */
   __IO uint32_t CSR;           /*!< RCC Control/status register,                                  Address offset: 0x50 */
} RCC_TypeDef;

typedef struct
{
   __IO uint32_t CR1;      /*!< SPI Control register 1 (not used in I2S mode),       Address offset: 0x00 */
   __IO uint32_t CR2;      /*!< SPI Control register 2,                              Address offset: 0x04 */
   __IO uint32_t SR;       /*!< SPI Status register,                                 Address offset: 0x08 */
   __IO uint32_t DR;       /*!< SPI data register,                                   Address offset: 0x0C */
   __IO uint32_t CRCPR;    /*!< SPI CRC polynomial register (not used in I2S mode),  Address offset: 0x10 */
   __IO uint32_t RXCRCR;   /*!< SPI Rx CRC register (not used in I2S mode),          Address offset: 0x14 */
   __IO uint32_t TXCRCR;   /*!< SPI Tx CRC register (not used in I2S mode),          Address offset: 0x18 */
   __IO uint32_t I2SCFGR;  /*!< SPI_I2S configuration register,                      Address offset: 0x1C */
   __IO uint32_t I2SPR;    /*!< SPI_I2S prescaler register,                          Address offset: 0x20 */
} SPI_TypeDef;

typedef struct
{
   __IO uint32_t CR1;       /*!< TIM control register 1,                       Address offset: 0x00 */
   __IO uint32_t CR2;       /*!< TIM control register 2,                       Address offset: 0x04 */
   __IO uint32_t SMCR;      /*!< TIM slave Mode Control register,              Address offset: 0x08 */
   __IO uint32_t DIER;      /*!< TIM DMA/interrupt enable register,            Address offset: 0x0C */
   __IO uint32_t SR;        /*!< TIM status register,                          Address offset: 0x10 */
   __IO uint32_t EGR;       /*!< TIM event generation register,                Address offset: 0x14 */
   __IO uint32_t CCMR1;     /*!< TIM  capture/compare mode register 1,         Address offset: 0x18 */
   __IO uint32_t CCMR2;     /*!< TIM  capture/compare mode register 2,         Address offset: 0x1C */
   __IO uint32_t CCER;      /*!< TIM capture/compare enable register,          Address offset: 0x20 */
   __IO uint32_t CNT;       /*!< TIM counter register,                         Address offset: 0x24 */
   __IO uint32_t PSC;       /*!< TIM prescaler register,                       Address offset: 0x28 */
   __IO uint32_t ARR;       /*!< TIM auto-reload register,                     Address offset: 0x2C */
   uint32_t      RESERVED12;/*!< Reserved                                      Address offset: 0x30 */
   __IO uint32_t CCR1;      /*!< TIM capture/compare register 1,               Address offset: 0x34 */
   __IO uint32_t CCR2;      /*!< TIM capture/compare register 2,               Address offset: 0x38 */
   __IO uint32_t CCR3;      /*!< TIM capture/compare register 3,               Address offset: 0x3C */
   __IO uint32_t CCR4;      /*!< TIM capture/compare register 4,               Address offset: 0x40 */
   uint32_t      RESERVED17;/*!< Reserved,                                     Address offset: 0x44 */
   __IO uint32_t DCR;       /*!< TIM DMA control register,                     Address offset: 0x48 */
   __IO uint32_t DMAR;      /*!< TIM DMA address for full transfer register,   Address offset: 0x4C */
   __IO uint32_t OR;        /*!< TIM option register,                          Address offset: 0x50 */
} TIM_TypeDef;

typedef struct
{
   __IO uint32_t CR1;    /*!< USART Control register 1,                 Address offset: 0x00 */
   __IO uint32_t CR2;    /*!< USART Control register 2,                 Address offset: 0x04 */
   __IO uint32_t CR3;    /*!< USART Control register 3,                 Address offset: 0x08 */
   __IO uint32_t BRR;    /*!< USART Baud rate register,                 Address offset: 0x0C */
   __IO uint32_t GTPR;   /*!< USART Guard time and prescaler register,  Address offset: 0x10 */
   __IO uint32_t RTOR;   /*!< USART Receiver Time Out register,         Address offset: 0x14 */
   __IO uint32_t RQR;    /*!< USART Request register,                   Address offset: 0x18 */
   __IO uint32_t ISR;    /*!< USART Interrupt and status register,      Address offset: 0x1C */
   __IO uint32_t ICR;    /*!< USART Interrupt flag Clear register,      Address offset: 0x20 */
   __IO uint32_t RDR;    /*!< USART Receive Data register,              Address offset: 0x24 */
   __IO uint32_t TDR;    /*!< USART Transmit Data register,             Address offset: 0x28 */
} USART_TypeDef;

typedef struct
{
	uint32_t              *Instance;         /*!< Register base address     */

	uint32_t           Init;             /*!< LPTIM required parameters */

	uint32_t           Status;           /*!< LPTIM peripheral status   */

	uint32_t             Lock;             /*!< LPTIM locking object      */

	__IO  uint32_t   State;            /*!< LPTIM peripheral state    */

}LPTIM_HandleTypeDef;

typedef struct
{
	uint32_t Mode;                /*!< Specifies the SPI operating mode.
								  This parameter can be a value of @ref SPI_Mode */

	uint32_t Direction;           /*!< Specifies the SPI bidirectional mode state.
								  This parameter can be a value of @ref SPI_Direction */

	uint32_t DataSize;            /*!< Specifies the SPI data size.
								  This parameter can be a value of @ref SPI_Data_Size */

	uint32_t CLKPolarity;         /*!< Specifies the serial clock steady state.
								  This parameter can be a value of @ref SPI_Clock_Polarity */

	uint32_t CLKPhase;            /*!< Specifies the clock active edge for the bit capture.
								  This parameter can be a value of @ref SPI_Clock_Phase */

	uint32_t NSS;                 /*!< Specifies whether the NSS signal is managed by
								  hardware (NSS pin) or by software using the SSI bit.
								  This parameter can be a value of @ref SPI_Slave_Select_management */

	uint32_t BaudRatePrescaler;   /*!< Specifies the Baud Rate prescaler value which will be
								  used to configure the transmit and receive SCK clock.
								  This parameter can be a value of @ref SPI_BaudRate_Prescaler
								  @note The communication clock is derived from the master
								  clock. The slave clock does not need to be set. */

	uint32_t FirstBit;            /*!< Specifies whether data transfers start from MSB or LSB bit.
								  This parameter can be a value of @ref SPI_MSB_LSB_transmission */

	uint32_t TIMode;              /*!< Specifies if the TI mode is enabled or not.
								  This parameter can be a value of @ref SPI_TI_mode */

	uint32_t CRCCalculation;      /*!< Specifies if the CRC calculation is enabled or not.
								  This parameter can be a value of @ref SPI_CRC_Calculation */

	uint32_t CRCPolynomial;       /*!< Specifies the polynomial used for the CRC calculation.
								  This parameter must be an odd number between Min_Data = 1 and Max_Data = 65535 */

	uint32_t CRCLength;           /*!< Specifies the CRC Length used for the CRC calculation.
								  CRC Length is only used with Data8 and Data16, not other data size
								  This parameter can be a value of @ref SPI_CRC_length */

	uint32_t NSSPMode;            /*!< Specifies whether the NSSP signal is enabled or not .
								  This parameter can be a value of @ref SPI_NSSP_Mode
								  This mode is activated by the NSSP bit in the SPIx_CR2 register and
								  it takes effect only if the SPI interface is configured as Motorola SPI
								  master (FRF=0) with capture on the first edge (SPIx_CR1 CPHA = 0,
								  CPOL setting is ignored).. */
} SPI_InitTypeDef;

typedef struct __SPI_HandleTypeDef
{
	SPI_TypeDef                *Instance;      /*!< SPI registers base address               */

	SPI_InitTypeDef            Init;           /*!< SPI communication parameters             */

	uint8_t                    *pTxBuffPtr;    /*!< Pointer to SPI Tx transfer Buffer        */

	uint16_t                   TxXferSize;     /*!< SPI Tx Transfer size                     */

	__IO uint16_t              TxXferCount;    /*!< SPI Tx Transfer Counter                  */

	uint8_t                    *pRxBuffPtr;    /*!< Pointer to SPI Rx transfer Buffer        */

	uint16_t                   RxXferSize;     /*!< SPI Rx Transfer size                     */

	__IO uint16_t              RxXferCount;    /*!< SPI Rx Transfer Counter                  */

	uint32_t                   CRCSize;        /*!< SPI CRC size used for the transfer       */

	void(*RxISR)(struct __SPI_HandleTypeDef *hspi);   /*!< function pointer on Rx ISR       */

	void(*TxISR)(struct __SPI_HandleTypeDef *hspi);   /*!< function pointer on Tx ISR       */

	struct DMA_HandleTypeDef          *hdmatx;        /*!< SPI Tx DMA Handle parameters             */

	struct DMA_HandleTypeDef          *hdmarx;        /*!< SPI Rx DMA Handle parameters             */

	HAL_LockTypeDef            Lock;           /*!< Locking object                           */

	__IO HAL_SPI_StateTypeDef  State;          /*!< SPI communication state                  */

	__IO uint32_t              ErrorCode;      /*!< SPI Error code                           */

} SPI_HandleTypeDef;

/** @defgroup I2C_Configuration_Structure_definition I2C Configuration Structure definition
* @brief  I2C Configuration Structure definition
* @{
*/
typedef struct
{
	uint32_t Timing;              /*!< Specifies the I2C_TIMINGR_register value.
								  This parameter calculated by referring to I2C initialization
								  section in Reference manual */

	uint32_t OwnAddress1;         /*!< Specifies the first device own address.
								  This parameter can be a 7-bit or 10-bit address. */

	uint32_t AddressingMode;      /*!< Specifies if 7-bit or 10-bit addressing mode is selected.
								  This parameter can be a value of @ref I2C_ADDRESSING_MODE */

	uint32_t DualAddressMode;     /*!< Specifies if dual addressing mode is selected.
								  This parameter can be a value of @ref I2C_DUAL_ADDRESSING_MODE */

	uint32_t OwnAddress2;         /*!< Specifies the second device own address if dual addressing mode is selected
								  This parameter can be a 7-bit address. */

	uint32_t OwnAddress2Masks;    /*!< Specifies the acknowledge mask address second device own address if dual addressing mode is selected
								  This parameter can be a value of @ref I2C_OWN_ADDRESS2_MASKS */

	uint32_t GeneralCallMode;     /*!< Specifies if general call mode is selected.
								  This parameter can be a value of @ref I2C_GENERAL_CALL_ADDRESSING_MODE */

	uint32_t NoStretchMode;       /*!< Specifies if nostretch mode is selected.
								  This parameter can be a value of @ref I2C_NOSTRETCH_MODE */

}I2C_InitTypeDef;

/** @defgroup I2C_handle_Structure_definition I2C handle Structure definition
* @brief  I2C handle Structure definition
* @{
*/
typedef struct __I2C_HandleTypeDef
{
	I2C_TypeDef                *Instance;      /*!< I2C registers base address                */

	I2C_InitTypeDef            Init;           /*!< I2C communication parameters              */

	uint8_t                    *pBuffPtr;      /*!< Pointer to I2C transfer buffer            */

	uint16_t                   XferSize;       /*!< I2C transfer size                         */

	__IO uint16_t              XferCount;      /*!< I2C transfer counter                      */

	__IO uint32_t              XferOptions;    /*!< I2C sequantial transfer options, this parameter can
											   be a value of @ref I2C_XFEROPTIONS */

	__IO uint32_t              PreviousState;  /*!< I2C communication Previous state          */

	HAL_StatusTypeDef(*XferISR)(struct __I2C_HandleTypeDef *hi2c, uint32_t ITFlags, uint32_t ITSources); /*!< I2C transfer IRQ handler function pointer */

	DMA_HandleTypeDef          *hdmatx;        /*!< I2C Tx DMA handle parameters              */

	DMA_HandleTypeDef          *hdmarx;        /*!< I2C Rx DMA handle parameters              */

	HAL_LockTypeDef            Lock;           /*!< I2C locking object                        */

	__IO HAL_I2C_StateTypeDef  State;          /*!< I2C communication state                   */

	__IO HAL_I2C_ModeTypeDef   Mode;           /*!< I2C communication mode                    */

	__IO uint32_t              ErrorCode;      /*!< I2C Error code                            */

	__IO uint32_t              AddrEventCount; /*!< I2C Address Event counter                 */
}I2C_HandleTypeDef;

/**
* @brief   GPIO Init structure definition
*/
typedef struct
{
	uint32_t Pin;        /*!< Specifies the GPIO pins to be configured.
						 This parameter can be any value of @ref GPIO_pins */

	uint32_t Mode;       /*!< Specifies the operating mode for the selected pins.
						 This parameter can be a value of @ref GPIO_mode */

	uint32_t Pull;       /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
						 This parameter can be a value of @ref GPIO_pull */

	uint32_t Speed;      /*!< Specifies the speed for the selected pins.
						 This parameter can be a value of @ref GPIO_speed */

	uint32_t Alternate;  /*!< Peripheral to be connected to the selected pins
						 This parameter can be a value of @ref GPIOEx_Alternate_function_selection */
}GPIO_InitTypeDef;

/**
\brief  Structure type to access the System Timer (SysTick).
*/
typedef struct
{
	__IOM uint32_t CTRL;                   /*!< Offset: 0x000 (R/W)  SysTick Control and Status Register */
	__IOM uint32_t LOAD;                   /*!< Offset: 0x004 (R/W)  SysTick Reload Value Register */
	__IOM uint32_t VAL;                    /*!< Offset: 0x008 (R/W)  SysTick Current Value Register */
	__IM  uint32_t CALIB;                  /*!< Offset: 0x00C (R/ )  SysTick Calibration Register */
} SysTick_Type;

/**
\brief  Structure type to access the System Control Block (SCB).
*/
typedef struct
{
	__IM  uint32_t CPUID;                  /*!< Offset: 0x000 (R/ )  CPUID Base Register */
	__IOM uint32_t ICSR;                   /*!< Offset: 0x004 (R/W)  Interrupt Control and State Register */
	__IOM uint32_t VTOR;                   /*!< Offset: 0x008 (R/W)  Vector Table Offset Register */
	__IOM uint32_t AIRCR;                  /*!< Offset: 0x00C (R/W)  Application Interrupt and Reset Control Register */
	__IOM uint32_t SCR;                    /*!< Offset: 0x010 (R/W)  System Control Register */
	__IOM uint32_t CCR;                    /*!< Offset: 0x014 (R/W)  Configuration Control Register */
	__IOM uint8_t  SHP[12U];               /*!< Offset: 0x018 (R/W)  System Handlers Priority Registers (4-7, 8-11, 12-15) */
	__IOM uint32_t SHCSR;                  /*!< Offset: 0x024 (R/W)  System Handler Control and State Register */
	__IOM uint32_t CFSR;                   /*!< Offset: 0x028 (R/W)  Configurable Fault Status Register */
	__IOM uint32_t HFSR;                   /*!< Offset: 0x02C (R/W)  HardFault Status Register */
	__IOM uint32_t DFSR;                   /*!< Offset: 0x030 (R/W)  Debug Fault Status Register */
	__IOM uint32_t MMFAR;                  /*!< Offset: 0x034 (R/W)  MemManage Fault Address Register */
	__IOM uint32_t BFAR;                   /*!< Offset: 0x038 (R/W)  BusFault Address Register */
	__IOM uint32_t AFSR;                   /*!< Offset: 0x03C (R/W)  Auxiliary Fault Status Register */
	__IM  uint32_t PFR[2U];                /*!< Offset: 0x040 (R/ )  Processor Feature Register */
	__IM  uint32_t DFR;                    /*!< Offset: 0x048 (R/ )  Debug Feature Register */
	__IM  uint32_t ADR;                    /*!< Offset: 0x04C (R/ )  Auxiliary Feature Register */
	__IM  uint32_t MMFR[4U];               /*!< Offset: 0x050 (R/ )  Memory Model Feature Register */
	__IM  uint32_t ISAR[5U];               /*!< Offset: 0x060 (R/ )  Instruction Set Attributes Register */
	uint32_t RESERVED0[5U];
	__IOM uint32_t CPACR;                  /*!< Offset: 0x088 (R/W)  Coprocessor Access Control Register */
} SCB_Type;

/**
* @brief External Interrupt/Event Controller
*/

typedef struct
{
	__IO uint32_t IMR1;        /*!< EXTI Interrupt mask register 1,             Address offset: 0x00 */
	__IO uint32_t EMR1;        /*!< EXTI Event mask register 1,                 Address offset: 0x04 */
	__IO uint32_t RTSR1;       /*!< EXTI Rising trigger selection register 1,   Address offset: 0x08 */
	__IO uint32_t FTSR1;       /*!< EXTI Falling trigger selection register 1,  Address offset: 0x0C */
	__IO uint32_t SWIER1;      /*!< EXTI Software interrupt event register 1,   Address offset: 0x10 */
	__IO uint32_t PR1;         /*!< EXTI Pending register 1,                    Address offset: 0x14 */
	uint32_t      RESERVED1;   /*!< Reserved, 0x18                                                   */
	uint32_t      RESERVED2;   /*!< Reserved, 0x1C                                                   */
	__IO uint32_t IMR2;        /*!< EXTI Interrupt mask register 2,             Address offset: 0x20 */
	__IO uint32_t EMR2;        /*!< EXTI Event mask register 2,                 Address offset: 0x24 */
	__IO uint32_t RTSR2;       /*!< EXTI Rising trigger selection register 2,   Address offset: 0x28 */
	__IO uint32_t FTSR2;       /*!< EXTI Falling trigger selection register 2,  Address offset: 0x2C */
	__IO uint32_t SWIER2;      /*!< EXTI Software interrupt event register 2,   Address offset: 0x30 */
	__IO uint32_t PR2;         /*!< EXTI Pending register 2,                    Address offset: 0x34 */
} EXTI_TypeDef;

typedef struct
{
	uint32_t DR;            /*!< CRC Data register,                            Address offset: 0x00 */
	uint8_t IDR;            /*!< CRC Independent data register,                Address offset: 0x04 */
	uint8_t RESERVED0;           /*!< Reserved,                                                     0x05 */
	uint16_t RESERVED1;          /*!< Reserved,                                                     0x06 */
	uint32_t CR;            /*!< CRC Control register,                         Address offset: 0x08 */
	uint32_t RESERVED2;          /*!< Reserved,                                                     0x0C */
	uint32_t INIT;          /*!< Initial CRC value register,                   Address offset: 0x10 */
	uint32_t POL;           /*!< CRC polynomial register,                      Address offset: 0x14 */
} CRC_TypeDef;

/**
* @brief CRC Init Structure definition
*/
typedef struct
{
	uint8_t DefaultPolynomialUse;       /*!< This parameter is a value of @ref CRC_Default_Polynomial and indicates if default polynomial is used.
										If set to DEFAULT_POLYNOMIAL_ENABLE, resort to default
										X^32 + X^26 + X^23 + X^22 + X^16 + X^12 + X^11 + X^10 +X^8 + X^7 + X^5 + X^4 + X^2+ X +1.
										In that case, there is no need to set GeneratingPolynomial field.
										If otherwise set to DEFAULT_POLYNOMIAL_DISABLE, GeneratingPolynomial and CRCLength fields must be set */

	uint8_t DefaultInitValueUse;        /*!< This parameter is a value of @ref CRC_Default_InitValue_Use and indicates if default init value is used.
										If set to DEFAULT_INIT_VALUE_ENABLE, resort to default
										0xFFFFFFFF value. In that case, there is no need to set InitValue field.
										If otherwise set to DEFAULT_INIT_VALUE_DISABLE,  InitValue field must be set */

	uint32_t GeneratingPolynomial;      /*!< Set CRC generating polynomial. 7, 8, 16 or 32-bit long value for a polynomial degree
										respectively equal to 7, 8, 16 or 32. This field is written in normal representation,
										e.g., for a polynomial of degree 7, X^7 + X^6 + X^5 + X^2 + 1 is written 0x65.
										No need to specify it if DefaultPolynomialUse is set to DEFAULT_POLYNOMIAL_ENABLE   */

	uint32_t CRCLength;                 /*!< This parameter is a value of @ref CRC_Polynomial_Sizes and indicates CRC length.
										Value can be either one of
										CRC_POLYLENGTH_32B                  (32-bit CRC)
										CRC_POLYLENGTH_16B                  (16-bit CRC)
										CRC_POLYLENGTH_8B                   (8-bit CRC)
										CRC_POLYLENGTH_7B                   (7-bit CRC) */

	uint32_t InitValue;                 /*!< Init value to initiate CRC computation. No need to specify it if DefaultInitValueUse
										is set to DEFAULT_INIT_VALUE_ENABLE   */

	uint32_t InputDataInversionMode;    /*!< This parameter is a value of @ref CRCEx_Input_Data_Inversion and specifies input data inversion mode.
										Can be either one of the following values
										CRC_INPUTDATA_INVERSION_NONE      no input data inversion
										CRC_INPUTDATA_INVERSION_BYTE      byte-wise inversion, 0x1A2B3C4D becomes 0x58D43CB2
										CRC_INPUTDATA_INVERSION_HALFWORD  halfword-wise inversion, 0x1A2B3C4D becomes 0xD458B23C
										CRC_INPUTDATA_INVERSION_WORD      word-wise inversion, 0x1A2B3C4D becomes 0xB23CD458 */

	uint32_t OutputDataInversionMode;   /*!< This parameter is a value of @ref CRCEx_Output_Data_Inversion and specifies output data (i.e. CRC) inversion mode.
										Can be either
										CRC_OUTPUTDATA_INVERSION_DISABLE   no CRC inversion, or
										CRC_OUTPUTDATA_INVERSION_ENABLE    CRC 0x11223344 is converted into 0x22CC4488 */
}CRC_InitTypeDef;

/**
* @brief  CRC Handle Structure definition
*/
typedef struct
{
	CRC_TypeDef                 *Instance;   /*!< Register base address        */

	CRC_InitTypeDef             Init;        /*!< CRC configuration parameters */

	uint8_t             Lock;        /*!< CRC Locking object           */

	uint8_t   State;       /*!< CRC communication state      */

	uint32_t InputDataFormat;                /*!< This parameter is a value of @ref CRC_Input_Buffer_Format and specifies input data format.
											 Can be either
											 CRC_INPUTDATA_FORMAT_BYTES       input data is a stream of bytes (8-bit data)
											 CRC_INPUTDATA_FORMAT_HALFWORDS   input data is a stream of half-words (16-bit data)
											 CRC_INPUTDATA_FORMAT_WORDS       input data is a stream of words (32-bits data)
											 Note that constant CRC_INPUT_FORMAT_UNDEFINED is defined but an initialization error
											 must occur if InputBufferFormat is not one of the three values listed above  */
}CRC_HandleTypeDef;

/**
* @brief Random numbers generator
*/
typedef struct
{
	__IO uint32_t CR;  /*!< RNG control register, Address offset: 0x00 */
	__IO uint32_t SR;  /*!< RNG status register,  Address offset: 0x04 */
	__IO uint32_t DR;  /*!< RNG data register,    Address offset: 0x08 */
} RNG_TypeDef;

/** @defgroup RNG_Exported_Types_Group2 RNG Handle Structure definition
* @{
*/
typedef struct
{
	RNG_TypeDef                 *Instance;    /*!< Register base address   */

	HAL_LockTypeDef             Lock;         /*!< RNG locking object      */

	__IO HAL_RNG_StateTypeDef   State;        /*!< RNG communication state */

	uint32_t                    RandomNumber; /*!< Last Generated RNG Data */

}RNG_HandleTypeDef;


typedef enum
{
   HAL_UART_STATE_RESET = 0x00U,   /*!< Peripheral is not initialized
                                   Value is allowed for gState and RxState */
   HAL_UART_STATE_READY = 0x20U,   /*!< Peripheral Initialized and ready for use
                                   Value is allowed for gState and RxState */
   HAL_UART_STATE_BUSY = 0x24U,   /*!< an internal process is ongoing
                                  Value is allowed for gState only */
   HAL_UART_STATE_BUSY_TX = 0x21U,   /*!< Data Transmission process is ongoing
                                     Value is allowed for gState only */
   HAL_UART_STATE_BUSY_RX = 0x22U,   /*!< Data Reception process is ongoing
                                     Value is allowed for RxState only */
   HAL_UART_STATE_BUSY_TX_RX = 0x23U,   /*!< Data Transmission and Reception process is ongoing
                                        Not to be used for neither gState nor RxState.
                                        Value is result of combination (Or) between gState and RxState values */
   HAL_UART_STATE_TIMEOUT = 0xA0U,   /*!< Timeout state
                                     Value is allowed for gState only */
   HAL_UART_STATE_ERROR = 0xE0U    /*!< Error
                                   Value is allowed for gState only */
}HAL_UART_StateTypeDef;

typedef struct
{
   uint32_t BaudRate;                  /*!< This member configures the UART communication baud rate.
                                       The baud rate register is computed using the following formula:
                                       UART:
                                       =====
                                       - If oversampling is 16 or in LIN mode,
                                       Baud Rate Register = ((uart_ker_ckpres) / ((huart->Init.BaudRate)))
                                       - If oversampling is 8,
                                       Baud Rate Register[15:4] = ((2 * uart_ker_ckpres) / ((huart->Init.BaudRate)))[15:4]
                                       Baud Rate Register[3] =  0
                                       Baud Rate Register[2:0] =  (((2 * uart_ker_ckpres) / ((huart->Init.BaudRate)))[3:0]) >> 1
                                       LPUART:
                                       =======
                                       Baud Rate Register = ((256 * lpuart_ker_ckpres) / ((huart->Init.BaudRate)))

                                       where (uart/lpuart)_ker_ck_pres is the UART input clock divided by a prescaler */

   uint32_t WordLength;                /*!< Specifies the number of data bits transmitted or received in a frame.
                                       This parameter can be a value of @ref UARTEx_Word_Length. */

   uint32_t StopBits;                  /*!< Specifies the number of stop bits transmitted.
                                       This parameter can be a value of @ref UART_Stop_Bits. */

   uint32_t Parity;                    /*!< Specifies the parity mode.
                                       This parameter can be a value of @ref UART_Parity
                                       @note When parity is enabled, the computed parity is inserted
                                       at the MSB position of the transmitted data (9th bit when
                                       the word length is set to 9 data bits; 8th bit when the
                                       word length is set to 8 data bits). */

   uint32_t Mode;                      /*!< Specifies whether the Receive or Transmit mode is enabled or disabled.
                                       This parameter can be a value of @ref UART_Mode. */

   uint32_t HwFlowCtl;                 /*!< Specifies whether the hardware flow control mode is enabled
                                       or disabled.
                                       This parameter can be a value of @ref UART_Hardware_Flow_Control. */

   uint32_t OverSampling;              /*!< Specifies whether the Over sampling 8 is enabled or disabled, to achieve higher speed (up to f_PCLK/8).
                                       This parameter can be a value of @ref UART_Over_Sampling. */

   uint32_t OneBitSampling;            /*!< Specifies whether a single sample or three samples' majority vote is selected.
                                       Selecting the single sample method increases the receiver tolerance to clock
                                       deviations. This parameter can be a value of @ref UART_OneBit_Sampling. */

#if defined(USART_PRESC_PRESCALER)
   uint32_t ClockPrescaler;            /*!< Specifies the prescaler value used to divide the UART clock source.
                                       This parameter can be a value of @ref UART_ClockPrescaler. */
#endif

}UART_InitTypeDef;

typedef struct
{
   uint32_t AdvFeatureInit;        /*!< Specifies which advanced UART features is initialized. Several
                                   Advanced Features may be initialized at the same time .
                                   This parameter can be a value of @ref UART_Advanced_Features_Initialization_Type. */

   uint32_t TxPinLevelInvert;      /*!< Specifies whether the TX pin active level is inverted.
                                   This parameter can be a value of @ref UART_Tx_Inv.  */

   uint32_t RxPinLevelInvert;      /*!< Specifies whether the RX pin active level is inverted.
                                   This parameter can be a value of @ref UART_Rx_Inv.  */

   uint32_t DataInvert;            /*!< Specifies whether data are inverted (positive/direct logic
                                   vs negative/inverted logic).
                                   This parameter can be a value of @ref UART_Data_Inv. */

   uint32_t Swap;                  /*!< Specifies whether TX and RX pins are swapped.
                                   This parameter can be a value of @ref UART_Rx_Tx_Swap. */

   uint32_t OverrunDisable;        /*!< Specifies whether the reception overrun detection is disabled.
                                   This parameter can be a value of @ref UART_Overrun_Disable. */

   uint32_t DMADisableonRxError;   /*!< Specifies whether the DMA is disabled in case of reception error.
                                   This parameter can be a value of @ref UART_DMA_Disable_on_Rx_Error. */

   uint32_t AutoBaudRateEnable;    /*!< Specifies whether auto Baud rate detection is enabled.
                                   This parameter can be a value of @ref UART_AutoBaudRate_Enable */

   uint32_t AutoBaudRateMode;      /*!< If auto Baud rate detection is enabled, specifies how the rate
                                   detection is carried out.
                                   This parameter can be a value of @ref UART_AutoBaud_Rate_Mode. */

   uint32_t MSBFirst;              /*!< Specifies whether MSB is sent first on UART line.
                                   This parameter can be a value of @ref UART_MSB_First. */
} UART_AdvFeatureInitTypeDef;


typedef struct __UART_HandleTypeDef
{
   uint8_t                  Index;      		/*!< UART Module Index */

   USART_TypeDef            *Instance;                /*!< UART registers base address        */

   UART_InitTypeDef         Init;                     /*!< UART communication parameters      */

   UART_AdvFeatureInitTypeDef AdvancedInit;           /*!< UART Advanced Features initialization parameters */

   uint8_t                  *pTxBuffPtr;              /*!< Pointer to UART Tx transfer Buffer */

   uint16_t                 TxXferSize;               /*!< UART Tx Transfer size              */

   __IO uint16_t            TxXferCount;              /*!< UART Tx Transfer Counter           */

   uint8_t                  *pRxBuffPtr;              /*!< Pointer to UART Rx transfer Buffer */

   uint16_t                 RxXferSize;               /*!< UART Rx Transfer size              */

   __IO uint16_t            RxXferCount;              /*!< UART Rx Transfer Counter           */

   uint16_t                 Mask;                     /*!< UART Rx RDR register mask          */

   void(*RxISR)(struct __UART_HandleTypeDef *huart); /*!< Function pointer on Rx IRQ handler   */

   void(*TxISR)(struct __UART_HandleTypeDef *huart); /*!< Function pointer on Tx IRQ handler   */

   DMA_HandleTypeDef        *hdmatx;                  /*!< UART Tx DMA Handle parameters      */

   DMA_HandleTypeDef        *hdmarx;                  /*!< UART Rx DMA Handle parameters      */

   HAL_LockTypeDef           Lock;                    /*!< Locking object                     */

   __IO HAL_UART_StateTypeDef    gState;              /*!< UART state information related to global Handle management
                                                      and also related to Tx operations.
                                                      This parameter can be a value of @ref HAL_UART_StateTypeDef */

   __IO HAL_UART_StateTypeDef    RxState;             /*!< UART state information related to Rx operations.
                                                      This parameter can be a value of @ref HAL_UART_StateTypeDef */

   __IO uint32_t                 ErrorCode;           /*!< UART Error code                    */

}UART_HandleTypeDef;


/**
* @brief System configuration controller
*/

typedef struct
{
	__IO uint32_t MEMRMP;      /*!< SYSCFG memory remap register,                      Address offset: 0x00      */
	__IO uint32_t CFGR1;       /*!< SYSCFG configuration register 1,                   Address offset: 0x04      */
	__IO uint32_t EXTICR[4];   /*!< SYSCFG external interrupt configuration registers, Address offset: 0x08-0x14 */
	__IO uint32_t SCSR;        /*!< SYSCFG SRAM2 control and status register,          Address offset: 0x18      */
	__IO uint32_t CFGR2;       /*!< SYSCFG configuration register 2,                   Address offset: 0x1C      */
	__IO uint32_t SWPR;        /*!< SYSCFG SRAM2 write protection register,            Address offset: 0x20      */
	__IO uint32_t SKR;         /*!< SYSCFG SRAM2 key register,                         Address offset: 0x24      */
} SYSCFG_TypeDef;

/* Public Functions Prototypes
*************************************************************************************/

uint32_t HAL_LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim);
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef HAL_I2CEx_ConfigAnalogFilter(I2C_HandleTypeDef *hi2c, uint32_t AnalogFilter);
HAL_StatusTypeDef HAL_I2CEx_ConfigDigitalFilter(I2C_HandleTypeDef *hi2c, uint32_t DigitalFilter);
void HAL_NVIC_SetPriority(IRQn_Type IRQn, uint32_t PreemptPriority, uint32_t SubPriority);
void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);
HAL_StatusTypeDef  HAL_FLASH_Unlock(void);
HAL_StatusTypeDef  HAL_FLASH_Lock(void);
void  HAL_GPIO_Init(GPIO_TypeDef  *GPIOx, GPIO_InitTypeDef *GPIO_Init);
void  HAL_GPIO_DeInit(GPIO_TypeDef  *GPIOx, uint32_t GPIO_Pin);
uint32_t HAL_GetTick(void);
HAL_StatusTypeDef HAL_I2C_Mem_Write(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef HAL_I2C_Mem_Read(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData, uint16_t Size, uint32_t Timeout);
uint32_t             HAL_I2C_GetError(I2C_HandleTypeDef *hi2c);
void HAL_NVIC_SystemReset(void);
HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);
HAL_StatusTypeDef HAL_CRC_Init(CRC_HandleTypeDef *hcrc);
HAL_StatusTypeDef HAL_CRC_DeInit(CRC_HandleTypeDef *hcrc);
uint32_t HAL_CRC_Calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength);
void HAL_RCC_DeInit(void);
HAL_StatusTypeDef HAL_Init(void);
HAL_StatusTypeDef HAL_DeInit(void);
HAL_StatusTypeDef HAL_RNG_Init(RNG_HandleTypeDef *hrng);
HAL_StatusTypeDef HAL_RNG_DeInit(RNG_HandleTypeDef *hrng);
HAL_StatusTypeDef HAL_UART_DeInit(UART_HandleTypeDef *huart);
HAL_CRC_StateTypeDef HAL_CRC_GetState(CRC_HandleTypeDef *hcrc);
HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef HAL_DMA_Init(DMA_HandleTypeDef *hdma);
HAL_StatusTypeDef HAL_DMA_DeInit(DMA_HandleTypeDef *hdma);
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn);
HAL_StatusTypeDef HAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef HAL_UART_AbortReceive(UART_HandleTypeDef *huart);
void HAL_UART_IRQHandler(UART_HandleTypeDef *huart);
void HAL_DMA_IRQHandler(DMA_HandleTypeDef *hdma);

/* Macros
*************************************************************************************/
#define __RAM_FUNC HAL_StatusTypeDef

#define SET_BIT(REG, BIT)     ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define PERIPH_BASE            ((uint32_t)0x40000000U) /*!< Peripheral base address in the alias region */
/*!< Peripheral memory map */
#define APBPERIPH_BASE        PERIPH_BASE
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x00020000U)
#define IOPPERIPH_BASE        (PERIPH_BASE + 0x10000000U)#define GPIOB_BASE            (IOPPERIPH_BASE + 0x00000400U)
#define GPIO_AF1_I2C1          ((uint8_t)0x01U)  /* I2C1 Alternate Function mapping     */
#define GPIO_AF4_I2C1            ((uint8_t)0x04U)  /* I2C1 Alternate Function mapping    */
#define RCC_BASE              (AHBPERIPH_BASE + 0x00001000U)

#define RCC_APB1ENR_I2C1EN_Pos           (21U)                                 
#define RCC_APB1ENR_I2C1EN_Msk           (0x1U << RCC_APB1ENR_I2C1EN_Pos)      /*!< 0x00200000 */
#define RCC_APB1ENR_I2C1EN               RCC_APB1ENR_I2C1EN_Msk                /*!< I2C1 clock enable */

#define I2C_CR1_GCEN_Pos             (19U)                                     
#define I2C_CR1_GCEN_Msk             (0x1U << I2C_CR1_GCEN_Pos)                /*!< 0x00080000 */
#define I2C_CR1_GCEN                 I2C_CR1_GCEN_Msk                          /*!< General call enable */

#define RCC                 ((RCC_TypeDef *) RCC_BASE)

#define __HAL_RCC_I2C1_CLK_ENABLE()
#define __HAL_RCC_I2C1_CLK_DISABLE()
#define __HAL_FLASH_CLEAR_FLAG(x)
#define UNUSED(x) ((void)(x))

extern I2C_TypeDef I2C1_concrete;
extern I2C_TypeDef* I2C1;

extern GPIO_TypeDef GPIOB_concrete;
extern GPIO_TypeDef* GPIOB;

extern SysTick_Type dummy_SysTick;
extern SCB_Type     dummy_SCB;
extern EXTI_TypeDef dummy_Exti;
extern SYSCFG_TypeDef dummy_SYSCFG;
extern RNG_TypeDef  dummy_RNG;

#define SysTick ((SysTick_Type*)&dummy_SysTick )   /*!< SysTick configuration struct */
#define SCB     ((SCB_Type    *)&dummy_SCB     )   /*!< SCB configuration struct */
#define SYSCFG  ((SYSCFG_TypeDef *)&dummy_SYSCFG)
#define RNG     ((RNG_TypeDef *)&dummy_RNG)

#define __set_MSP(x) (regMainStackPointer = (x))
#define __attribute__(x)
#define __HAL_CRC_DR_RESET(x)
#define __enable_irq()
#define __disable_irq()
#define HAL_Delay(x)
#define EXTI     ((EXTI_TypeDef *) &dummy_Exti)
#define __HAL_RCC_CRC_CLK_ENABLE()
#define __HAL_RCC_GET_FLAG(x) RESET


#define PERIPH_BASE           ((uint32_t)0x40000000U) /*!< Peripheral base address */
#define APB1PERIPH_BASE        PERIPH_BASE
#define AHB1PERIPH_BASE       (PERIPH_BASE + 0x00020000U)
#define AHB2PERIPH_BASE       (PERIPH_BASE + 0x08000000U)
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x00010000U)
#define USART1_BASE           (APB2PERIPH_BASE + 0x3800U)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400U)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800U)
#define UART4_BASE            (APB1PERIPH_BASE + 0x4C00U)
#define UART5_BASE            (APB1PERIPH_BASE + 0x5000U)
#define LPUART1_BASE          (APB1PERIPH_BASE + 0x8000U)

#define USART1              ((USART_TypeDef *) USART1_BASE)
#define USART2              ((USART_TypeDef *) USART2_BASE)
#define USART3              ((USART_TypeDef *) USART3_BASE)
#define UART4               ((USART_TypeDef *) UART4_BASE)
#define UART5               ((USART_TypeDef *) UART5_BASE)
#define LPUART1             ((USART_TypeDef *) LPUART1_BASE)

#define USART_CR1_PS_Pos              (9U)
#define USART_CR1_PS_Msk              (0x1U << USART_CR1_PS_Pos)               /*!< 0x00000200 */
#define USART_CR1_PS                  USART_CR1_PS_Msk                         /*!< Parity Selection */
#define USART_CR1_PCE_Pos             (10U)
#define USART_CR1_PCE_Msk             (0x1U << USART_CR1_PCE_Pos)              /*!< 0x00000400 */
#define USART_CR1_PCE                 USART_CR1_PCE_Msk                        /*!< Parity Control Enable */
#define USART_CR1_RE_Pos              (2U)
#define USART_CR1_RE_Msk              (0x1U << USART_CR1_RE_Pos)               /*!< 0x00000004 */
#define USART_CR1_RE                  USART_CR1_RE_Msk                         /*!< Receiver Enable */
#define USART_CR1_TE_Pos              (3U)
#define USART_CR1_TE_Msk              (0x1U << USART_CR1_TE_Pos)               /*!< 0x00000008 */
#define USART_CR1_TE                  USART_CR1_TE_Msk                         /*!< Transmitter Enable */
#define USART_CR1_OVER8_Pos           (15U)
#define USART_CR1_OVER8_Msk           (0x1U << USART_CR1_OVER8_Pos)            /*!< 0x00008000 */
#define USART_CR1_OVER8               USART_CR1_OVER8_Msk                      /*!< Oversampling by 8-bit or 16-bit mode */

#define UART_PARITY_NONE                    0x00000000U                        /*!< No parity   */
#define UART_PARITY_EVEN                    USART_CR1_PCE                      /*!< Even parity */
#define UART_PARITY_ODD                     (USART_CR1_PCE | USART_CR1_PS)     /*!< Odd parity  */
#define DMA_CCR_DIR_Pos        (4U)
#define DMA_CCR_DIR_Msk        (0x1U << DMA_CCR_DIR_Pos)                       /*!< 0x00000010 */
#define DMA_CCR_DIR            DMA_CCR_DIR_Msk                                 /*!< Data transfer direction             */
#define DMA_CCR_MINC_Pos       (7U)
#define DMA_CCR_MINC_Msk       (0x1U << DMA_CCR_MINC_Pos)                      /*!< 0x00000080 */
#define DMA_CCR_MINC           DMA_CCR_MINC_Msk                                /*!< Memory increment mode               */
#define DMA_CCR_CIRC_Pos       (5U)
#define DMA_CCR_CIRC_Msk       (0x1U << DMA_CCR_CIRC_Pos)                      /*!< 0x00000020 */
#define DMA_CCR_CIRC           DMA_CCR_CIRC_Msk                                /*!< Circular mode                       */

#define GPIOA_BASE            (AHB2PERIPH_BASE + 0x0000U)
#define GPIOB_BASE            (AHB2PERIPH_BASE + 0x0400U)
#define GPIOC_BASE            (AHB2PERIPH_BASE + 0x0800U)
#define GPIOD_BASE            (AHB2PERIPH_BASE + 0x0C00U)
#define GPIOE_BASE            (AHB2PERIPH_BASE + 0x1000U)
#define GPIOF_BASE            (AHB2PERIPH_BASE + 0x1400U)
#define GPIOG_BASE            (AHB2PERIPH_BASE + 0x1800U)
#define GPIOH_BASE            (AHB2PERIPH_BASE + 0x1C00U)
//#define GPIOA               ((GPIO_TypeDef *) GPIOA_BASE)
//#define GPIOB               ((GPIO_TypeDef *) GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef *) GPIOD_BASE)
#define GPIOE               ((GPIO_TypeDef *) GPIOE_BASE)
#define GPIOF               ((GPIO_TypeDef *) GPIOF_BASE)
#define GPIOG               ((GPIO_TypeDef *) GPIOG_BASE)
#define GPIOH               ((GPIO_TypeDef *) GPIOH_BASE)

#define DMA1_BASE             (AHB1PERIPH_BASE)
#define DMA2_BASE             (AHB1PERIPH_BASE + 0x0400U)
//#define RCC_BASE              (AHB1PERIPH_BASE + 0x1000U)
#define FLASH_R_BASE          (AHB1PERIPH_BASE + 0x2000U)
#define CRC_BASE              (AHB1PERIPH_BASE + 0x3000U)
#define TSC_BASE              (AHB1PERIPH_BASE + 0x4000U)


#define DMA1_Channel1_BASE    (DMA1_BASE + 0x0008U)
#define DMA1_Channel2_BASE    (DMA1_BASE + 0x001CU)
#define DMA1_Channel3_BASE    (DMA1_BASE + 0x0030U)
#define DMA1_Channel4_BASE    (DMA1_BASE + 0x0044U)
#define DMA1_Channel5_BASE    (DMA1_BASE + 0x0058U)
#define DMA1_Channel6_BASE    (DMA1_BASE + 0x006CU)
#define DMA1_Channel7_BASE    (DMA1_BASE + 0x0080U)
#define DMA1_CSELR_BASE       (DMA1_BASE + 0x00A8U)


#define DMA2_Channel1_BASE    (DMA2_BASE + 0x0008U)
#define DMA2_Channel2_BASE    (DMA2_BASE + 0x001CU)
#define DMA2_Channel3_BASE    (DMA2_BASE + 0x0030U)
#define DMA2_Channel4_BASE    (DMA2_BASE + 0x0044U)
#define DMA2_Channel5_BASE    (DMA2_BASE + 0x0058U)
#define DMA2_Channel6_BASE    (DMA2_BASE + 0x006CU)
#define DMA2_Channel7_BASE    (DMA2_BASE + 0x0080U)
#define DMA2_CSELR_BASE       (DMA2_BASE + 0x00A8U)


#define DMA1_Channel1       ((DMA_Channel_TypeDef *) DMA1_Channel1_BASE)
#define DMA1_Channel2       ((DMA_Channel_TypeDef *) DMA1_Channel2_BASE)
#define DMA1_Channel3       ((DMA_Channel_TypeDef *) DMA1_Channel3_BASE)
#define DMA1_Channel4       ((DMA_Channel_TypeDef *) DMA1_Channel4_BASE)
#define DMA1_Channel5       ((DMA_Channel_TypeDef *) DMA1_Channel5_BASE)
#define DMA1_Channel6       ((DMA_Channel_TypeDef *) DMA1_Channel6_BASE)
#define DMA1_Channel7       ((DMA_Channel_TypeDef *) DMA1_Channel7_BASE)
#define DMA1_CSELR          ((DMA_Request_TypeDef *) DMA1_CSELR_BASE)


#define DMA2_Channel1       ((DMA_Channel_TypeDef *) DMA2_Channel1_BASE)
#define DMA2_Channel2       ((DMA_Channel_TypeDef *) DMA2_Channel2_BASE)
#define DMA2_Channel3       ((DMA_Channel_TypeDef *) DMA2_Channel3_BASE)
#define DMA2_Channel4       ((DMA_Channel_TypeDef *) DMA2_Channel4_BASE)
#define DMA2_Channel5       ((DMA_Channel_TypeDef *) DMA2_Channel5_BASE)
#define DMA2_Channel6       ((DMA_Channel_TypeDef *) DMA2_Channel6_BASE)
#define DMA2_Channel7       ((DMA_Channel_TypeDef *) DMA2_Channel7_BASE)
#define DMA2_CSELR          ((DMA_Request_TypeDef *) DMA2_CSELR_BASE)

#define DMA_REQUEST_2                     ((uint32_t)0x00000002)
#define DMA_REQUEST_4                     ((uint32_t)0x00000004)
#define DMA_MEMORY_TO_PERIPH         ((uint32_t)DMA_CCR_DIR)
#define DMA_PINC_DISABLE       ((uint32_t)0x00000000)    /*!< Peripheral increment mode Disable */
#define DMA_MINC_ENABLE         ((uint32_t)DMA_CCR_MINC)  /*!< Memory increment mode Enable  */
#define DMA_PDATAALIGN_BYTE          ((uint32_t)0x00000000)       /*!< Peripheral data alignment : Byte     */
#define DMA_MDATAALIGN_BYTE          ((uint32_t)0x00000000)       /*!< Memory data alignment : Byte     */
#define DMA_NORMAL         ((uint32_t)0x00000000)       /*!< Normal mode                  */
#define DMA_PRIORITY_LOW             ((uint32_t)0x00000000)    /*!< Priority level : Low       */
#define DMA_PERIPH_TO_MEMORY         ((uint32_t)0x00000000)
#define DMA_CIRCULAR       ((uint32_t)DMA_CCR_CIRC)

#define GPIO_AF7_USART1        ((uint8_t)0x07)  /* USART1 Alternate Function mapping     */
#define GPIO_AF7_USART2        ((uint8_t)0x07)  /* USART2 Alternate Function mapping     */
#define GPIO_AF7_USART3        ((uint8_t)0x07)  /* USART3 Alternate Function mapping     */
#define GPIO_AF8_UART4         ((uint8_t)0x08)  /* UART4 Alternate Function mapping   */
#define GPIO_AF8_LPUART1       ((uint8_t)0x08)  /* LPUART1 Alternate Function mapping */


#define UART_WORDLENGTH_8B           0x00000000U    /*!< 8-bit long UART frame */
#define UART_STOPBITS_1              0x00000000U
#define UART_HWCONTROL_NONE          0x00000000U                          /*!< No hardware control       */
#define UART_MODE_TX_RX              (USART_CR1_TE |USART_CR1_RE)
#define UART_OVERSAMPLING_8          USART_CR1_OVER8
#define UART_ONE_BIT_SAMPLE_DISABLE  0x00000000U
#define UART_ADVFEATURE_NO_INIT      0x00000000U

#define __HAL_LINKDMA(__HANDLE__, __PPP_DMA_FIELD__, __DMA_HANDLE__)             \
                        do{                                                      \
                            (__HANDLE__)->__PPP_DMA_FIELD__ = &(__DMA_HANDLE__); \
                            (__DMA_HANDLE__).Parent = (__HANDLE__);              \
                        } while(0)

#define __HAL_RCC_USART1_CLK_ENABLE()
#define __HAL_RCC_USART2_CLK_ENABLE()
#define __HAL_RCC_USART3_CLK_ENABLE()
#define __HAL_RCC_UART4_CLK_ENABLE()
#define __HAL_RCC_LPUART1_CLK_ENABLE()

#define __HAL_RCC_USART1_CLK_DISABLE()
#define __HAL_RCC_USART2_CLK_DISABLE()
#define __HAL_RCC_USART3_CLK_DISABLE()
#define __HAL_RCC_UART4_CLK_DISABLE()
#define __HAL_RCC_LPUART1_CLK_DISABLE()
#define __HAL_RCC_DMA1_IS_CLK_DISABLED() (false)
#define __HAL_RCC_DMA2_IS_CLK_DISABLED() (false)
#define __HAL_RCC_DMA1_CLK_ENABLE()
#define __HAL_RCC_DMA2_CLK_ENABLE()

extern uint8_t hal_dma_counter;
#define __HAL_DMA_GET_COUNTER(__HANDLE__) (hal_dma_counter)
#define __align(x)
#define __HAL_UART_SEND_REQ(__HANDLE__, __REQ__) 
#define __HAL_RCC_GPIOA_IS_CLK_DISABLED() 0
#define __HAL_RCC_GPIOA_CLK_ENABLE() 
#define __HAL_RCC_GPIOB_IS_CLK_DISABLED() 0
#define __HAL_RCC_GPIOB_CLK_ENABLE() 
#define __HAL_RCC_GPIOC_IS_CLK_DISABLED() 0
#define __HAL_RCC_GPIOC_CLK_ENABLE() 
#define __HAL_RCC_GPIOD_IS_CLK_DISABLED() 0
#define __HAL_RCC_GPIOD_CLK_ENABLE() 
#define __HAL_RCC_GPIOH_IS_CLK_DISABLED() 0
#define __HAL_RCC_GPIOH_CLK_ENABLE() 
#define HAL_IS_BIT_SET(REG, BIT)         (((REG) & (BIT)) == (BIT))
#define HAL_IS_BIT_CLR(REG, BIT)         (((REG) & (BIT)) == (BIT))

#endif /* ST_STM32L4XX_TEST_H */