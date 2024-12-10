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
*  File         : TxBerTestSignal.h
*
*  Description  : Header for the Bit Error Rate test.
*                 Used in conjunction with Rx test RxBerTest.h
*
*************************************************************************************/

#ifndef TEST_TXBERTESTSIGNAL_H
#define TEST_TXBERTESTSIGNAL_H

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
   BER_EVENT_NONE_E,
   BER_EVENT_TIMER_E,
} TxBerTestSignalEvent_t;


class TxBerTestSignal : public TestBase
{
   bool m_OutputValue;
   TxBerTestSignalEvent_t m_Event;
   uint32_t m_Frequency;
   uint32_t m_FrequencyBand;
   uint32_t m_TxPower;

   void TimerCallback(void);
   
   public:
      TxBerTestSignal( void );
      virtual ~TxBerTestSignal( void);
      
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
