
/* Touch screen */



 
 /* UARTS */
 
#include "usart.h"

extern UART_HandleTypeDef* cie_gsm_uart 			;
extern UART_HandleTypeDef* cie_debug_uart 			;
extern UART_HandleTypeDef* cie_panel_notwork_uart 	;

/* I2C */

#include "i2c.h"

extern I2C_HandleTypeDef* cie_main_i2c ;

/* TIMERS */

#include "tim.h"

extern TIM_HandleTypeDef* cie_backlight_timer;


/* RTC */
#include "stm32f7xx_hal_rtc.h"

extern RTC_HandleTypeDef* cie_rtc;