/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : MM_PpuManagement.h
*
*  Description  : Source file for handling PPU messages
*
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "cmsis_os.h"


/* User Include Files
*************************************************************************************/
#include "MM_Main.h"
#include "MM_MeshAPI.h"
#include "MC_MacConfiguration.h"
#include "DM_RNG.h"
#include "DM_LED.h"
#include "MC_StateManagement.h"
#include "MM_ApplicationCommon.h"
#include "MM_PpuManagement.h"

/* Private definitions
*************************************************************************************/
#define PPU_ROUTE_DROP_DELAY  10u
#define PPU_TIMEOUT 7200u     // 2 Hours

/* Private Functions Prototypes
*************************************************************************************/
static void MM_PPU_SendPpuVersion(const ApplicationMessage_t* pAppMessage);
static void MM_PPU_ProcessConfigurationCommand(const ApplicationMessage_t* pAppMessage);
static bool MM_PPU_Connect(const ApplicationMessage_t* pAppMessage);
static bool MM_PPU_Disconnect(const ApplicationMessage_t* pAppMessage);
static void MM_PPU_Reset(const uint32_t command, const uint16_t ppuMaster);
static bool MM_PPU_SecurityCheck(const uint16_t command);
static void MM_PPU_AcknowledgeMessage(const PPU_Message_t* pPpuCmd);

/* Global Variables
*************************************************************************************/
extern HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);
extern osPoolId AppPool;
extern osMessageQId(AppQ);
extern uint32_t rbu_pp_master_address;

/* Global Functions
*************************************************************************************/
extern void Error_Handler(char* error_msg);

/* Private Variables
*************************************************************************************/
static uint32_t gPpuAnnouncementCount = PPU_ANNOUNCEMENT_DEFAULT_PERIOD;
static uint32_t gPpuMasterAddress = 0;
static bool gSecurityMode;
static uint32_t gRouteDropHandle = 0;
static uint32_t gRouteDropCount = 0;
static uint32_t gTimeoutCount = 0;

/*************************************************************************************/
/**
* MM_PPU_Initialise
* Initialise the neighbour information data
*
* param - void
*
* return - void
*/
void MM_PPU_Initialise( void )
{
   MC_SetPpuMode( PPU_STATE_NOT_ACTIVE_E );
   gPpuAnnouncementCount = PPU_ANNOUNCEMENT_DEFAULT_PERIOD;
   gPpuMasterAddress = ADDRESS_GLOBAL;
   gRouteDropHandle = 0;
   gSecurityMode = true;         //!!DEBUG should be false but security handshake not implemented yet
   gTimeoutCount = 0;
}

void MM_PPU_SetPpuMasterAddress(const uint16_t address)
{
   gPpuMasterAddress = address;
}
/*************************************************************************************/
/**
* function name   : MM_PPU_SecurityCheck
* description     : Checks the security bit in the supplied command and verifies that
*                   the RBU is in secure mode if the bit is set.
*
* @param - command   The command to check
*
* @return - True if security check passes
*/
bool MM_PPU_SecurityCheck(const uint16_t command)
{
   bool allow_command = false;
   
   if ( command & PPU_STATUS_BIT_SECURITY )
   {
      //the security bit is set.  check for security mode
      if ( gSecurityMode )
      {
         allow_command = true;
      }
   }
   else 
   {
      //the security bit is not set
      allow_command = true;
   }
   
   return allow_command;
}

