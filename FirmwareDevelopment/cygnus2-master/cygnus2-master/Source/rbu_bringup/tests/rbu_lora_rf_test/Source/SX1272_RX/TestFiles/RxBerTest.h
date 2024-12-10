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
*  File         : RxBerTest.h
*
*  Description  : Header for the Bit Error Rate test.
*                 Used in conjunction with Tx test TxBerTestSignal.h
*
*************************************************************************************/

#ifndef TEST_BERTEST_H
#define TEST_BERTEST_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "RxTestBase.h"

/* Type Declarations
*************************************************************************************/

#define BER_MESSAGE_LENGTH              12
#define BER_MESSAGE_LENGTH_WITH_CRC     (BER_MESSAGE_LENGTH + 2)

typedef enum
{
   BER_EVENT_NONE_E,
   BER_EVENT_TIMER_E,
   BER_EVENT_RXDONE_E,
   BER_EVENT_RXTIMEOUT_E,
   BER_EVENT_RXERROR_E,
   BER_EVENT_CADDONE_E,
   BER_EVENT_CHANNEL_E
} RxBerEvent_t;


class RxBerTest : public RxTestBase
{
   uint32_t m_Count;
   RxBerEvent_t m_Event;
   uint32_t m_CurrentFrequency;
   uint32_t m_FrequencyBand;
   uint32_t m_CurrentPeriod;
   bool m_EnableOutput;
   double m_RunningAveBitError;

   uint32_t m_RssiValue;
   uint32_t m_SnrValue;
   uint32_t m_Ferror;
   uint32_t m_FeiValue;
   uint8_t m_Lna;
   

   void TimerCallback(void);
   
   void StartReceiving(void);
   
   public:
   RxBerTest( void );
   virtual ~RxBerTest( void);
   
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
