/**
  ******************************************************************************
  * @file    stm32f7xx_hal_conf.h
  * @brief   HAL configuration file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2019 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F7xx_HAL_CONF_H
#define __STM32F7xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
/**
  * @brief This is the list of modules to be used in the HAL driver 
  */

//#define USE_FULL_ASSERT 0
	 
typedef struct
{
  volatile unsigned int CR;               /*!< HASH control register,          Address offset: 0x00        */
  volatile unsigned int DIN;              /*!< HASH data input register,       Address offset: 0x04        */
  volatile unsigned int STR;              /*!< HASH start register,            Address offset: 0x08        */
  volatile unsigned int HR[5];             /*!< HASH digest registers,          Address offset: 0x0C-0x1C   */
  volatile unsigned int IMR;              /*!< HASH interrupt enable register, Address offset: 0x20        */
  volatile unsigned int SR;               /*!< HASH status register,           Address offset: 0x24        */
  unsigned int     RESERVED[52];     /*!< Reserved, 0x28-0xF4                                         */
  volatile unsigned int CSR[54];          /*!< HASH context swap registers,    Address offset: 0x0F8-0x1CC */
} HASH_TypeDef;

typedef struct
{
  volatile unsigned int HR[8];     /*!< HASH digest registers,          Address offset: 0x310-0x32C */
} HASH_DIGEST_TypeDef;

#define HASH                ((HASH_TypeDef *) HASH_BASE)
 
  	 
#define HAL_MODULE_ENABLED  
#define HASH_BASE             (AHB2PERIPH_BASE + 0x60400UL)
#define HASH_DIGEST_BASE      (AHB2PERIPH_BASE + 0x60710UL)	 

#define HASH_DIGEST         ((HASH_DIGEST_TypeDef *) HASH_DIGEST_BASE)
#define HASH_CR_INIT_Pos          (2U)
#define HASH_CR_INIT_Msk          (0x1UL << HASH_CR_INIT_Pos)                   /*!< 0x00000004 */
#define HASH_CR_INIT              HASH_CR_INIT_Msk
#define HASH_CR_DMAE_Pos          (3U)
#define HASH_CR_DMAE_Msk          (0x1UL << HASH_CR_DMAE_Pos)                   /*!< 0x00000008 */
#define HASH_CR_DMAE              HASH_CR_DMAE_Msk
#define HASH_CR_DATATYPE_Pos      (4U)
#define HASH_CR_DATATYPE_Msk      (0x3UL << HASH_CR_DATATYPE_Pos)               /*!< 0x00000030 */
#define HASH_CR_DATATYPE          HASH_CR_DATATYPE_Msk
#define HASH_CR_DATATYPE_0        (0x1UL << HASH_CR_DATATYPE_Pos)               /*!< 0x00000010 */
#define HASH_CR_DATATYPE_1        (0x2UL << HASH_CR_DATATYPE_Pos)               /*!< 0x00000020 */
#define HASH_CR_MODE_Pos          (6U)
#define HASH_CR_MODE_Msk          (0x1UL << HASH_CR_MODE_Pos)                   /*!< 0x00000040 */
#define HASH_CR_MODE              HASH_CR_MODE_Msk
#define HASH_CR_ALGO_Pos          (7U)
#define HASH_CR_ALGO_Msk          (0x801UL << HASH_CR_ALGO_Pos)                 /*!< 0x00040080 */
#define HASH_CR_ALGO              HASH_CR_ALGO_Msk
#define HASH_CR_ALGO_0            (0x001UL << HASH_CR_ALGO_Pos)                 /*!< 0x00000080 */
#define HASH_CR_ALGO_1            (0x800UL << HASH_CR_ALGO_Pos)                 /*!< 0x00040000 */
#define HASH_CR_NBW_Pos           (8U)
#define HASH_CR_NBW_Msk           (0xFUL << HASH_CR_NBW_Pos)                    /*!< 0x00000F00 */
#define HASH_CR_NBW               HASH_CR_NBW_Msk
#define HASH_CR_NBW_0             (0x1UL << HASH_CR_NBW_Pos)                    /*!< 0x00000100 */
#define HASH_CR_NBW_1             (0x2UL << HASH_CR_NBW_Pos)                    /*!< 0x00000200 */
#define HASH_CR_NBW_2             (0x4UL << HASH_CR_NBW_Pos)                    /*!< 0x00000400 */
#define HASH_CR_NBW_3             (0x8UL << HASH_CR_NBW_Pos)                    /*!< 0x00000800 */
#define HASH_CR_DINNE_Pos         (12U)
#define HASH_CR_DINNE_Msk         (0x1UL << HASH_CR_DINNE_Pos)                  /*!< 0x00001000 */
#define HASH_CR_DINNE             HASH_CR_DINNE_Msk
#define HASH_CR_MDMAT_Pos         (13U)
#define HASH_CR_MDMAT_Msk         (0x1UL << HASH_CR_MDMAT_Pos)                  /*!< 0x00002000 */
#define HASH_CR_MDMAT             HASH_CR_MDMAT_Msk
#define HASH_CR_LKEY_Pos          (16U)
#define HASH_CR_LKEY_Msk          (0x1UL << HASH_CR_LKEY_Pos)                   /*!< 0x00010000 */
#define HASH_CR_LKEY              HASH_CR_LKEY_Msk

