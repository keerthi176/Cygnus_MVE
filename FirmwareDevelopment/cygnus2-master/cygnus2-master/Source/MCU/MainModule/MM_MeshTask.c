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
*  File         : MM_MeshTask.c
*
*  Description  : Mesh Task functions
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "MM_MeshTask.h"
#include "SM_StateMachine.h"
#include "CO_Message.h"
#include "CO_ErrorCode.h"
#include "CO_Defines.h"
#include "MC_PUP.h"
#include "DM_NVM.h"
#include "MC_MAC.h"
#include "DM_SerialPort.h"
#include "DM_Log.h"
#include "MC_SessionManagement.h"
#include "board.h"
#include "MC_SACH_Management.h"
#include "MC_MeshFormAndHeal.h"
#include "MC_Encryption.h"
#include "MC_MacConfiguration.h"
#include "MC_TDM.h"

/* Private macros
*************************************************************************************/
#if (defined _DEBUG) 
   #define CO_LOG_PHY_DATA_REQ(slot, addr, freq, data, size) DM_LogPhyDataReq(slot, addr, freq, data, size)
   #define CO_LOG_PHY_DATA_IND(slot, addr, freq, rssi, snr, data, size, freq_dev) DM_LogPhyDataInd(slot, addr, freq, rssi, snr, data, size, freq_dev)
#else
   #define CO_LOG_PHY_DATA_REQ(slot, addr, freq, data, size) // Do nothing
   #define CO_LOG_PHY_DATA_IND(slot, addr, freq, rssi, snr, data, size, freq_dev) // Do nothing
#endif

/* Private Functions Prototypes
*************************************************************************************/
void MM_MeshTaskMain (void const *argument);
static ErrorCode_t MM_MapMessageToEvent(FrameType_t frameType,
                                        bool appMsgPresent,
                                        ApplicationLayerMessageType_t appMsgType,
                                        SM_Event_t *pSmEvent);
static void ReportFailedMessage(const CO_Message_t* pMsg, const ErrorCode_t reason);


/* Global Variables
*************************************************************************************/
uint32_t gMeshPoolCount = 0;
uint32_t gNetworkAddress = 0xffffffff;

#define MESH_POOL_SIZE 30
#define MESH_Q_SIZE    20

extern osMessageQId(DCHQ);
extern osMessageQId(RACHPQ);
extern osMessageQId(RACHSQ);
extern osMessageQId(ACKQ);
extern osMessageQId(DLCCHPQ);
extern osMessageQId(DLCCHSQ);

osThreadId tid_MeshTask;
osThreadDef(MM_MeshTaskMain, osPriorityAboveNormal, 1, 1500);
osPoolDef(MeshPool, MESH_POOL_SIZE, CO_Message_t);
osPoolId MeshPool;
osMessageQDef(MeshQ, MESH_Q_SIZE, &CO_Message_t);
osMessageQId(MeshQ);
#ifdef DEBUG_MESH_QUEUE
uint32_t meshpoolcount = 0;
#endif

bool gAtModeOverTheAir = false;

#ifdef USE_PIN_12
extern Gpio_t Pin12;
#endif
/* Private Variables
*************************************************************************************/
typedef struct
{
   FrameType_t frameType;
   bool appMsgPresent;
   ApplicationLayerMessageType_t appMsgType;
   SM_Event_t smEvent;
} msgMap_t;

