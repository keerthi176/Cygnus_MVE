/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Generic GPIO driver implementation

Comment: Relies on the specific board GPIO implementation as well as on
         IO expander driver implementation if one is available on the target
         board.

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#ifndef __GPIO_H__
#define __GPIO_H__

#include "pinName-board.h"
//#include "pinName-ioe.h"

/*!
 * Board GPIO pin names
 */
typedef enum
{
    MCU_PINS_E,
    //IOE_PINS_E,

    // Not connected
    NC_E = (int32_t)0xFFFFFFFF
} PinNames;

/*!
 * Operation Mode for the GPIO
 */
typedef enum
{
    PIN_INPUT_E,
    PIN_OUTPUT_E,
    PIN_ALTERNATE_FCT_E,
    PIN_ANALOGIC_E
} PinModes;

/*!
 * Add a pull-up, a pull-down or nothing on the GPIO line
 */
typedef enum
{
    PIN_NO_PULL_E,
    PIN_PULL_UP_E,
    PIN_PULL_DOWN_E
} PinTypes;

/*!
 * Define the GPIO as Push-pull type or Open Drain
 */
typedef enum
{
    PIN_PUSH_PULL_E,
    PIN_OPEN_DRAIN_E
} PinConfigs;

/*!
 * Define the GPIO IRQ on a rising, falling or both edges
 */
typedef enum
{
    NO_IRQ_E,
    IRQ_RISING_EDGE_E,
    IRQ_FALLING_EDGE_E,
    IRQ_RISING_FALLING_EDGE_E
} IrqModes;

/*!
 * Define the IRQ priority on the GPIO
 */
typedef enum
{
    IRQ_VERY_LOW_PRIORITY_E,
    IRQ_LOW_PRIORITY_E,
    IRQ_MEDIUM_PRIORITY_E,
    IRQ_HIGH_PRIORITY_E,
    IRQ_VERY_HIGH_PRIORITY_E
} IrqPriorities;

/*!
 * Structure for the GPIO
 */
typedef struct
{
    PinNames  pin;
    uint16_t pinIndex;
    void *port;
    uint16_t portIndex;
} Gpio_t;

/*!
 * GPIO IRQ handler function prototype
 */
typedef void( GpioIrqHandler )( void );

/*!
 * \brief Initializes the given GPIO object
 *
 * \param [IN] obj    Pointer to the GPIO object
 * \param [IN] pin    Pin name ( please look in pinName-board.h file )
 * \param [IN] mode   Pin mode [PIN_INPUT_E, PIN_OUTPUT_E,
 *                              PIN_ALTERNATE_FCT_E, PIN_ANALOGIC_E]
 * \param [IN] config Pin config [PIN_PUSH_PULL_E, PIN_OPEN_DRAIN_E]
 * \param [IN] type   Pin type [PIN_NO_PULL_E, PIN_PULL_UP_E, PIN_PULL_DOWN_E]
 * \param [IN] value  Default output value at initialisation
 */
void GpioInit( Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value );

/*!
 * \brief GPIO IRQ Initialization
 *
 * \param [IN] obj         Pointer to the GPIO object
 * \param [IN] irqMode     IRQ mode [NO_IRQ_E, IRQ_RISING_EDGE_E,
 *                                   IRQ_FALLING_EDGE_E, IRQ_RISING_FALLING_EDGE_E]
 * \param [IN] irqPriority IRQ priority [IRQ_VERY_LOW_PRIORITY_E, IRQ_LOW_PRIORITY_E
 *                                       IRQ_MEDIUM_PRIORITY_E, IRQ_HIGH_PRIORITY_E
 *                                       IRQ_VERY_HIGH_PRIORITY_E]
 * \param [IN] irqHandler  Callback function pointer
 */
void GpioSetInterrupt( Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler, uint32_t pull );

/*!
 * \brief Removes the interrupt from the object
 *
 * \param [IN] obj Pointer to the GPIO object
 */
void GpioRemoveInterrupt( Gpio_t *obj );

/*!
 * \brief Writes the given value to the GPIO output
 *
 * \param [IN] obj   Pointer to the GPIO object
 * \param [IN] value New GPIO output value
 */
void GpioWrite( Gpio_t *obj, uint32_t value );

/*!
 * \brief Toggle the value to the GPIO output
 *
 * \param [IN] obj   Pointer to the GPIO object
 */
void GpioToggle( Gpio_t *obj );

/*!
 * \brief Reads the current GPIO input value
 *
 * \param [IN] obj Pointer to the GPIO object
 * \retval value   Current GPIO input value
 */
uint32_t GpioRead( Gpio_t *obj );

#endif // __GPIO_H__
