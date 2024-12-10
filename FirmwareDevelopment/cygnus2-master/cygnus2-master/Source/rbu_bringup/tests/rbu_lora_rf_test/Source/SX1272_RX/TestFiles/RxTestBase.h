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
*  File         : RxTestBase.h
*
*  Description  : Base class for the receive test modules used in the radio Board tests.
*
*************************************************************************************/

#ifndef RX_TEST_BASE_H
#define RX_TEST_BASE_H

//#define FREQUENCY_BAND_915MHZ

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


class RxTestBase
{
   public:
      RxTestBase( void ){};
      virtual ~RxTestBase( void){};
      
      virtual bool Initialise( void ){ return false; }
      virtual bool RunProcess( void ) {return false;}

      virtual void OnTxDone( void ){}
      virtual void OnTxTimeout( void ){}
      virtual void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror){}
      virtual void OnRxTimeout( void ){}
      virtual void OnRxError( void ){}
      virtual void OnCadDone( bool channelActivityDetected ){}
      virtual void OnFhssChangeChannel( uint8_t channelIndex ){}
      virtual bool ProcessCommand(const char* pCommand){return false;}
      virtual void PrintHelp(Serial &serialPort){};
      /*************************************************************************************/
      /**
      * function name   : IsHexChar
      * description     : Returns true if the supplied char is a hex character
      *
      * @param - pChar - The character to be tested.
      *
      * @return - TRUE if the char is a Hex value
      */
      virtual bool IsHexChar(const char* pChar)
      {
         bool result = false;
         
         if ( (*pChar >= '0') && (*pChar < 'G'))
         {
            if ( (*pChar <= '9') || (*pChar >= 'A') )
            {
               result = true;
            }
         }
         
         return result;
      }
      /*************************************************************************************/
      /**
      * function name   : CharToHexValue
      * description     : Returns a hex value from the supplied character
      *
      * @param - pChar - The character to be converted.
      *
      * @return - TRUE if the char is a Hex value
      */
      virtual uint8_t CharToHexValue(const char* pChar)
      {
         uint8_t value = false;
         
         if ( IsHexChar(pChar) )
         {
            if ( (*pChar >= '0') && (*pChar <= '9') )
            {
               value = *pChar - '0';
            }
            else 
            {
               value = *pChar - 'A' + 10;
            }
         }
         
         return value;
      }

      /*************************************************************************************/
      /**
      * function name   : HexTextToUint8
      * description     : Read Hex chars into a uint8_t
      *
      * @param - pText - The first byte of the Hex chars to be converted.
      *
      * @return - the uint8_t value of the supplied Hex chars
      */
      virtual uint8_t HexTextToUint8(const char* pText)
      {
         uint8_t value = 0;
         uint32_t count = 0;
         char* pPtr = (char*)pText;
         
         while ( IsHexChar(pPtr) && (count < 2))
         {
            value <<= 4;
            value += CharToHexValue(pPtr);
            pPtr++;
            count++;
         }
         
         return value;
      }

};
#endif
