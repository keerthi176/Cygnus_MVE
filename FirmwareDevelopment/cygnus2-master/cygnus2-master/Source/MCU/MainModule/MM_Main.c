/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : MM_Main.c
*
*  Description  : main routines
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "CO_SerialDebug.h"
#include "board.h"
#include "lptim.h"
#include "DM_Device.h"
#include "DM_SerialPort.h"
#include "gpio_config.h"
#include "DM_SystemClock.h"
#include "DM_NVM.h"
#include "DM_IWDG.h"
//#include "MM_DataManager.h"
#include "DM_i2c.h"
#include "DM_svi.h"
#include "DM_LED.h"
#include "DM_CRC.h"
#include "MM_MeshTask.h"
#include "MM_MACTask.h"
#include "MM_ConfigSerialTask.h"
#include "MM_ATHandleTask.h"
#include "MM_Main.h"
#include "MM_NCUApplicationTask.h"
#include "MM_RBUApplicationTask.h"
#ifdef USE_NEW_HEAD_INTERFACE
#include "MM_PluginInterfaceTask.h"
#else
#include "MM_HeadInterfaceTask.h"
#endif
#include "MM_GpioTask.h"
#include "MM_SviTask.h"
#include "MC_Encryption.h"
#include "MC_MacConfiguration.h"
#include "common.h"
#include "MM_TimedEventTask.h"
#include "CFG_Device_cfg.h"
#include "MM_PpuManagement.h"
#include "DM_BatteryMonitor.h"
#include "main.h"

#ifdef SE_BATTERY
 #warning "SE_BATTERY is defined in CO_Defines.h"
#endif
#ifdef SHOW_DOWNLINK_LED
 #warning "SHOW_DOWNLINK_LED is defined in CO_Defines.h"
#endif
#ifdef APPLY_RSSI_OFFSET
 #warning "APPLY_RSSI_OFFSET is defined in CO_Defines.h"
#endif
#ifdef ENABLE_TDM_CALIBRATION
 #warning "ENABLE_TDM_CALIBRATION is defined in CO_Defines.h"
#endif
#ifdef ENABLE_LINK_ADD_OR_REMOVE
 #warning "ENABLE_LINK_ADD_OR_REMOVE is defined in CO_Defines.h"
#endif
#ifdef DISABLE_NCU_SELECTION_WHEN_FORMING
 #warning "DISABLE_NCU_SELECTION_WHEN_FORMING is defined in CO_Defines.h"
#endif
#ifdef WAKE_FOR_ALL_SLOTS
 #warning "WAKE_FOR_ALL_SLOTS is defined in CO_Defines.h"
#endif
#ifdef SEND_DUMMY_BATTERY_READINGS
 #warning "SEND_DUMMY_BATTERY_READINGS is defined in CO_Defines.h"
#endif
#ifdef TXRX_TIMING_PULSE
 #warning "TXRX_TIMING_PULSE is defined in CO_Defines.h"
#endif
#ifdef ENABLE_HEARTBEAT_TX_SKIP
 #warning "ENABLE_HEARTBEAT_TX_SKIP is defined in CO_Defines.h"
#endif
#ifdef USE_OLD_CONSTRUCTION_BKP_BATTERY_PIN_LOGIC
 #warning "USE_OLD_CONSTRUCTION_BKP_BATTERY_PIN_LOGIC is defined in CO_Defines.h"
#endif
#ifdef REPORT_RXD_MESSAGES_RAW_BINARY
 #warning "REPORT_RXD_MESSAGES_RAW_BINARY is defined in CO_Defines.h"
#endif


#if LOW_POWER_AWAKE_DURATION != 1800
#warning "LOW_POWER_AWAKE_DURATION modified in CO_Defines.h"
#endif
/* Private Functions Prototypes
*************************************************************************************/



/* Global Variables
*************************************************************************************/
#define APP_POOL_SIZE 15
#define APP_Q_SIZE    10

uint32_t gSystemID = 0;

