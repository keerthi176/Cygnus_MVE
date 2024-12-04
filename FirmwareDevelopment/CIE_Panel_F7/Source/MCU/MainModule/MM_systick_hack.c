
#include "cmsis_os.h"

#include "MM_rtos.h"

#if _USE_RTOS_	== 1

/*************************************************************************/
/**  \fn      uint32_t HAL_GetTick(void)
***  \brief   Global helper function
**************************************************************************/

uint32_t HAL_GetTick(void)
{	
	return osKernelSysTick( ) / osKernelSysTickMicroSec( 1000 );
}

#endif