/******************  Bits definition for HASH_STR register  *******************/
#define HASH_STR_NBLW_Pos         (0U)
#define HASH_STR_NBLW_Msk         (0x1FUL << HASH_STR_NBLW_Pos)                 /*!< 0x0000001F */
#define HASH_STR_NBLW             HASH_STR_NBLW_Msk
#define HASH_STR_NBLW_0           (0x01UL << HASH_STR_NBLW_Pos)                 /*!< 0x00000001 */
#define HASH_STR_NBLW_1           (0x02UL << HASH_STR_NBLW_Pos)                 /*!< 0x00000002 */
#define HASH_STR_NBLW_2           (0x04UL << HASH_STR_NBLW_Pos)                 /*!< 0x00000004 */
#define HASH_STR_NBLW_3           (0x08UL << HASH_STR_NBLW_Pos)                 /*!< 0x00000008 */
#define HASH_STR_NBLW_4           (0x10UL << HASH_STR_NBLW_Pos)                 /*!< 0x00000010 */
#define HASH_STR_DCAL_Pos         (8U)
#define HASH_STR_DCAL_Msk         (0x1UL << HASH_STR_DCAL_Pos)                  /*!< 0x00000100 */
#define HASH_STR_DCAL             HASH_STR_DCAL_Msk

/* legacy defines */
#define HASH_STR_NBW                         HASH_STR_NBLW
#define HASH_STR_NBW_0                       HASH_STR_NBLW_0
#define HASH_STR_NBW_1                       HASH_STR_NBLW_1
#define HASH_STR_NBW_2                       HASH_STR_NBLW_2
#define HASH_STR_NBW_3                       HASH_STR_NBLW_3
#define HASH_STR_NBW_4                       HASH_STR_NBLW_4

/******************  Bits definition for HASH_IMR register  *******************/
#define HASH_IMR_DINIE_Pos        (0U)
#define HASH_IMR_DINIE_Msk        (0x1UL << HASH_IMR_DINIE_Pos)                 /*!< 0x00000001 */
#define HASH_IMR_DINIE            HASH_IMR_DINIE_Msk
#define HASH_IMR_DCIE_Pos         (1U)
#define HASH_IMR_DCIE_Msk         (0x1UL << HASH_IMR_DCIE_Pos)                  /*!< 0x00000002 */
#define HASH_IMR_DCIE             HASH_IMR_DCIE_Msk