osPoolDef(AppPool, APP_POOL_SIZE, CO_Message_t);
osPoolId AppPool;

osMessageQDef(AppQ, APP_Q_SIZE, &CO_Message_t);
osMessageQId(AppQ);

extern DM_BaseType_t gBaseType;

/* Non initialised variable to store any information when a SW reset is triggered by SW */
uint32_t rbu_pp_mode_request __attribute__( ( section( "NoInit"),zero_init) ) ;
uint32_t rbu_pp_master_address __attribute__( ( section( "NoInit"),zero_init) ) ;
uint32_t reprogram_request_status __attribute__( ( section( "NoInit"),zero_init) ) ;
uint32_t sw_reset_msg_indication_flag __attribute__( ( section( "NoInit"),zero_init) ) ;
char sw_reset_debug_message[126] __attribute__( ( section( "NoInit"),zero_init) ) ;

extern bool MM_ATDecodeSerialNumber(const uint32_t value, char* pParamBuffer);


/* Private Variables
*************************************************************************************/

#ifdef BOOTLOADER_BUILD
const char sw_version_nbr[12] __attribute__(( at(0x08020188))) = VERSION_NUMBER_STRING;
const char sw_version_date[12] __attribute__(( at(0x08020194))) = VERSION_DATE_STRING;
#else
const char sw_version_nbr[12] __attribute__(( at(0x08000188))) = VERSION_NUMBER_STRING;
const char sw_version_date[12] __attribute__(( at(0x08000194))) = VERSION_DATE_STRING;
#endif

