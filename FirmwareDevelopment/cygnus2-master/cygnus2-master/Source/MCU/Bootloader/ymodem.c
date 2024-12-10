/**
  ******************************************************************************
  * @file    STM32L476G_EVAL/DualBank/src/ymodem.c
  * @author  MCD Application Team
  * @version 1.0.0
  * @date    15-September-2016
  * @brief   This file provides all the software functions related to the ymodem
  *          protocol.
  ******************************************************************************
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/** @addtogroup STM32L4xx_DB
  * @{
  */

/** @addtogroup Ymodem
  * @brief Ymodem data reception.
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "common.h"
#include "eeprom_emul.h"
#include "flash_interface.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"
#include "menu.h"
#include "DM_SerialPort.h"
#include "BLF_serial_if.h"



/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define CRC16_F       /* activate the CRC16 integrity */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* @note ATTENTION - please keep this variable 32bit alligned */
#if defined ( __ICCARM__ )    /* IAR Compiler */
#pragma data_alignment=4
#elif defined ( __CC_ARM )   /* ARM Compiler */
__align(4)
#elif defined ( __GNUC__ ) /* GCC Compiler */
__attribute__ ((aligned (4)))
#endif
/* The aPacketData buffer is no more used but dm_serial_data_buffer is accessed through 
 * the dm_serial_data_buffer.global_buffer pointer to save 1k of RAM as only a single UART port
 * is used by the bootloader
 * The original ST code instantiates the buffer below
 * uint8_t aPacketData[PACKET_1K_SIZE + PACKET_DATA_INDEX + PACKET_TRAILER_SIZE];
 */
uint8_t *const aPacketData = dm_serial_data_buffer.global_buffer;
#ifdef ENCRYPT
#if defined ( __ICCARM__ )    /* IAR Compiler */
#pragma data_alignment=4
#elif defined ( __CC_ARM )   /* ARM Compiler */
__align(4)
#elif defined ( __GNUC__ ) /* GCC Compiler */
__attribute__ ((aligned (4)))
#endif
uint8_t aDecryptData[PACKET_1K_SIZE];
#endif

/* The aFileName buffer is no more used but dm_serial_data_buffer is accessed through 
 * the dm_serial_data_buffer.global_buffer pointer to save RAM as only a single UART port
 * is used by the bootloader
 * The original ST code instantiates the buffer below
 * uint8_t aFileName[FILE_NAME_LENGTH];
 */
uint8_t *const aFileName = (uint8_t*)(dm_serial_data_buffer.global_buffer + 1048u);

static int32_t total_flashed = 0;

extern osMutexId(CRCMutexId);

/* Private function prototypes -----------------------------------------------*/
static COM_StatusTypeDef ReceivePacket(const uint8_t request_key, uint8_t *p_data, uint32_t *p_length, uint32_t timeout);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Receive a packet from sender
  * @param  p_data
  * @param  p_length
  *     0: end of transmission
  *     2: abort by sender
  *    >0: packet length
  * @param  timeout
  * @retval HAL_OK: normally return
  *         HAL_BUSY: abort by user
  */