/* legacy defines */
#define HASH_IMR_DINIM                       HASH_IMR_DINIE
#define HASH_IMR_DCIM                        HASH_IMR_DCIE
/******************  Bits definition for HASH_SR register  ********************/
#define HASH_SR_DINIS_Pos         (0U)
#define HASH_SR_DINIS_Msk         (0x1UL << HASH_SR_DINIS_Pos)                  /*!< 0x00000001 */
#define HASH_SR_DINIS             HASH_SR_DINIS_Msk
#define HASH_SR_DCIS_Pos          (1U)
#define HASH_SR_DCIS_Msk          (0x1UL << HASH_SR_DCIS_Pos)                   /*!< 0x00000002 */
#define HASH_SR_DCIS              HASH_SR_DCIS_Msk
#define HASH_SR_DMAS_Pos          (2U)
#define HASH_SR_DMAS_Msk          (0x1UL << HASH_SR_DMAS_Pos)                   /*!< 0x00000004 */
#define HASH_SR_DMAS              HASH_SR_DMAS_Msk
#define HASH_SR_BUSY_Pos          (3U)
#define HASH_SR_BUSY_Msk          (0x1UL << HASH_SR_BUSY_Pos)                   /*!< 0x00000008 */
#define HASH_SR_BUSY              HASH_SR_BUSY_Msk	 

//#define HAL_ADC_MODULE_ENABLED 
// #define HAL_CRYP_MODULE_ENABLED 
//#define HAL_CAN_MODULE_ENABLED
///* #define HAL_CEC_MODULE_ENABLED   */
//#define HAL_CRC_MODULE_ENABLED
///* #define HAL_CRYP_MODULE_ENABLED   */
//#define HAL_DAC_MODULE_ENABLED
///* #define HAL_DCMI_MODULE_ENABLED   */
//#define HAL_DMA2D_MODULE_ENABLED
//#define HAL_ETH_MODULE_ENABLED
///* #define HAL_NAND_MODULE_ENABLED   */
///* #define HAL_NOR_MODULE_ENABLED   */
///* #define HAL_SRAM_MODULE_ENABLED   */
//#define HAL_SDRAM_MODULE_ENABLED
 #define HAL_HASH_MODULE_ENABLED  
///* #define HAL_I2S_MODULE_ENABLED   */
//#define HAL_IWDG_MODULE_ENABLED
//#define HAL_LPTIM_MODULE_ENABLED
//#define HAL_LTDC_MODULE_ENABLED
//#define HAL_QSPI_MODULE_ENABLED
///* #define HAL_RNG_MODULE_ENABLED   */
//#define HAL_RTC_MODULE_ENABLED
///* #define HAL_SAI_MODULE_ENABLED   */
//#define HAL_SD_MODULE_ENABLED
///* #define HAL_MMC_MODULE_ENABLED   */
///* #define HAL_SPDIFRX_MODULE_ENABLED   */
//#define HAL_SPI_MODULE_ENABLED
//#define HAL_TIM_MODULE_ENABLED
//#define HAL_UART_MODULE_ENABLED
///* #define HAL_USART_MODULE_ENABLED   */
///* #define HAL_IRDA_MODULE_ENABLED   */
///* #define HAL_SMARTCARD_MODULE_ENABLED   */
///* #define HAL_WWDG_MODULE_ENABLED   */
//#define HAL_PCD_MODULE_ENABLED
//#define HAL_HCD_MODULE_ENABLED
///* #define HAL_DFSDM_MODULE_ENABLED   */
///* #define HAL_DSI_MODULE_ENABLED   */
//#define HAL_JPEG_MODULE_ENABLED
///* #define HAL_MDIOS_MODULE_ENABLED   */
///* #define HAL_SMBUS_MODULE_ENABLED   */
///* #define HAL_MMC_MODULE_ENABLED   */
//#define HAL_GPIO_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
//#define HAL_PWR_MODULE_ENABLED
//#define HAL_I2C_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED

/* ########################## HSE/HSI Values adaptation ##################### */
/**
  * @brief Adjust the value of External High Speed oscillator (HSE) used in your application.
  *        This value is used by the RCC HAL module to compute the system frequency
  *        (when HSE is used as system clock source, directly or through the PLL).  
  */