/*************************************************************************************/
/**
* main
* main routine for MCU
*
* @param - none
*
* @return - returns zero

*/
void app_main (void)
{
   bool isSyncMaster = false;
   uint32_t address = 0;
   uint32_t frequencyChannelIndex = 0;
   uint32_t deviceCombination = 0;
   uint32_t unitSerialNo = 0;
   uint32_t defaultZone = 0;
   uint32_t shortFramesPerLongFrame = 0;
   uint32_t fwVersion = 0;
   uint32_t dulchWrap = 0;
   int32_t rssiThreshold = 0;
   ErrorCode_t ErrorCode = SUCCESS_E;
   bool reset_reason_identified = false;
   bool battery_monitor_reset_flag = false;
   #ifdef IWDG_ENABLE
   /* Initialise the Independent Watchdog */
   (void)DM_IndependentWdgInit();
   #endif  

   /* Initialise the CRC module */
   DM_CrcInit();

   //Load the device configuration parameters
   CFG_InitialiseDeviceConfig();
   rssiThreshold = CFG_GetRssiMarSensitivity(false);
   deviceCombination = CFG_GetDeviceCombination();
   defaultZone = CFG_GetZoneNumber();
   dulchWrap = CFG_GetDulchWrap();
   isSyncMaster = CFG_IsSyncMaster();
   
   /* read configuration from NV ram */
   DM_NVMRead(NV_ADDRESS_E, &address, sizeof(address));
   DM_NVMRead(NV_FREQUENCY_CHANNEL_E, &frequencyChannelIndex, sizeof(frequencyChannelIndex));
   DM_NVMRead(NV_SYSTEM_ID_E, &gSystemID, sizeof(gSystemID));
   DM_NVMRead(NV_UNIT_SERIAL_NO_E, &unitSerialNo, sizeof(unitSerialNo));
   DM_NVMRead(NV_SHORT_FRAMES_PER_LONG_FRAME_E, &shortFramesPerLongFrame, sizeof(shortFramesPerLongFrame));
   /*Read FW version from Config Area*/
   DM_NVMRead(NV_FW_VERSION_E, &fwVersion, sizeof(fwVersion));
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//   uint32_t tx_power_low = 0;
//   DM_NVMRead(NV_TX_POWER_LOW_E, &tx_power_low, sizeof(tx_power_low));
//   if ( 7 != tx_power_low )
//   {
//      tx_power_low = 7;
//      DM_NVMWrite(NV_TX_POWER_LOW_E, &tx_power_low, sizeof(tx_power_low));
//   }
//   
//   uint32_t tx_power_hi = 0;
//   DM_NVMRead(NV_TX_POWER_HIGH_E, &tx_power_hi, sizeof(tx_power_hi));
//   if ( 10 != tx_power_hi )
//   {
//      tx_power_hi = 10;
//      DM_NVMWrite(NV_TX_POWER_HIGH_E, &tx_power_hi, sizeof(tx_power_hi));
//   }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
   
   MC_SetNetworkAddress(address);
   MC_SetShortFramesPerLongFrame( shortFramesPerLongFrame );
   MC_SetProgrammedShortFramesPerLongFrame( shortFramesPerLongFrame );
   MC_SetSystemID(gSystemID);
   
   char serial_no_str[32];
   MM_ATDecodeSerialNumber(unitSerialNo, serial_no_str);
   
   /* Initialise Serial Port Module */
   SerialPort_Init();
      
   /* Init Debug */
   CO_SerialDebug_Init();

   if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST) != RESET)
   {
      reset_reason_identified = true;
      battery_monitor_reset_flag = true;
      CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO THE INDEPENDENT WATCHDOG!!!\r\n");
      HAL_Delay(10); // Give some time for the serial Tx to finish
   }

   if (__HAL_RCC_GET_FLAG(RCC_FLAG_LPWRRST) != RESET)
   {
      reset_reason_identified = true;
      battery_monitor_reset_flag = true;
      CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO ILLEGAL LOW POWER MODE!!!\r\n");
      HAL_Delay(10); // Give some time for the serial Tx to finish
   }

   if (__HAL_RCC_GET_FLAG(RCC_FLAG_WWDGRST) != RESET)
   {
      reset_reason_identified = true;
      battery_monitor_reset_flag = true;
      CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO WINDOW WATCHDOG!!!\r\n");
      HAL_Delay(10); // Give some time for the serial Tx to finish
   }

   if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST) != RESET)
   {
      reset_reason_identified = true;
      /* Don't reset the flag as we have to monitor previous status of battery monitor test */ 
      battery_monitor_reset_flag = false; 
      CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO POWER_FAILURE!!!\r\n");
      HAL_Delay(10); // Give some time for the serial Tx to finish
   }

   if (__HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST) != RESET)
   {
      reset_reason_identified = true;
      battery_monitor_reset_flag = true;
      CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO OPTION BYTE LOADING!!!\r\n");
      HAL_Delay(10); // Give some time for the serial Tx to finish
   }

   if (__HAL_RCC_GET_FLAG(RCC_FLAG_FWRST) != RESET)
   {
      reset_reason_identified = true;
      battery_monitor_reset_flag = true;
      CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO FIREWALL!!!\r\n");
      HAL_Delay(10); // Give some time for the serial Tx to finish
   }

  if ((__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST) != RESET) && (sw_reset_msg_indication_flag == SW_RESET_FLAG_KEY))
  {
     /* printf any debug info about the SW reset cause */
     reset_reason_identified = true;
     battery_monitor_reset_flag = true;   
     CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO SOFTWARE RESET!!!\r\n");
     HAL_Delay(10); // Give some time for the serial Tx to finish

     if ( sw_reset_msg_indication_flag == SW_RESET_FLAG_KEY )
     {
         sw_reset_debug_message[sizeof(sw_reset_debug_message)-1u] = '\0';
         CO_PRINT_A_1(DBG_ERROR_E, "%s\n\n\n\n\n\n\n\n\n\r",sw_reset_debug_message);
         HAL_Delay(10); // Give some time for the serial Tx to finish
     }
     else
     {
         CO_PRINT_A_0(DBG_ERROR_E, "Not requested by Cygnus\r\n");
     }
     HAL_Delay(10); // Give some time for the serial Tx to finish
  }
  
  if ( false == reset_reason_identified )
  {
      if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) != RESET)
      {
         battery_monitor_reset_flag = true;
         CO_PRINT_A_0(DBG_ERROR_E, "LAST RESET WAS DUE TO NRST PIN!!!\r\n");
         HAL_Delay(10); // Give some time for the serial Tx to finish
         reset_reason_identified = true;
      }
  }
  
  if( battery_monitor_reset_flag == true)
  {
      // Default to normal start if we fail to read the NVM.
      DM_BAT_RestartFlags_t restartNvmBatFlag = DM_BAT_NORMAL_RESTART_E;
      // Read Battery test status
      restartNvmBatFlag = DM_BAT_GetBatteryRestartFlag();
      CO_PRINT_A_1(DBG_INFO_E, "Read NVM Battery Test status flags: %d\r\n", restartNvmBatFlag);

      CO_PRINT_A_0(DBG_INFO_E, "Clear NVM Battery Test status flags\r\n");
      DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E);
  }


  /* Reset the Indication Flag */
  sw_reset_msg_indication_flag = 0u;
  
  /* Clear reset flags in any cases */
  __HAL_RCC_CLEAR_RESET_FLAGS();
  
  //Load the device configuration parameters
  CFG_InitialiseDeviceConfig();

   // Serial Port Print Test
   CO_PRINT_A_0(DBG_INFO_E, "###########################################\r\n");
   CO_PRINT_A_0(DBG_INFO_E, "Cygnus2 Ready\r\n");
