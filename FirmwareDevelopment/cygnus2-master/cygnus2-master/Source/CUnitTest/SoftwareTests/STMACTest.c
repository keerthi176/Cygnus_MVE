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
*  File         : STMACTest.c
*
*  Description  : Implementation of the MAC tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>



/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "STMACTest.h"
#include "MC_MAC.h"
#include "gpio.h"
#include "lptim.h"
#include "radio.h"
#include "sx1272.h"
#include "CO_Defines.h"
#include "MC_SessionManagement.h"


/* Private Functions Prototypes
*************************************************************************************/
static void MC_SetDlcchActionsTest(void);


/* External Functions Prototypes
*************************************************************************************/
extern void MC_SetDlcchActions(const uint16_t shortFrameIndex);

/* Global Variables
*************************************************************************************/
extern SMGR_ParentDetails_t gParent[NUMBER_OF_PARENTS_AND_TRACKING_NODES];

/* Table containing the test settings */
CU_TestInfo ST_MACTests[] =
{
   { "SetDlcchActionsTest",                    MC_SetDlcchActionsTest },

   CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/


/* SX1272 test stubs */
void SX1272Init(RadioEvents_t *events)
{
}

RadioState_t SX1272GetStatus(void)
{
   return RF_IDLE;
}

void SX1272SetChannel(uint32_t freq)
{
}

bool SX1272IsChannelFree(RadioModems_t modem, uint32_t freq, int16_t rssiThresh)
{
   return true;
}

uint32_t SX1272Random(void)
{
   return 0;
}

void SX1272SetRxConfig(RadioModems_t modem, uint32_t bandwidth,
   uint32_t datarate, uint8_t coderate,
   uint32_t bandwidthAfc, uint16_t preambleLen,
   uint16_t symbTimeout, bool fixLen,
   uint8_t payloadLen,
   bool crcOn, bool freqHopOn, uint8_t hopPeriod,
   bool iqInverted, bool rxContinuous)
{
}

void SX1272SetTxConfig(RadioModems_t modem, int8_t power, uint32_t fdev,
   uint32_t bandwidth, uint32_t datarate,
   uint8_t coderate, uint16_t preambleLen,
   bool fixLen, bool crcOn, bool freqHopOn,
   uint8_t hopPeriod, bool iqInverted, uint32_t timeout)
{
}

uint32_t SX1272GetTimeOnAir(RadioModems_t modem, uint8_t pktLen)
{
   return 0;
}

void SX1272Send(uint8_t *buffer, uint8_t size)
{
}

void SX1272SetSleep(void)
{
}

void SX1272SetStby(void)
{
}

void SX1272SetRx(void)
{
}

void SX1272SetTx(uint32_t timeout)
{
}

void SX1272StartCad(void)
{
}

int16_t SX1272ReadRssi(RadioModems_t modem)
{
   return 0;
}

void SX1272Reset(void)
{
}

void SX1272SetOpMode(uint8_t opMode)
{
}

void SX1272SetModem(RadioModems_t modem)
{
}

void SX1272Write(uint8_t addr, uint8_t data)
{
}

uint8_t SX1272Read(uint8_t addr)
{
   return 0;
}

void SX1272WriteBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
}

void SX1272ReadBuffer(uint8_t addr, uint8_t *buffer, uint8_t size)
{
}

void SX1272WriteFifo(uint8_t *buffer, uint8_t size)
{
}

void SX1272ReadFifo(uint8_t *buffer, uint8_t size)
{
}

void SX1272SetMaxPayloadLength(RadioModems_t modem, uint8_t max)
{
}

bool SX1272CheckRfFrequency(uint32_t frequency)
{
   return true;
}


const struct Radio_s Radio =
{
   SX1272Init,
   SX1272GetStatus,
   SX1272SetModem,
   SX1272SetChannel,
   SX1272IsChannelFree,
   SX1272Random,
   SX1272SetRxConfig,
   SX1272SetTxConfig,
   SX1272CheckRfFrequency,
   SX1272GetTimeOnAir,
   SX1272Send,
   SX1272SetSleep,
   SX1272SetStby,
   SX1272SetRx,
   SX1272StartCad,
   SX1272ReadRssi,
   SX1272Write,
   SX1272Read,
   SX1272WriteBuffer,
   SX1272ReadBuffer,
   SX1272SetMaxPayloadLength
};