#if !defined  (HSE_VALUE) 
  #define HSE_VALUE    ((uint32_t)25000000U) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT    ((uint32_t)100U)   /*!< Time out for HSE start up, in ms */
#endif /* HSE_STARTUP_TIMEOUT */

/**
  * @brief Internal High Speed oscillator (HSI) value.
  *        This value is used by the RCC HAL module to compute the system frequency
  *        (when HSI is used as system clock source, directly or through the PLL). 
  */
#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)16000000U) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

/**
  * @brief Internal Low Speed oscillator (LSI) value.
  */
#if !defined  (LSI_VALUE) 
 #define LSI_VALUE  ((uint32_t)32000U)       /*!< LSI Typical Value in Hz*/
#endif /* LSI_VALUE */                      /*!< Value of the Internal Low Speed oscillator in Hz
                                             The real value may vary depending on the variations
                                             in voltage and temperature.  */
/**
  * @brief External Low Speed oscillator (LSE) value.
  */
#if !defined  (LSE_VALUE)
 #define LSE_VALUE  ((uint32_t)32768U)    /*!< Value of the External Low Speed oscillator in Hz */
#endif /* LSE_VALUE */

#if !defined  (LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT    ((uint32_t)5000U)   /*!< Time out for LSE start up, in ms */
#endif /* LSE_STARTUP_TIMEOUT */

/**
  * @brief External clock source for I2S peripheral
  *        This value is used by the I2S HAL module to compute the I2S clock source 
  *        frequency, this source is inserted directly through I2S_CKIN pad. 
  */
#if !defined  (EXTERNAL_CLOCK_VALUE)
  #define EXTERNAL_CLOCK_VALUE    ((uint32_t)12288000U) /*!< Value of the Internal oscillator in Hz*/
#endif /* EXTERNAL_CLOCK_VALUE */

/* Tip: To avoid modifying this file each time you need to use different HSE,
   ===  you can define the HSE value in your toolchain compiler preprocessor. */

/* ########################### System Configuration ######################### */
/**
  * @brief This is the HAL system configuration section
  */     
#define  VDD_VALUE                    ((uint32_t)3300U) /*!< Value of VDD in mv */
#define  TICK_INT_PRIORITY            ((uint32_t)0U) /*!< tick interrupt priority */
#define  USE_RTOS                     0U
#define  PREFETCH_ENABLE              1U
#define  ART_ACCLERATOR_ENABLE        1U /* To enable instruction cache and prefetch */

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
  

/* ################## Ethernet peripheral configuration ##################### */

/* Section 1 : Ethernet peripheral configuration */

/* MAC ADDRESS: MAC_ADDR0:MAC_ADDR1:MAC_ADDR2:MAC_ADDR3:MAC_ADDR4:MAC_ADDR5 */
#define MAC_ADDR0   2U
#define MAC_ADDR1   0U
#define MAC_ADDR2   0U
#define MAC_ADDR3   0U
#define MAC_ADDR4   0U
#define MAC_ADDR5   0U

/* Definition of the Ethernet driver buffers size and count */   
#define ETH_RX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for receive               */
#define ETH_TX_BUF_SIZE                ETH_MAX_PACKET_SIZE /* buffer size for transmit              */
#define ETH_RXBUFNB                    ((uint32_t)4U)       /* 4 Rx buffers of size ETH_RX_BUF_SIZE  */
#define ETH_TXBUFNB                    ((uint32_t)4U)       /* 4 Tx buffers of size ETH_TX_BUF_SIZE  */

/* Section 2: PHY configuration section */

/* LAN8742A_PHY_ADDRESS Address*/ 
#define LAN8742A_PHY_ADDRESS           1
/* PHY Reset delay these values are based on a 1 ms Systick interrupt*/ 
#define PHY_RESET_DELAY                 ((uint32_t)0x000000FFU)
/* PHY Configuration delay */
#define PHY_CONFIG_DELAY                ((uint32_t)0x00000FFFU)

#define PHY_READ_TO                     ((uint32_t)0x0000FFFFU)
#define PHY_WRITE_TO                    ((uint32_t)0x0000FFFFU)