msgMap_t msgMap[] = 
{
   /* Frame                           AppMsgPres AppMsg                               smEvent */
   {  FRAME_TYPE_HEARTBEAT_E,         false,     (ApplicationLayerMessageType_t)0,    EVENT_HEARTBEAT_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_FIRE_SIGNAL_E,          EVENT_FIRE_SIGNAL_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_ALARM_SIGNAL_E,         EVENT_ALARM_SIGNAL_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_FAULT_SIGNAL_E,         EVENT_FAULT_SIGNAL_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_OUTPUT_SIGNAL_E,        EVENT_OUTPUT_SIGNAL_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_LOGON_E,                EVENT_LOGON_REQ_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_STATUS_INDICATION_E,    EVENT_STATUS_INDICATION_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_ROUTE_ADD_E,            EVENT_ROUTE_ADD_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_ROUTE_DROP_E,           EVENT_ROUTE_DROP_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_COMMAND_E,              EVENT_COMMAND_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_RESPONSE_E,             EVENT_RESPONSE_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_STATE_SIGNAL_E,         EVENT_SET_STATE_SIGNAL_E },
   {  FRAME_TYPE_TEST_MESSAGE_E,      true,      APP_MSG_TYPE_TEST_SIGNAL_E,          EVENT_TEST_MESSAGE_E },
   {  FRAME_TYPE_ACKNOWLEDGEMENT_E,   false,     (ApplicationLayerMessageType_t)0,    EVENT_ACK_SIGNAL_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E,   EVENT_ROUTE_ADD_RESPONSE_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_RBU_DISABLE_E,          EVENT_RBU_DISABLE_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E, EVENT_ALARM_OUTPUT_STATE_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E, EVENT_BATTERY_STATUS_MESSAGE_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_PING_E,                 EVENT_PING_REQUEST_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_ZONE_ENABLE_E,          EVENT_ZONE_ENABLE_E },
   {  FRAME_TYPE_DATA_E,              true,      APP_MSG_TYPE_GLOBAL_DELAYS_E,        EVENT_GLOBAL_DELAYS_E },
   {  FRAME_TYPE_AT_E,                true,      APP_MSG_TYPE_AT_COMMAND_E,           EVENT_AT_COMMAND_E },
   {  FRAME_TYPE_PPU_MODE_E,          true,      APP_MSG_TYPE_PPU_MODE_E,             EVENT_PPU_MODE_E },
   {  FRAME_TYPE_EXIT_TEST_MODE_E,    true,      APP_MSG_TYPE_EXIT_TEST_MODE_E,       EVENT_EXIT_TEST_MODE_E },
};


/* False CAD Counting Variables
*************************************************************************************/
#ifdef COUNT_FALSE_CADS
extern uint16_t gCAD_TimeoutCount;
static uint16_t gLocal_CAD_TimeoutCount;
#endif


/*************************************************************************************/
/**
* MM_MapMessageToEvent
* Routine to map Frame type and Application Layer Message type to a state machine
* event.
*
* @param - FrameType_t frameType
* @param - bool appMsgPresent
* @param - ApplicationLayerMessageType_t appMsgType
* @param - pointer to state machine event
*
* @return - returns status - o=success, any other value is an error

*/
static ErrorCode_t MM_MapMessageToEvent(FrameType_t frameType,
                                        bool appMsgPresent,
                                        ApplicationLayerMessageType_t appMsgType,
                                        SM_Event_t *pSmEvent)
{
   ErrorCode_t status = ERR_OUT_OF_RANGE_E;
   int32_t i;
   //CO_PRINT_B_2(DBG_INFO_E,"MapMessageToEvent fr=%d, mtype=%d\r\n", (int)frameType, (int)appMsgType);
   for (i=0; i<(sizeof(msgMap)/sizeof(msgMap_t)); i++)
   {
      if ((msgMap[i].frameType == frameType) &&
          (msgMap[i].appMsgPresent == false) &&
          (appMsgPresent == false))
      {
         *pSmEvent = msgMap[i].smEvent;
         status = SUCCESS_E;
         break;
      }

      if ((msgMap[i].frameType == frameType) &&
          (msgMap[i].appMsgPresent == true) &&
          (appMsgPresent == true) &&
          (msgMap[i].appMsgType == appMsgType))
      {
         *pSmEvent = msgMap[i].smEvent;
         status = SUCCESS_E;
         break;
      }
   }
   
   return status;
}

