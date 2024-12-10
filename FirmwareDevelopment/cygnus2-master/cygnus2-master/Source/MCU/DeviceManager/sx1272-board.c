/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: SX1272 driver specific target board functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "radio.h"
#include "sx1272/sx1272.h"
#include "sx1272-board.h"

extern LPTIM_HandleTypeDef hlptim1;

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    SX1272Init,                  // Radio.Init()
    SX1272GetStatus,             // Radio.GetStatus()
    SX1272SetModem,              // Radio.SetModem()
    SX1272SetChannel,            // Radio.SetChannel()
    SX1272IsChannelFree,         // Radio.IsChannelFree()
    SX1272Random,                // Radio.Random()
    SX1272SetRxConfig,           // Radio.SetRxConfig()
    SX1272SetTxConfig,           // Radio.SetTxConfig()
    SX1272CheckRfFrequency,      // Radio.CheckRfFrequency()
    SX1272GetTimeOnAir,          // Radio.TimeOnAir()
    SX1272Send,                  // Radio.Send()
    SX1272SetSleep,              // Radio.Sleep()
    SX1272SetStby,               // Radio.Standby()
    SX1272SetRx,                 // Radio.Rx()
    SX1272StartCad,              // Radio.StartCad()
    SX1272ReadRssi,              // Radio.Rssi()
    SX1272Write,                 // Radio.Write()
    SX1272Read,                  // Radio.Read()
    SX1272WriteBuffer,           // Radio.WriteBuffer()
    SX1272ReadBuffer,            // Radio.ReadBuffer()
    SX1272SetMaxPayloadLength    // Radio.SetMaxPayloadLength()
};

/*!
 * Antenna switch GPIO pins objects
 */
Gpio_t AntRx;
Gpio_t AntTx;

uint32_t RxTxLogic = 0;
uint32_t TxRxLogic = 1;

void SX1272IoInit( void )
{
	GpioInit( &SX1272.Spi.Nss, RADIO_NSS, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );
	GpioInit( &SX1272.DIO0, RADIO_DIO_0, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );
	GpioInit( &SX1272.DIO1, RADIO_DIO_1, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );
	//GpioInit( &SX1272.DIO2, RADIO_DIO_2, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );
	GpioInit( &SX1272.DIO3, RADIO_DIO_3, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );
	//GpioInit( &SX1272.DIO4, RADIO_DIO_4, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );			// RM: not connected on mbed-SX1272 board 
	//GpioInit( &SX1272.DIO5, RADIO_DIO_5, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );
   
}

void SX1272IoIrqInit( DioIrqHandler **irqHandlers )
{
	GpioSetInterrupt( &SX1272.DIO0, IRQ_RISING_EDGE_E, IRQ_VERY_HIGH_PRIORITY_E, irqHandlers[0], GPIO_NOPULL );
	GpioSetInterrupt( &SX1272.DIO1, IRQ_RISING_EDGE_E, IRQ_VERY_HIGH_PRIORITY_E, irqHandlers[1], GPIO_NOPULL );
	//GpioSetInterrupt( &SX1272.DIO2, IRQ_RISING_EDGE_E, IRQ_HIGH_PRIORITY_E, irqHandlers[2], GPIO_NOPULL );
	GpioSetInterrupt( &SX1272.DIO3, IRQ_RISING_EDGE_E, IRQ_VERY_HIGH_PRIORITY_E, irqHandlers[3], GPIO_NOPULL );
	//GpioSetInterrupt( &SX1272.DIO4, IRQ_RISING_EDGE_E, IRQ_HIGH_PRIORITY_E, irqHandlers[4] );	// RM: not connected on mbed-SX1272 board 
	//GpioSetInterrupt( &SX1272.DIO5, IRQ_RISING_EDGE_E, IRQ_HIGH_PRIORITY_E, irqHandlers[5] );
}

void SX1272IoDeInit( void )
{
	GpioInit( &SX1272.Spi.Nss, RADIO_NSS, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );

	GpioInit( &SX1272.DIO0, RADIO_DIO_0, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
	GpioInit( &SX1272.DIO1, RADIO_DIO_1, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
	GpioInit( &SX1272.DIO2, RADIO_DIO_2, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
	GpioInit( &SX1272.DIO3, RADIO_DIO_3, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
}

uint8_t SX1272GetPaSelect( uint32_t channel )
{
	//return RF_PACONFIG_PASELECT_PABOOST;		// RM: Not using PA_BOOST (Not connected)
	return RF_PACONFIG_PASELECT_RFO;				// RM: Using RFO instead
}

void SX1272AntSwInit( void )
{
	GpioInit( &AntTx, RADIO_ANT_SWITCH_TX, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );
	GpioInit( &AntRx, RADIO_ANT_SWITCH_RX, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );
}

void SX1272AntSwDeInit( void )
{
	GpioInit( &AntTx, RADIO_ANT_SWITCH_TX, PIN_ANALOGIC_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
	GpioInit( &AntRx, RADIO_ANT_SWITCH_RX, PIN_ANALOGIC_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
}

void SX1272SetAntSw( uint8_t rxTx )
{
   if( 0 == rxTx ) // 0:RX, 1:TX
   {
      GpioWrite( &AntRx, RxTxLogic );
      GpioWrite( &AntTx, TxRxLogic );
   }
   else
   {
      GpioWrite( &AntRx, TxRxLogic );
      GpioWrite( &AntTx, RxTxLogic );
   }
}

bool SX1272CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}
