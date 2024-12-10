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
*  File         : RxContinuous.h
*
*  Description  : Header for the continuous receive test.  This test reports the '0' 
*                 or '1' value on the data output pin of the SX1273 chip in continuous
*                 FSK mode.
*                 Used in conjunction with Tx test TxContinuous.h
*
*************************************************************************************/

#ifndef TEST_RXCONTINUOUS_H
#define TEST_RXCONTINUOUS_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "RxTestBase.h"

/* Type Declarations
*************************************************************************************/

#define RXC_MESSAGE_LENGTH              12
#define RXC_MESSAGE_LENGTH_WITH_CRC     (RXC_MESSAGE_LENGTH + 2)

typedef enum
{
   RXC_EVENT_NONE_E,
   RXC_EVENT_TIMER_E,
   RXC_EVENT_RXDONE_E,
   RXC_EVENT_RXTIMEOUT_E,
   RXC_EVENT_RXERROR_E,
   RXC_EVENT_CADDONE_E,
   RXC_EVENT_CHANNEL_E
} RxContinuousEvent_t;


class RxContinuousTest : public RxTestBase
{
   RxContinuousEvent_t m_Event;
   uint32_t m_CurrentFrequency;
   uint32_t m_FrequencyBand;
   bool m_EnableOutput;
   bool m_CurrentData;

   uint32_t m_RssiValue;
   uint32_t m_SnrValue;
   uint32_t m_Ferror;
   uint32_t m_FeiValue;
   uint8_t m_Lna;
   
   void TimerCallback(void);
   
   void StartReceiving(void);
   
   public:
   RxContinuousTest( void );
   virtual ~RxContinuousTest( void);
   
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
