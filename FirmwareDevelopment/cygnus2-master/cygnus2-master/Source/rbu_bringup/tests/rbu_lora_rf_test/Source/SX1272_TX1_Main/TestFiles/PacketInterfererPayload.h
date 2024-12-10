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
*  File         : PacketInterfererPayload.h
*
*  Description  : Header for the test that sends a fixed number of packets with the
*                 payload set to "TWO2"
*
*************************************************************************************/

#ifndef TEST_PACKETINTERFERERPAYLOAD_H
#define TEST_PACKETINTERFERERPAYLOAD_H

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
   PINTP_EVENT_NONE_E,
   PINTP_EVENT_TXDONE_E,
   PINTP_EVENT_TXTIMEOUT_E,
   PINTP_EVENT_CHANNEL_E
} PacketInterfererPayloadEvent_t;


class PacketInterfererPayload : public TestBase
{
   uint32_t m_Count;
   uint32_t m_MaxMessages;
   PacketInterfererPayloadEvent_t m_Event;
   
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
      PacketInterfererPayload( void );
      virtual ~PacketInterfererPayload( void);
      
      virtual bool Initialise( void );
      virtual bool RunProcess( void );
      virtual void OnTxDone( void );
      virtual void OnTxTimeout( void );
      virtual void OnFhssChangeChannel( uint8_t channelIndex );
      virtual bool ProcessCommand(const char* pCommand);
      virtual void PrintHelp(Serial &serialPort);
};
#endif
