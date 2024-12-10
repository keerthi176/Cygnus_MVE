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
*  File         : PacketLossTest.h
*
*  Description  : Header for the Packet loss test.
*
*************************************************************************************/

#ifndef TEST_PACKETLOSSTEST_H
#define TEST_PACKETLOSSTEST_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "RxTestBase.h"

/* Type Declarations
*************************************************************************************/

#define PL_MESSAGE_LENGTH              5
#define PL_MESSAGE_LENGTH_WITH_CRC     (PL_MESSAGE_LENGTH + 2)

typedef enum
{
   PL_EVENT_NONE_E,
   PL_EVENT_RXDONE_E,
   PL_EVENT_RXTIMEOUT_E,
   PL_EVENT_RXERROR_E,
   PL_EVENT_RXCRCERROR_E,
   PL_EVENT_CADDONE_E,
   PL_EVENT_CHANNEL_E
} PacketLossEvent_t;


class PacketLossTest : public RxTestBase
{
   uint32_t m_Count;
   PacketLossEvent_t m_Event;
   uint32_t m_EventTime;
   uint32_t m_PreviousEventTime;
   char m_Buffer[PL_MESSAGE_LENGTH_WITH_CRC];
   bool m_LastMessageErrorDetected;
   uint32_t m_Frequency;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   uint32_t m_FrequencyBand;
   bool m_LoraCrc;
   uint32_t m_MessageInterval;//time between received messages in timer ticks
   bool m_EnableOutput;
   uint8_t m_Lna;

   uint32_t m_RxDoneCount;
   uint32_t m_RxTimeoutCount;
   uint32_t m_RxErrorCount;
   uint32_t m_CadDoneCount;
   uint32_t m_ChannelChangeCount;
   int32_t m_PayloadNumber ;
   int32_t m_LastReceivedPayload;
   uint32_t m_LostMessageCount;
   uint16_t m_CrcValue;

   uint32_t m_RssiValue;
   uint32_t m_SnrValue;
   uint32_t m_Ferror;
   uint32_t m_FeiValue;

   
   
   void StartReceiving(void);
   void ResetRadio(void);
   
   public:
   PacketLossTest( void );
   virtual ~PacketLossTest( void);
   
   virtual bool Initialise( void );
   virtual bool RunProcess( void );

   virtual void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror);
   virtual void OnRxTimeout( void );
   virtual void OnRxError( void );
   virtual void OnCadDone( bool channelActivityDetected );
   virtual void OnFhssChangeChannel( uint8_t channelIndex );

   virtual bool ProcessCommand(const char* pCommand);
   virtual void PrintHelp(Serial &serialPort);
};
#endif