#ifdef SE_BATTERY
  CO_PRINT_A_2(DBG_INFO_E, "Version           : %sse  Date: %s\r\n", VERSION_NUMBER_STRING, sw_version_date);
#else
   CO_PRINT_A_2(DBG_INFO_E, "Version           : %s  Date: %s\r\n", VERSION_NUMBER_STRING, sw_version_date);
#endif
   CO_PRINT_A_1(DBG_INFO_E, "Address           : %d\r\n", address);
   CO_PRINT_A_1(DBG_INFO_E, "Zone              : %d\r\n", defaultZone);
   CO_PRINT_A_1(DBG_INFO_E, "IsSyncMaster      : %d\r\n", (int)isSyncMaster);
   CO_PRINT_A_1(DBG_INFO_E, "FrequencyChannel  : %d\r\n", frequencyChannelIndex);
   CO_PRINT_A_1(DBG_INFO_E, "DeviceCombination : %d\r\n", deviceCombination);
   CO_PRINT_A_1(DBG_INFO_E, "SystemId          : %d\r\n", gSystemID);
   CO_PRINT_A_1(DBG_INFO_E, "Frame Length      : %d\r\n", shortFramesPerLongFrame);
   CO_PRINT_A_1(DBG_INFO_E, "UnitSerialNo      : %s\r\n", serial_no_str);
   //Give the banner time to leave the serial port before continuing
   osDelay(500);
   #ifdef _DEBUG
   CO_PRINT_A_0(DBG_INFO_E, "Build target      : DEBUG\r\n");
   #else 
   CO_PRINT_A_0(DBG_INFO_E, "Build target      : RELEASE\r\n");
   #endif
   #ifndef ENABLE_STOP_MODE 
   CO_PRINT_A_0(DBG_INFO_E, "Stop mode status  : DISABLED\r\n");
   #endif
   #ifndef IWDG_ENABLE
   CO_PRINT_A_0(DBG_INFO_E, "Watchdog status   : DISABLED\r\n");
   #endif
   CO_PRINT_A_1(DBG_INFO_E, "DULCH Wrap        : %d\r\n", dulchWrap);
   if ( false == CFG_GetBatteryCheckEnabled() )
   {
      CO_PRINT_A_0(DBG_INFO_E, "Battery checks : DISABLED\r\n");
   }
   if ( FREQUENCY_BAND_865_E == MC_GetFrequencyBand() )
   {
      CO_PRINT_A_0(DBG_INFO_E, "Frequency Band : 865MHz\r\n");
   }
   else 
   {
      CO_PRINT_A_0(DBG_INFO_E, "Frequency Band : 915MHz\r\n");
   }
   CO_PRINT_A_1(DBG_INFO_E, "JRSSI          : %ddB\r\n", rssiThreshold);
   CO_PRINT_A_0(DBG_INFO_E, "###########################################\n\r");
   
   //Give the banner time to leave the serial port before continuing
   osDelay(200);

   /*Write FW version in Config Area if FW is updated*/
   uint32_t packed_version = encode_version_string((char*)VERSION_NUMBER_STRING );
   if(fwVersion != packed_version)
   {
		 CO_PRINT_A_1(DBG_INFO_E, "Writing New FW Version: 0x%08X\r\n", packed_version);
		 DM_NVMWrite(NV_FW_VERSION_E, &packed_version, sizeof(packed_version));
   }