/*************************************************************************************/
/**
* MM_MeshTaskInit
* Initialisation function for the Mesh Task
*
* @param - uint32_t isSyncMaster - flag true for TDM master and false for TDM slave
* @param - uint32_t address - address of node in mesh
* @param - uint32_t systemId - system ID of the mesh
*
* @return - ErrorCode_t 

*/
ErrorCode_t MM_MeshTaskInit(const uint32_t isSyncMaster, const uint32_t address, const uint32_t systemId)
{
   /* set the network node address for this unit */
   gNetworkAddress = address;
   
   /* create pools */
   MeshPool = osPoolCreate(osPool(MeshPool));
   CO_ASSERT_RET_MSG(NULL != MeshPool, ERR_FAILED_TO_CREATE_MEMPOOL_E, "ERROR - Failed to create Mesh MemPool");
   /* create queues */
   MeshQ = osMessageCreate(osMessageQ(MeshQ), NULL);
   CO_ASSERT_RET_MSG(NULL != MeshQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create Mesh Queue");

   // Create thread (return on error)
   tid_MeshTask = osThreadCreate(osThread(MM_MeshTaskMain), NULL);
   CO_ASSERT_RET_MSG(NULL != tid_MeshTask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create Mesh Task thread");

   SMInitialise(isSyncMaster, address, systemId);
   MC_SMGR_Initialise(isSyncMaster, address);
   MC_SACH_Initialise( address );
    
   return(SUCCESS_E);
}


/*************************************************************************************/
/**
* MM_MeshTaskMain
* Main function for Mesh Task
*
* @param - arguments
*
* @return - void

*/
void MM_MeshTaskMain(void const *argument)
{
   while (true)
   {
      osEvent event;
      CO_Message_t *pMsg = NULL;
      ErrorCode_t status = SUCCESS_E;
     
      //GpioWrite(&Pin12, 0);
      event = osMessageGet(MeshQ, osWaitForever);
      //GpioWrite(&Pin12, 1);
      if (osEventMessage == event.status)
      {
         /* read message */
         pMsg = (CO_Message_t *)event.value.p;
         if (pMsg)
         {
            // map message to state machine event
            if (CO_MESSAGE_GENERATE_HEARTBEAT_E == pMsg->Type)
            {
               SMHandleEvent(EVENT_GENERATE_HEARTBEAT_E, (uint8_t *)pMsg);
            }
            else if (CO_MESSAGE_PHY_DATA_IND_E == pMsg->Type)
            {
               FrameType_t frameType;
               bool appMsgPresent;
               ApplicationLayerMessageType_t appMsgType;
               SM_Event_t smEvent;
                
#ifdef USE_ENCPRYPTED_PAYLOAD  
               if ( FRAME_TYPE_DATA_E == MC_PUP_ParameterGet((const uint8_t *)pMsg->Payload.PhyDataInd.Data, PUP_FRAME_TYPE_START, PUP_FRAME_TYPE_SIZE) )
               {                                               
                  /* decrypt the payload before processing */
                  CO_Message_t* pCryptoMsg = (CO_Message_t*)event.value.p;
                  status = MC_MessageDecrypt(pCryptoMsg);
               }
#endif               
               if (SUCCESS_E == status)
               {
               /* extract message type from phy data ind */
               status = MC_PUP_GetMessageType(&pMsg->Payload.PhyDataInd,
                                              &frameType, 
                                              &appMsgPresent, 
                                              &appMsgType);
               }
               
               //CO_PRINT_B_4(DBG_INFO_E, "stat=%d ft=%d amp=%d mt=%d\r\n", status, frameType, appMsgPresent, appMsgType);

               if (SUCCESS_E == status)
               {
                  status = MM_MapMessageToEvent(frameType,
                                                appMsgPresent,
                                                appMsgType,
                                                &smEvent);
               }
               
               //CO_PRINT_B_2(DBG_INFO_E, "stat=%d ev=%d\r\n", status, smEvent);

               if (SUCCESS_E == status)
               {
                  if ( FRAME_TYPE_AT_E == frameType || FRAME_TYPE_PPU_MODE_E == frameType || FRAME_TYPE_EXIT_TEST_MODE_E == frameType)
                  {
                     CO_PRINT_B_5(DBG_INFO_E, "FRAME_TYPE_AT_E : %02x%02x%02x%02x%02x", pMsg->Payload.PhyDataInd.Data[0], pMsg->Payload.PhyDataInd.Data[1], pMsg->Payload.PhyDataInd.Data[2], pMsg->Payload.PhyDataInd.Data[3], pMsg->Payload.PhyDataInd.Data[4]);
                     CO_PRINT_B_5(DBG_NOPREFIX_E, "%02x%02x%02x%02x%02x\r\n", pMsg->Payload.PhyDataInd.Data[5], pMsg->Payload.PhyDataInd.Data[6], pMsg->Payload.PhyDataInd.Data[7], pMsg->Payload.PhyDataInd.Data[8], pMsg->Payload.PhyDataInd.Data[9]);
                     CO_PRINT_B_1(DBG_INFO_E, "gAtModeOverTheAir=%d\r\n", gAtModeOverTheAir);
                     if ( (gAtModeOverTheAir && (AT_FRAME_TYPE_COMAND_E == pMsg->Payload.PhyDataInd.Data[0])) ||
                          (AT_FRAME_TYPE_ENTER_PPU_MODE_E == pMsg->Payload.PhyDataInd.Data[0]) ||
                          (AT_FRAME_TYPE_EXIT_TEST_MODE_E == pMsg->Payload.PhyDataInd.Data[0]) )
                     {
                        SMHandleEvent(smEvent, (uint8_t *)pMsg);
                     }
                     else 
                     {
                        gAtModeOverTheAir = MC_MAC_ProcessATRequest( (uint8_t *)pMsg->Payload.PhyDataInd.Data );
                     }
                  }
                  else 
                  {
                     SMHandleEvent(smEvent, (uint8_t *)pMsg);
                  }


               }
               else
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "Mesh Task: Map message to Event Failed\r\n");
                  /* The message was corrupted.  If we are in a heartbeat slot we need to send
                     a missing heartbeat message to the state machine */
                  MC_TDM_SlotType_t slotType = MC_TDM_GetCurrentSlotType();
                  if ( MC_TDM_SLOT_TYPE_DCH_E == slotType )
                  {
                     bool ProcessSyncMessage = MC_MAC_IsaTrackingNodeDchSlot();
                     MC_SendMissedHeartbeat(CO_RXTE_ERROR_E, ProcessSyncMessage);
                  }
                  /* send a failed message report so that it is reported in test mode */
                  ReportFailedMessage(pMsg, ERR_MESSAGE_FAIL_E);
               }

               if ( MC_MAC_TEST_MODE_OFF_E == MC_GetTestMode() )
               {
                  /* log received message */
                  CO_LOG_PHY_DATA_IND(pMsg->Payload.PhyDataInd.slotIdxInSuperframe, gNetworkAddress,
                                      pMsg->Payload.PhyDataInd.freqChanIdx, pMsg->Payload.PhyDataInd.RSSI,
                                      pMsg->Payload.PhyDataInd.SNR, pMsg->Payload.PhyDataReq.Data,
                                      pMsg->Payload.PhyDataReq.Size, pMsg->Payload.PhyDataInd.freqDeviation);
               }
            }
            else if (CO_MESSAGE_PHY_DATA_REQ_E == pMsg->Type)
            {
               if ( MC_MAC_TEST_MODE_OFF_E == MC_GetTestMode() )
               {
#ifdef USE_ENCPRYPTED_PAYLOAD  
                  if ( FRAME_TYPE_DATA_E == MC_PUP_ParameterGet((const uint8_t *)pMsg->Payload.PhyDataReq.Data, PUP_FRAME_TYPE_START, PUP_FRAME_TYPE_SIZE) )
                  {                                               
                     /* decrypt the payload before logging */
                     CO_Message_t* pCryptoMsg = (CO_Message_t*)event.value.p;
                     status = MC_MessageDecrypt(pCryptoMsg);
                  }
#endif

                  /* log sent message */
                  CO_LOG_PHY_DATA_REQ(pMsg->Payload.PhyDataReq.slotIdxInSuperframe, gNetworkAddress,
                                      pMsg->Payload.PhyDataReq.freqChanIdx, pMsg->Payload.PhyDataReq.Data,
                                      pMsg->Payload.PhyDataReq.Size+2);
               }
            }
            else if (CO_MESSAGE_GENERATE_FIRE_SIGNAL_E == pMsg->Type)
            {
               SMHandleEvent(EVENT_GENERATE_FIRE_SIGNAL_E, (uint8_t *)pMsg);
            }
            else if (CO_MESSAGE_GENERATE_ALARM_SIGNAL_E == pMsg->Type)
            {
               SMHandleEvent(EVENT_GENERATE_ALARM_SIGNAL_E, (uint8_t *)pMsg);
            }
            else if (CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E == pMsg->Type)
            {
               SMHandleEvent(EVENT_GENERATE_OUTPUT_SIGNAL_E, (uint8_t *)pMsg);
            }
            else if (CO_MESSAGE_GENERATE_ALARM_OUTPUT_STATE_E == pMsg->Type)
            {
               SMHandleEvent(EVENT_GENERATE_ALARM_OUTPUT_STATE_E, (uint8_t *)pMsg);
            }
            else if (CO_MESSAGE_GENERATE_FAULT_SIGNAL_E == pMsg->Type)
            {
               SMHandleEvent(EVENT_GENERATE_FAULT_SIGNAL_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_MAC_EVENT_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_MAC_EVENT_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_LOGON_REQ == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_LOGON_REQ_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_COMMAND_REQ_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_RESPONSE_SIGNAL_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_RESPONSE_REQ_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_TEST_MESSAGE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_TEST_MESSAGE_REQ_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_SET_STATE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_SET_STATE_REQ_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_ROUTE_ADD_RESPONSE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_ROUTE_ADD_RESPONSE_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_ROUTE_ADD_REQUEST_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_ROUTE_ADD_REQUEST_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_RBU_DISABLE_MESSAGE_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_STATUS_INDICATION_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_STATUS_INDICATION_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_ROUTE_DROP_REQUEST_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_ROUTE_DROP_REQUEST_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_APPLICATION_REQUEST_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_APPLICATION_REQUEST_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_BATTERY_STATUS_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_BATTERY_STATUS_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_PING_REQUEST_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_PING_REQUEST_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_ZONE_ENABLE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_ZONE_ENABLE_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_ATTX_COMMAND_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_AT_COMMAND_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_ATTX_RESPONSE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_AT_COMMAND_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_GENERATE_ENTER_AT_MODE_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_PPU_COMMAND_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_PPU_COMMAND_E, (uint8_t *)pMsg);
            }
            else if ( CO_MESSAGE_PPU_RESPONSE_E == pMsg->Type )
            {
               SMHandleEvent(EVENT_PPU_RESPONSE_E, (uint8_t *)pMsg);
            }
            else
            {
               CO_PRINT_B_1(DBG_ERROR_E, "MM_MeshTaskMain - Unknown message type received %d\n\r", pMsg->Type);
               ReportFailedMessage(pMsg, ERR_MESSAGE_TYPE_UNKNOWN_E);
            }
            
            FREEMESHPOOL(pMsg);

            #ifdef COUNT_FALSE_CADS
            // Report the number of false CADs that have occured (i.e. the CAD has detected activity, but the resulting RxSingle timed-out rather than receiving a packet).
            // If gCAD_TimeoutCount has changed, print the new value and record its new value.
            if (gLocal_CAD_TimeoutCount != gCAD_TimeoutCount)
            {
               CO_PRINT_B_1(DBG_INFO_E, "CAD RxTimeout = %d\r\n", gCAD_TimeoutCount);
               gLocal_CAD_TimeoutCount = gCAD_TimeoutCount;
            }
            #endif
         }
      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E, "Mesh task received bad event status\r\n");
         pMsg = (CO_Message_t *)event.value.p;
         if (pMsg)
         {
            FREEMESHPOOL(pMsg);
         }
      }
   }
}

static void ReportFailedMessage(const CO_Message_t* pMsg, const ErrorCode_t reason)
{
   /* only report failed messages in test mode RECEIVE */
   MC_MAC_TestMode_t test_mode = MC_GetTestMode();
   if (  pMsg && 
         ((MC_MAC_TEST_MODE_RECEIVE_E == test_mode) ||
          (MC_MAC_TEST_MODE_TRANSPARENT_E == test_mode) ||
          (MC_MAC_TEST_MODE_MONITORING_E == test_mode) ||
          (MC_MAC_TEST_MODE_NETWORK_MONITOR_E == test_mode)))
   {
      TestModeReport_t test_report;
      test_report.type = APP_MSG_TYPE_UNKNOWN_E;
      test_report.reason = reason;
      test_report.pMessage = (CO_Message_t*)pMsg;
      
      /* pass message to state machine */
      SMHandleEvent(EVENT_TEST_REPORT_E, (uint8_t *)&test_report);
   }
}
