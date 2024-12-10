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
*  File         : TestBase.h
*
*  Description  : Base class for the test modules used in the radio Board tests.
*
*************************************************************************************/

#ifndef TEST_BASE_H
#define TEST_BASE_H

#define FREQUENCY_BAND_915MHZ

/* System Include Files
*************************************************************************************/
#include <stdint.h>

/* User Include Files
*************************************************************************************/
#include "mbed.h"
#include "mbed_debug.h"
#include "mbed_wait_api.h"
#include "sx1272-hal.h"



/* Type Declarations
*************************************************************************************/
extern SX1272MB2xAS Radio;
extern DigitalOut ant_tx_nrx;
extern DigitalOut ant_rx_ntx;


class TestBase
{
   public:
      TestBase( void ){};
      virtual ~TestBase( void){};
      
      virtual bool Initialise( void ){ return false; }
      virtual bool RunProcess( void ) {return false;}
      virtual void OnTxDone( void ){}
      virtual void OnTxTimeout( void ){}
      virtual void OnFhssChangeChannel( uint8_t channelIndex ){}
      virtual void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror){}
      virtual void OnRxTimeout( void ){}
      virtual void OnRxError( void ){}
      virtual void OnCadDone( bool channelActivityDetected ){}
      virtual bool ProcessCommand(const char* pCommand){return false;}
      virtual void PrintHelp(Serial &serialPort){}
};
#endif