/* Section 3: Common PHY Registers */

#define PHY_BCR                         ((uint16_t)0x00U)    /*!< Transceiver Basic Control Register   */
#define PHY_BSR                         ((uint16_t)0x01U)    /*!< Transceiver Basic Status Register    */
 
#define PHY_RESET                       ((uint16_t)0x8000U)  /*!< PHY Reset */
#define PHY_LOOPBACK                    ((uint16_t)0x4000U)  /*!< Select loop-back mode */
#define PHY_FULLDUPLEX_100M             ((uint16_t)0x2100U)  /*!< Set the full-duplex mode at 100 Mb/s */
#define PHY_HALFDUPLEX_100M             ((uint16_t)0x2000U)  /*!< Set the half-duplex mode at 100 Mb/s */
#define PHY_FULLDUPLEX_10M              ((uint16_t)0x0100U)  /*!< Set the full-duplex mode at 10 Mb/s  */
#define PHY_HALFDUPLEX_10M              ((uint16_t)0x0000U)  /*!< Set the half-duplex mode at 10 Mb/s  */
#define PHY_AUTONEGOTIATION             ((uint16_t)0x1000U)  /*!< Enable auto-negotiation function     */
#define PHY_RESTART_AUTONEGOTIATION     ((uint16_t)0x0200U)  /*!< Restart auto-negotiation function    */
#define PHY_POWERDOWN                   ((uint16_t)0x0800U)  /*!< Select the power down mode           */
#define PHY_ISOLATE                     ((uint16_t)0x0400U)  /*!< Isolate PHY from MII                 */

#define PHY_AUTONEGO_COMPLETE           ((uint16_t)0x0020U)  /*!< Auto-Negotiation process completed   */
#define PHY_LINKED_STATUS               ((uint16_t)0x0004U)  /*!< Valid link established               */
#define PHY_JABBER_DETECTION            ((uint16_t)0x0002U)  /*!< Jabber condition detected            */
  
/* Section 4: Extended PHY Registers */
#define PHY_SR                          ((uint16_t)0x10U)    /*!< PHY status register Offset                      */

#define PHY_SPEED_STATUS                ((uint16_t)0x0002U)  /*!< PHY Speed mask                                  */
#define PHY_DUPLEX_STATUS               ((uint16_t)0x0004U)  /*!< PHY Duplex mask                                 */

#define PHY_ISFR                        ((uint16_t)0x000BU)    /*!< PHY Interrupt Source Flag register Offset   */
#define PHY_ISFR_INT4                   ((uint16_t)0x000BU)  /*!< PHY Link down inturrupt       */  

#define RCC_AHB2ENR_HASHEN_Pos             (5U)
#define RCC_AHB2ENR_HASHEN_Msk             (0x1UL << RCC_AHB2ENR_HASHEN_Pos)    /*!< 0x00000020 */
#define RCC_AHB2ENR_HASHEN                 RCC_AHB2ENR_HASHEN_Msk


#define __HAL_RCC_HASH_CLK_ENABLE()   do { \
                                        __IO uint32_t tmpreg; \
                                        SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_HASHEN);\
                                        /* Delay after an RCC peripheral clock enabling */ \
                                        tmpreg = READ_BIT(RCC->AHB2ENR, RCC_AHB2ENR_HASHEN);\
                                        UNUSED(tmpreg); \
                                      } while(0)



/* ################## SPI peripheral configuration ########################## */

/* CRC FEATURE: Use to activate CRC feature inside HAL SPI Driver
* Activated: CRC code is present inside driver
* Deactivated: CRC code cleaned from driver
*/

#define USE_SPI_CRC                     0U

/* Includes ------------------------------------------------------------------*/
/**
  * @brief Include module's header file 
  */

#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f7xx_hal_rcc.h"
#endif /* HAL_RCC_MODULE_ENABLED */

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f7xx_hal_gpio.h"
#endif /* HAL_GPIO_MODULE_ENABLED */

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f7xx_hal_dma.h"
#endif /* HAL_DMA_MODULE_ENABLED */
   