/*************************************************************************************/
/**
* MM_PPU_ApplicationProcessPpuModeMessage
* Function to receive the command to enter Disconnected PPU mode from the Mesh.
*
* @param pAppMessage   The contents of the message.
*
* @return - bool        SUCCESS_E if the MAC was set for PPU Mode.  Error code on failure.
*/
ErrorCode_t MM_PPU_ApplicationProcessPpuModeMessage(const ApplicationMessage_t* pAppMessage)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   CO_PRINT_B_0(DBG_INFO_E,"MM_PPU_ApplicationProcessPpuModeMessage\r\n");
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_PPU_MODE_E == pAppMessage->MessageType )
      {
         //Check that we are not already in PPU mode
         if ( PPU_STATE_NOT_ACTIVE_E == MC_GetPpuMode() )
         {
            //If we are active on the mesh we need to send a ROUTE DROP message and defer moving to PPU Disconnected until
            //the message confirmation comes back
            if ( STATE_ACTIVE_E == MC_STATE_GetDeviceState() )
            {
               MC_SetPpuMode( PPU_STATE_REQUESTED_E );
               //Start the 10 second countdown to reset.  Route drops are sent at 5 secoonds.
               gRouteDropCount = PPU_ROUTE_DROP_DELAY;
               CO_PRINT_B_0(DBG_INFO_E,"PPU Mode - ROUTE DROP requested\r\n");
//               //Send a route drop message, global address.
//               gRouteDropHandle = GetNextHandle();
//               MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_REQUESTED_E, PPU_SEND_ROUTE_DROP_E, 0, gRouteDropHandle);
               result = SUCCESS_E;
            }
            else 
            {
               CO_PRINT_B_0(DBG_INFO_E,"PPU DISCONNECTED mode resquested\r\n");
               //Not active on the mesh.
               //Tell the MAC to switch to PPU Mode
               MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_DISCONNECTED_E, PPU_START_DISCONNECTED_MODE, gPpuMasterAddress, 0);
               result = SUCCESS_E;
            }
         }
         else 
         {
            CO_PRINT_B_1(DBG_INFO_E,"PPU Mode change fail.  Current state = %d\r\n", MC_GetPpuMode());
            result = ERR_MESSAGE_DUPLICATE_E;
         }
      }
      else 
      {
         CO_PRINT_B_1(DBG_INFO_E,"PPU Mode change Rx'd bad msg type = %d\r\n", pAppMessage->MessageType);
         result = ERR_INVALID_PARAMETER_E;
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E,"PPU Mode change Rx'd NULL msg\r\n");
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_ApplicationProcessPpuPeriodicAnnouncement
* Function to generate the PPU announcement message at the appropriate time.
*
* @param None.
*
* @return - void
*/
void MM_PPU_ApplicationProcessPpuPeriodicAnnouncement(void)
{
   //Check that we are in PPU Disconnected mode
   if ( PPU_STATE_DISCONNECTED_E == MC_GetPpuMode() )
   {
      //increment the count
      gPpuAnnouncementCount--;
      if ( 0 == gPpuAnnouncementCount )
      {
         CO_PRINT_B_0(DBG_INFO_E,"PPU Anounce\r\n");
         //Time to send the announcement
         MM_PPU_SendPpuPeriodicAnnouncement();
         //Reset the count.  
         gPpuAnnouncementCount = DM_RNG_GetRandomNumber(PPU_ANNOUNCEMENT_PERIOD_SPAN);
         if ( gPpuAnnouncementCount )
         {
            gPpuAnnouncementCount += PPU_ANNOUNCEMENT_MIN_PERIOD;
         }
         else
         {
            gPpuAnnouncementCount = PPU_ANNOUNCEMENT_DEFAULT_PERIOD;
         }
      }
   }
 }

 /*************************************************************************************/
/**
* MM_ApplicationProcessPpuPeriodicAnnouncement
* Function to send the PPU announcement message.
*
* @param None.
*
* @return - bool   True if the message is sent to the Mesh
*/
bool MM_PPU_SendPpuPeriodicAnnouncement(void)
 {
   return MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_DISCONNECTED_E, PPU_ANNOUNCEMENT_SEND, gPpuMasterAddress, 0);
 }
 
