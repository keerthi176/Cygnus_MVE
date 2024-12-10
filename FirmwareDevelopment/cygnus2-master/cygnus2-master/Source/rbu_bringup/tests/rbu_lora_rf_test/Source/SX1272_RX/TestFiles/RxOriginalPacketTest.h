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
*  File         : RxOriginalPacketTest.h
*
*  Description  : Header for the Cygnus1 style packet test with main and interferer packets.
*
*************************************************************************************/

#ifndef TEST_ORIGINALPACKETTEST_H
#define TEST_ORIGINALPACKETTEST_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "RxTestBase.h"

/* Type Declarations
*************************************************************************************/

#define OPT_MESSAGE_LENGTH              16
#define OPT_MESSAGE_LENGTH_WITH_CRC     (OPT_MESSAGE_LENGTH + 2)

typedef enum
{
   OPT_EVENT_NONE_E,
   OPT_EVENT_START_E,
   OPT_EVENT_RXDONE_E,
   OPT_EVENT_RXTIMEOUT_E,
   OPT_EVENT_RXERROR_E,
   OPT_EVENT_CADDONE_E,
   OPT_EVENT_CHANNEL_E
} OriginalPacketTestEvent_t;


class RxOriginalPacketTest : public RxTestBase
{
   uint32_t m_Count;
   OriginalPacketTestEvent_t m_Event;
   char m_Buffer[OPT_MESSAGE_LENGTH_WITH_CRC];
   bool m_LastMessageErrorDetected;
   uint32_t m_Frequency;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   bool m_LoraCrc;

   uint32_t m_MessageBatchSize;
   uint32_t m_RxDoneCount;
   uint32_t m_RxTimeoutCount;
   uint32_t m_RxErrorCount;
   uint32_t m_CadDoneCount;
   uint32_t m_ChannelChangeCount;
   uint32_t m_PayloadNumber ;
   uint32_t m_LastReceivedPayload;
   uint32_t m_LostMessageCount;
   uint16_t m_CrcValue;
   
   uint32_t m_MainMsgCount;
   uint32_t m_InterfererMsgCount;
   uint32_t m_UnknownMsgCount;

   uint32_t m_RssiValue;
   uint32_t m_SnrValue;
   uint32_t m_Ferror;
   uint32_t m_FeiValue;
   uint32_t m_CadDetectedCount;
   uint32_t m_PllLockCount;

    time_t m_CurrentTimeSecs;
   
   void StartReceiving(void);
   void ResetRadio(void);
   
   public:
   RxOriginalPacketTest( void );
   virtual ~RxOriginalPacketTest( void);
   
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