#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f7xx_hal_cortex.h"
#endif /* HAL_CORTEX_MODULE_ENABLED */

#ifdef HAL_ADC_MODULE_ENABLED
  #include "stm32f7xx_hal_adc.h"
#endif /* HAL_ADC_MODULE_ENABLED */

#ifdef HAL_CAN_MODULE_ENABLED
  #include "stm32f7xx_hal_can.h"
#endif /* HAL_CAN_MODULE_ENABLED */

#ifdef HAL_CEC_MODULE_ENABLED
  #include "stm32f7xx_hal_cec.h"
#endif /* HAL_CEC_MODULE_ENABLED */

#ifdef HAL_CRC_MODULE_ENABLED
  #include "stm32f7xx_hal_crc.h"
#endif /* HAL_CRC_MODULE_ENABLED */

#ifdef HAL_CRYP_MODULE_ENABLED
  #include "stm32f7xx_hal_cryp.h" 
#endif /* HAL_CRYP_MODULE_ENABLED */

#ifdef HAL_DMA2D_MODULE_ENABLED
  #include "stm32f7xx_hal_dma2d.h"
#endif /* HAL_DMA2D_MODULE_ENABLED */

#ifdef HAL_DAC_MODULE_ENABLED
  #include "stm32f7xx_hal_dac.h"
#endif /* HAL_DAC_MODULE_ENABLED */

#ifdef HAL_DCMI_MODULE_ENABLED
  #include "stm32f7xx_hal_dcmi.h"
#endif /* HAL_DCMI_MODULE_ENABLED */

#ifdef HAL_ETH_MODULE_ENABLED
  #include "stm32f7xx_hal_eth.h"
#endif /* HAL_ETH_MODULE_ENABLED */

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f7xx_hal_flash.h"
#endif /* HAL_FLASH_MODULE_ENABLED */
 
#ifdef HAL_SRAM_MODULE_ENABLED
  #include "stm32f7xx_hal_sram.h"
#endif /* HAL_SRAM_MODULE_ENABLED */

#ifdef HAL_NOR_MODULE_ENABLED
  #include "stm32f7xx_hal_nor.h"
#endif /* HAL_NOR_MODULE_ENABLED */

#ifdef HAL_NAND_MODULE_ENABLED
  #include "stm32f7xx_hal_nand.h"
#endif /* HAL_NAND_MODULE_ENABLED */

#ifdef HAL_SDRAM_MODULE_ENABLED
  #include "stm32f7xx_hal_sdram.h"
#endif /* HAL_SDRAM_MODULE_ENABLED */      

#ifdef HAL_HASH_MODULE_ENABLED
 #include "stm32f7xx_hal_hash.h"
#endif /* HAL_HASH_MODULE_ENABLED */

#ifdef HAL_I2C_MODULE_ENABLED
 #include "stm32f7xx_hal_i2c.h"
#endif /* HAL_I2C_MODULE_ENABLED */

#ifdef HAL_I2S_MODULE_ENABLED
 #include "stm32f7xx_hal_i2s.h"
#endif /* HAL_I2S_MODULE_ENABLED */

#ifdef HAL_IWDG_MODULE_ENABLED
 #include "stm32f7xx_hal_iwdg.h"
#endif /* HAL_IWDG_MODULE_ENABLED */

#ifdef HAL_LPTIM_MODULE_ENABLED
 #include "stm32f7xx_hal_lptim.h"
#endif /* HAL_LPTIM_MODULE_ENABLED */

#ifdef HAL_LTDC_MODULE_ENABLED
 #include "stm32f7xx_hal_ltdc.h"
#endif /* HAL_LTDC_MODULE_ENABLED */

#ifdef HAL_PWR_MODULE_ENABLED
 #include "stm32f7xx_hal_pwr.h"
#endif /* HAL_PWR_MODULE_ENABLED */

#ifdef HAL_QSPI_MODULE_ENABLED
 #include "stm32f7xx_hal_qspi.h"
