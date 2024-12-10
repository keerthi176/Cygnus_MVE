/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2019 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File        : MC_Encryption.c
*
*  Description : Utility for ecrypting and decrypting message payloads
*
*************************************************************************************/
#include "CO_Defines.h"

#ifdef USE_ENCPRYPTED_PAYLOAD
/* System Include Files
*************************************************************************************/
#include <cmsis_os.h>
#include <stdlib.h>

/* User Include Files
*************************************************************************************/
#include "stm32l4xx_hal.h"
#include "MC_Encryption.h"


/* Private variables ---------------------------------------------------------*/
#define PAYLOAD_SIZE_BYTES   8
#define MESSAGE_PAYLOAD_START_INDEX    8

/* MC_Key to be used for encryption/decryption */
uint8_t MC_Key[CRL_TDES_KEY] =
  {
    0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x23, 0x45, 0x67, 0x89, 0xAB,
    0xCD, 0xEF, 0x01, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23
  };
  
/* CBC initial vector */
uint8_t iv[CRL_TDES_BLOCK] = { "BULLPROD" };


extern osMutexId(CRCMutexId);

  /* Private Functions Prototypes
*************************************************************************************/
static int32_t STM32_TDES_CBC_Encrypt(uint8_t* InputMessage,
                               uint32_t InputMessageLength,
                               uint8_t  *OutputMessage,
                               uint32_t *OutputMessageLength);

static int32_t STM32_TDES_CBC_Decrypt(uint8_t* InputMessage,
                               uint32_t InputMessageLength,
                               uint8_t  *OutputMessage,
                               uint32_t *OutputMessageLength);



/* Public Functions
*************************************************************************************/

