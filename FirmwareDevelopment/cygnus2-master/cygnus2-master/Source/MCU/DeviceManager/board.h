/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __BOARD_H__
#define __BOARD_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "stm32l4xx.h"
#include "utilities.h"
#include "delay.h"
#include "gpio.h"
#include "spi.h"
#include "uart.h"
#include "radio.h"
#include "sx1272/sx1272.h"
#include "sx1272-board.h"
#include "uart-board.h"
#include "CO_Defines.h"


#if defined( USE_USB_CDC )
#include "uart-usb-board.h"
#endif

#include "DM_RadioBaseBoard.h"

/*!
 * Define indicating if an external IO expander is to be used
 */
#define BOARD_IOE_EXT


/*!
 * Board MCU pins definitions
 */
#define RADIO_MOSI                                  SX_MOSI
#define RADIO_MISO                                  SX_MISO
#define RADIO_SCLK                                  SX_SCLK

#define RADIO_RESET                                 SX_RESET
#define RADIO_NSS                                   SX_NSS
#define RADIO_DIO_0                                 SX_DIO0
#define RADIO_DIO_1                                 SX_DIO1
#define RADIO_DIO_2                                 SX_DIO2
#define RADIO_DIO_3                                 SX_DIO3
#define RADIO_DIO_4                                 SX_DIO4
#define RADIO_DIO_5                                 SX_DIO5

#define RADIO_ANT_SWITCH_RX                         ANT_RX_NTX
#define RADIO_ANT_SWITCH_TX                         ANT_TX_NRX

typedef enum
{
   BASE_NCU_E,
   BASE_RBU_E,
   BASE_MCP_E,
   BASE_IOU_E,
   BASE_CONSTRUCTION_E,
   BASE_REPEATER_E,
   BASE_UNKNOWN_E
} DM_BaseType_t;

typedef enum
{
   PA2_NOT_USED_E,
   PA2_BAT_I1_MON_E,
   PA2_AN_BKP_CELL_1_MON_E
} pa2Function_t;

typedef enum
{
   PA3_NOT_USED_E,
   PA3_BAT_I2_MON_E,
   PA3_AN_BKP_BAT_MON_E
} pa3Function_t;

typedef enum
{
   PA11_NOT_USED_E,
   PA11_SVI_PWR_ON_E,
   PA11_IOD_SET_E,
   PA11_SNDR_BEACON_ON_E
} pa11Function_t;

typedef enum
{
   PB1_NOT_USED_E,
   PB1_HEAD_PWR_ON_E,
   PB1_EN_DET_1_E,
} pb1Function_t;

typedef enum 
{
   PB14_NOT_USED_E,
   PB14_DET_R1_E,
   PB14_VBAT_MON_EN_E
} pb14Function_t;

typedef enum
{
   PC0_NOT_USED_E,
   PC0_HEAD_RX_E,
   PC0_FAULT_DET_1_E,
} pc0Function_t;

typedef enum 
{
   PC1_NOT_USED_E,
   PC1_HEAD_TX_E,
   PC1_FAULT_DET_2_E
} pc1Function_t;

typedef enum 
{
   PC2_NOT_USED_E,
   PC2_BAT_I3_MON_E,
   PC2_AN_VBAT_MON_E
} pc2Function_t;

typedef enum 
{
   PC13_NOT_USED_E,
   PC13_BKP_BATTERY_EN_E,
   PC13_BKP_BAT_FIT_E
} pc13Function_t;

/* configuration options for variant 1 radio board */
typedef struct
{
   pa2Function_t pa2;
   pa3Function_t pa3;
   pa11Function_t pa11;
   pb1Function_t pb1;
   pb14Function_t pb14;
   pc0Function_t pc0;
   pc1Function_t pc1;
   pc2Function_t pc2;
   pc13Function_t pc13;
} boardConfig_t;


/*!
 * MCU objects
 */
extern Uart_t Uart1;
#if defined( USE_USB_CDC )
extern Uart_t UartUsb;
#endif
extern Gpio_t StatusLedRed;
extern Gpio_t StatusLedGreen;
extern Gpio_t StatusLedBlue;

extern Gpio_t FireMCP;
extern Gpio_t FirstAidMCP;
extern Gpio_t Pir;
extern Gpio_t InstallationTamper;
extern Gpio_t PhOscIn;
extern Gpio_t PhOscOut;
extern Gpio_t HeadRx;
extern Gpio_t HeadTx;
extern Gpio_t HeadPowerOn;
extern Gpio_t PC2;
extern Gpio_t PC3;
extern Gpio_t DismantleTamper;
extern Gpio_t AdcIn1;
extern Gpio_t NcuCpTx;
extern Gpio_t NcuCpRx;
extern Gpio_t PC4;
extern Gpio_t PC5;
extern Gpio_t AntRxNtx;
extern Gpio_t AntTxNrx;
extern Gpio_t HeadPwrOn;
extern Gpio_t PrimaryBatIrq;
extern Gpio_t PB13;
extern Gpio_t PB14;
extern Gpio_t PB15;
extern Gpio_t SVIPowerOn;
extern Gpio_t I2CPullupsOn;
extern Gpio_t PpuTx;
extern Gpio_t PpuRx;
extern Gpio_t DebugTx;
extern Gpio_t DebugRx;
extern Gpio_t SndBaseVisInd;
extern Gpio_t SxReset;
extern Gpio_t SndVisIndData;

enum BoardPowerSource
{
    USB_POWER_E,
    BATTERY_POWER_E
};

/*!
 * \brief Initializes the MCU pin configuration
 */
void BoardConfigureGpio( const DM_BaseType_t baseType, const bool noHeadFitted );
#endif // __BOARD_H__
