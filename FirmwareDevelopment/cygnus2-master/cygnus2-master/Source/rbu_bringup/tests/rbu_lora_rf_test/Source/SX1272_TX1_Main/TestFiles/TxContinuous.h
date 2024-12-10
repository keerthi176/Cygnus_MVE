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
*  File         : TxContinuous.h
*
*  Description  : Header for continuous Carrier
*
*************************************************************************************/

#ifndef TEST_TXCONTINUOUS_H
#define TEST_TXCONTINUOUS_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "TestBase.h"

/* Type Declarations
*************************************************************************************/


class TxContinuous : public TestBase
{
   uint32_t m_Frequency;
   uint32_t m_FrequencyBand;
   uint32_t m_TxPower;
   bool m_CurrentBit;

   public:
      TxContinuous( void );
      virtual ~TxContinuous( void);
      
      virtual bool Initialise( void );
      virtual bool RunProcess( void );
      virtual void OnTxDone( void );
      virtual void OnTxTimeout( void );
      virtual void OnFhssChangeChannel( uint8_t channelIndex );
      virtual bool ProcessCommand(const char* pCommand);
      virtual void PrintHelp(Serial &serialPort);
};
#endif