/*************************************************************************************/
/**
* MM_ApplicationProcessPpuCommand
* Function to handle commands from the PPU Master.
*
* @param pAppMessage   The command message.
*
* @return - void
*/
void MM_PPU_ApplicationProcessPpuCommand(const ApplicationMessage_t* pAppMessage)
{
   bool force_ack = false;
   bool ppu_master_valid = false;
   PPU_Message_t* pCommand;
   uint16_t command;
   
   if ( pAppMessage )
   {
      if ( APP_MSG_TYPE_PPU_COMMAND_E == pAppMessage->MessageType )
      {
         //A message has been received.  Reset the timeout counter.
         gTimeoutCount = 0;
         
         pCommand = (PPU_Message_t*)&pAppMessage->MessageBuffer;
         
         //check that the mesage is from the connected PPU Master
         command = pCommand->Command & PPU_COMMAND_MASK;
         if ( MM_PPU_SecurityCheck(command) )
         {
            switch ( command )
            {
               case PPU_CMD_PPU_VERSION:
                     MM_PPU_SendPpuVersion(pAppMessage);
                  break;
                  
               case PPU_CMD_PPU_RESET_TO_BOOT1: // intentional drop-through
               case PPU_CMD_PPU_RESET_TO_BOOT2:
               case PPU_CMD_PPU_RESET_NORMAL:
               case PPU_CMD_PPU_RESET_DISCONNECTED:
               case PPU_CMD_PPU_RESET_CONNECTED:
               case PPU_CMD_PPU_RESET:
                     MM_PPU_Reset(pCommand->Command, pCommand->PpuAddress);
                  break;
                  
               case PPU_CMD_CONFIG:
                     MM_PPU_ProcessConfigurationCommand(pAppMessage);
                  break;
                  
               case PPU_CMD_PING:
                     force_ack = true;
                     ppu_master_valid = true;
                  break;
                  
               case PPU_CMD_PPU_CONNECT:
                   ppu_master_valid = MM_PPU_Connect(pAppMessage);
                  break;
               
               case PPU_CMD_PPU_DISCONNECT:
                   MM_PPU_Disconnect(pAppMessage);
                  break;
               
               default:
                  //Unknown command
                  CO_PRINT_B_1(DBG_ERROR_E,"App Rxd unknown PPU command %04X\r\n", pCommand->Command);
                  break;
            }
            
            if ( ppu_master_valid && ((pCommand->Command & PPU_STATUS_BIT_ACK_REQ) || force_ack) )
            {
               MM_PPU_AcknowledgeMessage(pCommand);
            }
         }
         else 
         {
            CO_PRINT_B_1(DBG_ERROR_E,"Rejected msg - not is security mode %04X\r\n", command);
         }
      }
      else 
      {
         CO_PRINT_B_1(DBG_ERROR_E,"MM_PPU_ApplicationProcessPpuCommand Rxd wrong MsgType %04X\r\n", pAppMessage->MessageType);
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"MM_PPU_ApplicationProcessPpuCommand Rxd NULL msg\r\n");
   }
}