#endif /* HAL_QSPI_MODULE_ENABLED */

#ifdef HAL_RNG_MODULE_ENABLED
 #include "stm32f7xx_hal_rng.h"
#endif /* HAL_RNG_MODULE_ENABLED */

#ifdef HAL_RTC_MODULE_ENABLED
 #include "stm32f7xx_hal_rtc.h"
#endif /* HAL_RTC_MODULE_ENABLED */

#ifdef HAL_SAI_MODULE_ENABLED
 #include "stm32f7xx_hal_sai.h"
#endif /* HAL_SAI_MODULE_ENABLED */

#ifdef HAL_SD_MODULE_ENABLED
 #include "stm32f7xx_hal_sd.h"
#endif /* HAL_SD_MODULE_ENABLED */

#ifdef HAL_MMC_MODULE_ENABLED
 #include "stm32f7xx_hal_mmc.h"
#endif /* HAL_MMC_MODULE_ENABLED */

#ifdef HAL_SPDIFRX_MODULE_ENABLED
 #include "stm32f7xx_hal_spdifrx.h"
#endif /* HAL_SPDIFRX_MODULE_ENABLED */

#ifdef HAL_SPI_MODULE_ENABLED
 #include "stm32f7xx_hal_spi.h"
#endif /* HAL_SPI_MODULE_ENABLED */

#ifdef HAL_TIM_MODULE_ENABLED
 #include "stm32f7xx_hal_tim.h"
#endif /* HAL_TIM_MODULE_ENABLED */

#ifdef HAL_UART_MODULE_ENABLED
 #include "stm32f7xx_hal_uart.h"
#endif /* HAL_UART_MODULE_ENABLED */

#ifdef HAL_USART_MODULE_ENABLED
 #include "stm32f7xx_hal_usart.h"
#endif /* HAL_USART_MODULE_ENABLED */

#ifdef HAL_IRDA_MODULE_ENABLED
 #include "stm32f7xx_hal_irda.h"
#endif /* HAL_IRDA_MODULE_ENABLED */

#ifdef HAL_SMARTCARD_MODULE_ENABLED
 #include "stm32f7xx_hal_smartcard.h"
#endif /* HAL_SMARTCARD_MODULE_ENABLED */

#ifdef HAL_WWDG_MODULE_ENABLED
 #include "stm32f7xx_hal_wwdg.h"
#endif /* HAL_WWDG_MODULE_ENABLED */

#ifdef HAL_PCD_MODULE_ENABLED
 #include "stm32f7xx_hal_pcd.h"
#endif /* HAL_PCD_MODULE_ENABLED */

#ifdef HAL_HCD_MODULE_ENABLED
 #include "stm32f7xx_hal_hcd.h"
#endif /* HAL_HCD_MODULE_ENABLED */

#ifdef HAL_DFSDM_MODULE_ENABLED
 #include "stm32f7xx_hal_dfsdm.h"
#endif /* HAL_DFSDM_MODULE_ENABLED */

#ifdef HAL_DSI_MODULE_ENABLED
 #include "stm32f7xx_hal_dsi.h"
#endif /* HAL_DSI_MODULE_ENABLED */

#ifdef HAL_JPEG_MODULE_ENABLED
 #include "stm32f7xx_hal_jpeg.h"
#endif /* HAL_JPEG_MODULE_ENABLED */

#ifdef HAL_MDIOS_MODULE_ENABLED
 #include "stm32f7xx_hal_mdios.h"
#endif /* HAL_MDIOS_MODULE_ENABLED */   

#ifdef HAL_SMBUS_MODULE_ENABLED
 #include "stm32f7xx_hal_smbus.h"
#endif /* HAL_SMBUS_MODULE_ENABLED */

/* Exported macro ------------------------------------------------------------*/
#ifdef  USE_FULL_ASSERT
/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function
  *         which reports the name of the source file and the source
  *         line number of the call that failed. 
  *         If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif /* USE_FULL_ASSERT */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_HAL_CONF_H */
 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

