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

#ifndef ST_STM32L0XX_TEST_H
#define ST_STM32L0XX_TEST_H


/* System Include Files
*************************************************************************************/



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

/* Public Structures
*************************************************************************************/ 
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
   __IO uint32_t CCR;          /*!< DMA channel x configuration register */
   __IO uint32_t CNDTR;        /*!< DMA channel x number of data register */
   __IO uint32_t CPAR;         /*!< DMA channel x peripheral address register */
   __IO uint32_t CMAR;         /*!< DMA channel x memory address register */
} DMA_Channel_TypeDef;

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

/* Public Enumerations
*************************************************************************************/
/*!< Interrupt Number Definition */
typedef enum
{
   /******  Cortex-M0 Processor Exceptions Numbers ******************************************************/
   NonMaskableInt_IRQn = -14,    /*!< 2 Non Maskable Interrupt                                */
   HardFault_IRQn = -13,    /*!< 3 Cortex-M0+ Hard Fault Interrupt                       */
   SVC_IRQn = -5,     /*!< 11 Cortex-M0+ SV Call Interrupt                         */
   PendSV_IRQn = -2,     /*!< 14 Cortex-M0+ Pend SV Interrupt                         */
   SysTick_IRQn = -1,     /*!< 15 Cortex-M0+ System Tick Interrupt                     */

   /******  STM32L-0 specific Interrupt Numbers *********************************************************/
   WWDG_IRQn = 0,      /*!< Window WatchDog Interrupt                               */
   PVD_IRQn = 1,      /*!< PVD through EXTI Line detect Interrupt                  */
   RTC_IRQn = 2,      /*!< RTC through EXTI Line Interrupt                         */
   FLASH_IRQn = 3,      /*!< FLASH Interrupt                                         */
   RCC_CRS_IRQn = 4,      /*!< RCC and CRS Interrupts                                  */
   EXTI0_1_IRQn = 5,      /*!< EXTI Line 0 and 1 Interrupts                            */
   EXTI2_3_IRQn = 6,      /*!< EXTI Line 2 and 3 Interrupts                            */
   EXTI4_15_IRQn = 7,      /*!< EXTI Line 4 to 15 Interrupts                            */
   TSC_IRQn = 8,      /*!< TSC Interrupt                                           */
   DMA1_Channel1_IRQn = 9,      /*!< DMA1 Channel 1 Interrupt                                */
   DMA1_Channel2_3_IRQn = 10,     /*!< DMA1 Channel 2 and Channel 3 Interrupts                 */
   DMA1_Channel4_5_6_7_IRQn = 11,     /*!< DMA1 Channel 4, Channel 5, Channel 6 and Channel 7 Interrupts */
   ADC1_COMP_IRQn = 12,     /*!< ADC1, COMP1 and COMP2 Interrupts                        */
   LPTIM1_IRQn = 13,     /*!< LPTIM1 Interrupt                                        */
   USART4_5_IRQn = 14,     /*!< USART4 and USART5 Interrupt                             */
   TIM2_IRQn = 15,     /*!< TIM2 Interrupt                                          */
   TIM3_IRQn = 16,     /*!< TIM3 Interrupt                                          */
   TIM6_DAC_IRQn = 17,     /*!< TIM6 and DAC Interrupts                                 */
   TIM7_IRQn = 18,     /*!< TIM7 Interrupt                                          */
   TIM21_IRQn = 20,     /*!< TIM21 Interrupt                                         */
   I2C3_IRQn = 21,     /*!< I2C3 Interrupt                                          */
   TIM22_IRQn = 22,     /*!< TIM22 Interrupt                                         */
   I2C1_IRQn = 23,     /*!< I2C1 Interrupt                                          */
   I2C2_IRQn = 24,     /*!< I2C2 Interrupt                                          */
   SPI1_IRQn = 25,     /*!< SPI1 Interrupt                                          */
   SPI2_IRQn = 26,     /*!< SPI2 Interrupt                                          */
   USART1_IRQn = 27,     /*!< USART1 Interrupt                                        */
   USART2_IRQn = 28,     /*!< USART2 Interrupt                                        */
   RNG_LPUART1_IRQn = 29,     /*!< RNG and LPUART1 Interrupts                              */
   LCD_IRQn = 30,     /*!< LCD Interrupt                                           */
   USB_IRQn = 31,     /*!< USB global Interrupt                                    */
} IRQn_Type;


typedef enum
{
   RESET = 0,
   SET = !RESET
} FlagStatus, ITStatus;



/* Public Functions Prototypes
*************************************************************************************/






/* Macros
*************************************************************************************/
#define __RAM_FUNC HAL_StatusTypeDef



#endif /* ST_STM32L0XX_TEST_H */