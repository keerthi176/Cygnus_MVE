
 
/******************************************************************************/
/*                                                                            */
/*                                    HASH                                    */
/*                                                                            */
/******************************************************************************/
/******************  Bits definition for HASH_CR register  ********************/


#include "stm32f767xx.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_hash.h"
#include "stm32f7xx_hal_hash_ex.h"
#include "stm32f7xx_hal_rcc_ex.h"

static HASH_HandleTypeDef     HashHandle;


int hmac_sha256( const char* aKey, size_t keyLen, const char* payload, size_t payloadLen, uint8_t* hash )
{
	__HAL_RCC_HASH_CLK_ENABLE();
	
  HAL_HASH_DeInit(&HashHandle);
  HashHandle.Init.DataType = HASH_DATATYPE_8B;
  HashHandle.Init.pKey = (uint8_t*)aKey;
  HashHandle.Init.KeySize = keyLen;

  if (HAL_HASH_Init(&HashHandle) != HAL_OK)
  {
	  *hash = 0;
    return 0; 
  }

  /* Compute HMAC-SHA256 */
  if (HAL_HMACEx_SHA256_Start(&HashHandle, (uint8_t *)payload, payloadLen, hash, 0xFF) != HAL_OK)
  {
     *hash = 0;
    return 0; 
  }
  return 64;
}