#ifdef USE_ENCPRYPTED_PAYLOAD
   /* Report crypto library version */
   STM32CryptoLibVer_TypeDef libVersion;
   STM32_CryptoLibraryVersion(&libVersion);
   libVersion.T[2] = 0;/* the crypto lib doesn't null terminate the type string */
#endif // USE_ENCPRYPTED_PAYLOAD

   /* initialise LED module */
   DM_LedInit();
   
   /*initialise the device channels */
   DM_DeviceInitialise();
   
//   for (uint32_t dc = 0; dc < DC_NUMBER_OF_DEVICE_COMBINATIONS_E; dc++)
//   {
//      gBaseType = DM_DeviceGetBaseType(dc, isSyncMaster);
//      switch (gBaseType)
//      {
//         case BASE_NCU_E:
//            CO_PRINT_B_1(DBG_INFO_E,"DC %d = BASE_NCU_E\r\n", dc);
//            break;
//         case BASE_RBU_E:
//            CO_PRINT_B_1(DBG_INFO_E,"DC %d = BASE_RBU_E\r\n", dc);
//            break;
//         case BASE_MCP_E:
//            CO_PRINT_B_1(DBG_INFO_E,"DC %d = BASE_MCP_E\r\n", dc);
//            break;
//         case BASE_IOU_E:
//            CO_PRINT_B_1(DBG_INFO_E,"DC %d = BASE_IOU_E\r\n", dc);
//            break;
//         case BASE_CONSTRUCTION_E:
//            CO_PRINT_B_1(DBG_INFO_E,"DC %d = BASE_CONSTRUCTION_E\r\n", dc);
//            break;
//         case BASE_REPEATER_E:
//            CO_PRINT_B_1(DBG_INFO_E,"DC %d = BASE_REPEATER_E\r\n", dc);
//            break;
//         default:
//            CO_PRINT_B_1(DBG_INFO_E,"DC %d = BASE_UNKNOWN_E\r\n", dc);
//            break;
//      }
//      osDelay(100);
//   }

   gBaseType = DM_DeviceGetBaseType(deviceCombination, isSyncMaster);
   if ( BASE_UNKNOWN_E == gBaseType )
   {
      Error_Handler("FATAL ERROR: Base type unknown");
   }
#ifdef _DEBUG
   else 
   {
      #define MAX_BASE_TYPE (BASE_UNKNOWN_E+1)
      const char boardType[MAX_BASE_TYPE][16] = { {"NCU"}, {"RBU"}, {"MCP"}, {"IOU"}, {"CONSTRUCTION"}, {"REPEATER"}, {"UNKNOWN"} };
      CO_PRINT_B_1(DBG_INFO_E,"Selected base type = %s\r\n", boardType[gBaseType]);
   }
