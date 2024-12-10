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
***************************************************************************************
*  File        : MC_Encryption.h
*
*  Description : Utility for ecrypting and decrypting message payloads
*
*************************************************************************************/

#ifndef MC_ENCRYPTION_H
#define MC_ENCRYPTION_H

#include "CO_Defines.h"

#ifdef USE_ENCPRYPTED_PAYLOAD
/* System Include Files
*************************************************************************************/
#include <stdint.h>

/* User Include Files
*************************************************************************************/
#include "CO_Message.h"
#include "CO_ErrorCode.h"



/* Public Structures
*************************************************************************************/ 

/* Structure that contains STM32 Crypto verion and Crypto Type: HW or FW */
typedef struct
{
  uint8_t X;     /* Used to get the X parameter of the current STM32 crypto library verion */
  uint8_t Y;     /* Used to get the Y parameter of the current STM32 crypto library verion */
  uint8_t Z;     /* Used to get the Z parameter of the current STM32 crypto library verion */
  uint8_t T[3];  /* Used to get the Type of the verion. This parameter can be "HW" or "FW" */
  uint8_t CortexConf ;
  uint8_t MicroConf ;
  uint8_t EndiannessConf ;
  uint8_t MisalignedConf ;
  uint8_t EncDecConf;
  uint16_t SymKeyConf  ; 
  uint16_t SymKeyModesConf ;
  uint16_t AsymKeyConf ;
  uint16_t HashConf ;
  uint8_t DrbgConf ;
  uint8_t AesConf ;
  uint8_t RsaConf ;
  uint8_t GcmConf ;  
  
}STM32CryptoLibVer_TypeDef;



/* Public Enumerations
*************************************************************************************/
typedef enum {  
  E_SK_DEFAULT_E =                       (uint32_t) (0x00000000),   /* User Flag: No flag specified. This is the default value that should be set to this flag  */  
  E_SK_DONT_PERFORM_KEY_SCHEDULE_E =     (uint32_t) (0x00000001),   /* User Flag: Used to force the init to not reperform key schedule.\n
                                                                     The classic example is where the same key is used on a new message, in this case to redo key scheduling is
                                                                     a useless waste of computation, could be particularly useful on GCM, where key schedule is very complicated. */    
  E_SK_USE_DMA_E =                       (uint32_t) (0x00000008),   /* User Flag: Used only when there is an HW engine for AES/DES, it specifies if the DMA should be used to transfer
                                                                     data or the CPU should be used instead. It is common to always use the DMA, except when DMA is very busy or
                                                                     input data is very small */
  E_SK_FINAL_APPEND_E =                  (uint32_t) (0x00000020),   /* User Flag: Must be set in CMAC mode before the final Append call occurs. */
  E_SK_OPERATION_COMPLETED_E  =          (uint32_t) (0x00000002),   /* Internal Flag (not to be set/read by user): used to check that the Finish function has been already called */  
  E_SK_NO_MORE_APPEND_ALLOWED_E =        (uint32_t) (0x00000004),   /* Internal Flag (not to be set/read by user): it is set when the last append has been called. Used where the append is called with an InputSize not
                                                                     multiple of the block size, which means that is the last input.*/
  E_SK_NO_MORE_HEADER_APPEND_ALLOWED_E = (uint32_t) (0x00000010),   /* Internal Flag (not to be set/read by user): only for authenticated encryption modes. \n
                                                                      It is set when the last header append has been called. Used where the header append is called with an InputSize not
                                                                      multiple of the block size, which means that is the last input.*/
  E_SK_APPEND_DONE_E =                   (uint32_t) (0x00000040),   /* Internal Flag (not to be set/read by user): only for CMAC.It is set when the first append has been called */
} SKflags_et; /* Type definitation for Symmetric Key Flags */




/* Public Constants
*************************************************************************************/

/* Used to specify the memory representation of the platform.
  *   - CRL_ENDIANNESS=1 for LITTLE ENDIAN
  *   - CRL_ENDIANNESS=2 for BIG ENDIAN
  *   This setting is VERY IMPORTANT */
#define CRL_ENDIANNESS  1  /* LITTLE ENDIAN is selected */

#define INCLUDE_ENCRYPTION    /* Enables encryption functionalities in STM32 Crypto Lib */
#define INCLUDE_DECRYPTION    /* Enables decryption functionalities in STM32 Crypto Lib */

#define INCLUDE_TDES          /* Enable Triple DES (TDES) functions in STM32 Crypto Lib */
#define INCLUDE_CBC           /* Enable CBC encoding functions in STM32 Crypto Lib */

#define TDES_SUCCESS                (0) //Success.
#define TDES_ERR_BAD_INPUT_SIZE  (3311) //Invalid input size, it must be multiple of 8.
#define TDES_ERR_BAD_OPERATION   (3312) //Invalid operation.
#define TDES_ERR_BAD_CONTEXT     (3313) //TDES context contains some invalid or uninitialized values.
#define TDES_ERR_BAD_PARAMETER   (3314) //One of the expected function parameters is invalid.


/* Crypto include files
*************************************************************************************/
#include "TDES/tdes.h"


/* Public Functions Prototypes
*************************************************************************************/
void STM32_CryptoLibraryVersion(STM32CryptoLibVer_TypeDef * LibVersion);
ErrorCode_t MC_MessageEncrypt(CO_Message_t* pMessage);
ErrorCode_t MC_MessageDecrypt(CO_Message_t* pMessage);

#endif //#ifdef USE_ENCPRYPTED_PAYLOAD

#endif // MC_ENCRYPTION_H
