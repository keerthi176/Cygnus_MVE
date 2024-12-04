
#include "stm32f7xx_hal.h"
#include "stm32f7xx.h"
#include "stm32f7xx_it.h"


extern QSPI_HandleTypeDef hqspi;
extern UART_HandleTypeDef huart7;
extern DMA_HandleTypeDef hdma_quadspi;



void DMA2_Stream7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Stream7_IRQn 0 */

  /* USER CODE END DMA2_Stream7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_quadspi);
  /* USER CODE BEGIN DMA2_Stream7_IRQn 1 */

  /* USER CODE END DMA2_Stream7_IRQn 1 */
}


void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
 // HAL_SYSTICK_IRQHandler();
  /* USER CODE BEGIN SysTick_IRQn 1 */
	
	
  /* USER CODE END SysTick_IRQn 1 */
}

/**
* @brief This function handles UART7 global interrupt.
*/
void UART7_IRQHandler(void)
{
   /* USER CODE BEGIN UART7_IRQn 0 */
	
	 	
   HAL_UART_IRQHandler(&huart7);
 
  /* USER CODE END UART7_IRQn 1 */
}



void QUADSPI_IRQHandler(void)
{
  /* USER CODE BEGIN QUADSPI_IRQn 0 */

  /* USER CODE END QUADSPI_IRQn 0 */
  HAL_QSPI_IRQHandler(&hqspi);
  /* USER CODE BEGIN QUADSPI_IRQn 1 */

  /* USER CODE END QUADSPI_IRQn 1 */
}