/*************************************************************************************/
/**
* MM_PPU_AcknowledgeMessage
* Function to send a ping response to the PPU Master.
*
* @param pAppMessage   The command message.
*
* @return - void
*/
void MM_PPU_AcknowledgeMessage(const PPU_Message_t* pPpuCmd)
{
   PPU_Message_t response;
   bool send_response = false;
   
   if ( pPpuCmd )
   {
      //We should not respond if we are not in PPU mode
      if ( PPU_STATE_NOT_ACTIVE_E != MC_GetPpuMode() )
      {
         //use the PPU Master address from the command
         response.PpuAddress = pPpuCmd->PpuAddress;
         //Do we use network addressing or serial number?
         if ( pPpuCmd->RbuAddress == MC_GetNetworkAddress() )
         {
            CO_PRINT_B_1(DBG_INFO_E,"App Rxd msg from address %d \r\n", pPpuCmd->PpuAddress);
            //Using the network address and system ID
            response.RbuAddress = MC_GetNetworkAddress();
            response.SystemID = MC_GetSystemID();
            send_response = true;
         }
         else if ( pPpuCmd->SerialNumber == MC_GetSerialNumber() )
         {
            CO_PRINT_B_1(DBG_INFO_E,"App Rxd msg from address %d \r\n", pPpuCmd->PpuAddress);
            response.SerialNumber = MC_GetSerialNumber();
            send_response = true;
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E,"App Rxd msg - no address match\r\n");
         }
         
         if ( send_response )
         {
            response.Command = pPpuCmd->Command | PPU_STATUS_BIT_ACK;
            response.PacketLength = PPU_MESSAGE_HEADER_SIZE;
#ifndef PPU_EMBEDDED_CHECKSUM
            response.PacketLength+=2;
#endif
            MM_MeshAPIGeneratePpuMessage(&response);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_PPU_Connect
* Function to change the PPU state from 'disconnected' to 'connected.'.
*
* @param pAppMessage   The connect message from the PPU Master.
*
* @return - bool   True if the connect command was accepted
*/
bool MM_PPU_Connect(const ApplicationMessage_t* pAppMessage)
{
   bool connection_accepted = false;
   PPU_Message_t* pPpuCmd;
   
   if ( pAppMessage )
   {
      pPpuCmd = (PPU_Message_t*)pAppMessage->MessageBuffer;
      //Make sure that we are not already connected to a different unit.  Only accept 'connect'
      //messages if the PPU Master address is not set (ADDRESS_GLOBAL) or the same Master is trying to reconnect.
      if ( (ADDRESS_GLOBAL == gPpuMasterAddress) || (gPpuMasterAddress == pPpuCmd->PpuAddress) )
      {
         //We should be in PPU Disonnected mode, or PPU Connected to the same PPU Master (reconnecting)
         if ( PPU_STATE_DISCONNECTED_E == MC_GetPpuMode() ||
            ((PPU_STATE_CONNECTED_E == MC_GetPpuMode()) && (gPpuMasterAddress == pPpuCmd->PpuAddress)) )
         {
            gPpuMasterAddress = pPpuCmd->PpuAddress;
            CO_PRINT_B_1(DBG_INFO_E,"PPU Connected to Master %d\r\n", gPpuMasterAddress);
            //Tell the MAC we are in PPU Connected mode
            MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_CONNECTED_E, PPU_START_CONNECTED_MODE, gPpuMasterAddress, 0);
            connection_accepted = true;
            MC_SetPpuMode(PPU_STATE_CONNECTED_E);
         }
         else 
         {
            CO_PRINT_B_1(DBG_INFO_E,"PPU Connect not in disconnected mode.  mode=%d\r\n", MC_GetPpuMode());
         }
      }
      else 
      {
         CO_PRINT_B_2(DBG_INFO_E,"MM_PPU_Connect rejecting connect on previous master address. prev=%X, msg=%X\r\n", gPpuMasterAddress, pPpuCmd->PpuAddress);
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E,"PPU Connect Rxd NULL msg\r\n");
   }
   
   return connection_accepted;
}

/*************************************************************************************/
/**
* MM_PPU_Disconnect
* Function to change the PPU state from 'connected' to 'disconnected'
*
* @param pAppMessage   The connect message from the PPU Master.
*
* @return - bool   True if the connect command was accepted
*/
bool MM_PPU_Disconnect(const ApplicationMessage_t* pAppMessage)
{
   bool connection_dropped = false;
   PPU_Message_t* pPpuCmd;
   
   if ( pAppMessage )
   {
      pPpuCmd = (PPU_Message_t*)pAppMessage->MessageBuffer;
      //Make sure that we are not already connected to a different unit.  Only accept 'connect'
      //messages if the PPU Master address is set (ADDRESS_GLOBAL) or the same Master is trying to reconnect.
      if ( (ADDRESS_GLOBAL == gPpuMasterAddress) || (gPpuMasterAddress == pPpuCmd->PpuAddress) )
      {
         if ( gPpuMasterAddress == pPpuCmd->PpuAddress )
         {
            //We should be in PPU Connected mode, with the same PPU Master
            if ( PPU_STATE_CONNECTED_E == MC_GetPpuMode() )
            {
               CO_PRINT_B_1(DBG_INFO_E,"PPU Disconnecting from Master %d\r\n", gPpuMasterAddress);
               //Tell the MAC we are in PPU Connected mode
               if ( MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_CONNECTED_E, PPU_START_DISCONNECTED_MODE, gPpuMasterAddress, 0) )
               {
                  gPpuMasterAddress = ADDRESS_GLOBAL;
                  MC_SetPpuMode(PPU_STATE_DISCONNECTED_E);
               }
               connection_dropped = true;
            }
            else 
            {
               CO_PRINT_B_1(DBG_INFO_E,"PPU Disconnect, not in connected mode.  mode=%d\r\n", MC_GetPpuMode());
            }
         }
         else 
         {
            CO_PRINT_B_2(DBG_INFO_E,"PPU Disconnect, PPU Master address not matching.  master=%d, msg=%d\r\n", gPpuMasterAddress, pPpuCmd->PpuAddress);
         }
      }
      else 
      {
         CO_PRINT_B_2(DBG_INFO_E,"MM_PPU_Disconnect rejecting connect on previous master address. prev=%X, msg=%X\r\n", gPpuMasterAddress, pPpuCmd->PpuAddress);
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E,"PPU Disconnect Rxd NULL msg\r\n");
   }
   
   return connection_dropped;
}

/*************************************************************************************/
/**
* MM_PPU_Reset
* Function to reset the device from PPU Mode.
*
* @param command     The type of reset that is required.
* @param ppuMaster   The address of the connected PPU Master.
*
* @return - void
*/
void MM_PPU_Reset(const uint32_t command, const uint16_t ppuMaster)
{
   bool perform_reset = false;
   
   //We should not respond if we are not in PPU mode
   if ( PPU_STATE_NOT_ACTIVE_E != MC_GetPpuMode() )
   {
      switch ( command )
      {
         case PPU_CMD_PPU_RESET_TO_BOOT2:
            CO_PRINT_B_0(DBG_ERROR_E,"PPU Reset- Radio Update\r\n");
            rbu_pp_mode_request = STARTUP_RADIO_BOOTLOADER;
            rbu_pp_master_address = (uint32_t)ppuMaster;
            perform_reset = true;
         break;
         case PPU_CMD_PPU_RESET_TO_BOOT1:
            CO_PRINT_B_0(DBG_ERROR_E,"PPU Reset- Serial Update\r\n");
            rbu_pp_mode_request = STARTUP_SERIAL_BOOTLOADER;
            rbu_pp_master_address = (uint32_t)ADDRESS_GLOBAL;
            perform_reset = true;
         break;
         case PPU_CMD_PPU_RESET_NORMAL:// Intentional drop-through
         case PPU_CMD_PPU_RESET:
            CO_PRINT_B_0(DBG_ERROR_E,"PPU Reset- Normal restart\r\n");
            rbu_pp_mode_request = STARTUP_NORMAL;
            rbu_pp_master_address = (uint32_t)ppuMaster;
            perform_reset = true;
            break;
         case PPU_CMD_PPU_RESET_CONNECTED:
            CO_PRINT_B_0(DBG_ERROR_E,"PPU Reset- Connected Mode\r\n");
         rbu_pp_mode_request = STARTUP_PPU_CONNECTED;//!!TODO: No need to reset for this change
            rbu_pp_master_address = (uint32_t)ppuMaster;
            perform_reset = true;
            break;
         case PPU_CMD_PPU_RESET_DISCONNECTED:
            CO_PRINT_B_0(DBG_ERROR_E,"PPU Reset- Disconnected Mode\r\n");
            rbu_pp_mode_request = STARTUP_PPU_DISCONNECTED;//!!TODO: No need to reset for this change
            rbu_pp_master_address = (uint32_t)ADDRESS_GLOBAL;
            perform_reset = true;
            break;
         default:
            //Unknown command
            CO_PRINT_B_1(DBG_ERROR_E,"Unknown PPU reset command %04X\r\n", command);
            break;
      }
      
      if ( perform_reset )
      {
         /* Request a micro reset */
         Error_Handler("Reset by PPU command");
      }
   }
}

/*************************************************************************************/
/**
* MM_PPU_SendPpuVersion
* Function to send the PPU version to the PPU Master.
*
* @param pAppMessage   The command message.
*
* @return - void
*/
void MM_PPU_SendPpuVersion(const ApplicationMessage_t* pAppMessage)
{
   // Unspecified action
   CO_PRINT_B_0(DBG_ERROR_E,"PPU Version Request\r\n");
}

/*************************************************************************************/
/**
* MM_PPU_ProcessConfigurationCommand
* Function to process AT commands from the PPU Master over the radio.
*
* @param pAppMessage   The command message.
*
* @return - void
*/
void MM_PPU_ProcessConfigurationCommand(const ApplicationMessage_t* pAppMessage)
{
   static ApplicationMessage_t appMessage;
   static AtMessageData_t msgData;
   PPU_Message_t* pPpuCmd;
   CO_Message_t* pMessage;
   osStatus osStat;

   if ( pAppMessage )
   {
      //We should not respond if we are not in PPU mode
      if ( PPU_STATE_NOT_ACTIVE_E != MC_GetPpuMode() )
      {
         pPpuCmd = (PPU_Message_t*)pAppMessage->MessageBuffer;
         //Do we use network addressing or serial number?
         if ( pPpuCmd->RbuAddress == MC_GetNetworkAddress() )
         {
            CO_PRINT_B_1(DBG_INFO_E,"PPU AT Command = %s\r\n", pPpuCmd->Payload);
            osDelay(100);
            //MM_RBUApplicationProcessAtCommandMessage( pAppMessage );
            appMessage.MessageType = APP_MSG_TYPE_AT_COMMAND_E;
            appMessage.port = MESH_INTERFACE_E;
            appMessage.Source = pPpuCmd->PpuAddress;
            
            msgData.Destination = pPpuCmd->RbuAddress;
            msgData.IsCommand = true;
            msgData.Source = pPpuCmd->PpuAddress;
            memset(msgData.Command, 0 , MAX_PPU_COMMAND_PAYLOAD_LENGTH);
            memcpy(msgData.Command, pPpuCmd->Payload, MAX_OTA_AT_COMMAND_LENGTH);
            memset(appMessage.MessageBuffer, 0 , (PHY_DATA_IND_LENGTH_MAX - 1));
            memcpy( appMessage.MessageBuffer, (uint8_t*)&msgData, sizeof(AtMessageData_t));
            
            /* create a status message and put into the App queue */
            pMessage = osPoolCAlloc(AppPool);
            if ( pMessage )
            {
               pMessage->Type = CO_MESSAGE_PHY_DATA_IND_E;
               memcpy(pMessage->Payload.PhyDataInd.Data, (uint8_t*)&appMessage, sizeof(ApplicationMessage_t));
               osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(AppPool, pMessage);
               }
            }
         }
         else 
         {
            CO_PRINT_B_1(DBG_ERROR_E,"PPU AT Command address missmatch %d\r\n", pPpuCmd->RbuAddress);
         }
      }
      else 
      {
         CO_PRINT_B_0(DBG_ERROR_E,"PPU AT Command - not in PPU mode\r\n");
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"MM_PPU_ProcessConfigurationCommand Rxd NULL msg\r\n");
   }
}