/* timer test stubs */
LPTIM_HandleTypeDef hlptim1;
void HAL_LPTIM_Set_CMP(LPTIM_HandleTypeDef *hlptim, uint32_t value)
{
}

void HAL_LPTIM_Enable_CMPM_Interrupt(LPTIM_HandleTypeDef *hlptim)
{
}

void HAL_LPTIM_Disable_CMPM_Interrupt(LPTIM_HandleTypeDef *hlptim)
{
}

uint32_t LPTIM_ReadCounter(LPTIM_HandleTypeDef *hlptim)
{
   return 0;
}

uint32_t frequencyChannels[] = { 868000000, 865150000, 865450000, 865750000, 866050000,
866350000, 866650000, 866950000, 867250000, 867550000, 867850000 };

const uint32_t NUM_FREQ_CHANS = (sizeof(frequencyChannels) / sizeof(uint32_t));

/*************************************************************************************/
/**
* MC_SetDlcchActionsTest
* Test the DLCCH slot behaviour for all node IDs
*
* no params
*
* @return - void

*/
static void MC_SetDlcchActionsTest(void)
{
   uint16_t nodeIndex;
   uint16_t parentIndex;
   uint16_t tableSlot;
   uint16_t node_id = 0;
   uint16_t parent_id = 0;
   extern uint8_t dlcch_slot_table[32][3];

   /* test for master*/
   MC_MAC_Init(true,0,0,0);
   MC_SMGR_Initialise(true, 0);

   for ( uint16_t ShortFrameNumber = 0; ShortFrameNumber < SHORT_FRAMES_PER_LONG_FRAME; ShortFrameNumber++)
   {
      MC_SetDlcchActions(ShortFrameNumber);
      tableSlot = ShortFrameNumber % 3;
      for (int i = 0; i < 5; i++)
      {
         if (i == dlcch_slot_table[node_id][tableSlot])
         {
            CU_ASSERT_EQUAL(MC_MAC_GetSlotBehaviourFromSlotNumber(i), MC_MAC_BEHAVIOUR_DLCCH_TX_E);
         }
         else
         {
            CU_ASSERT_EQUAL(MC_MAC_GetSlotBehaviourFromSlotNumber(i), MC_MAC_BEHAVIOUR_NONE_E);
         }
      }
   }
   

   /* test for RBUs*/
   for ( node_id = 1; node_id < MAX_NUMBER_OF_SYSTEM_NODES; node_id++ )
   {
      MC_MAC_Init(false, 0, 0, node_id);
      MC_SMGR_Initialise(false, node_id);
      uint16_t parent_node_id = node_id - 1;
      gParent[PRIMARY_PARENT].NodeInfo = parent_node_id;
      gParent[PRIMARY_PARENT].IsActive = true;
      nodeIndex = node_id % 32;
      parentIndex = parent_node_id % 32;
      for (uint16_t ShortFrameNumber = 0; ShortFrameNumber < SHORT_FRAMES_PER_LONG_FRAME; ShortFrameNumber++)
      {
         MC_SetDlcchActions(ShortFrameNumber);
         tableSlot = ShortFrameNumber % 3;
        
         for (int i = 0; i < 5; i++)
         {
            if (i == dlcch_slot_table[parentIndex][tableSlot])
            {
               CU_ASSERT_EQUAL(MC_MAC_GetSlotBehaviourFromSlotNumber(i), MC_MAC_BEHAVIOUR_DLCCH_RX_E);
            }
            else if (i == dlcch_slot_table[nodeIndex][tableSlot])
            {
               CU_ASSERT_EQUAL(MC_MAC_GetSlotBehaviourFromSlotNumber(i), MC_MAC_BEHAVIOUR_DLCCH_TX_E);
            }
            else
            {
               CU_ASSERT_EQUAL(MC_MAC_GetSlotBehaviourFromSlotNumber(i), MC_MAC_BEHAVIOUR_NONE_E);
            }
         }
      }
   }

}


