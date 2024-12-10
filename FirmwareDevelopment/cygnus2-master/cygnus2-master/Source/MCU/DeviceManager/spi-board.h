/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Bleeper board SPI driver implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __SPI_MCU_H__
#define __SPI_MCU_H__

#include "gpio.h"
#include "stm32l4xx.h"

/*!
 * SPI driver structure definition
 */
typedef struct
{
    SPI_HandleTypeDef Spi;
    Gpio_t Mosi;
    Gpio_t Miso;
    Gpio_t Sclk;
    Gpio_t Nss;
}Spi_t;


/* Public functions prototypes
*******************************************************************************/
uint16_t SpiInOut( Spi_t *obj, uint16_t outData );
void SpiFrequency( Spi_t *obj, uint32_t hz );
void SpiFormat( Spi_t *obj, int32_t bits, int8_t cpol, int8_t cpha, int8_t slave );
void SpiInit( Spi_t *obj, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss );

#endif  // __SPI_MCU_H__