#endif
   
  
   bool noHeadFitted = CFG_GetBlankingPlateFitted();
   BoardConfigureGpio( gBaseType, noHeadFitted );

   /* Initialize LPTIM */
   MX_LPTIM1_Init();
   /* Create the Application pool and queue */
   AppPool = osPoolCreate(osPool(AppPool));
   if ( NULL != AppPool )
   {
      AppQ = osMessageCreate(osMessageQ(AppQ), NULL);
      if ( NULL != AppQ )
      {
         ErrorCode = SUCCESS_E;
      }
      else 
      {
         ErrorCode = ERR_FAILED_TO_CREATE_QUEUE_E;
      }
   }
   else 
   {
      ErrorCode = ERR_FAILED_TO_CREATE_MEMPOOL_E;
   }

   if (SUCCESS_E == ErrorCode)
   {
      ErrorCode = MM_ATHandleTaskInit();
   }
   
   if (SUCCESS_E == ErrorCode)
   {
      /* Configure the Serial Task */
      ErrorCode = MM_ConfigSerialTaskInit(isSyncMaster ? CS_NCU_CONFIGURATION_E : CS_RBU_CONFIGURATION_E);
   }

   //init the PPU handler
   MM_PPU_Initialise();


   if (SUCCESS_E == ErrorCode)
   {
      //Check for PPU Mode request on start-up.  If is not requested, revert to the NVM setting.
      if ( STARTUP_NORMAL == rbu_pp_mode_request )
      {
         /* Check for PPU Mode */
         uint32_t pp_mode_enable = 0;
         DM_NVMRead(NV_PP_MODE_ENABLE_E, &pp_mode_enable, sizeof(uint32_t));
      
         rbu_pp_mode_request = pp_mode_enable;
      
         CO_PRINT_B_1(DBG_INFO_E,"Startup rbu_pp_mode_request=%d\r\n", rbu_pp_mode_request);
         
         //Reset the PPU Mode flag so that we don't get stuck in PPU mode
         uint32_t start_mode = STARTUP_NORMAL;
         DM_NVMWrite(NV_PP_MODE_ENABLE_E, &start_mode, sizeof(uint32_t));
      }
      
      if ( STARTUP_PPU_DISCONNECTED == rbu_pp_mode_request )
      {
         MM_PPU_SetPpuMasterAddress(ADDRESS_GLOBAL);
         MC_SetPpuMode(PPU_STATE_AUTO_DISCONNECT_E);
      }
      else if ( STARTUP_PPU_CONNECTED == rbu_pp_mode_request )
      {
         MM_PPU_SetPpuMasterAddress(rbu_pp_master_address);
         MC_SetPpuMode(PPU_STATE_AUTO_CONNECT_E);
   }
   }

   /* Reset the PP code for the next start-up */
   rbu_pp_mode_request = STARTUP_NORMAL;

#ifdef I2C_SVI_BOARD_SIMULATION   
   if (SUCCESS_E == ErrorCode)
   {
      ErrorCode = DM_I2C_Initialise();
   }
   /* SVI simulation mode */
   if (SUCCESS_E == ErrorCode)
   {
      ErrorCode = DM_I2C_Startup(I2C_SVI_ADDRESS);
   }   