/*************************************************************************************/
/**
* MM_PPU_ProcessAtResponseMessage
* Function to process AT responses from the Application for sending over the radio.
*
* @param pAtMessage   The AT Response message.
*
* @return - void
*/
void MM_PPU_ProcessAtResponseMessage(const AtMessageData_t* pAtMessage)
{
   uint32_t payload_length;
   if ( pAtMessage )
   {
      //check that we are in PPU Connected mode
      if ( PPU_STATE_CONNECTED_E == MC_GetPpuMode() )
      {
         payload_length = strlen((char *)pAtMessage->Command);
         if ( MAX_PPU_COMMAND_PAYLOAD_LENGTH < payload_length )
         {
            payload_length = MAX_PPU_COMMAND_PAYLOAD_LENGTH;
         }
         //build a PPU packet and send it to the mesh
         PPU_Message_t AtResponse;
         AtResponse.PpuAddress = gPpuMasterAddress;
         AtResponse.RbuAddress = MC_GetNetworkAddress();
         AtResponse.SystemID = MC_GetSystemID();
         AtResponse.Checksum = 0;
         AtResponse.PacketLength = PPU_MESSAGE_HEADER_SIZE + strlen((char *)pAtMessage->Command);
#ifndef EMBEDDED_CHECKSUM
         AtResponse.PacketLength += 2;
#endif
         AtResponse.Command = PPU_CMD_CONFIG | PPU_STATUS_BIT_SECURITY;
         memset((char *)AtResponse.Payload, 0, MAX_PPU_COMMAND_PAYLOAD_LENGTH);
         memcpy((char *)AtResponse.Payload, (char *)pAtMessage->Command, payload_length);
         
         if ( MM_MeshAPIGeneratePpuMessage(&AtResponse) )
         {
            CO_PRINT_B_1(DBG_INFO_E,"Sending AT Response : %s\r\n", (char *)AtResponse.Payload);
         }
         else 
         {
            CO_PRINT_B_0(DBG_ERROR_E,"FAILED to send AT Response");
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_PPU_ProcessRouteDropConfirmation
* Called when the App receives a route drop confirmation from the mesh.
* Initiates a countdown to entering PPU Disconnected mode if the transaction number
* matches the route drop request message.
*
* @param handle   The transaction number of the route drop message.
*
* @return - void
*/
void MM_PPU_ProcessRouteDropConfirmation(const uint32_t handle)
{
   if ( PPU_STATE_REQUESTED_E == MC_GetPpuMode() )
   {
      if ( gRouteDropHandle == handle )
      {
         //gRouteDropCount = PPU_ROUTE_DROP_DELAY;
         CO_PRINT_B_1(DBG_INFO_E,"Route Drop confirmed.  Handle=%d\r\n", gRouteDropHandle);
         gRouteDropHandle = 0;
      }
   }
}


/*************************************************************************************/
/**
* MM_PPU_PeriodicChecks
* Performs periodic checks related to PPU.
*
* @param - void.
*
* @return - void.
*/
void MM_PPU_PeriodicChecks(void)
{
   PpuState_t ppu_mode = MC_GetPpuMode();
   //Check countdown to entering PPU Disconnect mode after sending a ROUTE DROP
   if ( PPU_STATE_REQUESTED_E == ppu_mode )
   {
      if ( 0 < gRouteDropCount )
      {
         gRouteDropCount--;
         if ( 0 == gRouteDropCount )
         {
            //Enter PPU Disconnected mode
            //MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_DISCONNECTED_E, PPU_START_DISCONNECTED_MODE, gPpuMasterAddress, 0);
					  //rbu_pp_mode_request = STARTUP_PPU_DISCONNECTED;
					 //------------------------------------------------------------------------
					 
					  CO_PRINT_B_0(DBG_ERROR_E,"PPU Reset- Radio Update\r\n");
            rbu_pp_mode_request = STARTUP_RADIO_BOOTLOADER;
            rbu_pp_master_address = (uint32_t)0xFFFF;
					 
            Error_Handler("PPU mode requested by CIE\r\n");
         }
         else if ( 5 == gRouteDropCount )
         {
            // Five seconds before reset, send a route drop message, global address.
            gRouteDropHandle = GetNextHandle();
            MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_REQUESTED_E, PPU_SEND_ROUTE_DROP_E, 0, gRouteDropHandle);
         }
         else 
         {
            CO_PRINT_B_1(DBG_INFO_E,"PPU REQUESTED route drop count = %d\r\n", gRouteDropCount);
         }
      }
   }
   else if ( PPU_STATE_DISCONNECTED_E == ppu_mode )
   {
      //We are in PPU Disconnected mode, this will send the announcement messages
      MM_PPU_ApplicationProcessPpuPeriodicAnnouncement();
   }
   
   if ( PPU_STATE_AUTO_DISCONNECT_E == ppu_mode )
   {
      //We need to transition to PPU Connected mode.  Send PPU_START_DISCONNECTED_MODE
      MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_DISCONNECTED_E, PPU_START_DISCONNECTED_MODE, gPpuMasterAddress, 0);
   }
   else if ( PPU_STATE_AUTO_CONNECT_E == ppu_mode )
   {
      //We need to transition to PPU Connected mode.  Send PPU_START_DISCONNECTED_MODE followed by PPU_START_CONNECTED_MODE
      MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_DISCONNECTED_E, PPU_START_DISCONNECTED_MODE, gPpuMasterAddress, 0);
      MM_MeshAPIApplicationCommand(APP_MSG_TYPE_PPU_MODE_E, PPU_STATE_CONNECTED_E, PPU_START_CONNECTED_MODE, gPpuMasterAddress, 0);
   }
   
   if ( (PPU_STATE_DISCONNECTED_E == ppu_mode) || (PPU_STATE_CONNECTED_E == ppu_mode) )
   {
      gTimeoutCount++;
      if ( PPU_TIMEOUT < gTimeoutCount )
      {
         gTimeoutCount = 0;
         //No message has arrived for the trimeout period.  Reset the RBU.
         Error_Handler("PPU Timeout");
      }
   }
}
