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
*  File         : PacketSynchronised.h
*
*  Description  : Header for the test that sends a packet with an incrementing
*                 number in the payload, represented as an ascii string with an
*                 accurately timed interval to enable the receiving side to
*                 synchronise its receiver.
*
*************************************************************************************/

#ifndef TEST_PACKETSYNCHRONISED_H
#define TEST_PACKETSYNCHRONISED_H

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
   PS_EVENT_NONE_E,
   PS_EVENT_TIMER_E,
   PS_EVENT_TXDONE_E,
   PS_EVENT_TXTIMEOUT_E,
   PS_EVENT_CHANNEL_E
} PacketSynchronisedEvent_t;


class PacketSynchronisedTest : public TestBase
{
   uint32_t m_Count;
   PacketSynchronisedEvent_t m_Event;
   
   uint32_t m_TxDoneCount;
   uint32_t m_TxTimeoutCount;
   uint32_t m_ChannelChangeCount;
   
   void TimerCallback(void);
   
   
   public:
      PacketSynchronisedTest( void );
      virtual ~PacketSynchronisedTest( void);
      
      virtual bool Initialise( void );
      virtual bool RunProcess( void );
      virtual void OnTxDone( void );
      virtual void OnTxTimeout( void );
      virtual void OnFhssChangeChannel( uint8_t channelIndex );
};
#endif
