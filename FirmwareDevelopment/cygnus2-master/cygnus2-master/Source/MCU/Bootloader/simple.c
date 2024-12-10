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
*  File         : simple.c
*
*  Description  : Source for ymodem simple file transfer
*
*************************************************************************************/




/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "eeprom_emul.h"
#include "flash_interface.h"
#include "ymodem.h"
#include "string.h"
#include "main.h"
#include "menu.h"
#include "DM_SerialPort.h"
#include "BLF_serial_if.h"


static int32_t total_flashed = 0;


/* Public functions ---------------------------------------------------------*/
/**
  * @brief  Receive a file using the ymodem protocol with CRC16.
  * @param  p_size The size of the file.
  * @param  bank The actual active bank
  * @retval COM_StatusTypeDef result of reception/programming
  */
COM_StatusTypeDef Simple_Receive( uint32_t *p_size, uint32_t bank )
{
   uint32_t flashdestination;

   /* Initialize flashdestination variable to bank 1 or bank 2 */
   if (1 == bank)
   {
      flashdestination = FLASH_START_BANK1;
   }
   else
   {
      flashdestination = FLASH_START_BANK2;
   }

   //Erase the target bank
   debug_out( "Erasing..." );
   FLASH_If_Erase(bank);   
   debug_out( "done.\n" ); 

   uint32_t imagesize;
   HAL_StatusTypeDef r;

   //download the size of the new image
   debug_out( "rx size..." ); 
   if ( HAL_OK != ( r = serial_receive_data( (uint8_t*) &imagesize, 4, DOWNLOAD_TIMEOUT ) ) )
   {
      if ( HAL_TIMEOUT == r )
      {
         debug_out( "timed out.\n" );
      }
   }

   debug_out( "Filesize: %d\n", imagesize );
   
   //check that the image will fit in a bank
   if ( imagesize > (FLASH_START_BANK2 - FLASH_START_BANK1))
   {
      return COM_LIMIT_E;
   }

    
   //Prepare to download the new image.
   total_flashed = 0;

   debug_out("Flashing data..." );

   //Reset the cumulative CRC value
   __HAL_CRC_DR_RESET( &Ymodem_CrcHandle );

   uint32_t data_crc;

   //Download and flash the image in 1K chunks
   while ( total_flashed < imagesize )
   {
      int32_t size = 1024;
     
      //Correct the expected size of final packet
      if ( total_flashed + size > imagesize )
      {
         size = imagesize - total_flashed;
      }
     
      //Get the next 1K
      debug_out("rx..." );
      if ( HAL_OK != ( r = serial_receive_data( (uint8_t*)dm_serial_data_buffer.global_buffer, size, DOWNLOAD_TIMEOUT ) ) )
      {
         if ( HAL_TIMEOUT == r )
         {
            debug_out( "timed out.\n" );
            return COM_TIMEOUT_E;
         }
         return COM_DATA_E;
      }
      
      //Compute the overall CRC so far
      data_crc = HAL_CRC_Accumulate( &Ymodem_CrcHandle, (uint32_t*)dm_serial_data_buffer.global_buffer, size );

      debug_out("flashing\n" );
      /* Write received data in Flash */
      if ( FLASHIF_OK == FLASH_If_Write( flashdestination + total_flashed, (uint32_t*) dm_serial_data_buffer.global_buffer, size / 4 ) )
      {       
         total_flashed += size;
         debug_out( "%d\n", total_flashed / 1024 );
      }
      else /* An error occurred while writing to Flash memory */
      {
         /* End session */
         debug_out("ERROR. Ending session...\n");

         return COM_DATA_E;
      }
   }

   //Download the expected CRC
   uint16_t crc;
   if ( HAL_OK != ( r = serial_receive_data( (uint8_t*) &crc, 2, DOWNLOAD_TIMEOUT )) )
   {
      if ( HAL_TIMEOUT == r )
      {
        debug_out( "timed out.\n" );
        return COM_TIMEOUT_E;
      }
      return COM_DATA_E;
   }   

   debug_out( "rx crc=%X calced=%X\n", crc, data_crc );

   //Send the calculated CRC back to the programmer
   HAL_Delay( 25 );
   serial_send_data( (uint8_t*) &data_crc, 2, DOWNLOAD_TIMEOUT );
   HAL_Delay( 25 );

   //Report success if the computed CRC matches the expected value
   if ( crc == data_crc )
   {
      return COM_OK_E;
   }
   else
   {
      return COM_ERROR_E;
   }
}
