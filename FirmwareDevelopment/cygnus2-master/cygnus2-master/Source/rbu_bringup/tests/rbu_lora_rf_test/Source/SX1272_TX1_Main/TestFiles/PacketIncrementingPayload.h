/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : PacketIncrementingPayload.h
*
*  Description  : Header for the test that sends a packet with an incrementing
*                 number in the payload, represented as an ascii string.
*
*************************************************************************************/

#ifndef TEST_PACKETINCREMENTINGPAYLOAD_H
#define TEST_PACKETINCREMENTINGPAYLOAD_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "TestBase.h"

/* Type Declarations
*************************************************************************************/
typedef enum
{
   PIP_EVENT_NONE_E,
   PIP_EVENT_TXDONE_E,
   PIP_EVENT_TXTIMEOUT_E,
   PIP_EVENT_CHANNEL_E
} PacketIncrementingPayloadEvent_t;


class PacketIncrementingPayload : public TestBase
{
   uint32_t m_Count;
   PacketIncrementingPayloadEvent_t m_Event;
   uint32_t m_Frequency;
   uint32_t m_TxPower;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   uint32_t m_FrequencyBand;
   bool m_LoraCrc;
   uint32_t m_MessageInterval;
   bool m_EnableOutput;
   
   uint32_t m_TxDoneCount;
   uint32_t m_TxTimeoutCount;
   uint32_t m_ChannelChangeCount;
   
   public:
      PacketIncrementingPayload( void );
      virtual ~PacketIncrementingPayload( void);
      
      virtual bool Initialise( void );
      virtual bool RunProcess( void );
      virtual void OnTxDone( void );
      virtual void OnTxTimeout( void );
      virtual void OnFhssChangeChannel( uint8_t channelIndex );
      virtual bool ProcessCommand(const char* pCommand);
      virtual void PrintHelp(Serial &serialPort);
};
#endif