static COM_StatusTypeDef ReceivePacket(const uint8_t request_key, uint8_t *p_data, uint32_t *p_length, uint32_t timeout)
{
  uint32_t crc;
  uint32_t packet_size = 0;
  COM_StatusTypeDef status;
  uint16_t received_packet_size;


   if ( p_data && p_length )
   {
     HAL_StatusTypeDef hal_status = serial_request_new_packet(request_key, &p_data[1], (uint16_t)*p_length, &received_packet_size, timeout);
     
      if (   (HAL_OK       == hal_status)
          || ((HAL_TIMEOUT == hal_status) && (0 < received_packet_size)))
      {
         switch (p_data[1])
         {
            case SOH:
              packet_size = PACKET_SIZE;
              break;
            case STX:
              packet_size = PACKET_1K_SIZE;
              break;
            case EOT:
               status = COM_EOT_E;
              break;
            case CA:
              if (p_data[2] == CA)
              {
                packet_size = 2;
              }
              else
              {
                status = COM_ERROR_E;
              }
              break;
            case ABORT1:
            case ABORT2:
              status = COM_ABORT_E;
              break;
            default:
              status = COM_ERROR_E;
              break;
         }

         if (packet_size >= PACKET_SIZE )
         {
            /* Simple packet sanity check */
            if (p_data[PACKET_NUMBER_INDEX] != ((p_data[PACKET_CNUMBER_INDEX]) ^ NEGATIVE_BYTE))
            {
               packet_size = 0;
               status = COM_ERROR_E;
            }
            else
            {
               /* Check packet CRC */
               crc = p_data[ packet_size + PACKET_DATA_INDEX ] << 8;
               crc += p_data[ packet_size + PACKET_DATA_INDEX + 1 ];
               if (osOK == osMutexWait(CRCMutexId, 1)) 
               {
                  if (HAL_CRC_Calculate(&Ymodem_CrcHandle, (uint32_t*)&p_data[PACKET_DATA_INDEX], packet_size) != crc )
                  {
                     packet_size = 0;
                     status = COM_ERROR_E;
                  }
                  else
                  {
                     status = COM_OK_E;
                     debug_out("CRC check OK.\n" );
                  }
                  osMutexRelease(CRCMutexId);
               }
            }
         }
      }
      else
      {
         status = COM_TIMEOUT_E;
      }

      *p_length = packet_size;
   }
   else 
   {
      status = COM_ERROR_E;
   }

   return status;
}

/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @param  bank The actual active bank
  * @retval COM_StatusTypeDef result of reception/programming
  */