#else

   if (SUCCESS_E == ErrorCode)
   {
      if (true == DM_DeviceIsEnabled(deviceCombination, DEV_SVI_DEVICES, DC_MATCH_ANY_E))
      {

         if (SUCCESS_E != DM_I2C_Initialise())
         {
            CO_PRINT_A_0(DBG_ERROR_E,"Failed to start I2C\r\n");
         }
      }
   }
   

   // Check range of frequency channel index. If not in range use default value
   if (NUM_HOPPING_CHANS <= frequencyChannelIndex)
   {
      frequencyChannelIndex = 0;
   }
   
   if (SUCCESS_E == ErrorCode)
   {
      ErrorCode = MM_MeshTaskInit(isSyncMaster, address, gSystemID);
   }

   /* Initialise tasks */
   if (SUCCESS_E == ErrorCode)
   {
      ErrorCode = MM_MACTaskInit(isSyncMaster, address, gSystemID);
   }
      
   if ( SUCCESS_E == ErrorCode )
   {
#ifdef USE_ENCPRYPTED_PAYLOAD
      CO_PRINT_B_4(DBG_INFO_E,"STM32 Cryptographic Library Version %d.%d.%d %s\r\n", libVersion.X, libVersion.Y, libVersion.Z, libVersion.T);
#endif
      
      if (true == isSyncMaster)
      {
         /* initialise NCU app stub */
         ErrorCode = MM_NCUApplicationTaskInit((uint16_t)address,(uint16_t)deviceCombination, unitSerialNo, defaultZone);
         if ( SUCCESS_E == ErrorCode )
         {
            MM_NCUApplicationTaskMain();//Does not return
         }
      }
      else
      {
         if ( SUCCESS_E == ErrorCode )
         {      
            if( SUCCESS_E == MM_RBUApplicationTaskInit((uint16_t)address,(uint16_t)deviceCombination, unitSerialNo, defaultZone))
            {
               if (SUCCESS_E == ErrorCode)
               {
                  // Initialise HEAD interface - Standard RBU or sounder only
                  if (true == DM_DeviceIsEnabled(deviceCombination, DEV_HEAD_DEVICES, DC_MATCH_ANY_E))
                  {
                     if ( SUCCESS_E != MM_HeadInterfaceTaskInit((uint16_t)deviceCombination) )
                     {
                        CO_PRINT_A_0(DBG_ERROR_E,"Failed to start Head Interface task\r\n");
                     }
                  }
                  
                  // check the config for call points.
                  bool fire_mcp = DM_DeviceIsEnabled(deviceCombination, DEV_FIRE_CALL_POINT, DC_MATCH_ANY_E);
                  bool first_aid_mcp = DM_DeviceIsEnabled(deviceCombination, DEV_FIRST_AID_CALL_POINT, DC_MATCH_ANY_E);
                  
                  ErrorCode = MM_GpioTaskInit(fire_mcp, first_aid_mcp);
                  
                  if ( SUCCESS_E == ErrorCode )
                  {
                     ErrorCode = MM_TimedEventTaskInit();
                     if ( SUCCESS_E == ErrorCode )
                     {
                        MM_RBUApplicationTaskMain();//Does not return
                     }
                     else 
                     {
                        CO_PRINT_A_0(DBG_ERROR_E,"Failed to start Timed Event task\r\n");
                     }
                  }
                  else
                  {
                     CO_PRINT_A_0(DBG_ERROR_E,"Failed to start GPIO task\r\n");
                  }
                  
               }
            }
            else 
            {
               CO_PRINT_A_0(DBG_ERROR_E,"Failed to start RBU Application task\r\n");
               ErrorCode = ERR_INIT_FAIL_E;
            }
         }
      }
   }

#endif  

//   Error_Handler("Debug reset");
   // If any of the functions didn't returned SUCCESS_E, don't return from this function.
   if (SUCCESS_E != ErrorCode)
   {
      // Error trap.
      CO_PRINT_B_0(DBG_ERROR_E, "Code sitting in error trap in MM_Main.c, main()");
      while(1);
   }
}

/*******************************************************************************
Function name : void Error_Handler(void)
   returns    : nothing
Description   : This function is executed in case of error occurrence.
Notes         : restrictions, odd modes
*******************************************************************************/
void Error_Handler(char* error_msg)
{
   /* Capture debug information */
   if (strlen(error_msg) < (sizeof(sw_reset_debug_message) - 1u))
   {
      /* Copy the error message in the non initialised area */
      strcpy(sw_reset_debug_message, error_msg);
   }
   else
   {
      /* Copy the error message in the non initialised area 
       * Message is too long so truncate it
       */
      memcpy(sw_reset_debug_message, error_msg, sizeof(sw_reset_debug_message) - 1);
      sw_reset_debug_message[sizeof(sw_reset_debug_message) - 1] = '\0';
   }

   /* Set the Indication Flag */
   sw_reset_msg_indication_flag = SW_RESET_FLAG_KEY;
   
   /* This is an Error reset, skip the PP operatio next start-up */
//   rbu_pp_mode_request = RBU_SKIP_PP_MODE_REQ;
   
   /* Wait for the end of any pending EEPROM writing operation */
   FLASH_WaitForLastOperation(FLASH_TIMEOUT_VALUE);
   
   //a delay for all debug messages to be sent
   osDelay(1000);

   /* Reset the system now */
   HAL_NVIC_SystemReset();
}
