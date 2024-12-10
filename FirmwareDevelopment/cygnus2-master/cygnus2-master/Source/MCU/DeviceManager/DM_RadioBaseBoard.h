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
*  File         : DM_RadioBaseBoard.h
*
*  Description  : Header file for Cygnus2 Radio Base Board
*
*************************************************************************************/

#ifndef DM_RADIO_BASE_BOARD_H
#define DM_RADIO_BASE_BOARD_H



/* System Include files
*******************************************************************************/


/* User Include files
*******************************************************************************/
#include "CO_Defines.h"

/* Public Structures
*******************************************************************************/


/* Public Enumerations
*******************************************************************************/ 


/* Public functions prototypes
*******************************************************************************/


/* Public Constants
*******************************************************************************/
#define HEAD_TMP_FLT             PA_0  // RBU/Consruction
#define BASE_TMP_FLT             PA_1
#define BAT_I1_MON               PA_2  //RBU + MCP + IOU
#define AN_BKP_CELL_1_MON        PA_2
#define BAT_I2_MON               PA_3  //RBU + MCP + IOU
#define AN_BKP_BAT_MON           PA_3
#define SX_NSS                   PA_4
#define SX_SCLK                  PA_5
#define SX_MISO                  PA_6
#define SX_MOSI                  PA_7
#define IOD_RLY_EN_IOD_R1R2      PA_8
#define CN_PPU_TX                PA_9
#define CN_PPU_RX                PA_10
#define SVI_PWR_ON               PA_11
#define IOD_SET                  PA_11
#define SNDR_BEACON_ON           PA_11
#define I2C_PULLUPS_ON           PA_12
#define CN_SWD_SWDIO             PA_13
#define CN_SWD_SWCLK             PA_14
#define CN_STATUS_LED_BLUE       PA_15

#define ANT_RX_NTX               PB_0
#define CN_HEAD_PWR_ON           PB_1
#define EN_DET_1                 PB_1
#define ANT_TX_NRX               PB_2
#define SX_DIO3                  PB_3
#define SX_DIO2                  PB_4
#define SX_DIO1                  PB_5
#define PRI_BAT_IRQ              PB_6
#define SX_DIO0                  PB_7
#define SX_RESET                 PB_8
#define PRI_BAT_EN               PB_9  //RBU + MCP + IOU + CONST
#define I2C2_SCL                 PB_10
#define I2C2_SDA                 PB_11
#define BAT_MON_EN               PB_12
#define TAMPER_EN                PB_13
#define DET_R1                   PB_14
#define CONST_PRI_BAT_MON_EN     PB_14
#define BKP_BAT_MON_EN           PB_15

#define CN_HEAD_RX               PC_0  //RBU + MCP + Consruction
#define CN_FAULT_DET_1_IN        PC_0  //IOU
#define CN_HEAD_TX               PC_1  //RBU + MCP + Construction
#define CN_FAULT_DET_2_IN        PC_1  //IOU
#define BAT_I3_MON               PC_2  //RBU + MCP + IOU
#define AN_VBAT_MON              PC_2  //Construction
#define BAT_VMON                 PC_3  //RBU + MCP + IOU
#define IOD_RESET                PC_4
#define EN_DET_2                 PC_5
#define CN_STATUS_LED_GREEN      PC_6
#define CN_STATUS_LED_RED        PC_7
#define CONSTR_PIR_IN            PC_8  //Construction
#define SPARE_PC9                PC_9  //Spare
#define DET_R2                   PC_9
#define CN_DEBUG_TX              PC_10
#define CN_DEBUG_RX              PC_11
#define CN_FA_MCP_IN             PC_12
#define BKP_BAT_EN               PC_13 //RBU + MCP + IOU
#define BKP_BAT_FIT              PC_13 //Construction
#define OSC32_IN                 PC_14
#define OSC32_OUT                PC_15

#define CN_FIRE_MCP_IN           PD_2


/* UART definitions */
#define CN_DEBUG_UART                           UART4
#define CN_PPU_UART                             USART1
#define CN_HEAD_UART                            LPUART1


/* Macros
*******************************************************************************/



#endif // DM_RADIO_BASE_BOARD_H