COM_StatusTypeDef Ymodem_Receive ( uint32_t *p_size, uint32_t bank )
{
  uint32_t i, packet_length, session_done = 0, file_done, errors = 0, session_begin = 0;
  uint32_t flashdestination, ramsource, filesize;
  uint8_t *file_ptr;
  uint8_t file_size[FILE_SIZE_LENGTH], tmp, packets_received;
  COM_StatusTypeDef result = COM_OK_E;

   if ( p_size )
   {
      /* Initialize flashdestination variable */
      if (bank == 1 )
      {
         flashdestination = FLASH_START_BANK1;
      }
      else
      {
         flashdestination = FLASH_START_BANK2;
      }

      total_flashed = 0;

      while ((session_done == 0) && (result == COM_OK_E))
      {
          packets_received = 0;
          file_done = 0;
          /* Start with a CRC16 token */
          uint8_t request_key = CRC16;     

          while ((file_done == 0) && (result == COM_OK_E))
          {
            if (packets_received == 0)
            {
               packet_length = PACKET_SIZE + PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE;
            }
            else
            {
               packet_length = PACKET_1K_SIZE + PACKET_HEADER_SIZE + PACKET_TRAILER_SIZE;
            }

            switch (ReceivePacket(request_key, (uint8_t*)dm_serial_data_buffer.global_buffer, &packet_length, DOWNLOAD_TIMEOUT))
            {
               case COM_OK_E:
                errors = 0;
                switch (packet_length)
                {
                  case 2:
                    /* Abort by sender */
                    debug_out("Abort by sender...");
                    Serial_PutByte(ACK);
                    debug_out("ACK.\n");
                    result = COM_ABORT_E;
                    break;
                  case 0:
                    /* End of transmission */
                    debug_out("Abort by sender...");
                    Serial_PutByte(ACK);
                    debug_out("ACK.\n");
                    file_done = 1;
                    break;
                  default:
                    /* Normal packet */
                    if (dm_serial_data_buffer.global_buffer[PACKET_NUMBER_INDEX] != packets_received)
                    {
                      request_key = NAK;
                    }
                    else
                    {
                      if (packets_received == 0)
                      {
                        /* File name packet */
                        if (dm_serial_data_buffer.global_buffer[PACKET_DATA_INDEX] != 0)
                        {
                          /* File name extraction */
                          i = 0;
                          file_ptr = (uint8_t*)dm_serial_data_buffer.global_buffer + PACKET_DATA_INDEX;
                          while ( (*file_ptr != 0) && (i < FILE_NAME_LENGTH))
                          {
                            aFileName[i++] = *file_ptr++;
                          }

                          /* File size extraction */
                          aFileName[i++] = '\0';
                          i = 0;
                          file_ptr ++;
                          while ( (*file_ptr != ' ') && (i < FILE_SIZE_LENGTH))
                          {
                            file_size[i++] = *file_ptr++;
                          }
                          file_size[i++] = '\0';
                          Str2Int(file_size, &filesize);

                          debug_out("Filename: %s\n", aFileName );
                          debug_out("Filesize: %d\n", filesize );
                          
                          /* Test the size of the image to be sent */
                          /* Image size is greater than Flash size */
                          *p_size = filesize;
                          if (*p_size > (FLASH_START_BANK2 - FLASH_START_BANK1))
                          {
                            /* End session */
                            tmp = CA;
                            serial_send_data(&tmp, 1, NAK_TIMEOUT);
                            serial_send_data(&tmp, 1, NAK_TIMEOUT);
                            result = COM_LIMIT_E;
                          }
                          else
                          {
                            /* erase destination area - always the other bank mapped on 0x08018000*/
                            FLASH_If_Erase(bank);
                             
                            Serial_PutByte(ACK); 
                            
                            debug_out("ACK.\n");
                             
                            request_key = CRC16;
                          }
                        }
                        /* File header packet is empty, end session */
                        else
                        {
                           debug_out("ACK.\n");
                          Serial_PutByte(ACK);
                          file_done = 1;
                          session_done = 1;
                          debug_out("Empty header - exit.\n");
                           
                          break;
                        }
                      }
                      else /* Data packet */
                      {
               #ifdef ENCRYPT
                        if (HAL_CRYPEx_AES( &DecHandle, &aPacketData[PACKET_DATA_INDEX], packet_length, &aDecryptData[0], NAK_TIMEOUT) != HAL_OK)
                        {
                          /* End session */
                          Serial_PutByte(CA);
                          Serial_PutByte(CA);
                          result = COM_DATA_E;
                          break;
                        }
                        ramsource = (uint32_t) & aDecryptData;
               #else
                        ramsource = (uint32_t) & dm_serial_data_buffer.global_buffer[PACKET_DATA_INDEX];
               #endif
                        debug_out("Flashing data..." );
                        /* Write received data in Flash */
                        if (FLASH_If_Write(flashdestination, (uint32_t*) ramsource, packet_length / 4) == FLASHIF_OK)
                        {
                          flashdestination += packet_length;
                           total_flashed += packet_length;
                          request_key = ACK;
                           debug_out("OK.\n");
                        }
                        else /* An error occurred while writing to Flash memory */
                        {
                          /* End session */
                           debug_out("ERROR. Ending session...");
                          Serial_PutByte(CA);
                          Serial_PutByte(CA);
                           debug_out("Done.\n" );
                          result = COM_DATA_E;
                        }
                      }
                      packets_received ++;
                      debug_out( "Packets received %d.\n", packets_received );
                      session_begin = 1;
                    }
                    break;
                }
                break;
               case COM_EOT_E: /* End Of Transmission*/
                 debug_out( "End-of-Transmission..." );
                 Serial_PutByte(ACK);
                 debug_out( "ACK'd.\n" );
                 file_done = 1;
                 debug_out( "Filesize: %d   Total flashed: %d.\n", filesize, total_flashed );
                 if ( filesize == total_flashed )
                 {
                     session_done = true;
                 }
                 break;
               case COM_ABORT_E: /* Abort actually */
                 debug_out( "Abort..." );
                 Serial_PutByte(CA);
                 Serial_PutByte(CA);
                 debug_out( "done.\n" );
                 result = COM_ABORT_E;
                 break;
               default:
                if (session_begin > 0)
                {
                   errors ++;
                   debug_out( "(errors)\n");
                }
                if (errors > MAX_ERRORS)
                {
                  /* Abort communication */
                   debug_out( "Too many..." );
                   Serial_PutByte(CA);
                   Serial_PutByte(CA);
                   debug_out( "Aborted.\n");
                  result = COM_ABORT_E;
                }
                else
                {
                  request_key = CRC16;  /* Ask for a packet */
                   debug_out( "New key = %X\n", request_key );
                }
                break;
               }
            }
         }
      }
   else 
   {
      result = COM_ERROR_E;
   }
   return result;
}

/**
  * @}
  */

/**
  * @}
  */

/*******************(C)COPYRIGHT STMicroelectronics *****END OF FILE****/
