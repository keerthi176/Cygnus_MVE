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
*  File         : PacketMainPayload.h
*
*  Description  : Header for the test that sends a fixed number of packets with the
*                 payload set to "ONE1"
*
*************************************************************************************/

#ifndef TEST_PACKETMAINPAYLOAD_H
#define TEST_PACKETMAINPAYLOAD_H

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
   PMP_EVENT_NONE_E,
   PMP_EVENT_TXDONE_E,
   PMP_EVENT_TXTIMEOUT_E,
   PMP_EVENT_CHANNEL_E
} PacketMAINPayloadEvent_t;


class PacketMainPayload : public TestBase
{
   uint32_t m_Count;
   uint32_t m_MaxMessages;
   PacketMAINPayloadEvent_t m_Event;
   
   uint32_t m_Frequency;
   uint32_t m_TxPower;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   bool m_LoraCrc;
   bool m_EnableOutput;

   uint32_t m_TxDoneCount;
   uint32_t m_TxTimeoutCount;
   uint32_t m_ChannelChangeCount;
   
   public:
      PacketMainPayload( void );
      virtual ~PacketMainPayload( void);
      
      virtual bool Initialise( void );
      virtual bool RunProcess( void );
      virtual void OnTxDone( void );
      virtual void OnTxTimeout( void );
      virtual void OnFhssChangeChannel( uint8_t channelIndex );
      virtual bool ProcessCommand(const char* pCommand);
      virtual void PrintHelp(Serial &serialPort);
};
#endif