/*****************************************************************************
*  Function:   MC_MessageEncrypt
*  Description:      Encrypts the payload of the supplied message.
*  Param: pMessage   [OUT] Pointer to the message to be encrypted.
*  Returns:          SUCCESS_E or error code.
*****************************************************************************/
ErrorCode_t MC_MessageEncrypt(CO_Message_t* pMessage)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint8_t encryption_buffer[PAYLOAD_SIZE_BYTES];
   uint32_t payload_length;
   uint32_t tdes_status = TDES_ERR_BAD_OPERATION;
   
   memset(encryption_buffer, 0, PAYLOAD_SIZE_BYTES);
   
   if ( pMessage )
   {
      uint8_t* pPayload = (uint8_t*)&pMessage->Payload.PhyDataReq.Data[MESSAGE_PAYLOAD_START_INDEX];
      
      if (osOK == osMutexWait(CRCMutexId, 1)) 
      {
         __HAL_RCC_CRC_FORCE_RESET();
         __HAL_RCC_CRC_RELEASE_RESET();
         
         tdes_status = STM32_TDES_CBC_Encrypt( pPayload, PAYLOAD_SIZE_BYTES,
                                                         encryption_buffer, &payload_length);
         osMutexRelease(CRCMutexId);
      }
      
      if ( TDES_SUCCESS == tdes_status )
      {
         memcpy(pPayload, encryption_buffer, PAYLOAD_SIZE_BYTES);
         result = SUCCESS_E;
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return result;
}


/*****************************************************************************
*  Function:   MC_MessageDecrypt
*  Description:      Decrypts the payload of the supplied message.
*  Param: pMessage   [OUT] Pointer to the message to be decrypted.
*  Returns:          SUCCESS_E or error code.
*****************************************************************************/
ErrorCode_t MC_MessageDecrypt(CO_Message_t* pMessage)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint8_t encryption_buffer[PAYLOAD_SIZE_BYTES];
   uint32_t payload_length;
   
   memset(encryption_buffer, 0, PAYLOAD_SIZE_BYTES);
   
   if ( pMessage )
   {
      uint8_t* pPayload = (uint8_t*)&pMessage->Payload.PhyDataInd.Data[MESSAGE_PAYLOAD_START_INDEX];
      
      __HAL_RCC_CRC_FORCE_RESET();
      __HAL_RCC_CRC_RELEASE_RESET();
      
      uint32_t tdes_status = STM32_TDES_CBC_Decrypt( pPayload, PAYLOAD_SIZE_BYTES,
                                                      encryption_buffer, &payload_length);
      
      if ( TDES_SUCCESS == tdes_status )
      {
         memcpy(pPayload, encryption_buffer, PAYLOAD_SIZE_BYTES);
         result = SUCCESS_E;
      }
      else 
      {
         result = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   RCC_AHBPeriphClockCmd
*  Description:            Enables or disables the AHB peripheral clock.
*  Param: RCC_AHBPeriph    Specifies the AHB peripheral to be enabled/disabled.
*  Param: NewState         ENABLE or DISABLE.
*  Returns:                None.
*****************************************************************************/
void RCC_AHBPeriphClockCmd(uint32_t RCC_AHBPeriph, FunctionalState NewState)
{
   /* Check the parameters */
   assert_param(IS_RCC_AHB_PERIPH(RCC_AHBPeriph));
   assert_param(IS_FUNCTIONAL_STATE(NewState));

   if (NewState != DISABLE)
   {
      RCC->AHB1ENR |= RCC_AHBPeriph;
   }
   else
   {
      RCC->AHB1ENR &= ~RCC_AHBPeriph;
   }
}


/* Private Functions
*************************************************************************************/


/*****************************************************************************
*  Function:   STM32_TDES_CBC_Encrypt
*  Description:      Calls the STM32 Crypto Library to perform TDES encryption on a byte array.
*  Param: InputMessage        Pointer to the character buffer to be encrypted.
*  Param: InputMessageLength  Length of the character buffer to be encrypted.
*  Param: OutputMessage       [OUT] Pointer to the character buffer for the encrypted result.
*  Param: OutputMessageLength [OUT] Number of bytes written to OutputMessage.
*  Returns:                   Error code.  Could be one of
*                             TDES_SUCCESS                (0) Success.
*                             TDES_ERR_BAD_INPUT_SIZE  (3311) Invalid input size, it must be multiple of 8.
*                             TDES_ERR_BAD_OPERATION   (3312) Invalid operation.
*                             TDES_ERR_BAD_CONTEXT     (3313) TDES context contains some invalid or uninitialized values.
*                             TDES_ERR_BAD_PARAMETER   (3314) One of the expected function parameters is invalid.
*
*****************************************************************************/
int32_t STM32_TDES_CBC_Encrypt(uint8_t* InputMessage,
                               uint32_t InputMessageLength,
                               uint8_t  *OutputMessage,
                               uint32_t *OutputMessageLength)
{
   TDESCBCctx_stt TDESctx;
   
   uint32_t error_status = TDES_SUCCESS;

   int32_t outputLength = 0;

   /* Set flag field to default value */
   TDESctx.mFlags = E_SK_DEFAULT_E;
   TDESctx.mIvSize = CRL_TDES_BLOCK;

   /* Initialize the operation, by passing the key. */
   error_status = TDES_CBC_Encrypt_Init(&TDESctx, MC_Key, iv );

   /* check for initialization errors */
   if (error_status == TDES_SUCCESS)
   {
      /* Encrypt Data */
      error_status = TDES_CBC_Encrypt_Append(&TDESctx,
                                           InputMessage,
                                           InputMessageLength,
                                           OutputMessage,
                                           &outputLength);

      if (error_status == TDES_SUCCESS)
      {
         /* Write the number of data written*/
         *OutputMessageLength = outputLength;
         /* Do the Finalization */
         error_status = TDES_CBC_Encrypt_Finish(&TDESctx, OutputMessage + *OutputMessageLength, &outputLength);
         /* Add data written to the information to be returned */
         *OutputMessageLength += outputLength;
      }
   }

   return error_status;
}


/*****************************************************************************
*  Function:   STM32_TDES_CBC_Decrypt
*  Description:      Calls the STM32 Crypto Library to perform TDES decryption on a byte array.
*  Param: InputMessage        Pointer to the character buffer to be decrypted.
*  Param: InputMessageLength  Length of the character buffer to be decrypted.
*  Param: OutputMessage       [OUT] Pointer to the character buffer for the decrypted result.
*  Param: OutputMessageLength [OUT] Number of bytes written to OutputMessage.
*  Returns:                   Error code.  Could be one of
*                             TDES_SUCCESS                (0) Success.
*                             TDES_ERR_BAD_INPUT_SIZE  (3311) Invalid input size, it must be multiple of 8.
*                             TDES_ERR_BAD_OPERATION   (3312) Invalid operation.
*                             TDES_ERR_BAD_CONTEXT     (3313) TDES context contains some invalid or uninitialized values.
*                             TDES_ERR_BAD_PARAMETER   (3314) One of the expected function parameters is invalid.
*
*****************************************************************************/
int32_t STM32_TDES_CBC_Decrypt(uint8_t* InputMessage,
                               uint32_t InputMessageLength,
                               uint8_t  *OutputMessage,
                               uint32_t *OutputMessageLength)
{
   TDESCBCctx_stt TDESctx;

   uint32_t error_status = TDES_SUCCESS;

   int32_t outputLength = 0;

   /* Set flag field to default value */
   TDESctx.mFlags = E_SK_DEFAULT_E;
   TDESctx.mIvSize = CRL_TDES_BLOCK;

   /* Initialize the operation, by passing the key. */
   error_status = TDES_CBC_Decrypt_Init(&TDESctx, MC_Key, iv );

   /* check for initialization errors */
   if (error_status == TDES_SUCCESS)
   {
      /* Decrypt Data */
      error_status = TDES_CBC_Decrypt_Append(&TDESctx,
                                           InputMessage,
                                           InputMessageLength,
                                           OutputMessage,
                                           &outputLength);

      if (error_status == TDES_SUCCESS)
      {
         /* Write the number of data written*/
         *OutputMessageLength = outputLength;
         /* Do the Finalization */
         error_status = TDES_CBC_Decrypt_Finish(&TDESctx, OutputMessage + *OutputMessageLength, &outputLength);
         /* Add data written to the information to be returned */
         *OutputMessageLength += outputLength;
      }
   }

   return error_status;
}
#endif // USE_ENCPRYPTED_PAYLOAD

