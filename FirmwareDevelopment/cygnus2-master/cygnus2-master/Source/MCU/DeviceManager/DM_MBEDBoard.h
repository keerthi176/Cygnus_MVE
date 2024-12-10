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
*  File         : DM_MBEDBoard.h
*
*  Description  : Header file for MBED Board
*
*************************************************************************************/

#ifndef DM_MBED_BOARD_H
#define DM_MBED_BOARD_H



/* System Include files
*******************************************************************************/


/* User Include files
*******************************************************************************/


/* Public Structures
*******************************************************************************/


/* Public Enumerations
*******************************************************************************/ 


/* Public functions prototypes
*******************************************************************************/


/* Public Constants
*******************************************************************************/

/* Pin definitions */
#define CN_TAMPER1                              PC_13
#define OSC32_IN                                PC_14
#define OSC32_OUT                               PC_15
#define PH0_OSC_IN                              PH_0 // reserved
#define PH1_OSC_OUT                             PH_1 // reserved
#define CN_HEAD_RX                              PC_0
#define CN_HEAD_TX                              PC_1
#define CN_PC2                                  PC_2
#define CN_PC3                                  PC_3
#define CN_TAMPER2                              PA_4
// reserved                                     PA_1
#define CN_NCU_CP_TX                            PA_2
#define CN_NCU_CP_RX                            PA_3
#define SX_NSS                                  PB_6
#define SX_SCLK                                 PA_5
#define SX_MISO                                 PA_6
#define SX_MOSI                                 PA_7
#define CN_PC4                                  PC_4
#define CN_PC5                                  PC_5
#define ANT_RX_NTX                              PB_0
#define CN_HEAD_PWR_ON                          PB_1
#define ANT_TX_NRX                              PB_2
#define CN_STATUS_LED_GREEN                     PB_10
#define CN_STATUS_LED_RED                       PB_11
#define MAIN_BAT_IRQ                            PB_12
#define CN_PB13                                 PB_13
#define CN_PB14                                 PB_14
#define CN_PB15                                 PB_15// reserved
#define CN_SVI_PWR_ON                           PC_6
#define ANT2_SEL                                PC_7
#define ANT1_SEL                                PC_8
#define CN_FA_MCP_IN                            PC_9
#define CN_FIRE_MCP_IN                          PA_8
#define CN_PPU_TX                               PA_9
#define CN_PPU_RX                               PB_8
#define CN_PPU_CTS                              PA_11
#define CN_PPU_RTS                              PA_12
#define CN_SWD_SWDIO                            PA_13
#define CN_SWD_SWCLK                            PA_14
#define CN_STATUS_LED_BLUE                      PA_15
#define CN_DEBUG_TX                             PC_10
#define CN_DEBUG_RX                             PC_11
#define SX_DIO5                                 PC_12
#define SX_DIO4                                 PD_2
#define SX_DIO3                                 PB_4
#define SX_DIO2                                 PB_5
#define SX_DIO1                                 PB_3
#define CN_PB6                                  PB_7  // PB_6 is used for SX_NSS on this board
#define SX_DIO0                                 PA_10
#define SX_RESET                                PA_0
#define CN_PB9                                  PB_9

/* UART definitions */
#define CN_DEBUG_UART                           USART2
#define CN_PPU_UART                             USART1
#define CN_HEAD_UART                            LPUART1
#define CN_NCU_CP_UART                          USART4
#define CN_NCU_SBC_UART                         LPUART1


/* Macros
*******************************************************************************/



#endif // DM_MBED_BOARD_H
