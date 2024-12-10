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
*  File         : Standby.h
*
*  Description  : Header for no output Tx
*
*************************************************************************************/

#ifndef TEST_STANDBY_H
#define TEST_STANDBY_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "TestBase.h"

/* Type Declarations
*************************************************************************************/


class Standby : public TestBase
{
   uint32_t m_Frequency;
   uint32_t m_TxPower;

   public:
      Standby( void );
      virtual ~Standby( void);
      
      virtual bool Initialise( void );
      virtual bool RunProcess( void );
      virtual void OnTxDone( void );
      virtual void OnTxTimeout( void );
      virtual void OnFhssChangeChannel( uint8_t channelIndex );
      virtual bool ProcessCommand(const char* pCommand);
      virtual void PrintHelp(Serial &serialPort);


   virtual void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror){};//Not used
   virtual void OnRxTimeout( void ){};//Not used
   virtual void OnRxError( void ){};//Not used
   virtual void OnCadDone( bool channelActivityDetected ){};//Not used
};
#endif
