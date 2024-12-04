
#include "cmsis_os.h"


#ifdef _USE_RTOS_	

uint32_t HAL_GetTick(void)
{
 // return osKernelSysTickMicroSec( osKernelSysTick( ) ) / 1000;

	return osKernelSysTick()/osKernelSysTickMicroSec(1000);
}

#endif

