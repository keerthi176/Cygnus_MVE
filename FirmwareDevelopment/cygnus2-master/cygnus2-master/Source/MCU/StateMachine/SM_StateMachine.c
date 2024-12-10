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
*  File         : SM_StateMachine.c
*
*  Description  : Radio Unit State Machine
*                 This is used for RBU and for NCU
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>


/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "SM_StateMachine.h"
#include "CO_Message.h"
#include "CO_Defines.h"
#include "DM_NVM.h"
#include "MC_PUP.h"
#include "MC_MAC.h"
#include "DM_SerialPort.h"
#include "board.h"
#include "MC_AckManagement.h"
#include "MC_SessionManagement.h"
#include "MC_TestMode.h"
#include "MC_SACH_Management.h"
#include "MC_MeshFormAndHeal.h"
#include "MM_Main.h"
#include "MC_MacConfiguration.h"
#include "DM_OutputManagement.h"
#include "MM_TimedEventTask.h"
#include "MC_SyncAlgorithm.h"
#include "MC_TDM.h"
#include "MC_PingRecovery.h"
#include "MC_StateManagement.h"
#include "CFG_Device_cfg.h"
#include "MC_MacQueues.h"

/* Public Functions Definitions
*************************************************************************************/
#define NOT_SET         0xFFu
#define ULTRA_LOW_SNR   -20        // A very low value of SNR, used when packets have not been detected.
#define ULTRA_LOW_RSSI  -132       // A very low value of RSSI, used when packets have not been detected.
#define CO_LOG_ENABLE_RECEIVE_FAILURE_LOGS  // creates a log entry for each rx timeout or rx error (crc error)

/* Private Functions Prototypes
*************************************************************************************/
static ErrorCode_t SM_ProcessApplicationRequest(CO_CommandData_t* pCmdData);
static ErrorCode_t SM_SendACK(const uint16_t destination, const uint8_t ackChannel, const bool alwaysSend );
static void SM_TryToSendImmediately( const AcknowledgedQueueID_t ackQueue, const MacQueueType_t macQueue );
static void updateAckManagement(const uint32_t RachChannel);
static void MC_SMGR_ProcessPpuMessage(ApplicationMessage_t* pAppMessage);
static bool generateGlobalDelayMessage(const uint16_t nodeId);
static void noAction(const uint8_t * const pData);
static void generateHeartbeat(const uint8_t * const pData);
static void rbuHeartbeat(const uint8_t * const pData);
static void ncuHeartbeat(const uint8_t * const pDataa);
static void generateFireSignal(const uint8_t * const pData);
static void generateAlarmSignal(const uint8_t * const pData);
static void ncuFireSignal(const uint8_t * const pData);
static void rbuFireSignal(const uint8_t * const pData);
static void ncuAlarmSignal(const uint8_t * const pData);
static void rbuAlarmSignal(const uint8_t * const pData);
static void rbuOutputSignal(const uint8_t * const pData);
static void ackSignal(const uint8_t * const pData);
static void macEventUpdate(const uint8_t * const pData);
static void generateOutputSignal(const uint8_t * const pData);
static void GenerateRouteAdd(const uint8_t * const pData);
static void routeAdd(const uint8_t * const pData);
static void routeDrop(const uint8_t * const pData);
static void generateFaultSignal(const uint8_t * const pData);
static void ncuFaultSignal(const uint8_t * const pData);
static void rbuFaultSignal(const uint8_t * const pData);
static void generateLogonRequest(const uint8_t * const pData);
static void logonRequest(const uint8_t * const pData);
static void generateCommand(const uint8_t * const pData);
static void rxCommand(const uint8_t * const pData);
static void generateResponse(const uint8_t * const pData);
static void ncuResponse(const uint8_t * const pData);
static void rbuResponse(const uint8_t * const pData);
static void generateTestMessage(const uint8_t * const pData);
static void generateSetState(const uint8_t * const pData);
static void rbuSetState(const uint8_t * const pData);
static void generateRouteAddResponse(const uint8_t * const pData);
static void rbuRouteAddResponse(const uint8_t * const pData);
static void generateRBUDisable(const uint8_t * const pData);
static void rbuDisableMessage(const uint8_t * const pData);
static void generateStatusIndication(const uint8_t * const pData);
static void ncuStatusIndication(const uint8_t * const pData);
static void rbuStatusIndication(const uint8_t * const pData);
static void GenerateRouteDrop(const uint8_t * const pData);
static void applicationRequest(const uint8_t * const pData);
static void generateAlarmOutputState(const uint8_t * const pData);
static void rbuAlarmOutputState(const uint8_t * const pData);
static void generateBatteryStatus(const uint8_t * const pData);
static void ncuBatteryStatus(const uint8_t * const pData);
static void rbuBatteryStatus(const uint8_t * const pData);
static void generatePingRequest(const uint8_t * const pData);
static void pingRequest(const uint8_t * const pData);
static void generateZoneEnableMessage(const uint8_t * const pData);
static void zoneEnableMessage(const uint8_t * const pData);
static void generateAtCommandMessage(const uint8_t * const pData);
static void AtCommandMessage(const uint8_t * const pData);
static void generateAtModeMessage(const uint8_t * const pData);
static void ppuModeMesssage(const uint8_t * const pData);
static void ppuCommandMesssage(const uint8_t * const pData);
static void generatePpuResponse(const uint8_t * const pData);
static void globalDelayMessage(const uint8_t * const pData);

#define ___ noAction
#define GHB generateHeartbeat
#define RHB rbuHeartbeat
#define NHB ncuHeartbeat
#define GFS generateFireSignal
#define GAS generateAlarmSignal
#define NFS ncuFireSignal
#define RFS rbuFireSignal
#define NAS ncuAlarmSignal
#define RAS rbuAlarmSignal
#define OUT rbuOutputSignal
#define ACK ackSignal
#define GOS generateOutputSignal
#define MEU macEventUpdate
#define RAD routeAdd
#define RDP routeDrop
#define GFT generateFaultSignal
#define NFT ncuFaultSignal
#define RFT rbuFaultSignal
#define GLO generateLogonRequest
#define LOR logonRequest
#define GCM generateCommand
#define CMD rxCommand
#define GRS generateResponse
#define NRS ncuResponse
#define RRS rbuResponse
#define GST generateSetState
#define RST rbuSetState
#define GRR generateRouteAddResponse
#define RAR rbuRouteAddResponse
#define GRA GenerateRouteAdd
#define GRD generateRBUDisable
#define RDB rbuDisableMessage
#define GSI generateStatusIndication
#define NSI ncuStatusIndication
#define RSI rbuStatusIndication
#define GDR GenerateRouteDrop
#define ARQ applicationRequest
#define GAO generateAlarmOutputState
#define AOS rbuAlarmOutputState
#define GBS generateBatteryStatus
#define NBS ncuBatteryStatus
#define RBS rbuBatteryStatus
#define GPG generatePingRequest
#define PNG pingRequest
#define GZE generateZoneEnableMessage
#define ZEM zoneEnableMessage
#define GAT generateAtCommandMessage
#define ATC AtCommandMessage
#define GAM generateAtModeMessage
#define PPU ppuModeMesssage
#define PPC ppuCommandMesssage
#define PPR generatePpuResponse
#define GDM globalDelayMessage

#define TRHB TM_rxHeartbeat
#define TRFS TM_rxFireSignal
#define TRAS TM_rxAlarmSignal
#define TOUT TM_rxOutputSignal
#define TACK TM_rxAckSignal
#define TRAD TM_rxRouteAdd
#define TRAR TM_rxRouteAddResponse
#define TRDP TM_rxRouteDrop
#define TRFT TM_rxFaultSignal
#define TLOR TM_rxLogonRequest
#define TCMD TM_rxCommand
#define TRRS TM_rxResponse
#define TTMM TM_rxTestSignal
#define TRPT TM_rxTestReport
#define TRST TM_rxSetState
#define TRBD TM_rxRBUDisable
#define TGEN generateTestMessage
#define TSIN TM_rxStatusSignal
#define TPNG TM_rxPing

/* Global Variables
*************************************************************************************/
extern uint32_t gNetworkAddress;
extern uint32_t gZoneNumber;

extern osPoolId MeshPool;
extern osPoolId AppPool;

#ifndef USE_NEW_MAC_QUEUES
extern osMessageQId(RACHSQ);
#endif
extern osMessageQId(MeshQ);
extern osMessageQId(AppQ);
extern osMessageQId(DCHQ);
extern osMessageQId(RACHPQ);
extern osMessageQId(ACKQ);
extern osMessageQId(DLCCHPQ);
extern osMessageQId(DLCCHSQ);

extern uint8_t gNoOfPrimaryAcksInTxQueue;
extern uint8_t gNoOfSecondaryAcksInTxQueue;
extern uint8_t gPrimaryDLCCHMessageCount;
extern uint8_t gSecondaryDLCCHMessageCount;

extern uint32_t GetNextHandle(void);
extern HAL_StatusTypeDef FLASH_WaitForLastOperation(uint32_t Timeout);

extern LPTIM_HandleTypeDef hlptim1;

extern bool gFreqLockAchieved;

extern MC_TDM_DCHBehaviour_t gDchBehaviour[MAX_DEVICES_PER_SYSTEM];


bool gSM_UsingActiveFrameLength = false;


/* Private Variables
*************************************************************************************/

/* values associated with applying a hold-off on state checks (suspending checks that our state is not ahead of the mesh) after we change state */
static uint32_t gSM_LongframesSinceStateChange = 0;
static uint32_t gLastTransactionID = 0xffffffff;
#define NUM_OF_LONG_FRAMES_TO_SUSPEND_STATE_CHECKS (2)
#define MAX_STATE_CHECK_HOLDOFF (0xffffffff)
#define FREE_RUNNING_DETECTION_THRESHOLD 10 //The number of long frames that the RBU state can be behind the mesh state before resetting

// TODO... my version removes compiler warnings: 
static void (* const pfRBUTransition[EVENT_MAX_E][STATE_MAX_E])(const uint8_t * const pData) =
{
   /* {IDLE, CONFIG_SYNC, CONFIG_FORM, ACTIVE, Test Mode} */
      {RHB, RHB, RHB, RHB, TRHB}, /* Heartbeat */
      {___, ___, ___, RFS, TRFS}, /* Fire Signal */
      {___, ___, ___, RAS, TRAS}, /* Alarm Signal */
      {___, ___, ___, OUT, TOUT}, /* Output Signal */
      {___, ACK, ACK, ACK, TACK}, /* Ack Signal */
      {___, GHB, GHB, GHB, ___ }, /* Generate Heartbeat */
      {___, ___, ___, GFS, ___ }, /* Generate Fire Signal */
      {___, ___, ___, GAS, ___ }, /* Generate Alarm Signal */
      {___, ___, ___, GOS, ___ }, /* Generate Output Signal */
      {___, ___, ___, GFT, ___ }, /* Generate Fault Signal */
      {MEU, MEU, MEU, MEU, MEU }, /* MAC event update */
      {___, RAD, RAD, RAD, TRAD}, /* Route Add */
      {___, RDP, RDP, RDP, TRDP}, /* Route Drop */
      {___, ___, ___, RFT, TRFT}, /* Fault Signal */
      {___, GLO, GLO, GLO, ___ }, /* Generate Logon Request */
      {___, LOR, LOR, LOR, TLOR}, /* Rx'd Logon Request */
      {___, GCM, GCM, GCM, GCM }, /* Generate command */
      {___, CMD, CMD, CMD, TCMD}, /* Command */
      {___, GRS, GRS, GRS, ___ }, /* Generate response */
      {___, RRS, RRS, RRS, TRRS}, /* Responce */
      {___, ___, ___, ___, TTMM}, /* Test Mode Message */
      {___, ___, ___, ___, TRPT}, /* Test Mode Report */
      {___, ___, ___, ___, TGEN}, /* GenerateTestMessage */
      {___, GST, GST, GST, ___ }, /* GenerateSetState */
      {___, RST, RST, RST, TRST}, /* SetState */
      {___, GRR, GRR, GRR, ___ }, /* Generate Route Add Response */
      {___, RAR, RAR, RAR, TRAR}, /* Route Add Response */
      {___, GRA, GRA, GRA, ___ }, /* Generate Route Add Request */
      {___, ___, ___, ___, ___ }, /* Generate RBU Disable Message */
      {___, RDB, RDB, RDB, TRBD}, /* RBU Disable Message */
      {___, ___, GSI, GSI, ___ }, /* Generate Status Indication */
      {___, ___, RSI, RSI, TSIN}, /* Status Indication */
      {___, GDR, GDR, GDR, ___ }, /* Generate Route Drop */
      {ARQ, ARQ, ARQ, ARQ, ARQ }, /* Application Request */
      {___, ___, ___, ___, ___ }, /* Generate Alarm Output State */
      {___, ___, ___, AOS, ___ }, /* Alarm Output State */
      {GBS, GBS, GBS, GBS, ___ }, /* Generate Battery Status Message */
      {___, ___, RBS, RBS, ___ },  /* Battery Status */
      {___, GPG, GPG, GPG, GPG }, /* Generate Ping Request */
      {___, PNG, PNG, PNG, TPNG}, /* Ping Request */
      {___, ___, ___, ___, ___ }, /* Generate Zone Enable Message */
      {___, ZEM, ZEM, ZEM, ___ }, /* Zone Enable Message */
      {GAT, ___, ___, ___, ___ }, /* Generate AT Command Message */
      {ATC, ___, ___, ___, ATC }, /* AT Command Message */
      {GAM, ___, ___, ___, ___ }, /* GenerateAT Mode Message */
      {PPU, PPU, PPU, PPU, PPU }, /* PPU Mode Message */
      {PPC, PPC, PPC, PPC, PPC }, /* PPU Command */
      {PPR, PPR, PPR, PPR, PPR }, /* PPU Response */
      {___, ___, GDM, GDM, GDM }, /* Global Delay Message */
};

static void (* const pfNCUTransition[EVENT_MAX_E][STATE_MAX_E])(const uint8_t * const pData) =
{
   /* {CONFIG_SYNC, CONFIG_FORM, ACTIVE, Test Mode} */
      {___, ___, NHB, NHB, TRHB}, /* Heartbeat */
      {___, ___, ___, NFS, TRFS}, /* Fire Signal */
      {___, ___, ___, NAS, TRAS}, /* Alarm Signal */
      {___, ___, ___, ___, TOUT}, /* Output Signal */
      {___, ACK, ACK, ACK, TACK}, /* Ack Signal */
      {___, GHB, GHB, GHB, ___ }, /* Generate Heartbeat */
      {___, ___, ___, ___, ___ }, /* Generate Fire Signal */
      {___, ___, ___, ___, ___ }, /* Generate Alarm Signal */
      {___, ___, ___, GOS, ___ }, /* Generate Output Signal */
      {___, ___, ___, GFT, ___ }, /* Generate Fault Signal */
      {MEU, MEU, MEU, MEU, MEU }, /* MAC event update */
      {___, RAD, RAD, RAD, TRAD}, /* Route Add */
      {___, RDP, RDP, RDP, TRDP}, /* Route Drop */
      {___, ___, ___, NFT, TRFT}, /* Fault Signal */
      {___, ___, ___, ___, ___ }, /* Generate Logon Request */
      {___, LOR, LOR, LOR, TLOR}, /* Rx'd Logon Request */
      {___, GCM, GCM, GCM, GCM }, /* Generate command */
      {___, CMD, CMD, CMD, TCMD}, /* Command */
      {___, ___, ___, GRS, ___ }, /* Generate response */
      {___, NRS, NRS, NRS, TRRS}, /* Responce */
      {___, ___, ___, ___, TTMM}, /* Test Mode Message */
      {___, ___, ___, ___, TRPT}, /* Test Mode Report */
      {___, ___, ___, ___, TGEN}, /* GenerateTestMessage */
      {GST, GST, GST, GST, ___ }, /* GenerateSetState */
      {___, ___, ___, ___, TRST},  /* SetState */
      {___, GRR, GRR, GRR, ___ }, /* Generate Route Add Response */
      {___, RAR, RAR, RAR, TRAR}, /* Route Add Response */
      {___, ___, ___, ___, ___ }, /* Generate Route Add Request */
      {___, GRD, GRD, GRD, ___ }, /* Generate RBU Disable Message */
      {___, ___, ___, ___, TRBD}, /* RBU Disable Message */
      {___, ___, ___, ___, ___ }, /* Generate Status Indication */
      {___, ___, NSI, NSI, TSIN}, /* Status Indication */
      {___, GDR, GDR, GDR, ___ }, /* Generate Route Drop */
      {ARQ, ARQ, ARQ, ARQ, ARQ }, /* Application Request */
      {___, GAO, GAO, GAO, ___ }, /* Generate Alarm Output State */
      {___, ___, ___, ___, ___ }, /* Alarm Output State */
      {___, ___, ___, ___, ___ }, /* Generate Battery Status Message */
      {___, ___, NBS, NBS, ___ },  /* Battery Status */  /* Alarm Output State */
      {___, GPG, GPG, GPG, GPG }, /* Generate Ping Request */
      {___, PNG, PNG, PNG, TPNG}, /* Ping Request */
      {___, GZE, GZE, GZE, ___ }, /* Generate Zone Enable Message */
      {___, ___, ___, ___, ___ }, /* Zone Enable Message */
      {GAT, ___, ___, ___, ___ }, /* Generate AT Command Message */
      {ATC, ___, ___, ___, ___ }, /* AT Command Message */
      {GAM, ___, ___, ___, ___ }, /* GenerateAT Mode Message */
      {PPU, PPU, PPU, PPU, PPU }, /* PPU Mode Message */
      {PPC, PPC, PPC, PPC, PPC }, /* PPU Command */
      {PPR, PPR, PPR, PPR, PPR }, /* PPU Response */
      {___, ___, ___, ___, ___ }, /* Global Delay Message */
};

static const char *StateName[STATE_MAX_E] = {
   "CONFIG_SYNC",
   "CONFIG_FORM",
   "ACTIVE",
   "TEST_MODE"};

static const char *EventName[EVENT_MAX_E] = {
   "Heartbeat",
   "FireSignal",
   "AlarmSignal",
   "OutputSignal",
   "AckSignal",
   "GenerateHeartbeat",
   "GenerateFireSignal",
   "GenerateAlarmSignal",
   "GenerateOutputSignal",
   "GenerateFaultSignal",
   "MACEventUpdate",
   "RouteAdd",
   "RouteDrop",
   "FaultSignal",
   "GenerateLogonRequest",
   "LogonMessage",
   "GenerateCommand",
   "Command",
   "GenerateResponse",
   "ResponseMessage",
   "TestSignal",
   "TestReport",
   "GenerateTestMessage",
   "GenerateSetState",
   "SetState",
   "GenerateRouteAddResponse",
   "RouteAddResponse",
   "GenerateRouteAddRequest",
   "GenerateRBUDisable",
   "RBUDisable",
   "GenerateStatusIndication",
   "StatusIndication",
   "GenerateRouteDrop",
   "ApplicationRequest",
   "GenerateAlarmOutputState",
   "AlarmOutputState",
   "GeneratePingRequest",
   "PingRequest",
   "GenerateZoneEnable",
   "ZoneEnableMessage"
   };
   
static bool SM_IsNCU = false;
static uint32_t SM_address = 0U;
static uint32_t SM_SystemId = 0U;
//static uint8_t gHeartbeatHoldoff = 0;                           /* counter to delay heartbeats being generated for a few long frames after the TDM starts */

/* Map the Mesh state to the LED patterns */
const DM_LedPatterns_t SM_mesh_led_patterns[STATE_MAX_E] =
{
   [STATE_IDLE_E]        = LED_PATTERN_NOT_DEFINED_E,
   [STATE_CONFIG_SYNC_E] = LED_MESH_STATE_CONFIG_SYNC_E,
   [STATE_CONFIG_FORM_E] = LED_MESH_STATE_CONFIG_FORM_E,
   [STATE_ACTIVE_E]      = LED_MESH_STATE_ACTIVE_E,
};

/* Private Functions
*************************************************************************************/
static void SM_DuplicateMessageCallback(const uint32_t DiscardedHandle, const uint32_t NewHandle, const ApplicationLayerMessageType_t MsgType);



/*************************************************************************************/
/**
* SM_SendACK
* Send an acknowledgement message to the specified node id.
*
* @param  destination: The destination node's network ID.
* @param  const uint8_t ackChannel: ACK_RACHP for primary RACH ACKs, ACK_RACHS for secondary RACH ACKs
* @param  alwaysSend: Flag to override 'queue full' checks and force the ack to be sent.
*
* @return SUCCESS_E if the ack is queued for transmission, or ERR_UNEXPECTED_ERROR_E
*         on failure.
*/
ErrorCode_t SM_SendACK(const uint16_t destination, const uint8_t ackChannel, const bool alwaysSend )
{
   ErrorCode_t ack_status = ERR_UNEXPECTED_ERROR_E;
   FrameAcknowledge_t FrameAcknowledgement;
   CO_Message_t *pPhyDataReq = NULL;
   AcknowledgedQueueID_t AckChannel = ACK_RACHS_E;
   
   if ( (uint8_t)ACK_RACHP_E == ackChannel )
   {
      AckChannel = ACK_RACHP_E;
   }

   //Don't acknowledge messages if there is no room in the Ack Mgr queue to forward the message.  The NCU should always acknowledge
   if ( false == MC_ACK_QueueFull(AckChannel) || SM_IsNCU || alwaysSend)
   {
      //Tell the TDM to open the slot
      MC_TDM_OpenAckSlot(true);
      
      FrameAcknowledgement.FrameType = FRAME_TYPE_ACKNOWLEDGEMENT_E;
      FrameAcknowledgement.MACDestinationAddress = destination;
      FrameAcknowledgement.MACSourceAddress = gNetworkAddress;
      FrameAcknowledgement.SystemId = SM_SystemId;
      
      // Pack into phy data req and put into ACK Q.
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         ack_status = MC_PUP_PackAck(&FrameAcknowledgement, &pPhyDataReq->Payload.PhyDataReq);      

         if (SUCCESS_E == ack_status)
         {
            uint32_t retry = 3;
            do
            {
               retry--;
               
               ack_status = MC_MACQ_Push(MAC_ACK_Q_E, pPhyDataReq);
               if ( SUCCESS_E == ack_status )
               {
                  // Increment the ACK1 queue counter
                  if (ACK_RACHP_E == ackChannel)
                  {
                     gNoOfPrimaryAcksInTxQueue++;
                  }
                  // Increment the ACK1 queue counter
                  if (ACK_RACHS_E == ackChannel)
                  {
                     gNoOfSecondaryAcksInTxQueue++;
                  }
                  
                  CO_PRINT_B_2(DBG_INFO_E, "Sending ACK from node %d to node %d\r\n", FrameAcknowledgement.MACSourceAddress, FrameAcknowledgement.MACDestinationAddress);
                  //cancel retries
                  retry = 0;
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "Failed to add msg to ACK Queue - Error %d\r\n", ack_status);
                  MC_MACQ_ResetMessageQueue(MAC_ACK_Q_E);
               }
            } while (retry > 0);
         }
          //The MAC Queue makes a copy so we can free the msg
         FREEMESHPOOL(pPhyDataReq);

      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E,"SM_SendACK - MeshPool failed to allocate\r\n");
      }
   }

   return ack_status;
}

/*************************************************************************************/
/**
* noAction
* Null state handler
*
* @param - pointer to event data
*
* @return - void
*/
static void noAction(const uint8_t * const pData)
{
   /*Do Nothing*/
}

/*************************************************************************************/
/**
* generateHeartbeat
* state handler to generate heartbeat
*
* @param - pointer to event data
*
* @return - void

*/
static void generateHeartbeat(const uint8_t * const pData)
{
   CO_MessagePayloadGenerateHeartbeat_t *pGenHeartbeat = NULL;
   FrameHeartbeat_t heartbeat = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   
   //CO_PRINT_A_0(DBG_INFO_E,"generateHeartbeat+\r\n");
   
   /* check input data */
   if (NULL == pData)
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generateHeartbeat pData=NULL\r\n");
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   pGenHeartbeat = &pMsg->Payload.GenerateHeartbeat;

   
   /* populate heartbeat */
   heartbeat.FrameType                  = FRAME_TYPE_HEARTBEAT_E;
   heartbeat.SlotIndex                  = pGenHeartbeat->SlotIndex;
   heartbeat.ShortFrameIndex            = pGenHeartbeat->ShortFrameIndex;
   heartbeat.LongFrameIndex             = pGenHeartbeat->LongFrameIndex;
   heartbeat.State                      = MC_STATE_GetDeviceState();
   heartbeat.Rank                       = MC_SMGR_GetPublishedRank();
   heartbeat.ActiveFrameLength          = gSM_UsingActiveFrameLength;
   heartbeat.NoOfChildrenIdx            = MC_SMGR_GetNumberOfChildrenIndex();
   heartbeat.NoOfChildrenOfPTNIdx       = 0;
   heartbeat.SystemId                   = SM_SystemId;
   
   
   /* pack into phy data req and put into DCH Q */
   pPhyDataReq = ALLOCMESHPOOL;
   if (pPhyDataReq)
   {
      pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
      status = MC_PUP_PackHeartbeat(&heartbeat, &pPhyDataReq->Payload.PhyDataReq);
      
      if ( SM_IsNCU )
      {
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
      }
      else 
      {
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_GetTxPowerLow();
      }

      if (SUCCESS_E == status)
      {
         status = MC_MACQ_Push(MAC_DCH_Q_E, pPhyDataReq);
         if ( SUCCESS_E != status )
         {
            CO_PRINT_B_1(DBG_ERROR_E, "Failed to add msg to DCH Queue - Error %d\r\n", status);
         }
         // The MAC Queue makes a copy so we can delete the original
         FREEMESHPOOL(pPhyDataReq);
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateHeartbeat Pack failed\r\n");
         /* failed to write */
         FREEMESHPOOL(pPhyDataReq);
      }
   }
   else 
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generateHeartbeat - MeshPool failed to allocate\r\n");
   }
}

/*************************************************************************************/
/**
* rbuHeartbeat
* state handler for RBU receiving heartbeat
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuHeartbeat(const uint8_t * const pData)
{
   FrameHeartbeat_t heartbeat = { 0 };
   FrameHeartbeat_t *pHeartbeat = &heartbeat;
   ErrorCode_t status = SUCCESS_E;
   bool heartbeatValidated = false;
   uint32_t rxTimer = NO_TIMESTAMP_RECORDED;
   uint32_t SlotIndexInLongframe = 0;  // For NCU heartbeat this would be 16, for RBU heartbeat would be 36-39, 56-59, 76-79 etc.
   uint32_t SlotIndexInLongframeFromPayload = 0;
   uint32_t SlotInSuperFrame = 0;
   uint32_t ShortFrameSlot = 0;
   uint16_t node_id = 0;

   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   // Initialise pointer used to access the message.
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   // Unpack the message. (We expect it to be a heartbeat becuase of the slot it arrived in.)
   status = MC_PUP_UnpackHeartbeat(&pMsg->Payload.PhyDataInd, pHeartbeat);
   
   // If unpacking was successful...
   if (SUCCESS_E == status)
   {
      ShortFrameSlot = MC_TDM_ConvertHeartbeatValueToSlot(pHeartbeat->SlotIndex);
      // Derive the slot index in long-frame from the heartbeat payload (e.g. 16-19, 36-39, 56-59, ...)
      SlotIndexInLongframeFromPayload = (pHeartbeat->ShortFrameIndex * SLOTS_PER_SHORT_FRAME) + ShortFrameSlot;
      
      // If in TDM mode then read the slot index based on the reception time (e.g. 16-19, 36-39, 56-59, ...)
      // Otherwise we use the less reliable method of reading the content from the over-the-air packet, which may occasionally be corrupted.
      if (true == pMsg->Payload.PhyDataInd.isInTDMSyncSlaveMode)
      {
         // Read the slot index in long-frame from the value assigned upon reception of the packet.
         SlotIndexInLongframe = pMsg->Payload.PhyDataInd.slotIdxInLongframe;
      }
      else
      {
         // Use the value derived from the payload.
         SlotIndexInLongframe = SlotIndexInLongframeFromPayload;
      }
      
      SlotInSuperFrame = (pHeartbeat->LongFrameIndex * MC_SlotsPerLongFrame()) + SlotIndexInLongframe;
      
      // Calculate the nodeID associated with this heartbeat slot (0 for NCU in Zone 0, 32-63 for RBUs in Zone 1, etc)
      node_id = CO_CalculateNodeID(SlotInSuperFrame);
   
      
//      CO_PRINT_B_1(DBG_INFO_E, "Heartbeat Node = %3d unpacked successfully\r\n", node_id);
      
      // Decide if this heartbeat is valid by comparing payload parameters against expected values
      if (pHeartbeat->SystemId == SM_SystemId && SlotIndexInLongframe == SlotIndexInLongframeFromPayload)
      {
         // Mark that the timestamp is valid, since all necessary validity tests on the message have passed.
         heartbeatValidated = true;
         rxTimer = pMsg->Payload.PhyDataInd.timer;
      }
   }
   else
   {
      CO_PRINT_B_1(DBG_ERROR_E, "Heartbeat Node = %3d didn't unpack successfully.\r\n", node_id);
   }
   
   // If a heartbeat is received in a non-heartbeat slot then node_id will be set to CO_BAD_NODE_ID
   // We only process heartbeats that arrive in genuine heartbeat slots.
   if ( heartbeatValidated && (CO_BAD_NODE_ID != node_id) )
   {
      if ( node_id == MC_SYNC_GetSyncNode() )
      {
         // If the heatbeat for this tracking node is valid, and its Mesh Forming State field exceeds that of gSM_CurrentMeshState,
         // then we need to record the state of the mesh and turn on channel hopping if the mesh is active.
         if ( MC_STATE_GetMeshState() < pHeartbeat->State)
         {
            MC_STATE_SetMeshState(pHeartbeat->State);

            if ( STATE_ACTIVE_E == pHeartbeat->State )
            {
               MC_TDM_EnableChannelHopping();
            }
         }
         //Check to see if we can advance the device state
         if ( MC_STATE_GetDeviceState() < pHeartbeat->State )
         {
            switch ( MC_STATE_GetDeviceState() )
            {
               case STATE_CONFIG_SYNC_E:
                  if ( MC_STATE_ReadyForStateChange(STATE_CONFIG_FORM_E) )
                  {
                     MC_STATE_ScheduleStateChange(STATE_CONFIG_FORM_E, true);
                  }
                  break;
               case STATE_CONFIG_FORM_E:
                  if ( MC_STATE_ReadyForStateChange(STATE_ACTIVE_E) )
                  {
                     MC_STATE_ScheduleStateChange(STATE_ACTIVE_E, true);
                  }
                  break;
               case STATE_TEST_MODE_E:
                     MC_STATE_ScheduleStateChange(STATE_TEST_MODE_E, true);
                  break;
               default:
                  break;
            }
         }
      }
      
#ifdef APPLY_RSSI_OFFSET
      int32_t temp_rssi = pMsg->Payload.PhyDataInd.RSSI + CFG_GetRssiOffset(node_id);
      if( -128 > temp_rssi )
      {
         temp_rssi = -128;
      }
      else if ( 127 < temp_rssi )
      {
         temp_rssi = 127;
      }
      pMsg->Payload.PhyDataInd.RSSI = temp_rssi;
#endif
      // Call function to process the heartbeat.
      MC_MFH_Update( node_id, SlotInSuperFrame, pMsg->Payload.PhyDataInd.SNR, pMsg->Payload.PhyDataInd.RSSI, rxTimer, pHeartbeat->Rank, pHeartbeat->State, pHeartbeat->NoOfChildrenIdx, heartbeatValidated);
   }

   /* After a state change, we can receive heartbeats from non-parent/tracking nodes before our parents/tracking nodes are updated
      so we need a hold-off period before checking that our state is not ahead of our parents */
   if ( gSM_LongframesSinceStateChange > NUM_OF_LONG_FRAMES_TO_SUSPEND_STATE_CHECKS )
   {
      /* check the states of the parents */
      if (MC_SMGR_CheckParentStates(MC_STATE_GetDeviceState(), pHeartbeat->LongFrameIndex))
      {
         /* our state is ahead of the parents so return to CONFIG_SYNC state (i.e. reset the RBU)*/
         /* For a commanded state change, we won't see our parents state change until the next heartbeat so we 
            hold-off our reset for a longframe after the state change */
         MC_SMGR_DelayedReset("State ahead of Mesh State\n\r");
      }
   }
}

/*************************************************************************************/
/**
* ncuHeartbeat
* state handler for NCU receiving heartbeat
*
* @param - pointer to event data
*
* @return - void

*/
static void ncuHeartbeat(const uint8_t * const pData)
{
   FrameHeartbeat_t heartbeat = { 0 };
   FrameHeartbeat_t *pHeartbeat = &heartbeat;
   ErrorCode_t status = SUCCESS_E;
   uint32_t heartbeatSlot = 0;
   uint32_t SlotIndexInLongframe = 0;
   uint32_t SlotIndexInLongframeFromPayload = 0;
   bool heartbeatValidated = false;
   uint32_t rxTimer = NO_TIMESTAMP_RECORDED;

   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackHeartbeat(&pMsg->Payload.PhyDataInd, pHeartbeat);
   
   if (SUCCESS_E == status && pHeartbeat->SystemId == SM_SystemId)
   {
      //The heartbeat slot has only 2 bits in the message so has a range of 0-3.
      //This needs to be converted to a short frame slot.
      heartbeat.SlotIndex = MC_TDM_ConvertHeartbeatValueToSlot(heartbeat.SlotIndex);

      /* construct TDM slot from heartbeat using slot in superframe index format */
      heartbeatSlot =  MC_TDM_ConstructSlotInSuperframeIdx(heartbeat.LongFrameIndex,
                                                           heartbeat.ShortFrameIndex,
                                                           heartbeat.SlotIndex);
      CO_PRINT_B_3(DBG_INFO_E, "Rx Heartbeat: lfi %d sfi %d si %d\r\n", heartbeat.LongFrameIndex, heartbeat.ShortFrameIndex, heartbeat.SlotIndex);
      
      /* Tell the session manager that we received a heartbeat */
      uint16_t node_id = CO_CalculateNodeID(heartbeatSlot);
      MC_SMGR_ReceivedNodeHeartbeat(node_id, pMsg->Payload.PhyDataInd.SNR, pMsg->Payload.PhyDataInd.RSSI, pHeartbeat->State, pHeartbeat->Rank, pHeartbeat->NoOfChildrenOfPTNIdx);

// Derive the slot index in long-frame from the heartbeat payload (e.g. 16-19, 36-39, 56-59, ...)
      SlotIndexInLongframeFromPayload = (pHeartbeat->ShortFrameIndex * SLOTS_PER_SHORT_FRAME) + pHeartbeat->SlotIndex;
      
      // If in TDM mode then read the slot index based on the reception time (e.g. 16-19, 36-39, 56-59, ...)
      // Otherwise we use the less reliable method of reading the content from the over-the-air packet, which may occasionally be corrupted.
      if (true == pMsg->Payload.PhyDataInd.isInTDMSyncSlaveMode)
      {
         // Read the slot index in long-frame from the value assigned upon reception of the packet.
         SlotIndexInLongframe = pMsg->Payload.PhyDataInd.slotIdxInLongframe;
      }
      else
      {
         // Use the value derived from the payload.
         SlotIndexInLongframe = SlotIndexInLongframeFromPayload;
      }
   
      if (pHeartbeat->SystemId == SM_SystemId && SlotIndexInLongframe == SlotIndexInLongframeFromPayload)
      {
         // Mark that the timestamp is valid, since all necessary validity tests on the message have passed.
         heartbeatValidated = true;
         rxTimer = pMsg->Payload.PhyDataInd.timer;
      }
      MC_MFH_Update( node_id, heartbeatSlot, pMsg->Payload.PhyDataInd.SNR, pMsg->Payload.PhyDataInd.RSSI, rxTimer, pHeartbeat->Rank, pHeartbeat->State, pHeartbeat->NoOfChildrenIdx, heartbeatValidated);
   }
}


/*************************************************************************************/
/**
* generateFireSignal
* state handler to generate fire signal
*
* @param - pointer to event data
*
* @return - void

*/
static void generateFireSignal(const uint8_t * const pData)
{
   AlarmSignal_t firesignal = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   CO_RBUSensorData_t MessageData;
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
   // Cast pData input parameter to the message type
   memcpy(&MessageData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_RBUSensorData_t));
   
   
   /* pack into phy data req and put into RACHPQ */
   pPhyDataReq = ALLOCMESHPOOL;
   if (pPhyDataReq)
   {
       /* ask Session Management for our parent node ID */
      uint16_t parent_node_id = MC_SMGR_GetPrimaryParentID();

      if ( CO_BAD_NODE_ID != parent_node_id )
      {
        /* populate fire signal */
         firesignal.Header.FrameType             = FRAME_TYPE_DATA_E;
         firesignal.Header.MACDestinationAddress = parent_node_id;
         firesignal.Header.MACSourceAddress      = gNetworkAddress;
         firesignal.Header.HopCount              = MessageData.HopCount;
         firesignal.Header.DestinationAddress    = NCU_NETWORK_ADDRESS;
         firesignal.Header.SourceAddress         = gNetworkAddress;
         firesignal.Header.MessageType           = APP_MSG_TYPE_FIRE_SIGNAL_E;
         firesignal.Zone                         = MessageData.Zone;
         firesignal.RUChannel                    = MessageData.RUChannelIndex;
         firesignal.AlarmActive                  = MessageData.AlarmState;
         firesignal.SensorValue                  = MessageData.SensorValue;
         firesignal.SystemId                     = SM_SystemId;
         
         bool multi_hop = true;
         if ( firesignal.Header.MACDestinationAddress == firesignal.Header.DestinationAddress )
         {
            multi_hop = false;
         }
         
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         status = MC_PUP_PackAlarmSignal(&firesignal, &pPhyDataReq->Payload.PhyDataReq);
         
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_GetTxPowerHigh();

         if (SUCCESS_E == status)
         {
            /* Add the message to the ACK Manager and it will be sent in the next RACH slot */
            CO_PRINT_B_1(DBG_INFO_E, "Sending Fire Signal. power=%d\r\n", pPhyDataReq->Payload.PhyDataReq.txPower);
            /* Add the message to the ACK Manager and it will manage any resending */
            bool msg_queued = MC_ACK_AddMessage(firesignal.Header.MACDestinationAddress, ACK_RACHP_E, APP_MSG_TYPE_FIRE_SIGNAL_E, firesignal.RUChannel, multi_hop, MessageData.Handle, pPhyDataReq, true, SEND_NORMAL_E);
            
            if ( msg_queued )
            {
               SM_TryToSendImmediately( ACK_RACHP_E, MAC_RACHP_Q_E );
            }
            else
            {
               /* failed to write */
               CO_PRINT_B_0(DBG_ERROR_E, "Failed to queue Fire Signal in RACHPQ\r\n");
            }
#ifdef DUPLICATE_FIRE_SIGNAL_ON_SRACH
            /* Add the message to the ACK Manager on S-RACH */
            msg_queued = MC_ACK_AddMessage(firesignal.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_FIRE_SIGNAL_E, firesignal.RUChannel, multi_hop, MessageData.Handle, pPhyDataReq, true, SEND_NORMAL_E);
            
            if ( msg_queued )
            {
               SM_TryToSendImmediately( ACK_RACHS_E, MAC_RACHS_Q_E );
            }
            else
            {
               /* failed to write */
               CO_PRINT_B_0(DBG_ERROR_E, "Failed to queue Fire Signal in RACHSQ\r\n");
            }
#endif
         }
      }
      else
      {
         CO_PRINT_B_0(DBG_ERROR_E, "generateFireSignal - Session Manager failed to resolve parent nodeID\r\n");
      }
      //We don't need the pool allocation anymore.  If it succeeded the ACK Mgr will have made a copy.
      FREEMESHPOOL(pPhyDataReq);
   }
   else
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generateFireSignal - MeshPool failed to allocate\r\n");
   }
}


/*************************************************************************************/
/**
* ncuFireSignal
* state handler for NCU receiving FireSignal
*
* @param - pointer to event data
*
* @return - void

*/
static void ncuFireSignal(const uint8_t * const pData)
{
   AlarmSignal_t firesignal = { 0 };
   ErrorCode_t status = ERR_UNEXPECTED_ERROR_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackAlarmSignal(&pMsg->Payload.PhyDataInd, &firesignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (firesignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (firesignal.SystemId == SM_SystemId) )
   {
      SM_SendACK(firesignal.Header.MACSourceAddress, ACK_RACHP_E, true);

      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(firesignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(firesignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx FireSignal NCU\r\n");
         
         if ( firesignal.Header.DestinationAddress == gNetworkAddress )
         {
            /* The message was addressed to this node.  Send it to the application. */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_FIRE_SIGNAL_E;
            AlarmSignalIndication_t fireEvent;
            fireEvent.SourceAddress = firesignal.Header.SourceAddress;
            fireEvent.RUChannelIndex = firesignal.RUChannel;
            fireEvent.AlarmActive = firesignal.AlarmActive;
            fireEvent.SensorValue = firesignal.SensorValue;
            fireEvent.Zone = firesignal.Zone;
            memcpy(appMessage.MessageBuffer, &fireEvent, sizeof(AlarmSignalIndication_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );

         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(PRIMARY_RACH, true);
}

/*************************************************************************************/
/**
* rbuFireSignal
* state handler for RBU receiving FireSignal
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuFireSignal(const uint8_t * const pData)
{
   AlarmSignal_t firesignal = { 0 };
   ErrorCode_t status = ERR_UNEXPECTED_ERROR_E;
   CO_Message_t *pPhyDataReq = NULL;
   AcknowledgedQueueID_t rach_channel = ACK_RACHP_E;
   MacQueueType_t mac_queue = MAC_RACHP_Q_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackAlarmSignal(&pMsg->Payload.PhyDataInd, &firesignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (firesignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (firesignal.SystemId == SM_SystemId) )
  
   {
      SM_SendACK(firesignal.Header.MACSourceAddress, rach_channel, false);
      
      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(firesignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(firesignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         if ( FIRST_SRACH_ACK_SLOT_IDX <= pMsg->Payload.PhyDataInd.slotIdxInShortframe )
         {
            rach_channel = ACK_RACHS_E;
            mac_queue = MAC_RACHS_Q_E;
         }
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx FireSignal RBU\r\n");
          
         /* do multi-hop if firesignal.Header.DestinationAddress is not the address of this unit */
         if ( firesignal.Header.DestinationAddress != gNetworkAddress )
         {
            /* The message is not for us.  Forward it to a parent node. */
            uint32_t parent_node = MC_SMGR_GetPrimaryParentID();
            if ( CO_BAD_NODE_ID != parent_node )
            {
               uint16_t message_source = firesignal.Header.MACSourceAddress;
               (void)message_source;   // suppress warnings
               
               /* address the message to the parent and increment the hop count */
               firesignal.Header.MACDestinationAddress = parent_node;
               firesignal.Header.MACSourceAddress = gNetworkAddress;
               firesignal.Header.HopCount++;
               
               bool multi_hop = true;
               if( firesignal.Header.MACDestinationAddress == firesignal.Header.DestinationAddress )
               {
                  multi_hop = false;
               }
               
               if ( firesignal.Header.HopCount < CFG_GetMaxHops() )
               {
               
                  /* pack into phy data req and put into RACHPQ */
                  pPhyDataReq = ALLOCMESHPOOL;
                  if (pPhyDataReq)
                  {
                     pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                     pPhyDataReq->Payload.PhyDataReq.Downlink = false;
                     status = MC_PUP_PackAlarmSignal(&firesignal, &pPhyDataReq->Payload.PhyDataReq);
                     
                     pPhyDataReq->Payload.PhyDataReq.txPower = MC_GetTxPowerHigh();

                     if (SUCCESS_E == status)
                     {
                        CO_PRINT_B_2(DBG_INFO_E, "MULTI-HOP - Rx'd fire signal from node %d.  Forwarding to node %d\r\n", message_source, parent_node);

                        /* Add the message to the ACK Manager and it will manage any resending */
                        bool msg_queued = MC_ACK_AddMessage(firesignal.Header.MACDestinationAddress, rach_channel, APP_MSG_TYPE_FIRE_SIGNAL_E, firesignal.RUChannel, multi_hop, 0, pPhyDataReq, true, SEND_NORMAL_E);
                        if ( msg_queued )
                        {
                           SM_TryToSendImmediately( rach_channel, mac_queue );
                        }
                        else
                        {
                           /* failed to write */
                           CO_PRINT_B_1(DBG_ERROR_E, "Failed to queue Fire Signal in %s-RACH Queue\r\n", ACK_RACHP_E == rach_channel ? "P":"S");
                        }
                     }
                     /* the ack manager makes a copy so we can release the memory */
                     FREEMESHPOOL(pPhyDataReq);
                  }
                  else
                  {
                     CO_PRINT_A_0(DBG_ERROR_E,"rbuFireSignal - MeshPool failed to allocate\r\n");
                  }
               }
               else
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "MULTI-HOP - DISCARDED Fire Signal - hop limit reached\r\n");
               }
            }
            else
            {
               CO_PRINT_B_0(DBG_INFO_E, "MULTI-HOP - DISCARDED Fire Signal - failed to find parent\r\n");
            }
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(PRIMARY_RACH, true);
}

/*************************************************************************************/
/**
* ncuAlarmSignal
* state handler for NCU receiving AlarmSignal
*
* @param - pointer to event data
*
* @return - void

*/
static void ncuAlarmSignal(const uint8_t * const pData)
{
   AlarmSignal_t alarmSignal = { 0 };
   ErrorCode_t status = SUCCESS_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackAlarmSignal(&pMsg->Payload.PhyDataInd, &alarmSignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (alarmSignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (alarmSignal.SystemId == SM_SystemId) )
  
   {
      SM_SendACK(alarmSignal.Header.MACSourceAddress, ACK_RACHS_E, true);
      
      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(alarmSignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(alarmSignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Alarm Signal NCU\r\n");
        
         if ( alarmSignal.Header.DestinationAddress == gNetworkAddress )
         {
            /* The message was addressed to this node.  Send it to the application. */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_ALARM_SIGNAL_E;
            AlarmSignalIndication_t alarmEvent;
            alarmEvent.SourceAddress = alarmSignal.Header.SourceAddress;
            alarmEvent.RUChannelIndex = alarmSignal.RUChannel;
            alarmEvent.AlarmActive = alarmSignal.AlarmActive;
            alarmEvent.SensorValue = alarmSignal.SensorValue;
            alarmEvent.Zone = alarmSignal.Zone;
            memcpy(appMessage.MessageBuffer, &alarmEvent, sizeof(AlarmSignalIndication_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* rbuAlarmSignal
* state handler for RBU receiving AlarmSignal
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuAlarmSignal(const uint8_t * const pData)
{
   AlarmSignal_t alarmSignal = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackAlarmSignal(&pMsg->Payload.PhyDataInd, &alarmSignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (alarmSignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (alarmSignal.SystemId == SM_SystemId) )
  
   {
      SM_SendACK( alarmSignal.Header.MACSourceAddress, ACK_RACHS_E, false);
      
      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(alarmSignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(alarmSignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Alarm Signal RBU\r\n");
        
          /* do multi-hop if alarmSignal.Header.DestinationAddress is not the address of this unit */
         if ( alarmSignal.Header.DestinationAddress != gNetworkAddress )
         {
            /* The message is not for us.  Forward it to a parent node. */
            uint32_t parent_node = MC_SMGR_GetPrimaryParentID();
            if ( CO_BAD_NODE_ID != parent_node )
            {
               uint16_t message_source = alarmSignal.Header.MACSourceAddress;
               (void)message_source;   // suppress warnings
               
               /* address the message to the parent and increment the hop count */
               alarmSignal.Header.MACDestinationAddress = parent_node;
               alarmSignal.Header.MACSourceAddress = gNetworkAddress;
               alarmSignal.Header.HopCount++;
               
               bool multi_hop = true;
               if( alarmSignal.Header.MACDestinationAddress == alarmSignal.Header.DestinationAddress )
               {
                  multi_hop = false;
               }
               
               if ( alarmSignal.Header.HopCount < CFG_GetMaxHops() )
               {
                  /* pack into phy data req and put into RACHPQ */
                  pPhyDataReq = ALLOCMESHPOOL;
                  if (pPhyDataReq)
                  {
                     pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                     pPhyDataReq->Payload.PhyDataReq.Downlink = false;
                     pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
                     status = MC_PUP_PackAlarmSignal(&alarmSignal, &pPhyDataReq->Payload.PhyDataReq);

                     if (SUCCESS_E == status)
                     {
                        CO_PRINT_B_2(DBG_INFO_E, "MULTI-HOP - Rx'd alarm signal from node %d.  Forwarding to node %d\r\n", message_source, parent_node);
                        /* Add the message to the ACK Manager and it will manage any resending */
                        bool msg_queued = MC_ACK_AddMessage(alarmSignal.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, alarmSignal.RUChannel, multi_hop, 0, pPhyDataReq, true, SEND_NORMAL_E);
                        
                        if ( msg_queued )
                        {
                           SM_TryToSendImmediately( ACK_RACHS_E, MAC_RACHS_Q_E );
                        }
                        else
                        {
                           /* failed to write */
                           CO_PRINT_B_0(DBG_ERROR_E, "Failed to queue Alarm Signal in S-RACH\r\n");
                        }
                     }
                     /* the ack manager makes a copy so we can release the memory */
                     FREEMESHPOOL(pPhyDataReq);
                  }
                  else
                  {
                     CO_PRINT_A_0(DBG_ERROR_E,"rbuAlarmSignal - MeshPool failed to allocate\r\n");
                  }
               }
               else
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "MULTI-HOP - DISCARDED Alarm Signal - hop limit reached\r\n");
               }
            }
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* generateFaultSignal
* state handler to generate fault signal
*
* @param - pointer to event data
*
* @return - void

*/
void generateFaultSignal(const uint8_t * const pData)
{
   FaultSignal_t faultSignal = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   CO_FaultData_t FaultData;
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   uint16_t parent_address = MC_SMGR_GetPrimaryParentID();

   if (CO_BAD_NODE_ID != parent_address)
   {
      CO_Message_t* pMessage = (CO_Message_t*)pData;
         
      // Cast pData input parameter to the message type
      memcpy(&FaultData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_FaultData_t));
      
      /* pack into phy data req and put into RACH Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         /* populate fire signal */
         faultSignal.Header.FrameType             = FRAME_TYPE_DATA_E;
         faultSignal.Header.MACDestinationAddress = parent_address;
         faultSignal.Header.MACSourceAddress      = gNetworkAddress;
         faultSignal.Header.HopCount              = 0;
         faultSignal.Header.DestinationAddress    = NCU_NETWORK_ADDRESS;
         faultSignal.Header.SourceAddress         = gNetworkAddress;
         faultSignal.Header.MessageType           = APP_MSG_TYPE_FAULT_SIGNAL_E;
         faultSignal.RUChannelIndex               = FaultData.RUChannelIndex;
         faultSignal.FaultType                    = FaultData.FaultType;
         faultSignal.Value                        = FaultData.Value;
         faultSignal.OverallFault                 = FaultData.OverallFault;
         faultSignal.SystemId                     = SM_SystemId;
         
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         status = MC_PUP_PackFaultSignal(&faultSignal, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
            if ( FaultData.DelaySending )
            {
               if ( MC_SACH_ScheduleMessage( faultSignal.Header.MACDestinationAddress, APP_MSG_TYPE_FAULT_SIGNAL_E, faultSignal.FaultType, pPhyDataReq, SEND_NORMAL_E) )
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Put FAULT signal in DULCH Queue\r\n");
               }
               else 
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "FAILED to queue Fault Signal on DULCH.\n\r");
               }
            }
            else 
            {
               bool multi_hop = false;
               if ( faultSignal.Header.MACDestinationAddress != faultSignal.Header.DestinationAddress )
               {
                  multi_hop = true;
               }
               /* Add this signal to AckManagement and it will be sent in the next RACH slot */
               if ( false == MC_ACK_AddMessage(faultSignal.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_FAULT_SIGNAL_E, faultSignal.FaultType, multi_hop, FaultData.Handle, pPhyDataReq, true, SEND_NORMAL_E))
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "FAILED to queue Fault Signal.\n\r");
               }
            }

         }
         else 
         {
            CO_PRINT_B_0(DBG_ERROR_E, "FAILED to generate Fault Signal.\n\r");
         }
         /* the ACK and SACH manager make a copy so we can release the memory */
         FREEMESHPOOL(pPhyDataReq);
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateFaultSignal - MeshPool failed to allocate\r\n");
      }
   }
   else
   {
      CO_PRINT_B_0(DBG_ERROR_E, "FAILED to send Fault Signal.  No parent nodes found.\n\r");
   }

}



/*************************************************************************************/
/**
* ncuFaultSignal
* state handler for NCU receiving a Fault Signal
*
* @param - pointer to event data
*
* @return - void

*/
static void ncuFaultSignal(const uint8_t * const pData)
{
   FaultSignal_t faultSignal = { 0 };
   ErrorCode_t status = SUCCESS_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackFaultSignal(&pMsg->Payload.PhyDataInd, &faultSignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (faultSignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (faultSignal.SystemId == SM_SystemId) )
  
   {
      SM_SendACK( faultSignal.Header.MACSourceAddress, ACK_RACHS_E, false);
      
      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(faultSignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(faultSignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Fault Signal NCU\r\n");

         if ( faultSignal.Header.DestinationAddress == gNetworkAddress )
         {
            /* The message was addressed to this node.  Send it to the application. */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_FAULT_SIGNAL_E;
            FaultSignalIndication_t faultEvent;
            faultEvent.SourceAddress = faultSignal.Header.SourceAddress;
            faultEvent.RUChannelIndex = faultSignal.RUChannelIndex;
            faultEvent.FaultType = faultSignal.FaultType;
            faultEvent.Value = faultSignal.Value;
            faultEvent.OverallFault = faultSignal.OverallFault;
            memcpy(appMessage.MessageBuffer, &faultEvent, sizeof(FaultSignalIndication_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* rbuFaultSignal
* state handler for RBU receiving a Fault Signal
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuFaultSignal(const uint8_t * const pData)
{
   FaultSignal_t faultSignal = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackFaultSignal(&pMsg->Payload.PhyDataInd, &faultSignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (faultSignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (faultSignal.SystemId == SM_SystemId) )
  
   {
      SM_SendACK( faultSignal.Header.MACSourceAddress, ACK_RACHS_E, false);

      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(faultSignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(faultSignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Fault Signal RBU\r\n");

          /* do multi-hop if alarmSignal.Header.DestinationAddress is not the address of this unit */
         if ( faultSignal.Header.DestinationAddress != gNetworkAddress )
         {
            /* The message is not for us.  Forward it to a parent node. */
            uint32_t parent_node = MC_SMGR_GetPrimaryParentID();
            if ( CO_BAD_NODE_ID != parent_node )
            {
               uint16_t message_source = faultSignal.Header.MACSourceAddress;
               (void)message_source;   // suppress warnings

               /* address the message to the parent and increment the hop count */
               faultSignal.Header.MACDestinationAddress = parent_node;
               faultSignal.Header.MACSourceAddress = gNetworkAddress;
               faultSignal.Header.HopCount++;
               
               if ( faultSignal.Header.HopCount < CFG_GetMaxHops() )
               {
                  /* pack into phy data req and put into RACHPQ */
                  pPhyDataReq = ALLOCMESHPOOL;
                  if (pPhyDataReq)
                  {
                     bool multi_hop = true;
                     if( faultSignal.Header.MACDestinationAddress == faultSignal.Header.DestinationAddress )
                     {
                        multi_hop = false;
                     }
                     
                     pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                     pPhyDataReq->Payload.PhyDataReq.Downlink = false;
                     pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
                     status = MC_PUP_PackFaultSignal(&faultSignal, &pPhyDataReq->Payload.PhyDataReq);

                     if (SUCCESS_E == status)
                     {
                        CO_PRINT_B_2(DBG_INFO_E, "MULTI-HOP - Rx'd fault signal from node %d.  Forwarding to node %d\r\n", message_source, parent_node);
                        /* Add this signal to AckManagement and it will be sent in the next RACH slot */
                        MC_ACK_AddMessage(faultSignal.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_FAULT_SIGNAL_E, faultSignal.RUChannelIndex, multi_hop, 0, pPhyDataReq, true, SEND_NORMAL_E);
                     }
                     /* the ack manager makes a copy so we can release the memory */
                     FREEMESHPOOL(pPhyDataReq);
                  }
                  else
                  {
                     CO_PRINT_A_0(DBG_ERROR_E,"rbuFaultSignal - MeshPool failed to allocate\r\n");
                  }
               }
               else
               {
                  CO_PRINT_B_0(DBG_INFO_E, "MULTI-HOP - DISCARDED Fault Signal - hop limit reached\r\n");
               }
            }
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* generateAlarmSignal
* state handler to generate alarm signal
*
* @param - pointer to event data
*
* @return - void

*/
static void generateAlarmSignal(const uint8_t * const pData)
{
   AlarmSignal_t alarmSignal = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   CO_RBUSensorData_t MessageData;
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
	// Cast pData input parameter to the message type
   memcpy(&MessageData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_RBUSensorData_t));
	
   
   /* pack into phy data req and put into RACH Q */
   pPhyDataReq = ALLOCMESHPOOL;
   if (pPhyDataReq)
   {
      /* ask Session Management for our parent node ID */
      uint16_t parent_node_id = MC_SMGR_GetPrimaryParentID();
      if ( CO_BAD_NODE_ID != parent_node_id )
      {
         /* populate fire signal */
         alarmSignal.Header.FrameType             = FRAME_TYPE_DATA_E;
         alarmSignal.Header.MACDestinationAddress = parent_node_id;
         alarmSignal.Header.MACSourceAddress      = gNetworkAddress;
         alarmSignal.Header.HopCount              = MessageData.HopCount;
         alarmSignal.Header.DestinationAddress    = NCU_NETWORK_ADDRESS;
         alarmSignal.Header.SourceAddress         = gNetworkAddress;
         alarmSignal.Header.MessageType           = APP_MSG_TYPE_ALARM_SIGNAL_E;
         alarmSignal.Zone                         = MessageData.Zone;
         alarmSignal.RUChannel                    = MessageData.RUChannelIndex;
         alarmSignal.SensorValue                  = MessageData.SensorValue;
         alarmSignal.AlarmActive                  = MessageData.AlarmState;
         alarmSignal.SystemId                     = SM_SystemId;
         
         bool multi_hop = true;
         if( alarmSignal.Header.MACDestinationAddress == alarmSignal.Header.DestinationAddress )
         {
            multi_hop = false;
         }
                  
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         status = MC_PUP_PackAlarmSignal(&alarmSignal, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
            CO_PRINT_B_2(DBG_INFO_E, "Sending Alarm Signal from node %d to node %d\r\n", alarmSignal.Header.MACSourceAddress, alarmSignal.Header.MACDestinationAddress);
            /* Add the message to the ACK Manager and it will manage any resending */
            bool msg_queued = MC_ACK_AddMessage(alarmSignal.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_ALARM_SIGNAL_E, MessageData.RUChannelIndex, multi_hop, MessageData.Handle, pPhyDataReq, true, SEND_NORMAL_E);

            if ( msg_queued )
            {
               //If there is an alarm signal ready to send, do it now so that we get 'next slot' response
               SM_TryToSendImmediately( ACK_RACHS_E, MAC_RACHS_Q_E );
            }
            else
            {
               /* failed to write */
               CO_PRINT_B_0(DBG_ERROR_E, "Failed to queue Alarm Signal in S-RACH\r\n");
            }
         }
      }
      else
      {
         CO_PRINT_B_0(DBG_ERROR_E, "generateAlarmSignal - Session Manager failed to resolve parent nodeID\r\n");
      }
      
      /* the ack manager makes a copy so we can release the memory */
      FREEMESHPOOL(pPhyDataReq);
   }
   else
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generateAlarmSignal - MeshPool failed to allocate\r\n");
   }
}



/*************************************************************************************/
/**
* generateOutputSignal
* state handler to generate output signal
*
* @param - pointer to event data
*
* @return - void

*/
static void generateOutputSignal(const uint8_t * const pData)
{
   bool at_least_one_msg_was_queued = false;
   OutputSignal_t outputsignal = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   CO_OutputData_t OutData;
//   uint32_t i = 0;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
	// Cast pData input parameter to the message type
   memcpy(&OutData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_OutputData_t));
   
   /* populate fire signal */
   outputsignal.Header.FrameType             = FRAME_TYPE_DATA_E;
   outputsignal.Header.MACDestinationAddress = ADDRESS_GLOBAL;
   outputsignal.Header.MACSourceAddress      = SM_address;
   outputsignal.Header.HopCount              = 0;
   outputsignal.Header.DestinationAddress    = OutData.Destination;
   outputsignal.Header.SourceAddress         = SM_address;
   outputsignal.Header.MessageType           = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
   outputsignal.Zone                         = OutData.zone;
   outputsignal.OutputChannel                = OutData.OutputChannel;
   outputsignal.OutputProfile                = OutData.OutputProfile;
   outputsignal.OutputsActivated             = OutData.OutputsActivated;
   outputsignal.OutputDuration               = OutData.OutputDuration;
   outputsignal.SystemId                     = SM_SystemId;

   for (uint8_t i = 0; i < OutData.NumberToSend; i++)
   {
      /* pack into phy data req and put into DLCCHx Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         status = MC_PUP_PackOutputSignal(&outputsignal, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
            pPhyDataReq->Payload.PhyDataReq.txPower = MC_GetTxPowerHigh();
            
            status = MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq);
            if ( SUCCESS_E == status )
            {
               osStat = osOK;
               at_least_one_msg_was_queued = true;
               CO_PRINT_B_0(DBG_INFO_E, "Put output signal on DLCCHSQ\r\n");
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E, "Failed to add msg to DLCCHSQ Queue - Error %d\r\n", status);
               osStat = osErrorOS;
            }

         }
         //The MAC Queue makes a copy so we can free the message
         FREEMESHPOOL(pPhyDataReq);
         
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateOutputSignal - MeshPool failed to allocate\r\n");
         status = ERR_NO_MEMORY_E;
      }
      
      if ((SUCCESS_E != status) || (osOK != osStat))
      {
         break;
      }
   }
   
   if (at_least_one_msg_was_queued)
   {
      /* Let the application know the msg was queued */
      SM_SendConfirmationToApplication(OutData.Handle, 0, APP_CONF_OUTPUT_SIGNAL_E, SUCCESS_E);
   }
   else
   {
      /* Let the application know the msg failed */
      SM_SendConfirmationToApplication(OutData.Handle, 0, APP_CONF_OUTPUT_SIGNAL_E, ERR_MESSAGE_FAIL_E);
   }
}

/*************************************************************************************/
/**
* rbuOutputSignal
* state handler for RBU receiving OutputSignal
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuOutputSignal(const uint8_t * const pData)
{
   OutputSignal_t outputsignal = { 0 };
   OutputSignal_t *pOutputSignal = &outputsignal;
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackOutputSignal(&pMsg->Payload.PhyDataInd, pOutputSignal);
   if ( SUCCESS_E == status && pOutputSignal->SystemId == SM_SystemId)
   {
#ifdef SHOW_DOWNLINK_LED
      DM_LedPatternRequest(LED_DOWNLINK_MESSAGE_E);
#endif
      /* Only act on the message if it came from a parent */
      if ( MC_SMGR_IsParentNode(pOutputSignal->Header.MACSourceAddress) )
      {
         /* only retransmit the message if it is not addressed to us and we have children downlink */
         if (/*(0 < MC_SMGR_GetNumberOfChildren()) && */ (SM_address != outputsignal.Header.DestinationAddress) )
         {
            /* create PhyDataRequest for transmission on DLCCH to children */
            pPhyDataReq = ALLOCMESHPOOL;
            if (pPhyDataReq)
            {
               /* populate PhyDataRequest using received data. Update MACSourceAddress and hop count*/
               pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
               outputsignal.Header.MACSourceAddress = gNetworkAddress;
               outputsignal.Header.HopCount++;
               if ( outputsignal.Header.HopCount < CFG_GetMaxHops() )
               {
                  status = MC_PUP_PackOutputSignal(pOutputSignal, &pPhyDataReq->Payload.PhyDataReq);
                  pPhyDataReq->Payload.PhyDataReq.txPower = MC_GetTxPowerHigh();
                  
                  if (CO_PROFILE_FIRE_E == outputsignal.OutputProfile)
                  {
                     status = MC_MACQ_Push(MAC_DLCCHP_Q_E, pPhyDataReq);
                     if ( SUCCESS_E != status )
                     {
                        CO_PRINT_B_1(DBG_ERROR_E, "Failed to add output cmd to DLCCHPQ Queue - Error %d\r\n", status);
                     }
                     else
                     {
                        CO_PRINT_B_0(DBG_INFO_E, "Put output cmd on DLCCHPQ\r\n");
                     }
                  }
                  else 
                  {
                     status = MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq);
                     if ( SUCCESS_E != status )
                     {
                        CO_PRINT_B_1(DBG_ERROR_E, "Failed to add output cmd to DLCCHSQ Queue - Error %d\r\n", status);
                     }
                     else
                     {
                        CO_PRINT_B_0(DBG_INFO_E, "Put output cmd on DLCCHSQ\r\n");
                     }
                  }
               }
               //The MAC Queue makes a copy so we can free the message
               FREEMESHPOOL(pPhyDataReq);
            }
         }

         /* if the output signal is for this node, pass it to the application */
         if ( true == SMCheckDestinationAddress(pOutputSignal->Header.DestinationAddress, pOutputSignal->Zone) )
         {
            CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< Rx Output Signal to 0x%x\r\n", pOutputSignal->Header.DestinationAddress);
            
            /* Send the output request up to the Application */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_OUTPUT_SIGNAL_E;
            OutputSignalIndication_t outData;
            outData.nodeID = pOutputSignal->Header.DestinationAddress;
            outData.zone = outputsignal.Zone;
            outData.OutputChannel = outputsignal.OutputChannel;
            outData.OutputProfile = outputsignal.OutputProfile;
            outData.OutputsActivated = outputsignal.OutputsActivated;
            outData.OutputDuration = outputsignal.OutputDuration;
            memcpy(appMessage.MessageBuffer, &outData, sizeof(OutputSignalIndication_t));
      
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
}

/*************************************************************************************/
/**
* generateAlarmOutputState
* state handler to generate an alarm output state message
*
* @param - pointer to event data
*
* @return - void

*/
void generateAlarmOutputState(const uint8_t * const pData)
{
   bool at_least_one_msg_was_queued = false;
   AlarmOutputState_t alarmOutputState = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   CO_AlarmOutputStateData_t OutData;
//   uint32_t i = 0;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
	// Cast pData input parameter to the message type
   memcpy(&OutData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_AlarmOutputStateData_t));
   
   /* populate fire signal */
   alarmOutputState.Header.FrameType             = FRAME_TYPE_DATA_E;
   alarmOutputState.Header.MACDestinationAddress = OutData.Source;
   alarmOutputState.Header.MACSourceAddress      = NCU_NETWORK_ADDRESS;
   alarmOutputState.Header.HopCount              = 0;
   alarmOutputState.Header.DestinationAddress    = OutData.Source;
   alarmOutputState.Header.SourceAddress         = NCU_NETWORK_ADDRESS;
   alarmOutputState.Header.MessageType           = APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E;
   alarmOutputState.Silenceable                  = OutData.Silenceable;
   alarmOutputState.Unsilenceable                = OutData.Unsilenceable;
   alarmOutputState.DelayMask                    = OutData.DelayMask;
   alarmOutputState.SystemId                     = SM_SystemId;

   for (uint8_t i = 0; i < OutData.NumberToSend; i++)
   {
      /* pack into phy data req and put into DLCCHx Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_GetTxPowerHigh();
         status = MC_PUP_PackAlarmOutputState(&alarmOutputState, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
            status = MC_MACQ_Push(MAC_DLCCHP_Q_E, pPhyDataReq);
            if ( SUCCESS_E != status )
            {
               CO_PRINT_B_1(DBG_ERROR_E, "Failed to add AOS to DLCCHPQ Queue - Error %d\r\n", status);
            }
            else
            {
               CO_PRINT_B_0(DBG_INFO_E, "Put AOS on DLCCHPQ\r\n");
               at_least_one_msg_was_queued = true;
            }
         }
         //The MAC Queue makes a copy so we can free the message
         FREEMESHPOOL(pPhyDataReq);
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateOutputSignal - MeshPool failed to allocate\r\n");
         status = ERR_NO_MEMORY_E;
      }
      
   }
   
   if (at_least_one_msg_was_queued)
   {
      /* Let the application know the msg was queued */
      SM_SendConfirmationToApplication(OutData.Handle, 0, APP_CONF_ALARM_OUTPUT_STATE_SIGNAL_E, SUCCESS_E);
   }
   else
   {
      /* Let the application know the msg failed */
      SM_SendConfirmationToApplication(OutData.Handle, 0, APP_CONF_ALARM_OUTPUT_STATE_SIGNAL_E, ERR_MESSAGE_FAIL_E);
   }
}

/*************************************************************************************/
/**
* rbuAlarmOutputState
* state handler for RBU receiving an AOS signal
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuAlarmOutputState(const uint8_t * const pData)
{
   AlarmOutputState_t outputsignal = { 0 };
   AlarmOutputState_t *pOutputSignal = &outputsignal;
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackAlarmOutputState(&pMsg->Payload.PhyDataInd, pOutputSignal);
   if ( SUCCESS_E == status && pOutputSignal->SystemId == SM_SystemId)
   {
#ifdef SHOW_DOWNLINK_LED
      DM_LedPatternRequest(LED_DOWNLINK_MESSAGE_E);
#endif
      /* Only act on the message if it came from a parent */
      if ( MC_SMGR_IsParentNode(pOutputSignal->Header.MACSourceAddress) )
      {
         /* only retransmit the message if it is not addressed to us and we have children downlink */
         if (/*(0 < MC_SMGR_GetNumberOfChildren()) && */ (SM_address != outputsignal.Header.DestinationAddress) )
         {
            /* create PhyDataRequest for transmission on DLCCH to children */
            pPhyDataReq = ALLOCMESHPOOL;
            if (pPhyDataReq)
            {
               /* populate PhyDataRequest using received data. Update MACSourceAddress and hop count*/
               pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
               pPhyDataReq->Payload.PhyDataReq.txPower = MC_GetTxPowerHigh();
               outputsignal.Header.MACSourceAddress = gNetworkAddress;
               outputsignal.Header.HopCount++;
               status = MC_PUP_PackAlarmOutputState(pOutputSignal, &pPhyDataReq->Payload.PhyDataReq);
               /* add to DLCCHQ for downward transmission to children */

               status = MC_MACQ_Push(MAC_DLCCHP_Q_E, pPhyDataReq);
               if ( SUCCESS_E == status )
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Put AOS on DLCCHPQ\r\n");
               }
               else
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "Failed to add AOS to DLCCHPQ Queue - Error %d\r\n", status);
               }
               //The MAC Queue makes a copy so we can free the message
               FREEMESHPOOL(pPhyDataReq);
            }
         }

         /* if the output signal is for this node, pass it to the application */
         if ( true == SMCheckDestinationAddress(pOutputSignal->Header.DestinationAddress, ZONE_GLOBAL) )
         {
            CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< Rx Alarm Output State Signal to 0x%x\r\n", pOutputSignal->Header.DestinationAddress);
            
            /* Send the output request up to the Application */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E;
            CO_AlarmOutputStateData_t outData;
            outData.Silenceable = outputsignal.Silenceable;
            outData.Unsilenceable = outputsignal.Unsilenceable;
            outData.DelayMask = outputsignal.DelayMask;
            memcpy(appMessage.MessageBuffer, &outData, sizeof(CO_AlarmOutputStateData_t));
      
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
}


/*************************************************************************************/
/**
* SMCheckDestinationAddress
* Routine to check the destination address of a received message matches this unit
*
* @param - const uint32_t destAddress - from received message
* @param - const uint32_t targetZone - the zone that the message was sent to
*
* @return - bool - true if message is valid for this rbu, else false
*/
bool SMCheckDestinationAddress(const uint32_t destAddress, const uint32_t targetZone)
{
   bool isValid = false;
   // check for message addressed to all units
   if (ADDRESS_GLOBAL == destAddress)
   {
      /* Check to see if this unit is in the target zone */
      if ( gZoneNumber == targetZone || ZONE_GLOBAL == targetZone )
      {
         // address match
         isValid = true;
      }
   }
   else
   {
      if ( SM_address == destAddress )
      {
         // individual address match
         isValid = true;
      }
   }
   
   return isValid;
}



/*************************************************************************************/
/**
* SMInitialise
* Routine to initialise the state machine module
*
* @param - bool isNCU - true for NCU otherwise false
* @param - uint32_t address - address of node in mesh
* @param - uint32_t systemId - system ID of the mesh
*
* @return - void

*/
void SMInitialise(bool isNCU, uint32_t address, const uint32_t systemId)
{
   // Initialise mesh forming and healing module
   MC_MFH_Init(isNCU);
   MC_SYNC_Initialise((uint16_t)address);
   SM_IsNCU = isNCU;
   SM_address = address;
   MC_STATE_Initialise();
   SM_SystemId = systemId;
   
   /* Tell the ACK Management which function to call if it accepts a duplicate message */
   MC_ACK_SetDuplicateMessageCallback(SM_DuplicateMessageCallback);
}

/*************************************************************************************/
/**
* SMHandleEvent
* Routine to handle a single event using the state machine
*
* @param - event
* @param - pointer to event data structure
*
* @return - void
*/
void SMHandleEvent(SM_Event_t event, uint8_t *pData)
{
   if ( pData )
   {
      void (*pfAction) (const uint8_t * const pData);
      CO_State_t device_state = MC_STATE_GetDeviceState();
      
      if ( STATE_SLEEP_MODE_E == device_state )
      {
         device_state = STATE_IDLE_E;
      }

      // suppress warnings
      (void)StateName;
      (void)EventName;
      
      /* get handler function */
      if (true == SM_IsNCU)
      {
         pfAction = pfNCUTransition[event][device_state];
      }
      else
      {
         pfAction = pfRBUTransition[event][device_state];
      }
      
      /* execute the event */
      if ( pfAction )
      {
         (pfAction)(pData);
      }
      else 
      {
         CO_PRINT_A_1(DBG_ERROR_E,"FAILED to map state machine event %d\r\n",event);
      }
   }
}


/*************************************************************************************/
/**
* ackSignal
* state handler for RBU receiving ackSignal
*
* @param - pointer to event data
*
* @return - void

*/
static void ackSignal(const uint8_t * const pData)
{
   FrameAcknowledge_t FrameAcknowledgement;
   uint32_t MessageHandle;
   ApplicationLayerMessageType_t MessageType;
   ErrorCode_t status = SUCCESS_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackAck(&pMsg->Payload.PhyDataInd, &FrameAcknowledgement);

   if ( SUCCESS_E == status && FrameAcknowledgement.SystemId == SM_SystemId)
   {
      //CO_PRINT_B_4(DBG_INFO_E, "ft=%d isaP=%d isaT=%d isaC=%d\r\n", FrameAcknowledgement.FrameType, MC_SMGR_IsParentNode( FrameAcknowledgement.MACSourceAddress ), MC_SMGR_IsATrackingNode(FrameAcknowledgement.MACSourceAddress), MC_SMGR_IsAChild(FrameAcknowledgement.MACSourceAddress));
      uint16_t rxSlot = pMsg->Payload.PhyDataInd.slotIdxInShortframe;
      MC_TDM_SlotType_t rxSlotType = MC_TDM_GetSlotTypeDefault( rxSlot );
      
      if ( FRAME_TYPE_ACKNOWLEDGEMENT_E == FrameAcknowledgement.FrameType &&
           gNetworkAddress == FrameAcknowledgement.MACDestinationAddress )
      {
   
         if ( MC_TDM_SLOT_TYPE_PRIMARY_ACK_E == rxSlotType )
         {
            if ( MC_ACK_AcknowledgeMessage( ACK_RACHP_E, FrameAcknowledgement.MACSourceAddress, true, &MessageType, &MessageHandle ) )
            {
               CO_PRINT_B_0(DBG_INFO_E, "Message acknowledged RACHP\r\n");
               if ( APP_MSG_TYPE_FIRE_SIGNAL_E == MessageType )
               {
                  SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_FIRE_SIGNAL_E, SUCCESS_E);
               }
            }
            else
            {
               CO_PRINT_B_0(DBG_ERROR_E, "Ack Manager failed to find message for RACHP ACK\r\n");
            }
         }
         else if ( MC_TDM_SLOT_TYPE_SECONDARY_ACK_E == rxSlotType )
         {
            if ( MC_ACK_AcknowledgeMessage( ACK_RACHS_E, FrameAcknowledgement.MACSourceAddress, true, &MessageType, &MessageHandle ) )
            {
               CO_PRINT_B_0(DBG_INFO_E, "Message acknowledged RACHS\r\n");
               switch( MessageType )
               {
                  case APP_MSG_TYPE_FIRE_SIGNAL_E:
                     break;
                  case APP_MSG_TYPE_ALARM_SIGNAL_E:
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_ALARM_SIGNAL_E, SUCCESS_E);
                     break;
                  case APP_MSG_TYPE_FAULT_SIGNAL_E:
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_FAULT_SIGNAL_E, SUCCESS_E);
                     break;
                  case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
                     break;
                  case APP_MSG_TYPE_COMMAND_E:
                     break;
                  case APP_MSG_TYPE_RESPONSE_E:
                     break;
                  case APP_MSG_TYPE_LOGON_E:
                     /* let the application know that the logon was successful */
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_LOGON_SIGNAL_E, SUCCESS_E);
                     MC_SMGR_SetLoggedOn();
                     break;
                  case APP_MSG_TYPE_STATUS_INDICATION_E:
                     break;
                  case APP_MSG_TYPE_APP_FIRMWARE_E:
                     break;
                  case APP_MSG_TYPE_ROUTE_ADD_E:
                     break;
                  case APP_MSG_TYPE_ROUTE_DROP_E:
                     break;
                  case APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E:
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_BATTERY_STATUS_SIGNAL_E, SUCCESS_E);
                     break;
                  case APP_MSG_TYPE_PING_E:
                     break;
                 default:
                  break;
               }
            }
         }
         else if ( MC_TDM_SLOT_TYPE_DULCH_ACK_E == rxSlotType )
         {
            if ( MC_ACK_AcknowledgeMessage( ACK_DULCH_E, FrameAcknowledgement.MACSourceAddress, true, &MessageType, &MessageHandle ) )
            {
               CO_PRINT_B_0(DBG_INFO_E, "Message acknowledged DULCH\r\n");
               switch( MessageType )
               {
                  case APP_MSG_TYPE_FIRE_SIGNAL_E:
                     break;
                  case APP_MSG_TYPE_ALARM_SIGNAL_E:
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_ALARM_SIGNAL_E, SUCCESS_E);
                     break;
                  case APP_MSG_TYPE_FAULT_SIGNAL_E:
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_FAULT_SIGNAL_E, SUCCESS_E);
                     break;
                  case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
                     break;
                  case APP_MSG_TYPE_COMMAND_E:
                     break;
                  case APP_MSG_TYPE_RESPONSE_E:
                     break;
                  case APP_MSG_TYPE_LOGON_E:
                     /* let the application know that the logon was successful */
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_LOGON_SIGNAL_E, SUCCESS_E);
                     MC_SMGR_SetLoggedOn();
                     break;
                  case APP_MSG_TYPE_STATUS_INDICATION_E:
                     break;
                  case APP_MSG_TYPE_APP_FIRMWARE_E:
                     break;
                  case APP_MSG_TYPE_ROUTE_ADD_E:
                     break;
                  case APP_MSG_TYPE_ROUTE_DROP_E:
                     break;
                  case APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E:
                     SM_SendConfirmationToApplication(MessageHandle, 0, APP_CONF_BATTERY_STATUS_SIGNAL_E, SUCCESS_E);
                     break;
                  case APP_MSG_TYPE_PING_E:
                     break;
                 default:
                  break;
               }
            }
            else
            {
               CO_PRINT_B_0(DBG_INFO_E, "Ack Manager failed to find message for DULCH ACK\r\n");
            }
         }
         else
         {
            CO_PRINT_B_1(DBG_INFO_E, "ACK not processed - slot type = %d\r\n", rxSlotType);
         }
      }
      else
      {
         CO_PRINT_B_2(DBG_INFO_E, "!!!!!!!!!! ACK Received With Wrong Data source=%d, Dest=%d\r\n", FrameAcknowledgement.MACSourceAddress, FrameAcknowledgement.MACDestinationAddress);
         
         //If this is a ping response addressed to another node from the node that we pinged, we can
         //infer that the pinged device is present and the link can be maintained.
         
         //Get the rach channel
         AcknowledgedQueueID_t queue_id = ACK_MAX_QUEUE_ID_E;
         rxSlotType = MC_TDM_SLOT_TYPE_SECONDARY_ACK_E;
         if ( MC_TDM_SLOT_TYPE_SECONDARY_ACK_E == rxSlotType)
         {
            queue_id = ACK_RACHS_E;
         }
         else if ( MC_TDM_SLOT_TYPE_DULCH_ACK_E == rxSlotType)
         {
            queue_id = ACK_DULCH_E;
         }
         else if ( MC_TDM_SLOT_TYPE_PRIMARY_ACK_E == rxSlotType)
         {
            queue_id = ACK_RACHP_E;
         }
         
         if ( ACK_MAX_QUEUE_ID_E != queue_id )
         {
            //check the type of message at the front of the queue
            MessageType = MC_ACK_GetTypeOfCurrentMessage( queue_id );
            //if it's a ping, check if the rx'd ack is from the pinged device
            if ( APP_MSG_TYPE_PING_E == MessageType )
            {
               uint16_t target_device = MC_ACK_GetMacDestinationOfCurrentMessage(queue_id);
               if ( target_device == FrameAcknowledgement.MACSourceAddress )
               {
                  CO_PRINT_B_1(DBG_INFO_E, "Message infers that node %d is active\r\n", target_device);
                  //The device that we pinged acknowledged another nodes message.  We can infer that the node is still there
                  //and invoke the ack received procedure.
                  if ( MC_ACK_AcknowledgeMessage( ACK_RACHS_E, target_device, true, &MessageType, &MessageHandle ) )
                  {
                     CO_PRINT_B_2(DBG_INFO_E, "ACK to node %d accepted as evidence that node %d is active\r\n", FrameAcknowledgement.MACDestinationAddress, target_device);
                  }
               }
            }
         }
      }
   }

}


/*************************************************************************************/
/**
* macEventUpdate
* state handler for General MAC notifications.
*
* @param - pointer to event data
*
* @return - void

*/
void macEventUpdate(const uint8_t * const pData)
{
   static uint16_t lastShortFrameIndex = 0;
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   
   if ( pMsg )
   {
   
      MACEventMessage_t eventMessage;
      memcpy(&eventMessage, pMsg->Payload.PhyDataInd.Data, sizeof(MACEventMessage_t));
      
      switch ( eventMessage.EventType )
      {
         case CO_RACH_CHANNEL_UPDATE_E:
         {
            /* update the Session Manager's network usage statistics */
            if ( MC_TDM_SLOT_TYPE_PRIMARY_RACH_E == eventMessage.SlotType )
            {
               /* if this is a transmission slot, mark it at busy*/
               if ( eventMessage.TransmitSlot )
               {
                  MC_SMGR_UpdateNetworkStatistics(PRIMARY_RACH, true);
               }
               /*Advance the primary rach count */
               MC_SMGR_UpdateRachCount(ACK_RACHP_E);
               /* Update the ACK Management */
               updateAckManagement( MC_TDM_SLOT_TYPE_PRIMARY_RACH_E );
            }
            else if ( MC_TDM_SLOT_TYPE_SECONDARY_RACH_E == eventMessage.SlotType )
            {
               /* if this is a transmission slot, mark it at busy*/
               if ( eventMessage.TransmitSlot )
               {
                  MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
               }
               /*Advance the secondary rach count */
               MC_SMGR_UpdateRachCount(ACK_RACHS_E);
               /* Update the ACK Management */
               updateAckManagement( MC_TDM_SLOT_TYPE_SECONDARY_RACH_E );
               
               //CO_PRINT_B_2(DBG_INFO_E,"SF=%d s=%d\r\n", eventMessage.ShortFrameIndex, eventMessage.SlotIndex);
               //Update the ping response module
               MC_PR_Advance( eventMessage.ShortFrameIndex, eventMessage.SlotIndex);
            }
            else if ( MC_TDM_SLOT_TYPE_DULCH_E == eventMessage.SlotType )
            {
               /* if this is a transmission slot, mark it at busy*/
               if ( eventMessage.TransmitSlot )
               {
                  MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
               }
               /* Update the SACH Management */
               MC_SACH_UpdateSACHManagement( eventMessage.SlotIndex, eventMessage.ShortFrameIndex, eventMessage.LongFrameIndex );

               /* Update the ACK Management */
               //We only want to update the Ack management in a DULCH slot if it is our transmission slot
               if ( MC_SACH_IsSendSlot(eventMessage.SlotIndex, eventMessage.ShortFrameIndex, eventMessage.LongFrameIndex) )
               {
                  updateAckManagement( MC_TDM_SLOT_TYPE_DULCH_E );
               }
            }
            
            
            /* if the short frame index has advanced, send a message to the Application so that it can do its child checks once per short frame */
            if ( eventMessage.ShortFrameIndex != lastShortFrameIndex )
            {
               //CO_PRINT_B_1(DBG_INFO_E,"NewSF=%d\r\n", eventMessage.ShortFrameIndex);
               lastShortFrameIndex = eventMessage.ShortFrameIndex;
               ApplicationMessage_t appMessage;
               AppMACEventData_t eventdata;
               eventdata.EventType = CO_NEW_SHORT_FRAME_E;
               eventdata.Value = eventMessage.ShortFrameIndex;
               memcpy(appMessage.MessageBuffer, &eventdata, sizeof(AppMACEventData_t));
               SM_SendEventToApplication( CO_MESSAGE_MAC_EVENT_E, &appMessage );
#ifdef SHOW_NETWORK_USAGE
               MC_SMGR_ShowNetworkUsage();
#endif
            }
         }
         break;
         case CO_MISSING_HEARTBEAT_E:
            {
               // If the superframe slot index is in range, process the heartbeat slot
               uint32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
               if (slots_per_super_frame > eventMessage.SuperframeSlotIndex)
               {
                  // Calculate the node ID (0 to 543) and the slot index in the long-frame (e.g. 16-19, 36-39, 56-59, ...)
                  uint16_t node_id = CO_CalculateNodeID(eventMessage.SuperframeSlotIndex);
                  if ( CO_BAD_NODE_ID != node_id )
                  {
                     uint32_t SlotIndexInLongframe = (eventMessage.ShortFrameIndex * SLOTS_PER_SHORT_FRAME) + eventMessage.SlotIndex;
      
                     // Call function to process the heartbeat slot.  The argument "false" signals that no heartbeat was received: all subsequent parameters are not used.
                     MC_MFH_Update( node_id, SlotIndexInLongframe, ULTRA_LOW_SNR, ULTRA_LOW_RSSI, NO_TIMESTAMP_RECORDED, UNASSIGNED_RANK, STATE_NOT_DEFINED_E, 0, false);

#ifdef CO_LOG_ENABLE_RECEIVE_FAILURE_LOGS
                     //Only log failures if mesh forming and healing has a record of the node
                     MC_MAC_TestMode_t test_mode = MC_GetTestMode();
                     ShortListElement_t node_data;
                     if ( SUCCESS_E == MC_MFH_FindNode(node_id, &node_data) )
                     {
                        CO_PRINT_B_4(DBG_INFO_E, "[%d,%c,%d,%d,,,]\r\n", eventMessage.SuperframeSlotIndex, eventMessage.RxTimeoutError, gNetworkAddress, eventMessage.Frequency);
                     }
#endif
                  }
               }
               else
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "ERROR: Out of range value of eventMessage.SuperframeSlotIndex, = %d\r\n", eventMessage.SuperframeSlotIndex);
               }
            }
            break;
         case CO_TEST_MODE_MESSAGE_E:
            TM_generateTestMessage(NULL, SM_SystemId);
            break;
         case CO_NEW_LONG_FRAME_E:
            //If we just transitioned to the active frame length but we are not yet active,
            //we need to open up all of the extra dch slots that became available with the 
            //increased frame length
            if ( false == gSM_UsingActiveFrameLength )
            {
               gSM_UsingActiveFrameLength = eventMessage.Data[0] == 0 ? false : true;
               //Only change the DCH behaviour on RBUs
               if ( !SM_IsNCU )
               {
                  if ( gSM_UsingActiveFrameLength && (STATE_ACTIVE_E != MC_STATE_GetDeviceState()) )
                  {
                     MC_SMGR_SetAllDCHSlotBehaviour(MC_TDM_DCH_RX_E);
                  }
               }
            }

            /* Increment gSM_LongframesSinceStateChange in a way that never wraps around */
            /* When we change state, gSM_LongframesSinceStateChange is used to hold-off the checks on whether
               this node is at a higher state than the mesh, until all nodes are reporting their new state */
            if ( NUM_OF_LONG_FRAMES_TO_SUSPEND_STATE_CHECKS > gSM_LongframesSinceStateChange )
            {
               gSM_LongframesSinceStateChange++;
            }
            // Do the session manager validation checks for RBUs.
            else if ( false == SM_IsNCU )
            {
               MC_SMGR_DoLongframeStateValidation();
            }
            /* For NCU only, Revise the minimum power that we can transmit with in low power mode */
            if( SM_IsNCU )
            {
               MC_SMGR_CalculateTxPower();
            }
            
            //Do a free running check.  Reset if the RBU fails to escalate to the
            //mesh state within a set number of long frames
            if ( MC_STATE_GetDeviceState() != MC_STATE_GetMeshState() )
            {
               if ( FREE_RUNNING_DETECTION_THRESHOLD <= gSM_LongframesSinceStateChange )
               {
                  MC_SMGR_DelayedReset("Failed to advance state");
                  gSM_LongframesSinceStateChange = 0;
               }
            }
            
            //Check that we are not free running because no heartbeat slots are open
            if ( !SM_IsNCU && (false == MC_TDM_DchConfigurationValid()) )
            {
               MC_SMGR_DelayedReset("DCH configuration invalid");
            }

            
            /* For RBU only, Check that we have a sync node */
            if( (!SM_IsNCU) && gFreqLockAchieved )
            {
               uint16_t sync_node_id = MC_SYNC_GetSyncNode();
               if ( CO_BAD_NODE_ID == sync_node_id )
               {
                  //The sync node has been lost.  Reset to avoid free running.
                  MC_SMGR_DelayedReset("Synch node lost");
               }
               else 
               {
                  //Make sure the sync node DCH slot is open
                  if ( MC_TDM_DCH_SLEEP_E == gDchBehaviour[sync_node_id] )
                  {
                     gDchBehaviour[sync_node_id] = MC_TDM_DCH_TRACKING_E;
                  }
               }
            }
            
            //Check whether the 'connected' LED patern needs to be removed.
            if ( LED_MESH_CONNECTED_E == DM_LedGetCurrentPattern() )
            {
               //The 'connected' LED pattern is active.  Remove it if all of the DULCH messages have gone.
               if ( 0 == MC_SACH_MessageCount() )
               {
                  DM_LedPatternRemove(LED_MESH_CONNECTED_E);
                  //Inform the application that the joining phase is complete
                  ApplicationMessage_t joinMessage;
                  AppMACEventData_t join_event_data;
                  join_event_data.EventType = CO_JOINING_PHASE_COMPLETE_E;
                  memcpy(joinMessage.MessageBuffer, &join_event_data, sizeof(AppMACEventData_t));
                  SM_SendEventToApplication( CO_MESSAGE_MAC_EVENT_E, &joinMessage );
               }
            }
            
            //Update the mesh forming and healing module
            MC_MFH_NewLongFrame();

            //Send a new long frame event to the application
            lastShortFrameIndex = eventMessage.ShortFrameIndex;
            ApplicationMessage_t appMessage;
            AppMACEventData_t eventdata;
            eventdata.EventType = CO_NEW_LONG_FRAME_E;
            eventdata.Value = eventMessage.LongFrameIndex;
            memcpy(appMessage.MessageBuffer, &eventdata, sizeof(AppMACEventData_t));
            SM_SendEventToApplication( CO_MESSAGE_MAC_EVENT_E, &appMessage );
            
            //Tell the mesh forming and healing that we have advanced a long frame
            MC_MFH_AdvanceLongFrame();
            MC_STATE_AdvanceState();

            break;
         case CO_REPORT_NODES_E:
            MC_SMGR_ReportAssociatedNodes(eventMessage.Value);
            break;
         case CO_POWER_STATUS_E:
            MC_SMGR_SetPowerStatus(eventMessage.Value);
            break;
         case CO_CORRUPTED_TX_MESSAGE_E:
         {
            //The MAC blocked a message from being broadcast because it is corrupted
            //Print the bad msg on the debug log
            char tempStr[3];
            char buffer[64];
            for( int32_t index = 0; index < 22; index++)
            {
               uint8_t value = eventMessage.Data[index];
               uint8_t low_value = (value & 0x0f);
               uint8_t hi_value = (value >> 4);
               
               if ( low_value > 9 )
               {
                  low_value += 7;
               }
               
               if ( hi_value > 9 )
               {
                  hi_value +=7;
               }
               
               tempStr[0] = hi_value + '0';
               tempStr[1] = low_value + '0';
               tempStr[2] = 0;
               strcat(buffer, tempStr);
            }

            CO_PRINT_A_1(DBG_ERROR_E,"BLOCKED CORRUPTED MESSAGE [%s]\r\n", buffer);
            
            CO_Message_t message;
            if( MC_ACK_PeekMessage( ACK_RACHS_E,  &message ) )
            {
               //extract the destination node
               uint16_t destination_node = message.Payload.PhyDataReq.Data[1] + (( message.Payload.PhyDataReq.Data[0] & 0x0f) << 8);
               //extract the message type
               FrameType_t frame_type;
               bool app_msg_present;
               ApplicationLayerMessageType_t app_msg_type;
               ErrorCode_t status = MC_PUP_GetMessageType(&message.Payload.PhyDataInd, 
                                  &frame_type, 
                                  &app_msg_present, 
                                  &app_msg_type);
               if ( SUCCESS_E == status )
               {
                  ApplicationLayerMessageType_t ack_msg_type;
                  uint32_t handle;
                  if ( MC_ACK_DiscardMessage( ACK_RACHS_E, &ack_msg_type, &handle ) )
                  {
                     CO_PRINT_A_2(DBG_ERROR_E,"Original message type=%d, dest=%d\r\n", app_msg_type, destination_node);
                     CO_PRINT_A_2(DBG_INFO_E,"Discarded message type = %d, handle=%d\r\n", ack_msg_type, handle);
                     CO_PRINT_A_1(DBG_INFO_E,"SM System ID = %d\r\n", SM_SystemId);
                  }
               }
               
            }
         }
            break;
         case CO_CORRUPTED_RX_MESSAGE_E:
         {
            //Include the failed message as an occupied slot in the network utilisation statistics
            uint8_t channel = SECONDARY_RACH;
            uint32_t slot_in_short_frame = eventMessage.SlotIndex % SLOTS_PER_SHORT_FRAME;
            if ( MC_TDM_SLOT_TYPE_PRIMARY_RACH_E == MC_TDM_GetSlotTypeDefault(slot_in_short_frame) )
            {
               channel = PRIMARY_RACH;
            }
            MC_SMGR_UpdateNetworkStatistics(channel, true);

            // If the superframe slot index is for a heartbeat, process the heartbeat slot
            uint32_t slots_per_super_frame = MC_SlotsPerSuperFrame();
            if (slots_per_super_frame > eventMessage.SuperframeSlotIndex)
            {
               // Calculate the node ID (0 to 511) and the slot index in the long-frame
               uint16_t node_id = CO_CalculateNodeID(eventMessage.SuperframeSlotIndex);
               if ( CO_BAD_NODE_ID != node_id )
               {
                  uint32_t SlotIndexInLongframe = (eventMessage.ShortFrameIndex * SLOTS_PER_SHORT_FRAME) + eventMessage.SlotIndex;
   
                  // Call function to process the heartbeat slot.  The argument "false" signals that no heartbeat was received: all subsequent parameters are not used.
                  // Call function to process the heartbeat.
                  MC_MFH_Update( node_id, SlotIndexInLongframe, ULTRA_LOW_SNR, ULTRA_LOW_RSSI, NO_TIMESTAMP_RECORDED, UNASSIGNED_RANK, STATE_NOT_DEFINED_E, 0, false);
               }
            }
#ifdef CO_LOG_ENABLE_RECEIVE_FAILURE_LOGS
            CO_PRINT_A_4(DBG_INFO_E, "[%d,%c,%d,%d,,,]\n\r", eventMessage.SuperframeSlotIndex, eventMessage.RxTimeoutError, gNetworkAddress, eventMessage.Frequency);
#endif
         }
         break;
         case CO_ENTER_PPU_OTA_MODE_E:
            {
               ApplicationMessage_t appMessage;
               AppMACEventData_t eventdata;
               eventdata.EventType = CO_ENTER_PPU_OTA_MODE_E;
               memcpy(appMessage.MessageBuffer, &eventdata, sizeof(AppMACEventData_t));
               SM_SendEventToApplication( CO_MESSAGE_MAC_EVENT_E, &appMessage );
            }
            break;
         default:
         break;
      }
   }
}

/*************************************************************************************/
/**
* updateAckManagement
* state handler for updating the slot counters in the ACK Management module.
*
* @param - pointer to event data
*
* @return - void

*/
void updateAckManagement(  const uint32_t RachChannel  )
{
   /*CO_PRINT_B_0("Received Slot Update\r\n");*/
   AcknowledgedQueueID_t rachChannel;
   ErrorCode_t status = SUCCESS_E;
  
   switch ( RachChannel )
   {
      case MC_TDM_SLOT_TYPE_PRIMARY_RACH_E:
         rachChannel = ACK_RACHP_E;
         status = SUCCESS_E;
         break;
      case MC_TDM_SLOT_TYPE_SECONDARY_RACH_E:
         rachChannel = ACK_RACHS_E;
      break;
      case MC_TDM_SLOT_TYPE_DULCH_E:
         rachChannel = ACK_DULCH_E;
         status = SUCCESS_E;
         break;
      default:
         status = ERR_SLOT_INDEX_OUT_OF_RANGE_E;
         break;
   }
   
   if ( SUCCESS_E == status )
   {
      /*CO_PRINT_B_0("MBO updated for RACH%d\r\n", rachChannel+1);*/
      
      /* if MC_ACK_UpdateSlot() returns true, there is a message that needs to be transmitted */
      if ( true == MC_ACK_UpdateSlot(rachChannel) )
      {
         /* The update resulted in a message becoming ready to re-transmit */
         
         if ( true == MC_ACK_MessageReadyToSend( rachChannel ) )
         {
            CO_Message_t *pPhyDataReq = ALLOCMESHPOOL;
            if (pPhyDataReq)
            {
               if ( true == MC_ACK_GetMessage(rachChannel, pPhyDataReq) )
               {
                  CO_PRINT_B_2(DBG_INFO_E, "Sending message on RACH%d- type=%d\r\n", (rachChannel+1), pPhyDataReq->Type);

                  if ( ACK_RACHP_E == rachChannel )
                  {
                     status = MC_MACQ_Push(MAC_RACHP_Q_E, pPhyDataReq);
                     if ( SUCCESS_E != status )
                     {
                        CO_PRINT_B_1(DBG_ERROR_E, "Failed to add msg to P-RACH Queue - Error %d\r\n", status);
                     }
                  }
                  else 
                  {
                     status = MC_MACQ_Push(MAC_RACHS_Q_E, pPhyDataReq);
                     if ( SUCCESS_E != status )
                     {
                        CO_PRINT_B_1(DBG_ERROR_E, "Failed to add msg to S-RACH Queue - Error %d\r\n", status);
                     }
                  }
               }
               //The MAC Queue makes a copy so we can delete the original
               FREEMESHPOOL(pPhyDataReq);
            }
            else
            {
               CO_PRINT_A_0(DBG_ERROR_E,"updateAckManagement - MeshPool failed to allocate\r\n");
            }
         }
      }
   }
   
   /* if this update resulted in an unacknowledged message being discarded, call the application callback function */
   uint32_t DiscardedMessageHandle;
   ApplicationLayerMessageType_t MessageType;
   uint16_t MessageMacDestination;
   if ( MC_ACK_MessageWasDiscarded(&DiscardedMessageHandle, &MessageType, &MessageMacDestination))
   {
      switch ( MessageType )
      {
         case APP_MSG_TYPE_FIRE_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedMessageHandle, 0, APP_CONF_FIRE_SIGNAL_E, ERR_MESSAGE_FAIL_E);
            break;
         case APP_MSG_TYPE_ALARM_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedMessageHandle, 0, APP_CONF_ALARM_SIGNAL_E, ERR_MESSAGE_FAIL_E);
            break;
         case APP_MSG_TYPE_FAULT_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedMessageHandle, 0, APP_CONF_FAULT_SIGNAL_E, ERR_MESSAGE_FAIL_E);
            break;
         case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedMessageHandle, 0, APP_CONF_OUTPUT_SIGNAL_E, ERR_MESSAGE_FAIL_E);
            break;
         case APP_MSG_TYPE_LOGON_E:
            SM_SendConfirmationToApplication(DiscardedMessageHandle, 0, APP_CONF_LOGON_SIGNAL_E, ERR_MESSAGE_FAIL_E);
            break;
         case APP_MSG_TYPE_ROUTE_ADD_E:
            /* inform the session management that a parent failed to acknowledge a RouteAdd request */
            MC_SMGR_NotifyMessageFailure(MessageType, MessageMacDestination);
            break;
         case APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedMessageHandle, 0, APP_CONF_BATTERY_STATUS_SIGNAL_E, ERR_MESSAGE_FAIL_E);
            break;
         default:
            break;
      }
   }

   // Update the flags that indicate if a Primary or Secondary ACK is currently expected. 
   // We use these flags to avoid waking up to receive ACK slots if no ACK is expected.
   MC_ACK_UpdateAckExpectedFlags();
}

/*************************************************************************************/
/**
* GenerateRouteAdd
* state handler to generate a Route Add message.
*
* @param - pointer to event data
*
* @return - void

*/
void GenerateRouteAdd(const uint8_t * const pData)
{
   RouteAddMessage_t routeAdd;
   RouteAddData_t routeAddData;
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status;
   
   if (pData)
   {
      CO_Message_t* pMessage = (CO_Message_t*)pData;

      memcpy(&routeAddData, pMessage->Payload.PhyDataReq.Data, sizeof(RouteAddData_t));
      
      /*create a join message (Route Add) and send it to the parent */
         /* pack into phy data req and put into RACHPQ */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         /* populate fire signal */
         routeAdd.Header.FrameType             = FRAME_TYPE_DATA_E;
         routeAdd.Header.MACDestinationAddress = routeAddData.NodeID;
         routeAdd.Header.MACSourceAddress      = gNetworkAddress;
         routeAdd.Header.HopCount              = 0;
         routeAdd.Header.DestinationAddress    = routeAddData.NodeID;
         routeAdd.Header.SourceAddress         = gNetworkAddress;
         routeAdd.Header.MessageType           = APP_MSG_TYPE_ROUTE_ADD_E;
         routeAdd.Zone                         = routeAddData.Zone;
         routeAdd.Rank                         = MC_SMGR_GetRank();
         routeAdd.IsPrimary                    = routeAddData.IsPrimary;
         routeAdd.SystemId                     = SM_SystemId;
         
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         status = MC_PUP_PackRouteAddMsg(&routeAdd, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
#ifdef SEND_ROUTE_ADD_ON_DULCH
            if ( MC_SACH_ScheduleMessage( routeAddData.NodeID, APP_MSG_TYPE_ROUTE_ADD_E, 0, pPhyDataReq, SEND_NORMAL_E) )
            {
               CO_PRINT_B_0(DBG_INFO_E, "Put ROUTE ADD Request in DULCH Queue\r\n");
            }
#else
            if ( MC_ACK_AddMessage(routeAddData.NodeID, ACK_RACHS_E, APP_MSG_TYPE_ROUTE_ADD_E, false, 0, pPhyDataReq, true, SEND_NORMAL_E) )
            {
               CO_PRINT_B_0(DBG_INFO_E, "Put ROUTE ADD Request in ACK Manager Queue\r\n");
            }
#endif
         }
         /* The Ack Manager and SACH Manager make a copy of the message so we can release the memory */
         FREEMESHPOOL(pPhyDataReq);
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"GenerateRouteAdd - MeshPool failed to allocate\r\n");
      }
   }
}

/*************************************************************************************/
/**
* routeAdd
* state handler for the Route Add event.
*
* @param - pointer to event data
*
* @return - void

*/
void routeAdd(const uint8_t * const pData)
{
   RouteAddMessage_t routeAdd = { 0 };
   ErrorCode_t status = SUCCESS_E;
   bool child_accepted = false;
   uint8_t ackChannel = ACK_RACHS_E;
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   
   // If the slot index indicates this is a primary rach slot, overwrite the initial value of ackChannel;
   if (pMsg->Payload.PhyDataInd.slotIdxInShortframe < FIRST_SRACH_ACK_SLOT_IDX)
   {
      ackChannel = ACK_RACHP_E;
   }

   status = MC_PUP_UnpackRouteAddMsg(&pMsg->Payload.PhyDataInd, &routeAdd);
   
   if (SUCCESS_E == status)
   {
      if ( (routeAdd.Header.MACDestinationAddress == gNetworkAddress) && (routeAdd.SystemId == SM_SystemId))
      {
         SM_SendACK( routeAdd.Header.MACSourceAddress, ackChannel, true);
         
         if ( (UNASSIGNED_RANK == routeAdd.Rank) || (routeAdd.Rank == (MC_SMGR_GetRank()+1)))
         {
            bool IsPrimary = false;
            uint16_t node_id = routeAdd.Header.MACSourceAddress;
            
            if ( routeAdd.IsPrimary > 0 )
            {
               IsPrimary = true;
            }
            /* Add the child to the session manager */
            if ( MC_SMGR_RouteAddRequest(node_id, IsPrimary) )
            {
               child_accepted = true;
               CO_PRINT_B_1(DBG_INFO_E, "Added new child node : network ID %d\n\r", routeAdd.Header.MACSourceAddress);
            }
            else
            {
               CO_PRINT_B_1(DBG_INFO_E, "Session Manager REJECTED new child node : network ID %d\n\r", routeAdd.Header.MACSourceAddress);
            }
         }
         
         /* Send a RouteAdd Response message*/
         CO_RouteAddResponseData_t responseData;
         responseData.DestinationNodeID = routeAdd.Header.SourceAddress;
         //Build two responses.  The first with the lower half of the zone enablement map and the second with the upper half.
         //But only send one if the child is not accepted
         uint32_t number_to_send = child_accepted ? 2 : 1;
         uint32_t number_sent = 0;
         for ( uint8_t lower_upper = 0; lower_upper < number_to_send; lower_upper++ )
         {
            if ( MC_SMGR_PopulateRouteAddResponse( child_accepted, routeAdd.Zone, lower_upper, &responseData) )
            {
               CO_Message_t* pResponseMsg;
               pResponseMsg = ALLOCMESHPOOL;
               if (pResponseMsg)
               {
                  pResponseMsg->Type = CO_MESSAGE_GENERATE_ROUTE_ADD_RESPONSE_E;
                  pResponseMsg->Payload.PhyDataReq.Downlink = true;
                  memcpy(pResponseMsg->Payload.PhyDataInd.Data, &responseData, sizeof(CO_RouteAddResponseData_t));

                  osStatus osStat = osMessagePut(MeshQ, (uint32_t)pResponseMsg, 0);
                  if (osOK == osStat)
                  {
                     number_sent++;
                  }
                  else
                  {
                     /* failed to write */
                     FREEMESHPOOL(pResponseMsg);
                     break;
                  }
               }
               else
               {
                  CO_PRINT_A_0(DBG_ERROR_E,"routeAdd - MeshPool failed to allocate\r\n");
               }
            }
         }
         
         CO_PRINT_B_4(DBG_INFO_E,"RouteAdd node %d, child accepted=%d, responses sent=%d of %d\r\n", routeAdd.Header.SourceAddress, child_accepted, number_sent, number_to_send);
         if ( child_accepted && (number_sent == number_to_send) )
         {
            if ( false == generateGlobalDelayMessage(responseData.DestinationNodeID) )
            {
               CO_PRINT_B_1(DBG_INFO_E,"Failed to send Global Delays to node %d\r\n", responseData.DestinationNodeID);
            }
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}


/*************************************************************************************/
/**
* GenerateRouteDrop
* state handler to generate a Route Drop message.
*
* @param - pointer to event data
*
* @return - void

*/
void GenerateRouteDrop(const uint8_t * const pData)
{
   RouteDropMessage_t routeDrop;
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status;
   RouteDropData_t routeDropData;
   
   if (pData)
   {
      CO_Message_t* pMessage = (CO_Message_t*)pData;

      memcpy(&routeDropData, pMessage->Payload.PhyDataReq.Data, sizeof(RouteDropData_t));
   
      /*create a Route Drop message and send it to the node */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         /* populate message */
         routeDrop.Header.FrameType             = FRAME_TYPE_DATA_E;
         routeDrop.Header.MACDestinationAddress = routeDropData.NodeID;
         routeDrop.Header.MACSourceAddress      = gNetworkAddress;
         routeDrop.Header.HopCount              = 0;
         routeDrop.Header.DestinationAddress    = routeDropData.NodeID;
         routeDrop.Header.SourceAddress         = gNetworkAddress;
         routeDrop.Header.MessageType           = APP_MSG_TYPE_ROUTE_DROP_E;
         routeDrop.Reason                       = routeDropData.Reason;
         routeDrop.SystemId                     = SM_SystemId;
         
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         pPhyDataReq->Payload.PhyDataReq.Downlink = routeDropData.Downlink;
         status = MC_PUP_PackRouteDropMsg(&routeDrop, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
            CO_PRINT_B_1(DBG_INFO_E, "Sending ROUTE DROP to node %d\r\n", routeDropData.NodeID);
            /* If we expect an ACK, give the message to the ACK Manager, otherwise put in directly into the RACH queue */
            if ( true == routeDropData.WaitForAck )
            {
               /* Add the message to the Ack Manager.  It will send it in the next available slot */
               MC_ACK_AddMessage(routeDropData.NodeID, ACK_RACHS_E, APP_MSG_TYPE_ROUTE_DROP_E, 0, false, 0, pPhyDataReq, true, SEND_ONCE_E);
            }
            else
            {
               /* We are not expecting an ACK.  Put the message directly into the RACHS queue */
               MC_MACQ_Push(MAC_RACHS_Q_E, pPhyDataReq);
               SM_SendConfirmationToApplication(routeDropData.Handle, 0, APP_CONF_ROUTE_DROP_SIGNAL_E, SUCCESS_E);
            }
         }
         //The MAC Queue makes a copy so we can delete the original
         FREEMESHPOOL(pPhyDataReq);
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"GenerateRouteDrop - MeshPool failed to allocate\r\n");
      }
   }
}

/*************************************************************************************/
/**
* routeDrop
* state handler for the Route Drop event.
*
* @param - pointer to event data
*
* @return - void

*/
void routeDrop(const uint8_t * const pData)
{
   RouteDropMessage_t routeDrop = { 0 };
   ErrorCode_t status = SUCCESS_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackRouteDropMsg(&pMsg->Payload.PhyDataInd, &routeDrop);
   CO_PRINT_B_5(DBG_INFO_E, "Received ROUTE DROP addressed to node %d from node %d, systemID %d, status=%d, reason=%d\r\n", routeDrop.Header.MACDestinationAddress, routeDrop.Header.MACSourceAddress, routeDrop.SystemId, status, routeDrop.Reason );
   
   if (SUCCESS_E == status)
   {
      if ( ((routeDrop.Header.MACDestinationAddress == SM_address) || (ADDRESS_GLOBAL == routeDrop.Header.MACDestinationAddress) ) 
            && (routeDrop.SystemId == SM_SystemId) )
      {
         //Don't ACK the global address
         if ( ADDRESS_GLOBAL != routeDrop.Header.MACDestinationAddress )
         {
            SM_SendACK( routeDrop.Header.MACSourceAddress, ACK_RACHS_E, true);
         }
         
         /* tell the session manager to forget the node*/
         if ( MC_SMGR_RouteDropRequest(routeDrop.Header.MACSourceAddress) )
         {
            CO_PRINT_B_2(DBG_INFO_E, "RouteDrop - discarded node %d, reason %d\n\r", routeDrop.Header.MACSourceAddress, routeDrop.Reason);
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}


/*************************************************************************************/
/**
* generateLogonRequest
* state handler to generate a logon request
*
* @param - pointer to event data
*
* @return - void

*/
static void generateLogonRequest(const uint8_t * const pData)
{
   LogOnMessage_t logonMessage = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   CO_LogonData_t logonData = { 0 };
   bool message_sent = false;
   uint32_t error_status = SUCCESS_E;
   /* check input data */
   if (pData)
   {
      CO_Message_t* pMessage = (CO_Message_t*)pData;

      // Cast pData input parameter to the message type
      memcpy(&logonData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_LogonData_t));

      /* ask Session Management for our parent node ID */
      if (gFreqLockAchieved)
      {
         /* populate fire signal */
         logonMessage.Header.FrameType = FRAME_TYPE_DATA_E;
         logonMessage.Header.MACDestinationAddress = MC_SYNC_GetSyncNode();
         logonMessage.Header.MACSourceAddress = gNetworkAddress;
         logonMessage.Header.HopCount = 0;
         logonMessage.Header.DestinationAddress = NCU_NETWORK_ADDRESS;
         logonMessage.Header.SourceAddress = gNetworkAddress;
         logonMessage.Header.MessageType = APP_MSG_TYPE_LOGON_E;
         logonMessage.SerialNumber = logonData.SerialNumber;
         logonMessage.DeviceCombination = logonData.DeviceCombination;
         logonMessage.ZoneNumber = logonData.ZoneNumber;
         logonMessage.SystemId = SM_SystemId;


         pPhyDataReq = ALLOCMESHPOOL;
         if (pPhyDataReq)
         {
            
            pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
            pPhyDataReq->Payload.PhyDataReq.Downlink = false;
            pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
            status = MC_PUP_PackLogOnMsg(&logonMessage, &pPhyDataReq->Payload.PhyDataReq);

            if (SUCCESS_E == status)
            {
               message_sent = MC_SACH_ScheduleMessage( logonMessage.Header.MACDestinationAddress, APP_MSG_TYPE_LOGON_E, 0, pPhyDataReq, SEND_NORMAL_E);
               if ( message_sent )
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Put Logon Request in DULCH Queue\r\n");
                  error_status = SUCCESS_E;
               }
            }
            
            if( false == message_sent )
            {
               error_status = ERR_MESSAGE_FAIL_E;
            }
            /* The Ack/SACH Manager makes a copy of the message so we can release the memory */
            FREEMESHPOOL(pPhyDataReq);
            
         }
         else
         {
            CO_PRINT_A_0(DBG_ERROR_E,"generateLogonRequest - MeshPool failed to allocate\r\n");
            error_status = ERR_NO_MEMORY_E;
         }
      }
      else
      {
         error_status = WARNING_NO_NEIGHBOUR_FOUND_E;
      }
   }
   else
   {
      error_status = ERR_INVALID_POINTER_E;
   }
   /* Let the application know if the msg failed */
   if ( SUCCESS_E != error_status )
   {
      SM_SendConfirmationToApplication(logonData.Handle, 0, APP_CONF_LOGON_SIGNAL_E, error_status);
   }
}

/*************************************************************************************/
/**
* logonRequest
* state handler to process a received a logon request
*
* @param - pointer to event data
*
* @return - void

*/
void logonRequest(const uint8_t * const pData)
{
   LogOnMessage_t logonMessage = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackLogOnMsg(&pMsg->Payload.PhyDataInd, &logonMessage);
   
   /* only process messages that are addressed to this node */
   if (SUCCESS_E == status)
   {
      if ((logonMessage.Header.MACDestinationAddress == gNetworkAddress) &&
         (logonMessage.SystemId == SM_SystemId) )
      {
         /* Send an ACK to the source */
         SM_SendACK(logonMessage.Header.MACSourceAddress, ACK_RACHS_E, false);
         
         uint16_t responseTime = HAL_LPTIM_ReadCounter(&hlptim1);
         CO_PRINT_B_2(DBG_INFO_E,"Ack Rx=%d, Rsp=%d\r\n",pMsg->Payload.PhyDataInd.timer, responseTime);
         
         //Make sure that the tracking node path doesn't circle back to this device
         uint16_t sync_node = MC_SYNC_GetSyncNode();
         uint16_t initial_tracking_slot = CO_CalculateLongFrameSlotForDCH(sync_node);
         uint16_t slot_for_node = CO_CalculateLongFrameSlotForDCH(logonMessage.Header.SourceAddress);
         if ( (slot_for_node == initial_tracking_slot) ||
              (logonMessage.Header.SourceAddress == gNetworkAddress) )
         {
            for (uint16_t count = 0; count < 3; count++)
            {
               MC_SMGR_SendRouteDrop(ADDRESS_GLOBAL,CO_RESET_REASON_CIRCULAR_PATH_E,false,true,0);
            }
            // Inform the application that we lost our connection to the mesh
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_ROUTE_DROP_E;
            CO_StatusIndicationData_t statusEvent;
            statusEvent.Event = CO_MESH_EVENT_INITIAL_TRK_NODE_LOST_E;
            statusEvent.EventNodeId = 0;
            statusEvent.EventData = 0;
            statusEvent.DelaySending = false;
            memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(CO_StatusIndicationData_t));
            SM_SendEventToApplication( CO_MESSAGE_MESH_STATUS_EVENT_E, &appMessage );
         }
//         if ( logonMessage.Header.SourceAddress == gNetworkAddress )
//         {
//            //Tracking node has moved on the mesh.  Restart and rejoin.
//            MC_SMGR_DelayedReset("Trapped in circular path.");
//         }
         
         if ( true )  // 'if' and indentation left in place to minimise changes when using file diff.  Remove this as a refactor, not when making functional changes.
         {
            CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Logon Request\r\n");
           
             /* do multi-hop if logonMessage.Header.DestinationAddress is not the address of this unit */
            if (SUCCESS_E == status)
            {
               if ( false == SM_IsNCU && logonMessage.Header.DestinationAddress != gNetworkAddress )
               {
                  /* The message is not for us.  Forward it to a parent node. */
                  uint32_t parent_node = MC_SMGR_GetPrimaryParentID();
                  
                  // Unless the RBU is in the preliminary stages of mesh forming, in which case used the Initial Tracking Node instead.
                  if ( CO_BAD_NODE_ID == parent_node )
                  {
                     parent_node = MC_SYNC_GetSyncNode();
                  }
                  
                  // If there is a valid node to forward the message to, then forward the logon request.
                  if ( CO_BAD_NODE_ID != parent_node )
                  {
                     uint16_t message_source = logonMessage.Header.MACSourceAddress;
                     (void)message_source; // suppress warning
                     
                     /* address the message to the parent and increment the hop count */
                     logonMessage.Header.MACDestinationAddress = parent_node;
                     logonMessage.Header.MACSourceAddress = gNetworkAddress;
                     logonMessage.Header.HopCount++;
                     
                     if ( logonMessage.Header.HopCount < CFG_GetMaxHops() )
                     {
                        /* pack into phy data req and put into RACHSQ */
                        pPhyDataReq = ALLOCMESHPOOL;
                        if (pPhyDataReq)
                        {
                           pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                           pPhyDataReq->Payload.PhyDataReq.Downlink = false;
                           pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
                           status = MC_PUP_PackLogOnMsg(&logonMessage, &pPhyDataReq->Payload.PhyDataReq);

                           if (SUCCESS_E == status)
                           {
                              bool multi_hop = true;
                              if ( logonMessage.Header.MACDestinationAddress == logonMessage.Header.DestinationAddress )
                              {
                                 multi_hop = false;
                              }
                              CO_PRINT_B_2(DBG_INFO_E, "MULTI-HOP - Rx'd Logon message from node %d.  Forwarding to node %d\r\n", message_source, parent_node);
                              /* Add the message to the ACK Manager and it will be sent in the next RACH slot */
                              MC_ACK_AddMessage(logonMessage.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_LOGON_E, 0, multi_hop, 0, pPhyDataReq, true, SEND_NORMAL_E);
                           }
                           /* The ACK manager makes a copy so we can release the message memory */
                           FREEMESHPOOL(pPhyDataReq);
                        }
                        else 
                        {
                           CO_PRINT_A_0(DBG_ERROR_E,"logonRequest - MeshPool failed to allocate\r\n");
                        }
                     }
                     else
                     {
                        CO_PRINT_B_0(DBG_INFO_E, "MULTI-HOP - DISCARDED Logon message - hop limit reached\r\n");
                     }
                  }
               }
               else if ( logonMessage.Header.DestinationAddress == gNetworkAddress )
               {
                  /* The message was addressed to this node.  Send it to the application. */
                  ApplicationMessage_t appMessage;
                  appMessage.MessageType = APP_MSG_TYPE_LOGON_E;
                  LogOnSignalIndication_t logonEvent;
                  logonEvent.SourceAddress = logonMessage.Header.SourceAddress;
                  logonEvent.SerialNumber = logonMessage.SerialNumber;
                  logonEvent.ZoneNumber = logonMessage.ZoneNumber;
                  logonEvent.DeviceCombination = logonMessage.DeviceCombination;
                  memcpy(appMessage.MessageBuffer, &logonEvent, sizeof(LogOnSignalIndication_t));
                  SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
               }
            }
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}


/*************************************************************************************/
/**
* generateCommand
* state handler to generate a command message
*
* @param - pointer to event data
*
* @return - void

*/
void generateCommand(const uint8_t * const pData)
{
   CommandMessage_t command = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   CO_CommandData_t cmdData;
   bool at_least_one_message_sent = false;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
   // Cast pData input parameter to the message type
   memcpy(&cmdData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_CommandData_t));
   
   if ( cmdData.Destination != gNetworkAddress)
   {
      /* The command is addressed to a remote node.  Queue it for transmission */
      /* populate command */
      command.Header.FrameType             = FRAME_TYPE_DATA_E;
      command.Header.MACDestinationAddress = ADDRESS_GLOBAL;
      command.Header.MACSourceAddress      = gNetworkAddress;
      command.Header.HopCount              = 0;
      command.Header.DestinationAddress    = cmdData.Destination;
      command.Header.SourceAddress         = gNetworkAddress;
      command.Header.MessageType           = APP_MSG_TYPE_COMMAND_E;
      command.TransactionID                = cmdData.TransactionID;
      command.ParameterType                = cmdData.CommandType;
      command.P1                           = cmdData.Parameter1;
      command.P2                           = cmdData.Parameter2;
      command.Value                        = cmdData.Value;
      command.ReadWrite                    = cmdData.ReadWrite;
      command.SystemId                     = SM_SystemId;

      /* pack into phy data req and put into DLCCHx Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         status = MC_PUP_PackCommandMessage(&command, &pPhyDataReq->Payload.PhyDataReq);
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         for (uint32_t send_count = 0; send_count < cmdData.NumberToSend; send_count++ )
         {
            if (SUCCESS_E == status)
            {
               status = MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq);
               if ( SUCCESS_E == status )
               {
                  CO_PRINT_B_1(DBG_INFO_E, "Put Cmd on DLCCHSQ Queue - Type %d\r\n", command.ParameterType);
                  at_least_one_message_sent = true;
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "Failed to add msg to DLCCHSQ Queue - Error %d\r\n", status);
               }
            }
         }
         //The MAC Queue makes a copy so we can free the message
         FREEMESHPOOL(pPhyDataReq);
         
         if ( at_least_one_message_sent )
         {
            status = SUCCESS_E;
         }
         else 
         {
            status = ERR_MESSAGE_FAIL_E;
         }
      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateCommand - MeshPool failed to allocate\r\n");
         status = ERR_NO_MEMORY_E;
      }
   }
   else
   {
      /*The command was for this node.  This means that the command is from the application
         and it requires some property that is held in the Mesh*/
      status = SM_ProcessApplicationRequest(&cmdData);
   }
   
   //Check to see if the command is an RBU reset command.  If so, purge the session manager of affected child nodes.
   if ( PARAM_TYPE_REBOOT_E == cmdData.CommandType )
   {
      MC_SMGR_PurgeNodes(cmdData.Destination);
   }
   
   /* Let the application know the result */
   SM_SendConfirmationToApplication(cmdData.TransactionID, 0, APP_CONF_COMMAND_SIGNAL_E, (uint32_t)status);
}

/*************************************************************************************/
/**
* rxCommand
* state handler to process a received a command.
*
* @param - pointer to event data
*
* @return - void

*/
void rxCommand(const uint8_t * const pData)
{
   CommandMessage_t commandMsg = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackCommandMessage(&pMsg->Payload.PhyDataInd, &commandMsg);
   CO_PRINT_B_5(DBG_INFO_E,"upstatus=%d, sysid=%d, tid=%d, ptid=%d,parent=%d\r\n", status, commandMsg.SystemId, commandMsg.TransactionID, gLastTransactionID, MC_SMGR_IsParentNode(commandMsg.Header.MACSourceAddress));
   if ( (SUCCESS_E == status) && 
        (commandMsg.SystemId == SM_SystemId) &&
        (commandMsg.TransactionID != gLastTransactionID) &&
        (MC_SMGR_IsParentNode(commandMsg.Header.MACSourceAddress)) )
   {
#ifdef SHOW_DOWNLINK_LED
      DM_LedPatternRequest(LED_DOWNLINK_MESSAGE_E);
#endif
      gLastTransactionID = commandMsg.TransactionID;
      /* convert the 8-bit zone in the command message to the 12-bit of the RBU*/
      uint32_t zone = commandMsg.P2;
      bool addressedToThisNode = SMCheckDestinationAddress(commandMsg.Header.DestinationAddress, zone);
      CO_PRINT_B_2(DBG_INFO_E,"addressedToThisNode=%d, DestAddr=%X\r\n",addressedToThisNode, commandMsg.Header.DestinationAddress );
      if ( ((false == addressedToThisNode) || (ADDRESS_GLOBAL == commandMsg.Header.DestinationAddress)) )
//            &&( 0 < MC_SMGR_GetNumberOfChildren()) )
      {
         /* create PhyDataRequest for transmission on DLCCH to children */
         pPhyDataReq = ALLOCMESHPOOL;
         if (pPhyDataReq)
         {
            /* populate downlink msg using payload from received PhyDataInd*/
            commandMsg.Header.MACSourceAddress = gNetworkAddress;
            commandMsg.Header.HopCount++;
            if ( commandMsg.Header.HopCount < CFG_GetMaxHops() )
            {
               status = MC_PUP_PackCommandMessage(&commandMsg, &pPhyDataReq->Payload.PhyDataReq);
               
               if (SUCCESS_E == status)
               {
                  pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                  pPhyDataReq->Payload.PhyDataReq.Size = pMsg->Payload.PhyDataInd.Size;
                  pPhyDataReq->Payload.PhyDataReq.slotIdxInSuperframe = 0;   /* will be populated in MAC */
                  pPhyDataReq->Payload.PhyDataReq.freqChanIdx = 0;           /* will be populated in MAC */
                  pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();

                  status = MC_MACQ_Push(MAC_DLCCHS_Q_E,pPhyDataReq);
                  if ( SUCCESS_E != status )
                  {
                     CO_PRINT_B_1(DBG_ERROR_E, "CMD DLCCHSQ write fail - status=%d\r\n", status);
                  }
                  else 
                  {
                     CO_PRINT_B_1(DBG_INFO_E, "Put Cmd on DLCCHSQ Queue - Type %d\r\n", commandMsg.ParameterType);
                  }
               }
               else 
               {
                   CO_PRINT_B_1(DBG_ERROR_E, "MC_PUP_PackCommandMessage failed - status=%d\r\n", status);
               }
            }
            else 
            {
               CO_PRINT_B_2(DBG_INFO_E,"Not forwarding msg.  Hop limit reached = %d(%d)\r\n", commandMsg.Header.HopCount, CFG_GetMaxHops());
            }
            //The MAC Queue makes a copy so free the message
            FREEMESHPOOL(pPhyDataReq);
         }
         else 
         {
            CO_PRINT_A_0(DBG_ERROR_E,"rxCommand - MeshPool failed to allocate\r\n");
         }
      }
      
      if ( addressedToThisNode )
      {
         CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< Rx Command ID %d\r\n", commandMsg.TransactionID);
         
         /* Send the command up to the Application */
         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_COMMAND_E;
         CO_CommandData_t command;
         command.CommandType = commandMsg.ParameterType;
         command.TransactionID = commandMsg.TransactionID;
         command.Parameter1 = commandMsg.P1;
         command.Parameter2 = commandMsg.P2;
         command.ReadWrite = commandMsg.ReadWrite;
         command.Source = commandMsg.Header.SourceAddress;
         command.Destination = commandMsg.Header.DestinationAddress;
         command.Value = commandMsg.Value;
         memcpy(appMessage.MessageBuffer, &command, sizeof(CO_CommandData_t));
         SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         gLastTransactionID = commandMsg.TransactionID;
      }
      
      CO_PRINT_B_2(DBG_INFO_E, "Rx Command Type=%d, dest=%d\r\n", commandMsg.ParameterType, commandMsg.Header.DestinationAddress);
      //Check to see if the command is an RBU reset command.  If so, purge the session manager of affected child nodes.
      if ( PARAM_TYPE_REBOOT_E == commandMsg.ParameterType )
      {
         MC_SMGR_PurgeNodes(commandMsg.Header.DestinationAddress);
      }
   }
}

/*************************************************************************************/
/**
* generateResponse
* state handler to generate a command message
*
* @param - pointer to event data
*
* @return - void

*/
void generateResponse(const uint8_t * const pData)
{
   ResponseMessage_t responseMsg = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   CO_ResponseData_t responseData;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
   // Cast pData input parameter to the message type
   memcpy(&responseData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_ResponseData_t));
   
   uint16_t parent_address = MC_SMGR_GetPrimaryParentID();
   
   if ( CO_BAD_NODE_ID != parent_address )
   {
      /* populate response message */
      responseMsg.Header.FrameType             = FRAME_TYPE_DATA_E;
      responseMsg.Header.MACDestinationAddress = parent_address;
      responseMsg.Header.MACSourceAddress      = gNetworkAddress;
      responseMsg.Header.HopCount              = 0;
      responseMsg.Header.DestinationAddress    = responseData.Source;
      responseMsg.Header.SourceAddress         = gNetworkAddress;
      responseMsg.Header.MessageType           = APP_MSG_TYPE_RESPONSE_E;
      responseMsg.TransactionID                = responseData.TransactionID;
      responseMsg.ParameterType                = responseData.CommandType;
      responseMsg.P1                           = responseData.Parameter1;
      responseMsg.P2                           = responseData.Parameter2;
      responseMsg.Value                        = responseData.Value;
      responseMsg.ReadWrite                    = responseData.ReadWrite;
      responseMsg.SystemId                     = SM_SystemId;


      /* pack into phy data req and put into RACHSQ */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         status = MC_PUP_PackResponseMessage(&responseMsg, &pPhyDataReq->Payload.PhyDataReq);

 
         if (SUCCESS_E == status)
         {
            bool multi_hop = true;
            if ( responseMsg.Header.MACDestinationAddress == responseMsg.Header.DestinationAddress )
            {
               multi_hop = false;
            }
            //if the command was addressed to the GLOBAL address, send on DULCH.  Otherwise send on S-RACH
            if ( ADDRESS_GLOBAL == responseData.Destination )
            {
               /* Hand the message to SACH management.  It will schedule the message for the next DULCH slot */
               if (MC_SACH_ScheduleMessage(responseMsg.Header.MACDestinationAddress, APP_MSG_TYPE_RESPONSE_E, responseMsg.ParameterType, pPhyDataReq, SEND_NORMAL_E))
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Put Response Message in DULCH Queue\r\n");
               }
               else
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "generate response failed to queue on DULCH - status=%d\r\n", status);
               }
            }
            else 
            {
               /* Hand the message to Ack management.  It will schedule the message for the next RACHS slot */
               if (MC_ACK_AddMessage(responseMsg.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_RESPONSE_E, responseMsg.ParameterType, multi_hop, responseMsg.TransactionID, pPhyDataReq, true, SEND_NORMAL_E))
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Put Response Message in AckManagement Queue\r\n");
                  SM_TryToSendImmediately( ACK_RACHS_E, MAC_RACHS_Q_E );
               }
               else
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "generate response failed to queue on AckManagement - status=%d\r\n", status);
               }
            }
         }
         
         /* The Ack Manager makes a copy of the message so we can release the memory */
         FREEMESHPOOL(pPhyDataReq);
      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateResponse - MeshPool failed to allocate\r\n");
      }

      
      if ((NULL == pPhyDataReq) || (SUCCESS_E != status) )
      {
         /* Let the application know the msg failed */
         SM_SendConfirmationToApplication(responseData.TransactionID, 0, APP_CONF_RESPONSE_SIGNAL_E, ERR_MESSAGE_FAIL_E);
      }
      else
      {
         /* Let the application know the msg was queued */
         SM_SendConfirmationToApplication(responseData.TransactionID, 0, APP_CONF_RESPONSE_SIGNAL_E, SUCCESS_E);
      }
   }
}

/*************************************************************************************/
/**
* ncuResponse
* state handler for NCU receiving a Response message
*
* @param - pointer to event data
*
* @return - void

*/
void ncuResponse(const uint8_t * const pData)
{
   ResponseMessage_t responseMsg = { 0 };
   ErrorCode_t status = SUCCESS_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackResponseMessage(&pMsg->Payload.PhyDataInd, &responseMsg);
   
   /* only process messages that are addressed to this node */
   if (SUCCESS_E == status)
   {
      if ((responseMsg.Header.MACDestinationAddress == gNetworkAddress) &&
         (responseMsg.SystemId == SM_SystemId) && MC_SMGR_IsAChild(responseMsg.Header.MACSourceAddress))
      {
         SM_SendACK( responseMsg.Header.MACSourceAddress, ACK_RACHS_E, true);
         
         CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< Rx Response NCU - ID %d\r\n", responseMsg.TransactionID);

         if ( responseMsg.Header.DestinationAddress == gNetworkAddress )
         {
            /* The message was addressed to this node.  Send it to the application. */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_RESPONSE_E;
            CO_ResponseData_t response;
            response.CommandType = responseMsg.ParameterType;
            response.TransactionID = responseMsg.TransactionID;
            response.Parameter1 = responseMsg.P1;
            response.Parameter2 = responseMsg.P2;
            response.ReadWrite = responseMsg.ReadWrite;
            response.Source = responseMsg.Header.SourceAddress;
            response.Value = responseMsg.Value;
            memcpy(appMessage.MessageBuffer, &response, sizeof(CO_ResponseData_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* rbuResponse
* state handler for RBU receiving a Response message
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuResponse(const uint8_t * const pData)
{
   ResponseMessage_t responseMsg = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackResponseMessage(&pMsg->Payload.PhyDataInd, &responseMsg);
   
   /* only process messages that are addressed to this node */
   if (SUCCESS_E == status)
   {
      if ((responseMsg.Header.MACDestinationAddress == gNetworkAddress) &&
         (responseMsg.SystemId == SM_SystemId) && MC_SMGR_IsAChild(responseMsg.Header.MACSourceAddress))
      {
         SM_SendACK( responseMsg.Header.MACSourceAddress, ACK_RACHS_E, false);

         CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< Rx Response msg RBU - ID %d\r\n", responseMsg.TransactionID);

          /* do multi-hop if alarmSignal.Header.DestinationAddress is not the address of this unit */
         if ( responseMsg.Header.DestinationAddress != gNetworkAddress )
         {
            /* The message is not for us.  Forward it to a parent node. */
            uint32_t parent_node = MC_SMGR_GetPrimaryParentID();
            if ( CO_BAD_NODE_ID != parent_node )
            {
               uint16_t message_source = responseMsg.Header.MACSourceAddress;
               (void)message_source;   // suppress warnings

               /* address the message to the parent and increment the hop count */
               responseMsg.Header.MACDestinationAddress = parent_node;
               responseMsg.Header.MACSourceAddress = gNetworkAddress;
               responseMsg.Header.HopCount++;
               
               if ( responseMsg.Header.HopCount < CFG_GetMaxHops() )
               {
                  /* pack into phy data req and put into RACHPQ */
                  pPhyDataReq = ALLOCMESHPOOL;
                  if (pPhyDataReq)
                  {
                     pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                     pPhyDataReq->Payload.PhyDataReq.Downlink = false;
                     pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
                     status = MC_PUP_PackResponseMessage(&responseMsg, &pPhyDataReq->Payload.PhyDataReq);

                     if (SUCCESS_E == status)
                     {
                        bool multi_hop = true;
                        if ( responseMsg.Header.MACDestinationAddress == responseMsg.Header.DestinationAddress )
                        {
                           multi_hop = false;
                        }

                        CO_PRINT_B_2(DBG_INFO_E, "MULTI-HOP - Rx'd Response msg from node %d.  Forwarding to node %d\r\n", message_source, parent_node);
                        /* Add the message to the ACK Manager and it will be sent in the next RACH slot */
                        if ( MC_ACK_AddMessage(responseMsg.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_RESPONSE_E, responseMsg.ParameterType, multi_hop, 0, pPhyDataReq, true, SEND_NORMAL_E) )
                        {
                           SM_TryToSendImmediately( ACK_RACHS_E, MAC_RACHS_Q_E );
                        }
                     }
                     /* The ACK manager makes a copy so we can release the message memory */
                     FREEMESHPOOL(pPhyDataReq);
                  }
                  else 
                  {
                     CO_PRINT_A_0(DBG_ERROR_E,"rbuResponse - MeshPool failed to allocate\r\n");
                  }
               }
            }
         }
         else
         {
            /* The message was addressed to this node.  Send it to the application. */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_RESPONSE_E;
            CO_ResponseData_t response;
            response.CommandType = responseMsg.ParameterType;
            response.TransactionID = responseMsg.TransactionID;
            response.Parameter1 = responseMsg.P1;
            response.Parameter2 = responseMsg.P2;
            response.ReadWrite = responseMsg.ReadWrite;
            response.Source = responseMsg.Header.SourceAddress;
            response.Value = responseMsg.Value;
            memcpy(appMessage.MessageBuffer, &response, sizeof(CO_ResponseData_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* generateTestMessage
* Test Mode state handler for RBU generating a test message
*
* @param - pointer to msg data or NULL for auto incrementing counter msg
*
* @return - void

*/
void generateTestMessage(const uint8_t * const pData)
{
   if ( pData )
   {
      TM_generateTestMessage( pData, SM_SystemId );
   }
}


/*************************************************************************************/
/**
* SM_ProcessApplicationRequest
* Function to handle commands from the Application layer which
* require access to MAC properties
*
* @param pCmdData - pointer to a Comand structure
*
* @return - void

*/
static ErrorCode_t SM_ProcessApplicationRequest(CO_CommandData_t* pCmdData)
{
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   uint32_t value = 0;
   bool bSendResponse = false;
   
   if ( pCmdData )
   {
      switch ( pCmdData->CommandType )
      {
         case PARAM_TYPE_NEIGHBOUR_INFO_E:
            {
               if( MC_SMGR_GetNeighbourInformation(pCmdData->Parameter1, pCmdData->Value, &value) )
               {
                  bSendResponse = true;
                  result = SUCCESS_E;
               }
               else
               {
                  bSendResponse = true;
                  result = ERR_OUT_OF_RANGE_E;
               }
            }
            break;
         case PARAM_TYPE_RBU_DISABLE_E:
               /* put the unit to sleep */
               MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
               MC_SetTestMode( MC_MAC_TEST_MODE_SLEEP_E );
            break;
         case PARAM_TYPE_SCAN_FOR_DEVICES_E:
               MC_MFH_ScanForDevices();
               result = SUCCESS_E;
            break;
         default:
            result = ERR_OUT_OF_RANGE_E;
            break;
      }
      
      if ( bSendResponse )
      {
         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_RESPONSE_E;
         CO_ResponseData_t response;
         response.CommandType = pCmdData->CommandType;
         response.TransactionID = pCmdData->TransactionID;
         response.Parameter1 = pCmdData->Parameter1;
         response.Parameter2 = pCmdData->Parameter2;
         response.ReadWrite = pCmdData->ReadWrite;
         response.Source = pCmdData->Source;
         response.Value = value;
         memcpy(appMessage.MessageBuffer, &response, sizeof(CO_ResponseData_t));
         SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
      }
   }
   return result;
}


/*************************************************************************************/
/**
* generateSetState
* State handler to generate SetState signal.  
* Note that it only has any effect in the NCU: the RBUs forward the message rather than generate it.
* This function also schedules an NCU state change at the next longframe.
*
* @param - pointer to event data
*
* @return - void

*/
static void generateSetState(const uint8_t * const pData)
{
   SetStateMessage_t stateMessage = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* NCU only */
   if ( SM_IsNCU )
   {
      CO_Message_t* pMessage = (CO_Message_t*)pData;
      CO_State_t current_state = MC_STATE_GetDeviceState();
      CO_State_t commanded_state = (CO_State_t)pMessage->Payload.PhyDataReq.Data[0];
         
      if( STATE_IDLE_E != current_state)
      {
         if ( commanded_state != current_state &&
              commanded_state != MC_STATE_GetScheduledState())
         {
            /* populate message */
            stateMessage.Header.FrameType             = FRAME_TYPE_DATA_E;
            stateMessage.Header.MACDestinationAddress = ADDRESS_GLOBAL;
            stateMessage.Header.MACSourceAddress      = ADDRESS_NCU;
            stateMessage.Header.HopCount              = 0;
            stateMessage.Header.DestinationAddress    = ADDRESS_GLOBAL;
            stateMessage.Header.SourceAddress         = ADDRESS_NCU;
            stateMessage.Header.MessageType           = APP_MSG_TYPE_STATE_SIGNAL_E;
               stateMessage.State                        = commanded_state;
            stateMessage.SystemId                     = SM_SystemId;

            int32_t number_to_send = CFG_GetNumberOfResends();
            for ( int32_t msg_count = 0; msg_count < number_to_send; msg_count++ )
            {
               /* pack into phy data req and put into DLCCHx Q */
               pPhyDataReq = ALLOCMESHPOOL;
               if (pPhyDataReq)
               {
                  pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                  status = MC_PUP_PackSetState(&stateMessage, &pPhyDataReq->Payload.PhyDataReq);
                  pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();

                  if (SUCCESS_E == status)
                  {
                     if ( SUCCESS_E == MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq) )
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Put State msg on DLCCHSQ Queue - state %d\r\n", commanded_state);
                        osStat = osOK;
                     }
                     else 
                     {
                        CO_PRINT_B_0(DBG_ERROR_E, "Failed to put State msg on DLCCHSQ Queue\r\n");
                        osStat = osErrorOS;
                     }
                  }
                  //The MAC Queue makes a copy so we can free the message
                  FREEMESHPOOL(pPhyDataReq);
               }
               else 
               {
                  CO_PRINT_A_0(DBG_ERROR_E,"generateSetState - MeshPool failed to allocate\r\n");
                  status = ERR_NO_MEMORY_E;
               }

               if ((SUCCESS_E != status) || (osOK != osStat))
               {
                  break;
               }
            }
            
            if ((SUCCESS_E != status) || (osOK != osStat))
            {
               /* Let the application know the msg failed */
               SM_SendConfirmationToApplication(0, 0, APP_CONF_SET_STATE_E, ERR_MESSAGE_FAIL_E);
            }
            else
            {
               /* Let the application know the msg was queued */
               SM_SendConfirmationToApplication(0, 0, APP_CONF_SET_STATE_E, SUCCESS_E);
            }
         }
      }
      else 
      {
         /* Let the application know the msg succeded */
         SM_SendConfirmationToApplication(0, 0, APP_CONF_SET_STATE_E, SUCCESS_E);
         if ( STATE_CONFIG_SYNC_E == commanded_state )
         {
//            SM_SetCurrentDeviceState(STATE_CONFIG_SYNC_E);
//            SM_SetScheduledDeviceState(STATE_NOT_DEFINED_E);
//            /* we are switching from IDLE to CONFIG_SYNC.  Start the TDM in the MAC */
//            MC_TDM_StartTDMMasterMode();
            MC_STATE_SetMeshState(STATE_CONFIG_SYNC_E);
            MC_STATE_AdvanceState();
         }
      }

      /* NCU only: change state in accordance with the received instructions */
      if ((commanded_state != MC_STATE_GetDeviceState()) && commanded_state != MC_STATE_GetScheduledState())
      {
         char * str = NULL;
         
         MC_STATE_ScheduleStateChange(stateMessage.State, true);
         MC_STATE_AdvanceState();

         switch (commanded_state)
         {
            case STATE_ACTIVE_E:
            {
               str = "ACTIVE";
               break;
            }
            case STATE_IDLE_E:
            {
               str = "IDLE";
               break;
            }
            case STATE_CONFIG_SYNC_E:
            {
               str = "CONFIG_SYNC";
               break;
            }
            case STATE_CONFIG_FORM_E:
            {
               str = "STATE_CONFIG";
               break;
            }
            default:
               break;
         }
         
         CO_PRINT_A_1(DBG_INFO_E, "NCU AT Command received: state change to %s MODE\r\n", str);
      }      
   }
}

/*************************************************************************************/
/**
* rbuSetState
* state handler for RBU receiving a State command
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuSetState(const uint8_t * const pData)
{
   SetStateMessage_t stateMessage = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackSetState(&pMsg->Payload.PhyDataInd, &stateMessage);
   
   if ( SUCCESS_E == status && stateMessage.SystemId == SM_SystemId)
   {
#ifdef SHOW_DOWNLINK_LED
      DM_LedPatternRequest(LED_DOWNLINK_MESSAGE_E);
#endif
      /* check that the message comes from an uplink source */
      uint16_t source_node = stateMessage.Header.MACSourceAddress;      
      if ( MC_SMGR_IsParentNode(source_node) ||
         ( (STATE_ACTIVE_E != MC_STATE_GetDeviceState()) && (source_node == MC_SYNC_GetSyncNode())) )
      {
         /* create PhyDataRequest for transmission on DLCCH to children */
//         if ( 0 != MC_SMGR_GetNumberOfChildren() )
//         {
            pPhyDataReq = ALLOCMESHPOOL;
            if (pPhyDataReq)
            {
               /* populate PhyDataRequest using received data. Update MACSourceAddress and hop count*/
               pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
               stateMessage.Header.MACSourceAddress = gNetworkAddress;
               stateMessage.Header.HopCount++;
               
               if ( stateMessage.Header.HopCount < CFG_GetMaxHops() )
               {
                  status = MC_PUP_PackSetState(&stateMessage, &pPhyDataReq->Payload.PhyDataReq);
                  pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();

                  if (SUCCESS_E == status)
                  {
                     status = MC_MACQ_Push(MAC_DLCCHS_Q_E,pPhyDataReq);
                     if ( SUCCESS_E != status )
                     {
                        CO_PRINT_B_1(DBG_ERROR_E, "DLCCHSQ write fail - status=%d\r\n", status);
                     }
                     else 
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Put State msg on DLCCHSQ Queue - state %d\r\n", stateMessage.State);
                     }
                  }
               }
               //The MAC Queue makes a copy so free the message
               FREEMESHPOOL(pPhyDataReq);
            }
            else 
            {
               CO_PRINT_A_0(DBG_ERROR_E,"rbuSetState - MeshPool failed to allocate\r\n");
            }
//         }
      }
      
      /* check if the message is for this node */
      if (( true == SMCheckDestinationAddress(stateMessage.Header.DestinationAddress, ZONE_GLOBAL) ))
      {
         CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< Rx State Signal to node %d\r\n", stateMessage.Header.DestinationAddress);
         
         /* ignore the state change if we are already in the requested state, or scheduled to change to it */
         if ( (stateMessage.State != MC_STATE_GetDeviceState()) && (stateMessage.State!= MC_STATE_GetScheduledState()) )
         {
            if ( MC_STATE_ReadyForStateChange(stateMessage.State) )
            {
               // Call function to tell the MAC that a state change must be made at the beginning of the next long frame.
               // (The MAC will notify the SM when this has happened by calling the function SM_ActivateStateChange.)
               MC_STATE_ScheduleStateChange(stateMessage.State, true);
               /* Send the output request up to the Application */
               ApplicationMessage_t appMessage;
               appMessage.MessageType = APP_MSG_TYPE_STATE_SIGNAL_E;
               appMessage.MessageBuffer[0] = stateMessage.State;
               SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
            }
            else 
            {
               CO_PRINT_B_0(DBG_INFO_E, "Ignored state change command.  Device not ready.\r\n");
               //If the state change is to "ACTIVE" the NCU will start frequency hopping on the next long frame.
               //The channel hopping on the local device must be scheduled to occur at the same time.
               if ( STATE_ACTIVE_E == stateMessage.State )
               {
                  MC_TDM_StartFrequencyHoppingAtNextLongFrame();
                  CO_PRINT_B_0(DBG_INFO_E, "Scheduled channel hopping\r\n");
               }
            }
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E, "Ignored state change command.  Already in/scheduled requested state.\r\n");
         }
      }
   }
}

/*************************************************************************************/
/**
* generateRouteAddResponse
* state handler to generate the route add response message
*
* @param - pointer to event data
*
* @return - void

*/
static void generateRouteAddResponse(const uint8_t * const pData)
{

   RouteAddResponseMessage_t response = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   CO_RouteAddResponseData_t MessageData;
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
   // Cast pData input parameter to the message type
   memcpy(&MessageData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_RouteAddResponseData_t));
   
   
   /* pack into phy data req and put into RACH Q */
   pPhyDataReq = ALLOCMESHPOOL;
   if (pPhyDataReq)
   {
      /* populate response signal */
      response.Header.FrameType             = FRAME_TYPE_DATA_E;
      response.Header.MACDestinationAddress = MessageData.DestinationNodeID;
      response.Header.MACSourceAddress      = gNetworkAddress;
      response.Header.HopCount              = 0;
      response.Header.DestinationAddress    = MessageData.DestinationNodeID;
      response.Header.SourceAddress         = gNetworkAddress;
      response.Header.MessageType           = APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E;
      response.Accepted                     = MessageData.Accepted;
      response.DayNight                     = MessageData.DayNight;
      response.GlobalDelayEnabled           = MessageData.GlobalDelayEnabled;
      response.ZoneEnabled                  = MessageData.ZoneEnabled;
      response.ZoneLowerUpper               = MessageData.ZoneLowerUpper;
      response.ZoneMapWord                  = MessageData.ZoneMapWord;
      response.ZoneMapHalfWord              = MessageData.ZoneMapHalfWord;
      response.FaultsEnabled                = MessageData.FaultsEnabled;
      response.GlobalDelayOverride          = MessageData.GlobalDelayOverride;
      response.SystemId                     = SM_SystemId;
               
      pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
      pPhyDataReq->Payload.PhyDataReq.Downlink = true;
      pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
      status = MC_PUP_PackRouteAddResponseMsg(&response, &pPhyDataReq->Payload.PhyDataReq);

      if (SUCCESS_E == status)
      {
         CO_PRINT_B_2(DBG_INFO_E, "Sending RouteAdd response from node %d to node %d\r\n", response.Header.MACSourceAddress, response.Header.MACDestinationAddress);

         /* Check with Ack Management that we are not blocked, waiting for an ACK */
         uint32_t handle = GetNextHandle();
         if ( false == MC_ACK_AddMessage(response.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E, response.ZoneLowerUpper, false, handle, pPhyDataReq, true, SEND_ONCE_E) )
         {
            status = ERR_QUEUE_OVERFLOW_E;
         }
      }
      FREEMESHPOOL(pPhyDataReq);
   }
   else 
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generateRouteAddResponse - MeshPool failed to allocate\r\n");
   }

}

/*************************************************************************************/
/**
* rbuRouteAddResponse
* state handler for RBU receiving a Route Add Response
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuRouteAddResponse(const uint8_t * const pData)
{
   RouteAddResponseMessage_t response = { 0 };
   ErrorCode_t status = SUCCESS_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackRouteAddResponseMsg(&pMsg->Payload.PhyDataInd, &response);
   if (SUCCESS_E == status)
   {
      if ((gNetworkAddress == response.Header.DestinationAddress) && (response.SystemId == SM_SystemId) )
      {
         SM_SendACK( response.Header.MACSourceAddress, ACK_RACHS_E, true);
         if ( MC_SMGR_RouteAddResponse(&response) )
         {
            if ( 0 != response.Accepted )
            {
               /* The message was accepted and node address response.Header.SourceAddress is now a parent */
               // If the mesh is in the active state and the device itself isn't then auto-increment the device's state to ACTIVE.
               if (STATE_ACTIVE_E == MC_STATE_GetMeshState() && STATE_ACTIVE_E != MC_STATE_GetScheduledState() && STATE_ACTIVE_E != MC_STATE_GetDeviceState() )
               {
                  // Change RBU State to ACTIVE: the second parameter is not needed by the function in this case.
                  MC_STATE_ScheduleStateChange(STATE_ACTIVE_E, false);
                  MC_STATE_ActivateStateChange(0);
               }
               
               /* TODO: put any other acceptance code here */
            }
            else
            {
               /* Node response.Header.SourceAddress declined to be our parent.  We need to pick another one. */
               CO_PRINT_B_1(DBG_INFO_E, "RouteAdd REJECTED by node %d\r\n", response.Header.SourceAddress);

               /* TODO: put recovery code here */
            }         
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* void SM_ActivateStateChange
*
* Function to allow the MAC to notify the State Machine that its scheduled state
* change can now taken place.
* This can be called in three circumstances:
* 1) When the whole mesh is forming, the MAC calls this at the start of a new long frame,
*    so that all devices in the mesh change state at the same time.
* 2) When the heartbeat of the initial tracking node has been received, and the rest of
*    the mesh is beyond the CONFIG_SYNC state.  This RBU then needs to move to the MESH-FORM state.
* 3) When an Acknowledgement is received indicating that all parents have now accepted a ROUTE_ADD
*    request.  This RBU then needs to move to the ACTIVE state.
*
* @param - const uint8_t LongFrameIndex: Used to set the start and stop long frames for SNR averaging
* that occurs before selecting the rank of the device in CONFIG_FORM state.
*
* @return - void
*/
void SM_ActivateStateChange(const uint8_t LongFrameIndex)
{
   if (STATE_MAX_E > MC_STATE_GetScheduledState())
   {
      if ( STATE_CONFIG_SYNC_E == MC_STATE_GetDeviceState() || STATE_IDLE_E == MC_STATE_GetScheduledState())
      {         
         if ( false == SM_IsNCU )
         {
           // If the state is has changed to STATE_CONFIG_SYNC_E, reset the RBUs.
           //Error_Handler("Reset RBU after state change to STATE_CONFIG_SYNC_E or STATE_IDLE_E");
           MC_SMGR_DelayedReset("Reset RBU after state change to STATE_CONFIG_SYNC_E or STATE_IDLE_E");
         }
         else 
         {
           // If the state is has changed to STATE_CONFIG_SYNC_E, reset the NCU.
           Error_Handler("Reset NCU after state change to STATE_CONFIG_SYNC_E or STATE_IDLE_E");
         }
      }

      // Otherwise, set the requested state.
      MC_STATE_SetDeviceState(MC_STATE_GetScheduledState());

      CO_PRINT_B_1(DBG_INFO_E, "Device state set to %d\r\n", MC_STATE_GetScheduledState());
   }
   else
   {
      CO_PRINT_B_1(DBG_ERROR_E, "Invalid state value requested in SM_ActivateStateChange: %d\r\n", MC_STATE_GetScheduledState());
   }

   // Reset the scheduled device state to neutral value.
   MC_STATE_ScheduleStateChange(STATE_NOT_DEFINED_E, false);
   
   return;
}

/*************************************************************************************/
/**
* generateRBUDisable
* state handler to generate the RBU Disable signal
*
* @param - pointer to event data
*
* @return - void

*/
static void generateRBUDisable(const uint8_t * const pData)
{
   RBUDisableMessage_t rbuDisableMsg = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   osStatus osStat = osErrorOS;
   ErrorCode_t status = SUCCESS_E;
   CO_RBUDisableData_t rubDisableData;
   uint32_t i = 0;
   uint32_t number_of_messages = CFG_GetNumberOfResends();
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* Cast the pData input parameter to a CO_Message_t type and copy the payload into a CO_RBUDisableData_t */
   CO_Message_t* pMessage = (CO_Message_t*)pData;      
   memcpy(&rubDisableData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_RBUDisableData_t));
   
   /* populate the message to be sent */
   rbuDisableMsg.Header.FrameType             = FRAME_TYPE_DATA_E;
   rbuDisableMsg.Header.MACDestinationAddress = ADDRESS_GLOBAL;
   rbuDisableMsg.Header.MACSourceAddress      = ADDRESS_NCU;
   rbuDisableMsg.Header.HopCount              = 0;
   rbuDisableMsg.Header.DestinationAddress    = ADDRESS_GLOBAL;
   rbuDisableMsg.Header.SourceAddress         = ADDRESS_NCU;
   rbuDisableMsg.Header.MessageType           = APP_MSG_TYPE_RBU_DISABLE_E;
   rbuDisableMsg.UnitAddress                  = rubDisableData.UnitAddress;
   rbuDisableMsg.SystemId                     = SM_SystemId;

   for (i = 0; i < number_of_messages; i++)
   {
      /* pack into phy data req and put into DLCCHx Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         /* set the message type */
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         /*Pack our message into the pPhyDataReq payload */
         status = MC_PUP_PackRBUDisableMessage(&rbuDisableMsg, &pPhyDataReq->Payload.PhyDataReq);
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();

         if (SUCCESS_E == status)
         {
            if ( SUCCESS_E == MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq) )
            {
               CO_PRINT_B_1(DBG_INFO_E, "Put RBU disable msg on DLCCHSQ Queue - node %d\r\n", rubDisableData.UnitAddress);
               osStat = osOK;
            }
            else 
            {
               CO_PRINT_B_0(DBG_ERROR_E, "Failed to put RBU disable msg on DLCCHSQ Queue\r\n");
               osStat = osErrorOS;
            }
         }
         //The MAC Queue makes a copy so we can free the message
         FREEMESHPOOL(pPhyDataReq);
      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateRBUDisable - MeshPool failed to allocate\r\n");
         status = ERR_NO_MEMORY_E;
      }
      
      if ((SUCCESS_E != status) || (osOK != osStat))
      {
         break;
      }
   }
   
   /* perform the required callback to the Application code */
   if ((SUCCESS_E != status) || (osOK != osStat))
   {
      /* Let the application know the msg failed */
      SM_SendConfirmationToApplication(rubDisableData.Handle, 0, APP_CONF_RBU_DISABLE_E, ERR_MESSAGE_FAIL_E);
   }
   else
   {
      /* Let the application know the msg was queued */
      SM_SendConfirmationToApplication(rubDisableData.Handle, 0, APP_CONF_RBU_DISABLE_E, SUCCESS_E);      
   }
}

/*************************************************************************************/
/**
* rbuDisableMessage
* state handler for RBU Disable message
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuDisableMessage(const uint8_t * const pData)
{
   RBUDisableMessage_t rbuDisableMsg = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* cast the received pData to a CO_Message_t type and unpack it into a RBUDisableMessage_t */
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   status = MC_PUP_UnpackRBUDisableMessage(&pMsg->Payload.PhyDataInd, &rbuDisableMsg);
   
   if ( (SUCCESS_E == status) && 
        (rbuDisableMsg.SystemId == SM_SystemId) &&
        (MC_SMGR_IsParentNode(rbuDisableMsg.Header.MACSourceAddress)) )
   {
#ifdef SHOW_DOWNLINK_LED
      DM_LedPatternRequest(LED_DOWNLINK_MESSAGE_E);
#endif
      /* create PhyDataRequest for transmission on DLCCH to children */
//      if ( 0 < MC_SMGR_GetNumberOfChildren() )
//      {
         pPhyDataReq = ALLOCMESHPOOL;
         if (pPhyDataReq)
         {
            /* populate PhyDataRequest using cloned payload from received PhyDataInd*/
            pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
            memcpy(pPhyDataReq->Payload.PhyDataReq.Data, pMsg->Payload.PhyDataInd.Data, pMsg->Payload.PhyDataInd.Size);
            pPhyDataReq->Payload.PhyDataReq.Size = pMsg->Payload.PhyDataInd.Size;
            pPhyDataReq->Payload.PhyDataReq.slotIdxInSuperframe = 0;   /* will be populated in MAC */
            pPhyDataReq->Payload.PhyDataReq.freqChanIdx = 0;           /* will be populated in MAC */
            pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();

            if ( SUCCESS_E == MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq) )
            {
               CO_PRINT_B_1(DBG_INFO_E, "Put RBU disable msg on DLCCHSQ Queue - node %d\r\n", rbuDisableMsg.UnitAddress);
            }
            else 
            {
               CO_PRINT_B_0(DBG_ERROR_E, "Failed to put RBU disable msg on DLCCHSQ Queue\r\n");
            }
            //The MAC Queue makes a copy so we can free the message
            FREEMESHPOOL(pPhyDataReq);
         }
         else 
         {
            CO_PRINT_A_0(DBG_ERROR_E,"rbuDisableMessage - MeshPool failed to allocate\r\n");
         }
//      }

         
      /* Send the message up to the Application if this is the target node*/
         if (rbuDisableMsg.UnitAddress == SM_address)
         {
            CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx RBU Disable\r\n");
            /* Perform 'RBU Disable' on this node by putting the node into TEST MODE SLEEP*/
            MC_SetTestMode( MC_MAC_TEST_MODE_SLEEP_E );
            MC_STATE_SetDeviceState(STATE_TEST_MODE_E);
            /* notify the Application */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_RBU_DISABLE_E;
            uint16_t* pBuffer = (uint16_t*)appMessage.MessageBuffer;
            *pBuffer = rbuDisableMsg.UnitAddress;
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );

         }
         else
         {
            /* Check with Session Manager if rbuDisableMsg.UnitAddress is a parent or tracking node and discard if it is */
            // Note that we don't delete child nodes.  For a child node we report the child's missing heartbeat to the NCU
            // and this is how the NCU knows that the message was received by the target node.
            if ( (true == MC_SMGR_IsParentNode(rbuDisableMsg.UnitAddress)) || (true == MC_SMGR_IsATrackingNode(rbuDisableMsg.UnitAddress)) )
            {
               /* it's a parent or tracking node that has been disabled.  Remove it from our records.*/
               bool node_removed = MC_SMGR_DeleteNode(rbuDisableMsg.UnitAddress, "Detected disable msg from NCU");
               if ( node_removed )
               {
                  CO_PRINT_B_1(DBG_INFO_E, "Parent/Tracking node %d discarded in response to a RBU Disable command\r\n", rbuDisableMsg.UnitAddress);
                  /* Close the DCH slot for the disabled nodes heartbeat */
                  MC_SMGR_StopHeartbeatMonitorForNode(rbuDisableMsg.UnitAddress);
               }
            }
            else if ( true == MC_SMGR_IsATrackingNode(rbuDisableMsg.UnitAddress))
            {
               /* Close the DCH slot for the disabled nodes heartbeat */
               MC_SMGR_StopHeartbeatMonitorForNode(rbuDisableMsg.UnitAddress);
            }
         }
      }
}

/*************************************************************************************/
/**
* generateStatusIndication
* state handler to generate the Status Indication message and sends it to the NCU
*
* @param - pointer to event data
*
* @return - void

*/
static void generateStatusIndication(const uint8_t * const pData)
{
   StatusIndicationMessage_t statusIndicationMsg = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   CO_StatusIndicationData_t statusIndicationData;
   uint16_t macDestinationNode = 0;
   bool msg_queued = false;
   
   
   CO_PRINT_B_0(DBG_INFO_E, "Generating Status Ind\n\r");
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* get the MAC destination mode */
   macDestinationNode = MC_SMGR_GetPrimaryParentID();
   
   /* Make sure that the primary parent is established */
   if ( CO_BAD_NODE_ID != macDestinationNode )
   {   
      /* Cast the pData input parameter to a CO_Message_t type and copy the payload into a CO_StatusIndicationData_t */
      CO_Message_t* pMessage = (CO_Message_t*)pData;      
      memcpy(&statusIndicationData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_StatusIndicationData_t));
      
      /* populate the message to be sent */
      statusIndicationMsg.Header.FrameType             = FRAME_TYPE_DATA_E;
      statusIndicationMsg.Header.MACDestinationAddress = macDestinationNode;
      statusIndicationMsg.Header.MACSourceAddress      = gNetworkAddress;
      statusIndicationMsg.Header.HopCount              = 0;
      statusIndicationMsg.Header.DestinationAddress    = NCU_NETWORK_ADDRESS;
      statusIndicationMsg.Header.SourceAddress         = gNetworkAddress;
      statusIndicationMsg.Header.MessageType           = APP_MSG_TYPE_STATUS_INDICATION_E;   
      statusIndicationMsg.PrimaryParentID              = MC_SMGR_GetPrimaryParentID();
      statusIndicationMsg.SecondaryParentID            = MC_SMGR_GetSecondaryParentID();
      statusIndicationMsg.Rank                         = MC_SMGR_GetRank();
      statusIndicationMsg.Event                        = statusIndicationData.Event;
      statusIndicationMsg.OverallFault                 = statusIndicationData.OverallFault;
      statusIndicationMsg.SystemId                     = SM_SystemId;
      
      CO_PRINT_B_0(DBG_INFO_E, "Sending status Signal.\n\r");
      
      if ( (CO_MESH_EVENT_CHILD_NODE_ADDED_E <= statusIndicationData.Event) &&
           (CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E >= statusIndicationData.Event))
      {
         statusIndicationMsg.EventData = statusIndicationData.EventNodeId;
      }
      else 
      {
         statusIndicationMsg.EventData = statusIndicationData.EventData;
      }
      
      //If there is no secondary parent, we don't have enough bits in the message to assign an out-of-range value.
      //Set it to the same as the primary parent.  The receiving end interprets this as 'no secondary parent.'
      if ( CO_BAD_NODE_ID == statusIndicationMsg.SecondaryParentID )
      {
         statusIndicationMsg.SecondaryParentID = statusIndicationMsg.PrimaryParentID;
         statusIndicationMsg.AverageRssiSecondaryParent = 0;
      }

      /* Primary Parent average SNR. We need to convert the 16 bit signed snr value to fit the unsigned 6 bit message field */
      int16_t averageRssiPrimaryParent = MC_SMGR_GetAverageRssiForTrackingNode(PRIMARY_PARENT);   
      if ( MIN_REPORTABLE_RSSI > averageRssiPrimaryParent )
      {
         averageRssiPrimaryParent = MIN_REPORTABLE_RSSI;
      }
      statusIndicationMsg.AverageRssiPrimaryParent = (uint32_t)averageRssiPrimaryParent & STATUS_IND_AVERAGE_RSSI_MASK;
      
      /* Secondary Parent average SNR. We need to convert the 16 bit signed snr value to fit the unsigned 6 bit message field */
      int16_t averageRssiSecondaryParent = MC_SMGR_GetAverageRssiForTrackingNode(SECONDARY_PARENT);
      if ( MIN_REPORTABLE_RSSI > averageRssiSecondaryParent )
      {
         averageRssiSecondaryParent = MIN_REPORTABLE_RSSI;
      }
      statusIndicationMsg.AverageRssiSecondaryParent = (uint32_t)averageRssiSecondaryParent & STATUS_IND_AVERAGE_RSSI_MASK;
      
      CO_PRINT_B_4(DBG_INFO_E,"GenStatus Pnode=%d, Srssi=%d, Snode=%d, Srssi=%d\r\n", statusIndicationMsg.PrimaryParentID, statusIndicationMsg.AverageRssiPrimaryParent, statusIndicationMsg.SecondaryParentID, statusIndicationMsg.AverageRssiSecondaryParent);
      
      
      /* pack into phy data req and put into DLCCHx Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         /* set the message type */
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         /*Pack our message into the pPhyDataReq payload */
         status = MC_PUP_PackStatusIndicationMsg(&statusIndicationMsg, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
            /* If the NCU has broadcast a status request, RBUs should answer in their SACH slot.
               If the Status is being sent due to a change on the device, it can go on the RACHS */
            if ( statusIndicationData.DelaySending )//|| (0 < MC_SACH_MessageCount()) )
            {
               CO_PRINT_B_0(DBG_INFO_E,"Queueing Status Ind on DULCH\r\n");
               /* Add the message to the SACH Manager.  It will be sent in the scheduled slot */
               msg_queued = MC_SACH_ScheduleMessage(macDestinationNode, APP_MSG_TYPE_STATUS_INDICATION_E, statusIndicationData.Event, pPhyDataReq, SEND_SACH_ONLY_E);
            }
            else 
            {
               CO_PRINT_B_0(DBG_INFO_E,"Queueing Status Ind on ACK Mgr\r\n");
               /* Add the message to the ACK Manager.  It will be sent in the earliest RACHS slot */
               msg_queued = MC_ACK_AddMessage(macDestinationNode, ACK_RACHS_E, APP_MSG_TYPE_STATUS_INDICATION_E, statusIndicationData.Event, true, statusIndicationData.Handle, pPhyDataReq, true, SEND_NORMAL_E);
               if ( msg_queued )
               {
                  SM_TryToSendImmediately( ACK_RACHS_E, MAC_RACHS_Q_E );
               }
            }
            
         }
         
         /* The SACH Manager make a copy so we can release the original message*/
         FREEMESHPOOL(pPhyDataReq);
      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateStatusIndication - MeshPool failed to allocate\r\n");
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E, "Unable to send Status Ind - No Parent nodes\n\r");
   }
   
   /* send a confirmation msg to the application */
   SM_SendConfirmationToApplication(statusIndicationData.Handle, 0, APP_CONF_STATUS_SIGNAL_E, msg_queued);
}

/*************************************************************************************/
/**
* ncuStatusIndication
* state handler for NCU receiving Status Indication
*
* @param - pointer to event data
*
* @return - void

*/
static void ncuStatusIndication(const uint8_t * const pData)
{
   StatusIndicationMessage_t statusIndicationMsg = { 0 };
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* cast the received pData to a CO_Message_t type and unpack it into a StatusIndicationMessage_t */
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   
   status = MC_PUP_UnpackStatusIndicationMsg(&pMsg->Payload.PhyDataInd, &statusIndicationMsg);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (statusIndicationMsg.Header.MACDestinationAddress == gNetworkAddress) && 
         (statusIndicationMsg.SystemId == SM_SystemId) )
  
   {
      SM_SendACK(statusIndicationMsg.Header.MACSourceAddress, ACK_RACHS_E, false);
      
      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(statusIndicationMsg.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(statusIndicationMsg.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         //Resolve when there is no secondary parent.
         //This is indicated if the secondary parent address is the same as the primary parent.
         if ( statusIndicationMsg.SecondaryParentID == statusIndicationMsg.PrimaryParentID )
         {
            statusIndicationMsg.SecondaryParentID = CO_BAD_NODE_ID;
            statusIndicationMsg.AverageRssiSecondaryParent = MIN_REPORTABLE_UNSIGNED_RSSI;
         }
         
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Status Indication NCU\r\n");
        
         if ( statusIndicationMsg.Header.DestinationAddress == gNetworkAddress )
         {
            /* The message was addressed to this node.  Send it to the application. */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_STATUS_INDICATION_E;
            StatusIndication_t statusEvent;
            statusEvent.SourceNode = statusIndicationMsg.Header.SourceAddress;
            statusEvent.Event = statusIndicationMsg.Event;
            statusEvent.EventData = statusIndicationMsg.EventData;
            statusEvent.PrimaryParent = statusIndicationMsg.PrimaryParentID;
            statusEvent.SecondaryParent = statusIndicationMsg.SecondaryParentID;
            statusEvent.AverageRssiPrimaryParent = statusIndicationMsg.AverageRssiPrimaryParent;
            statusEvent.AverageRssiSecondaryParent = statusIndicationMsg.AverageRssiSecondaryParent;
            statusEvent.Rank = statusIndicationMsg.Rank;
            statusEvent.OverallFault = statusIndicationMsg.OverallFault;
            
            memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(StatusIndication_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* rbuStatusIndication
* state handler for RBU receiving StatusIndication
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuStatusIndication(const uint8_t * const pData)
{
   StatusIndicationMessage_t statusIndicationMsg = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackStatusIndicationMsg(&pMsg->Payload.PhyDataInd, &statusIndicationMsg);
   
   /* Check that it unpacked OK and comes form a node in our system */
   if ( (SUCCESS_E == status) && (statusIndicationMsg.Header.MACDestinationAddress == gNetworkAddress) && 
         (statusIndicationMsg.SystemId == SM_SystemId) )
  
   {
      SM_SendACK( statusIndicationMsg.Header.MACSourceAddress, ACK_RACHS_E, false);
      
      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(statusIndicationMsg.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(statusIndicationMsg.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Status Indication RBU\r\n");
        
          /* do multi-hop if alarmSignal.Header.DestinationAddress is not the address of this unit */
         if ( statusIndicationMsg.Header.DestinationAddress != gNetworkAddress )
         {
            /* The message is not for us.  Forward it to a parent node. */
            uint32_t parent_node = MC_SMGR_GetPrimaryParentID();
            if ( CO_BAD_NODE_ID != parent_node )
            {
               uint16_t message_source = statusIndicationMsg.Header.MACSourceAddress;
               (void)message_source;   // suppress warnings
               
               /* address the message to the parent and increment the hop count */
               statusIndicationMsg.Header.MACDestinationAddress = parent_node;
               statusIndicationMsg.Header.MACSourceAddress = gNetworkAddress;
               statusIndicationMsg.Header.HopCount++;
               
               bool multi_hop = true;
               if( statusIndicationMsg.Header.MACDestinationAddress == statusIndicationMsg.Header.DestinationAddress )
               {
                  multi_hop = false;
               }
               
               if ( statusIndicationMsg.Header.HopCount < CFG_GetMaxHops() )
               {
                  /* pack into phy data req and put into RACHPQ */
                  pPhyDataReq = ALLOCMESHPOOL;
                  if (pPhyDataReq)
                  {
                     pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                     pPhyDataReq->Payload.PhyDataReq.Downlink = false;
                     pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
                     status = MC_PUP_PackStatusIndicationMsg(&statusIndicationMsg, &pPhyDataReq->Payload.PhyDataReq);

                     if (SUCCESS_E == status)
                     {
                        CO_PRINT_B_2(DBG_INFO_E, "MULTI-HOP - Rx'd Status Indication Msg from node %d.  Forwarding to node %d\r\n", message_source, parent_node);
                        /* Add the message to the ACK Manager and it will be sent in the next RACH slot */
                        if ( MC_ACK_AddMessage(statusIndicationMsg.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_STATUS_INDICATION_E, statusIndicationMsg.Event, multi_hop, 0, pPhyDataReq, true, SEND_NORMAL_E) )
                        {
                           SM_TryToSendImmediately( ACK_RACHS_E, MAC_RACHS_Q_E );
                        }
                     }
                     /* The ACK manager makes a copy so we can release the message memory */
                     FREEMESHPOOL(pPhyDataReq);
                  }
                  else 
                  {
                     CO_PRINT_A_0(DBG_ERROR_E,"rbuStatusIndication - MeshPool failed to allocate\r\n");
                  }
               }
               else
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "MULTI-HOP - DISCARDED Status Indication Msg - hop limit reached\r\n");
               }
            }
         }
      }
      else 
      {
         /* The status message was not meant for us but we can use it to see if it came from a parent
            reporting that it has dropped this node.  We sometimes miss the RouteDrop message due to 
            packet loss so this is a second opportunity to capture the event. */
         if ( MC_SMGR_IsParentNode(statusIndicationMsg.Header.SourceAddress) )
         {
            MC_SMGR_ProcessParentStatusReport( &statusIndicationMsg );
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}




/*************************************************************************************/
/**
* generateBatteryStatus
* state handler to generate the Battery Status message
*
* @param - pointer to event data
*
* @return - void

*/
void generateBatteryStatus(const uint8_t * const pData)
{
   static bool  first_status_report = true;
   BatteryStatusMessage_t batterySignal = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   CO_BatteryStatusData_t batteryData;
   ErrorCode_t status = SUCCESS_E;
   bool message_failed = false;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
      
   // Cast pData input parameter to the message type
   memcpy(&batteryData, pMessage->Payload.PhyDataReq.Data, sizeof(CO_BatteryStatusData_t));
   
   //Force first battery status to use DULCH
   if ( first_status_report )
   {
      batteryData.DelaySending = true;
   }
   
   uint16_t parent_address = MC_SMGR_GetPrimaryParentID();
   if (CO_BAD_NODE_ID != parent_address)
   {
      CO_PRINT_B_0(DBG_INFO_E, "Sending Battery Status Signal.\r\n");
      
      
      /* pack into phy data req and put into RACH Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         /* populate fire signal */
         batterySignal.Header.FrameType             = FRAME_TYPE_DATA_E;
         batterySignal.Header.MACDestinationAddress = parent_address;
         batterySignal.Header.MACSourceAddress      = gNetworkAddress;
         batterySignal.Header.HopCount              = 0;
         batterySignal.Header.DestinationAddress    = NCU_NETWORK_ADDRESS;
         batterySignal.Header.SourceAddress         = gNetworkAddress;
         batterySignal.Header.MessageType           = APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E;
         batterySignal.PrimaryBatteryVoltage        = batteryData.PrimaryBatteryVoltage;
         batterySignal.BackupBatteryVoltage         = batteryData.BackupBatteryVoltage;
         batterySignal.DeviceCombination            = batteryData.DeviceCombination;
         batterySignal.ZoneNumber                   = batteryData.ZoneNumber;
         batterySignal.SmokeAnalogueValue           = batteryData.SmokeAnalogueValue;
         batterySignal.HeatAnalogueValue            = batteryData.HeatAnalogueValue;
         batterySignal.PirAnalogueValue            = batteryData.PirAnalogueValue;
         batterySignal.SystemId                     = SM_SystemId;
         
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         status = MC_PUP_PackBatteryStatusMessage(&batterySignal, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
            CO_PRINT_B_3(DBG_INFO_E, "Battery Status Signal.  smoke=%d heat=%d, pir=%d\r\n", batterySignal.SmokeAnalogueValue, batterySignal.HeatAnalogueValue, batterySignal.PirAnalogueValue);
            if ( batteryData.DelaySending || (0 < MC_SACH_MessageCount()) || MC_SMGR_NetworkBusy(SECONDARY_RACH) )
            {
               CO_PRINT_B_3(DBG_INFO_E, "Sending Battery Status on DULCH.  DelaySending=%d DulchCount=%d, nwk_busy=%d\r\n", batteryData.DelaySending, MC_SACH_MessageCount(), MC_SMGR_NetworkBusy(SECONDARY_RACH));
               /* Schedlue this signal to go by DULCH */
               if ( false == MC_SACH_ScheduleMessage(batterySignal.Header.DestinationAddress, APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E, 0, pPhyDataReq, SEND_SACH_ONLY_E))
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "FAILED to queue Battery Status Signal on DULCH.\n\r");
                  message_failed = true;
               }
               else 
               {
                  //set first_status_report to false so that later messages can go on the immediate queue
                  first_status_report = false;
               }
            }
            else 
            {
               /* Schedlue this signal to go by RACH */
               if ( false == MC_ACK_AddMessage(batterySignal.Header.DestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E, 0, false, batteryData.Handle, pPhyDataReq, true, SEND_NORMAL_E) )
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "FAILED to queue Battery Status Signal on RACH-S.\n\r");
                  message_failed = true;
               }
               else 
               {
                  //set first_status_report to false so that later messages can go on the immediate queue
                  first_status_report = false;
               }
            }
         }
         else 
         {
            CO_PRINT_B_0(DBG_ERROR_E, "FAILED to generate Battery Status Signal.\n\r");
            message_failed = true;
         }
         /* the ack manager makes a copy so we can release the memory */
         FREEMESHPOOL(pPhyDataReq);
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateBatteryStatus - MeshPool failed to allocate\r\n");
         message_failed = true;
      }
   }
   else
   {
      CO_PRINT_B_0(DBG_INFO_E, "FAILED to send Battery Status Signal.  No parent nodes found.\n\r");
      message_failed  = true;
   }

   if ( message_failed )
   {
      SM_SendConfirmationToApplication(batteryData.Handle, 0, APP_CONF_BATTERY_STATUS_SIGNAL_E, ERR_MESSAGE_FAIL_E);
   }
}



/*************************************************************************************/
/**
* ncuBatteryStatus
* state handler for NCU receiving a Battery Status Signal
*
* @param - pointer to event data
*
* @return - void

*/
static void ncuBatteryStatus(const uint8_t * const pData)
{
   BatteryStatusMessage_t batterySignal = { 0 };
   ErrorCode_t status = SUCCESS_E;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackBatteryStatusMessage(&pMsg->Payload.PhyDataInd, &batterySignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (batterySignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (batterySignal.SystemId == SM_SystemId) )
  
   {
      SM_SendACK( batterySignal.Header.MACSourceAddress, ACK_RACHS_E, false);
      
      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(batterySignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(batterySignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_3(DBG_INFO_E, "<<<<<<<<<< Rx Battery Status Signal NCU.  smoke=%d heat=%d, pir=%d\r\n", batterySignal.SmokeAnalogueValue, batterySignal.HeatAnalogueValue, batterySignal.PirAnalogueValue);

         if ( batterySignal.Header.DestinationAddress == gNetworkAddress )
         {
            /* The message was addressed to this node.  Send it to the application. */
            ApplicationMessage_t appMessage;
            appMessage.MessageType = APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E;
            BatteryStatusIndication_t batteryStatus;
            batteryStatus.SourceAddress = batterySignal.Header.SourceAddress;
            batteryStatus.PrimaryBatteryVoltage = batterySignal.PrimaryBatteryVoltage;
            batteryStatus.BackupBatteryVoltage = batterySignal.BackupBatteryVoltage;
            batteryStatus.DeviceCombination = batterySignal.DeviceCombination;
            batteryStatus.ZoneNumber = batterySignal.ZoneNumber;
            batteryStatus.SmokeAnalogueValue = batterySignal.SmokeAnalogueValue;
            batteryStatus.HeatAnalogueValue = batterySignal.HeatAnalogueValue;
            batteryStatus.PirAnalogueValue = batterySignal.PirAnalogueValue;
            memcpy(appMessage.MessageBuffer, &batteryStatus, sizeof(BatteryStatusIndication_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* rbuBatteryStatus
* State handler for RBU receiving a Battery Status Signal
*
* @param - pointer to event data
*
* @return - void

*/
static void rbuBatteryStatus(const uint8_t * const pData)
{
   BatteryStatusMessage_t batterySignal = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackBatteryStatusMessage(&pMsg->Payload.PhyDataInd, &batterySignal);
   
   /* only process messages that are addressed to this node */
   if ( (SUCCESS_E == status) && (batterySignal.Header.MACDestinationAddress == gNetworkAddress) && 
         (batterySignal.SystemId == SM_SystemId) )
  
   {
      SM_SendACK( batterySignal.Header.MACSourceAddress, ACK_RACHS_E, false);

      bool is_a_child = true;
      if ( false == MC_SMGR_IsAChild(batterySignal.Header.MACSourceAddress) )
      {
         //a non-child thinks this node is a parent, a sign of assymetry.
         //Try to fix the link
         is_a_child = MC_SMGR_RemakeLink(batterySignal.Header.MACSourceAddress);
      }
      
      if ( is_a_child )
      {
         CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Rx Battery Status Signal RBU\r\n");

          /* do multi-hop if alarmSignal.Header.DestinationAddress is not the address of this unit */
         if ( batterySignal.Header.DestinationAddress != gNetworkAddress )
         {
            /* The message is not for us.  Forward it to a parent node. */
            uint32_t parent_node = MC_SMGR_GetPrimaryParentID();
            if ( CO_BAD_NODE_ID != parent_node )
            {
               uint16_t message_source = batterySignal.Header.MACSourceAddress;
               (void)message_source;   // suppress warnings

               /* address the message to the parent and increment the hop count */
               batterySignal.Header.MACDestinationAddress = parent_node;
               batterySignal.Header.MACSourceAddress = gNetworkAddress;
               batterySignal.Header.HopCount++;
               
               if ( batterySignal.Header.HopCount < CFG_GetMaxHops() )
               {
                  /* pack into phy data req and put into RACHPQ */
                  pPhyDataReq = ALLOCMESHPOOL;
                  if (pPhyDataReq)
                  {
                     bool multi_hop = true;
                     if( batterySignal.Header.MACDestinationAddress == batterySignal.Header.DestinationAddress )
                     {
                        multi_hop = false;
                     }
                     
                     pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
                     pPhyDataReq->Payload.PhyDataReq.Downlink = false;
                     pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
                     status = MC_PUP_PackBatteryStatusMessage(&batterySignal, &pPhyDataReq->Payload.PhyDataReq);

                     if (SUCCESS_E == status)
                     {
                        CO_PRINT_B_2(DBG_INFO_E, "MULTI-HOP - Rx'd battery status signal from node %d.  Forwarding to node %d\r\n", message_source, parent_node);
                        /* Add this signal to AckManagement and it will be sent in the next RACH slot */
                        MC_ACK_AddMessage(batterySignal.Header.MACDestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E, 0, multi_hop, 0, pPhyDataReq, true, SEND_NORMAL_E);
                     }
                     /* the ack manager makes a copy so we can release the memory */
                     FREEMESHPOOL(pPhyDataReq);
                  }
                  else
                  {
                     CO_PRINT_A_0(DBG_ERROR_E,"rbuBatteryStatus - MeshPool failed to allocate\r\n");
                  }
               }
               else
               {
                  CO_PRINT_B_0(DBG_ERROR_E, "MULTI-HOP - DISCARDED Battery Status Signal - hop limit reached\r\n");
               }
            }
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}


/*************************************************************************************/
/**
* applicationRequest
* Handler of requests for information exchange, received from the application
*
* @param - pointer to event data
*
* @return - void

*/
void applicationRequest(const uint8_t * const pData)
{
   ApplicationMessage_t appMessage = { 0 };
   
   CO_PRINT_B_0(DBG_INFO_E,"SM applicationRequest\r\n");
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* Cast the pData input parameter to a CO_Message_t type and copy the payload into a CO_RBUDisableData_t */
   CO_Message_t* pMessage = (CO_Message_t*)pData;      
   memcpy(&appMessage, pMessage->Payload.PhyDataReq.Data, sizeof(ApplicationMessage_t));
   
   switch ( appMessage.MessageType )
   {
      case APP_MSG_TYPE_STATUS_INDICATION_E:
      {
            /* The message was addressed to this node.  Send it to the application. */
            StatusIndication_t statusEvent;
            statusEvent.SourceNode = gNetworkAddress;
            statusEvent.Event = CO_MESH_EVENT_NONE_E;
            statusEvent.EventData = 0;
            statusEvent.PrimaryParent                = MC_SMGR_GetPrimaryParentID();
            statusEvent.SecondaryParent              = MC_SMGR_GetSecondaryParentID();
            statusEvent.Rank                         = MC_SMGR_GetRank();
            statusEvent.SystemId                     = SM_SystemId;
            statusEvent.SourceNode                   = gNetworkAddress;
         
            /* Primary Parent average SNR. We need to convert the 16 bit signed snr value to fit the unsigned 6 bit message field */
            int16_t averageRssiPrimaryParent = MC_SMGR_GetAverageRssiForTrackingNode(PRIMARY_PARENT);   
            if ( MIN_REPORTABLE_RSSI > averageRssiPrimaryParent )
            {
               averageRssiPrimaryParent = MIN_REPORTABLE_RSSI;
               averageRssiPrimaryParent &= STATUS_IND_AVERAGE_RSSI_MASK;
            }
            statusEvent.AverageRssiPrimaryParent = (uint32_t)averageRssiPrimaryParent;
            
            /* Secondary Parent average SNR. We need to convert the 16 bit signed snr value to fit the unsigned 6 bit message field */
            int16_t averageRssiSecondaryParent = MC_SMGR_GetAverageRssiForTrackingNode(SECONDARY_PARENT);
            if ( MIN_REPORTABLE_RSSI > averageRssiSecondaryParent )
            {
               averageRssiSecondaryParent = MIN_REPORTABLE_RSSI;
               averageRssiSecondaryParent &= STATUS_IND_AVERAGE_RSSI_MASK;
            }
            statusEvent.AverageRssiSecondaryParent = (uint32_t)averageRssiSecondaryParent;
      
            memcpy(appMessage.MessageBuffer, &statusEvent, sizeof(StatusIndication_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
      }
         break;
      case APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E:
         if (SM_IsNCU)
         {
            int32_t* pValue;
            pValue = (int32_t*)appMessage.MessageBuffer;
            
            MC_SMGR_SetPowerStatus(*pValue);
         }
         break;
      case APP_MSG_TYPE_ROUTE_DROP_E:
         MC_SMGR_SendRouteDrop(ADDRESS_GLOBAL, CO_RESET_REASON_RESET_COMMAND_E, false, true, 0);
         MC_SMGR_DelayedReset("Commanded Reset");
         break;
      case APP_MSG_TYPE_PPU_MODE_E:
         CO_PRINT_B_0(DBG_INFO_E,"SM APP_MSG_TYPE_PPU_MODE_E\r\n");
         MC_SMGR_ProcessPpuMessage(&appMessage);
         break;
      case APP_MSG_CHECK_TDM_SYNCH_E:
         CO_PRINT_B_0(DBG_INFO_E,"SM APP_MSG_CHECK_TDM_SYNCH_E\r\n");
         MC_TDM_MissedSlotTestAndCompensate();
         break;
#ifdef ENABLE_LINK_ADD_OR_REMOVE
      case APP_MSG_TYPE_DROP_NODE_LINK_E:
      {
         int32_t* pValue = (int32_t*)appMessage.MessageBuffer;
         int32_t node_id = *pValue;
         MC_SMGR_DeleteNode(node_id, "User command");
      }
         break;
      case APP_MSG_TYPE_ADD_NODE_LINK_E:
      {
         int32_t* pValue = (int32_t*)appMessage.MessageBuffer;
         int32_t node_id = *pValue;
         pValue++;
         int32_t downlink = *pValue;
         MC_SMGR_ForceAddNode((uint16_t)node_id, (bool)downlink);
      }
         break;
#endif
      default:
         break;
   }
   
}

/*************************************************************************************/
/**
* generatePingRequest
* state handler to generate the Ping request message
*
* @param - pointer to event data
*
* @return - void
*/
void generatePingRequest(const uint8_t * const pData)
{
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   PingRequest_t pingRequest;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* Cast the pData input parameter to a CO_Message_t type and copy the payload into a CO_RBUDisableData_t */
   CO_Message_t* pMessage = (CO_Message_t*)pData;      
   memcpy(&pingRequest, pMessage->Payload.PhyDataReq.Data, sizeof(PingRequest_t));
   
   /* pack into phy data req and put into DLCCHx Q */
   pPhyDataReq = ALLOCMESHPOOL;
   if (pPhyDataReq)
   {
      /* set the message type */
      pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
      pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
      if ( MC_SMGR_IsParentNode(pingRequest.Header.MACDestinationAddress) )
      {
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
      }
      else 
      {
         pPhyDataReq->Payload.PhyDataReq.Downlink = true;
      }
      /*Pack our message into the pPhyDataReq payload */
      status = MC_PUP_PackPingRequestMessage(&pingRequest, &pPhyDataReq->Payload.PhyDataReq);

      /* push the message onto the ACK Manager for transmission */
      if (SUCCESS_E == status)
      {
         CO_PRINT_B_2(DBG_INFO_E,"Sending Ping to node %d, power=%d\r\n", pingRequest.Header.DestinationAddress, pPhyDataReq->Payload.PhyDataReq.txPower);
         if ( false == MC_ACK_AddMessage(pingRequest.Header.DestinationAddress, ACK_RACHS_E, APP_MSG_TYPE_PING_E, 0, false, 0, pPhyDataReq, true, pingRequest.SendBehaviour) )
         {
            CO_PRINT_B_1(DBG_ERROR_E, "Failed to queue Ping Request msg - status=%d\r\n", status);
         }
      }
      /* the ACK Manager makes a copy so we can release the original message*/
      FREEMESHPOOL(pPhyDataReq);
   }
   else 
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generatePingRequest - MeshPool failed to allocate\r\n");
   }
}

/*************************************************************************************/
/**
* pingRequest
* state handler for receiving a ping from a neighbour RBU
*
* @param - pointer to event data
*
* @return - void

*/
static void pingRequest(const uint8_t * const pData)
{
   PingRequest_t pingMsg = { 0 };
   ErrorCode_t status = SUCCESS_E;
  
   CO_PRINT_B_0(DBG_INFO_E, "ping+\r\n");
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackPingRequestMessage(&pMsg->Payload.PhyDataInd, &pingMsg);
   
   /* Check that it unpacked OK and comes form a node in our system */
   if (SUCCESS_E == status)
   {
      /* check that it's addressed to this node */
      if ( (pingMsg.Header.MACDestinationAddress == gNetworkAddress) && (pingMsg.SystemId == SM_SystemId) )
      {
         if ( KILL_COMMAND == pingMsg.Payload )
         {
            MC_SMGR_DelayedReset("Received Kill Command");
         }
         else
         {
            //The ACK is accepted as the ping response
            SM_SendACK( pingMsg.Header.MACSourceAddress, ACK_RACHS_E, true);
         }
      }
      else if (ADDRESS_GLOBAL == pingMsg.Header.MACDestinationAddress )
      {
         if ( KILL_COMMAND == pingMsg.Payload )
         {
            MC_SMGR_DelayedReset("Received Kill Command");
         }
      }
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

/*************************************************************************************/
/**
* generateZoneEnableMessage
* state handler to generate the Zone Enable message
*
* @param - pointer to event data
*
* @return - void
*/
void generateZoneEnableMessage(const uint8_t * const pData)
{
   ZoneEnableMessage_t zoneEnableMsg = { 0 };
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   ZoneEnableData_t zoneData;
   bool at_least_one_msg_was_queued = false;
   uint32_t number_to_send = CFG_GetNumberOfResends();
   
   CO_PRINT_B_0(DBG_INFO_E, "generateZoneEnableMessage+\r\n");
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   /* Cast the pData input parameter to a CO_Message_t type and copy the payload into a CO_RBUDisableData_t */
   CO_Message_t* pMessage = (CO_Message_t*)pData;      
   memcpy(&zoneData, pMessage->Payload.PhyDataReq.Data, sizeof(ZoneEnableData_t));
   
   uint16_t primary_parent_id = MC_SMGR_GetPrimaryParentID();
   
   /* populate the message to be sent */
   zoneEnableMsg.Header.FrameType             = FRAME_TYPE_DATA_E;
   zoneEnableMsg.Header.MACDestinationAddress = ADDRESS_GLOBAL;
   zoneEnableMsg.Header.MACSourceAddress      = gNetworkAddress;
   zoneEnableMsg.Header.HopCount              = 0;
   zoneEnableMsg.Header.DestinationAddress    = ADDRESS_GLOBAL;
   zoneEnableMsg.Header.SourceAddress         = gNetworkAddress;
   zoneEnableMsg.Header.MessageType           = APP_MSG_TYPE_ZONE_ENABLE_E;
   zoneEnableMsg.HalfWord                     = zoneData.HalfWord;
   zoneEnableMsg.Word                         = zoneData.Word;
   zoneEnableMsg.LowHigh                      = zoneData.LowHigh;
   zoneEnableMsg.SystemId                     = SM_SystemId;


   for (uint8_t i = 0; i < number_to_send; i++)
   {
      /* pack into phy data req and put into DLCCHx Q */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         /* set the message type */
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = true;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         /*Pack our message into the pPhyDataReq payload */
         status = MC_PUP_PackZoneEnableMessage(&zoneEnableMsg, &pPhyDataReq->Payload.PhyDataReq);

         /* push the message onto the ACK Manager for transmission */
         if (SUCCESS_E == status)
         {
            status = MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq);
            if ( SUCCESS_E != status )
            {
               CO_PRINT_B_1(DBG_ERROR_E, "Failed to add Zone Enable to DLCCHSQ Queue - Error %d\r\n", status);
            }
            else
            {
               CO_PRINT_B_0(DBG_INFO_E, "Put Zone Enable on DLCCHSQ\r\n");
               at_least_one_msg_was_queued = true;
            }
         }
         else 
         {
            CO_PRINT_A_0(DBG_ERROR_E,"generateZoneEnableMessage - Message Pack Fail\r\n");
         }
         /* the ACK Manager makes a copy so we can release the original message*/
         FREEMESHPOOL(pPhyDataReq);
      }
      else 
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateZoneEnableMessage - MeshPool failed to allocate\r\n");
      }
   }

   if (at_least_one_msg_was_queued)
   {
      /* Let the application know the msg was queued */
      SM_SendConfirmationToApplication(zoneData.Handle, 0, APP_CONF_ZONE_ENABLE_MESSAGE_E, SUCCESS_E);
   }
   else
   {
      /* Let the application know the msg failed */
      SM_SendConfirmationToApplication(zoneData.Handle, 0, APP_CONF_ZONE_ENABLE_MESSAGE_E, ERR_MESSAGE_FAIL_E);
   }
}

/*************************************************************************************/
/**
* zoneEnableMessage
* state handler for receiving a Zone Enable message
*
* @param - pointer to event data
*
* @return - void

*/
void zoneEnableMessage(const uint8_t * const pData)
{
   ZoneEnableMessage_t zoneMessage = { 0 };
   ErrorCode_t status = SUCCESS_E;
   CO_Message_t *pPhyDataReq = NULL;
  
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackZoneEnableMessage(&pMsg->Payload.PhyDataInd, &zoneMessage);
   
   if ( SUCCESS_E == status && zoneMessage.SystemId == SM_SystemId)
   {
#ifdef SHOW_DOWNLINK_LED
      DM_LedPatternRequest(LED_DOWNLINK_MESSAGE_E);
#endif
      /* check that the message comes from an uplink source */
      uint16_t source_node = zoneMessage.Header.MACSourceAddress;      
      if ( MC_SMGR_IsParentNode(source_node) ||
         ( (STATE_ACTIVE_E != MC_STATE_GetDeviceState()) && (source_node == MC_SYNC_GetSyncNode())) )
      {
         /* create PhyDataRequest for transmission on DLCCH to children */
//         if ( 0 != MC_SMGR_GetNumberOfChildren() )
//         {
            pPhyDataReq = ALLOCMESHPOOL;
            if (pPhyDataReq)
            {
               /* populate PhyDataRequest using received data. Update MACSourceAddress and hop count*/
               pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
               zoneMessage.Header.MACSourceAddress = gNetworkAddress;
               zoneMessage.Header.HopCount++;
               
               if ( zoneMessage.Header.HopCount < CFG_GetMaxHops() )
               {
                  status = MC_PUP_PackZoneEnableMessage(&zoneMessage, &pPhyDataReq->Payload.PhyDataReq);
                  pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();

                  if (SUCCESS_E == status)
                  {
                     status = MC_MACQ_Push(MAC_DLCCHS_Q_E,pPhyDataReq);
                     if ( SUCCESS_E != status )
                     {
                        CO_PRINT_B_1(DBG_ERROR_E, "DLCCHSQ write fail - status=%d\r\n", status);
                     }
                     else 
                     {
                        CO_PRINT_B_0(DBG_INFO_E, "Put Zone Enable msg on DLCCHSQ Queue\r\n");
                     }
                  }
               }
               //The MAC Queue makes a copy so free the message
               FREEMESHPOOL(pPhyDataReq);
            }
            else 
            {
               CO_PRINT_A_0(DBG_ERROR_E,"zoneEnableMessage - MeshPool failed to allocate\r\n");
            }
//         }
      }
      
      /* check if the message is for this node */
      if (( true == SMCheckDestinationAddress(zoneMessage.Header.DestinationAddress, ZONE_GLOBAL) ))
      {
         CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< Rx Zone Enable Message to node %d\r\n", zoneMessage.Header.DestinationAddress);
         
         /* Send the output request up to the Application */
         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_ZONE_ENABLE_E;
         ZoneEnableData_t zoneData;
         zoneData.LowHigh = zoneMessage.LowHigh  ? ZONE_BITS_HIGH_E:ZONE_BITS_LOW_E;
         zoneData.HalfWord = zoneMessage.HalfWord;
         zoneData.Word = zoneMessage.Word;
         memcpy(appMessage.MessageBuffer, &zoneData, sizeof(ZoneEnableData_t));
   
         SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
      }
   }
}


/*************************************************************************************/
/**
* generateAtCommandMessage
* state handler for sending an AT Command message over the air
*
* @param - pointer to event data
*
* @return - void

*/
void generateAtCommandMessage(const uint8_t * const pData)
{
   AtMessageData_t* pCommandData;
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
   pCommandData = (AtMessageData_t*)pMessage->Payload.PhyDataReq.Data;
      
   /* pack into phy data req and put into RACH Q */
   pPhyDataReq = ALLOCMESHPOOL;
   if (pPhyDataReq)
   {
      /* populate the Tx message */         
      pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
      pPhyDataReq->Payload.PhyDataReq.Downlink = false;
      pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
      char* pPayload = (char*)pPhyDataReq->Payload.PhyDataReq.Data;
      pPhyDataReq->Payload.PhyDataReq.Size = sizeof(AtMessageData_t) + 1;//+1 for the frame type byte.
      //Frame type goes into the top 4 bits of the first char
      *pPayload = AT_FRAME_TYPE_COMAND_E;
      //message starts at the next char.
      pPayload++;
      memcpy(pPayload, pCommandData, sizeof(AtMessageData_t));
      
      //Push the message into the RACHS queue
      status = MC_MACQ_Push(MAC_RACHS_Q_E, pPhyDataReq);
      if ( SUCCESS_E != status )
      {
         CO_PRINT_B_1(DBG_ERROR_E, "Failed to add AT Command to the send Queue - Error %d\r\n", status);
      }
      else 
      {
         //CO_PRINT_B_2(DBG_INFO_E, "generateAtCommandMessage sending to node=%d, size=%d\r\n", pCommandData->Destination, pPhyDataReq->Payload.PhyDataReq.Size);
         //Tell the MAC to send the message
         MC_MAC_SendAtCommand();
      }

      /* release the message memory */
      FREEMESHPOOL(pPhyDataReq);
   }
   else
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generateAtCommandMessage - MeshPool failed to allocate\r\n");
   }
}

/*************************************************************************************/
/**
* AtCommandMessage
* state handler for receiving AT messages (command and response)
*
* @param - pointer to event data
*
* @return - void

*/
static void AtCommandMessage(const uint8_t * const pData)
{
   static ApplicationMessage_t gAppMessage;
   AtMessageData_t atCommandData = { 0 };
   bool address_match = false;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   
   char* pPayload = (char*)pMsg->Payload.PhyDataInd.Data;
   //Skip over the frame type
   pPayload++;
   AtMessageData_t* pCmd = (AtMessageData_t*)pPayload;
   
   atCommandData.Destination = pCmd->Destination;
   atCommandData.IsCommand = pCmd->IsCommand;
   atCommandData.Source = pCmd->Source;
   strcpy((char*)atCommandData.Command, (char*)pCmd->Command);
   
   
   //Check the address
   if ( MAX_NUMBER_OF_SYSTEM_NODES <= atCommandData.Destination )
   {
      //the address is a serial number
      uint32_t serial_number;
      if ( SUCCESS_E == DM_NVMRead(NV_UNIT_SERIAL_NO_E, &serial_number, sizeof(uint32_t)) )
      {
         if ( atCommandData.Destination == serial_number )
         {
            address_match = true;
         }
      }
   }
   else 
   {
      //the address is a node ID
      uint16_t network_address = MC_GetNetworkAddress();
      if ( atCommandData.Destination == network_address )
      {
         address_match = true;
      }
   }
   
   /* check if the message is for this node */
   if ( address_match )
   {
      CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< AT Msg - %s\r\n", atCommandData.Command);
      
      /* Send the output request up to the Application */
      gAppMessage.MessageType = APP_MSG_TYPE_AT_COMMAND_E;
      memcpy(gAppMessage.MessageBuffer, &atCommandData, sizeof(AtMessageData_t));
      SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &gAppMessage );
   }
}

/*************************************************************************************/
/**
* generateAtModeMessage
* state handler for sending an AT mode message over the air
*
* @param - pointer to event data
*
* @return - void

*/
void generateAtModeMessage(const uint8_t * const pData)
{
   static ApplicationMessage_t appMessage;
   OtaMode_t* pCommandData;
   ATModeRequest_t mode_request;
   
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
   pCommandData = (OtaMode_t*)pMessage->Payload.PhyDataReq.Data;
   
   if ( pCommandData )
   {
      mode_request.Frametype = pCommandData->FrameType;
      mode_request.Address = pCommandData->Destination;
      mode_request.SystemID = pCommandData->SystemID;
      
      CO_PRINT_B_3(DBG_INFO_E, "generateAtModeMessage sending frame=%x node=%d, System=%d\r\n", mode_request.Frametype, mode_request.Address, mode_request.SystemID);
      
      if ( AT_FRAME_TYPE_ENTER_PPU_MODE_E == mode_request.Frametype )
      {
         //Command to enter PPU Disconnected mode
         if ( (mode_request.Address == MC_GetNetworkAddress()) &&
              (mode_request.SystemID == MC_GetSystemID()) )
         {
            //Addressed to this node
            //MC_MAC_ProcessPpuModeRequest(PPU_START_DISCONNECTED_MODE);
            CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Start PPU Msg\r\n");
            
            /* Send the output request up to the Application */
            appMessage.MessageType = APP_MSG_TYPE_PPU_MODE_E;
            memcpy(appMessage.MessageBuffer, &mode_request, sizeof(OtaMode_t));
            SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
         }
         else 
         {
            //remote node
      MC_MAC_SendAtMode(&mode_request);
   }
      }
      else if ( AT_FRAME_TYPE_ENTER_AT_MODE_E == mode_request.Frametype )
      {
         MC_MAC_SendAtMode(&mode_request);
      }
      
}

}

/*************************************************************************************/
/**
* generateAtModeMessage
* state handler for sending an AT mode message over the air
*
* @param - pointer to event data
*
* @return - void

*/
void generatePpuModeMessage(const uint8_t * const pData)
{
   OtaMode_t* pCommandData;
   ATModeRequest_t mode_request;
   
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
   pCommandData = (OtaMode_t*)pMessage->Payload.PhyDataReq.Data;
   
   if ( pCommandData )
   {
      mode_request.Frametype = pCommandData->FrameType;
      mode_request.Address = pCommandData->Destination;
      //CO_PRINT_B_2(DBG_ERROR_E, "generateAtModeMessage sending frame=%x node=%d\r\n", mode_request.Frametype, mode_request.Address);
      MC_MAC_SendAtMode(&mode_request);
   }
      
}


/*************************************************************************************/
/**
* ppuModeMesssage
* state handler for receiving the PPU MOde message
*
* @param - pointer to event data
*
* @return - void

*/
void ppuModeMesssage(const uint8_t * const pData)
{
   static ApplicationMessage_t appMessage;
   OtaMode_t ppuComand;
   bool address_match = false;
   uint8_t* pFrame;
   uint32_t* pAddress;
   uint32_t* pSystemID;
   
   CO_PRINT_B_0(DBG_INFO_E,"ppuModeMesssage\r\n");
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   
   pFrame = (uint8_t*)pMsg->Payload.PhyDataInd.Data;
   CO_PRINT_B_1(DBG_INFO_E,"FrameType %02X\r\n", *pFrame);
   if ( AT_FRAME_TYPE_ENTER_PPU_MODE_E == *pFrame )
   {
      ppuComand.FrameType = AT_FRAME_TYPE_ENTER_PPU_MODE_E;
      //Skip over the frame type
      pFrame++;
      pAddress = (uint32_t*)pFrame;
      ppuComand.Destination = *pAddress;
      //Read the System ID
      pAddress++;
      pSystemID = (uint32_t*)pAddress;
      ppuComand.SystemID = *pSystemID;
      CO_PRINT_B_2(DBG_INFO_E,"Destination=%08X, SystemID=%08X\r\n", ppuComand.Destination, ppuComand.SystemID );
      //Check the system ID
      if ( (ppuComand.SystemID == MC_GetSystemID()) || (SYSTEM_ID_GLOBAL == ppuComand.SystemID) )
      {
         //Check the address
         if( PPU_ADDRESS_GLOBAL == ppuComand.Destination)
         {
            //Global address
            address_match = true;
         }
         else if ( MAX_NUMBER_OF_SYSTEM_NODES <= ppuComand.Destination )
         {
            //the address is a serial number
            uint32_t serial_number;
            if ( SUCCESS_E == DM_NVMRead(NV_UNIT_SERIAL_NO_E, &serial_number, sizeof(uint32_t)) )
            {
               if ( ppuComand.Destination == serial_number )
               {
                  address_match = true;
               }
            }
         }
         else 
         {
            //the address is a node ID
            uint16_t network_address = MC_GetNetworkAddress();
            if ( ppuComand.Destination == network_address )
            {
               address_match = true;
            }
         }
      }
   }
      
   /* check if the message is for this node */
   if ( address_match )
   {
      CO_PRINT_B_0(DBG_INFO_E, "<<<<<<<<<< Start PPU Msg\r\n");
      
      /* Send the output request up to the Application */
      appMessage.MessageType = APP_MSG_TYPE_PPU_MODE_E;
      memcpy(appMessage.MessageBuffer, &ppuComand, sizeof(OtaMode_t));
      SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
   }
}

/*************************************************************************************/
/**
* ppuCommandMesssage
* state handler for receiving PPU commands
*
* @param - pointer to event data
*
* @return - void

*/
void ppuCommandMesssage(const uint8_t * const pData)
{
   static ApplicationMessage_t appMessage;
   static PPU_Message_t ppuComand;
   bool address_match = false;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   memset (ppuComand.Payload, 0, MAX_PPU_COMMAND_PAYLOAD_LENGTH);
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;
   
   ErrorCode_t status = MC_PUP_UnpackPpuMessage(&pMsg->Payload.PhyDataInd, &ppuComand);
   
   if ( SUCCESS_E == status )
   {
      CO_PRINT_B_3(DBG_INFO_E,"PPUCmd Source=%04X, Destination=%04X, SystemID=%08X\r\n", ppuComand.PpuAddress, ppuComand.RbuAddress, ppuComand.SystemID );
      //Check the system ID
      if ( (ppuComand.SystemID == MC_GetSystemID()) || (SYSTEM_ID_GLOBAL == ppuComand.SystemID) )
      {
         //Check the address
         if( PPU_ADDRESS_GLOBAL == ppuComand.RbuAddress)
         {
            //Global address
            address_match = true;
         }
         else if ( MAX_NUMBER_OF_SYSTEM_NODES <= ppuComand.RbuAddress )
         {
            //the address is a serial number
            uint32_t serial_number;
            if ( SUCCESS_E == DM_NVMRead(NV_UNIT_SERIAL_NO_E, &serial_number, sizeof(uint32_t)) )
            {
               if ( ppuComand.RbuAddress == serial_number )
               {
                  address_match = true;
               }
               else 
               {
                  CO_PRINT_B_0(DBG_INFO_E, "PPU Cmd ser num not matching\r\n");
               }
            }
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E, "PPU Command by node ID\r\n");
            //the address is a node ID
            uint16_t network_address = MC_GetNetworkAddress();
            if ( ppuComand.RbuAddress == network_address )
            {
               address_match = true;
            }
            else 
            {
               CO_PRINT_B_0(DBG_INFO_E, "PPU Cmd node address not matching\r\n");
            }
         }
      }
   }
   else 
   {
      CO_PRINT_B_1(DBG_INFO_E, "PPU Command failed to unpack.  Error=%d\r\n", status);
   }
      
   /* check if the message is for this node */
   if ( address_match )
   {
      CO_PRINT_B_1(DBG_INFO_E, "<<<<<<<<<< PPU Command 0x%04X\r\n", ppuComand.Command);
      if ( PPU_CMD_CONFIG == ppuComand.Command )
      {
         CO_PRINT_B_1(DBG_INFO_E, "PPU CFG Command %s\r\n", ppuComand.Payload);
      }
      
      /* Send the PPU Command up to the Application */
      memset(appMessage.MessageBuffer, 0, (PHY_DATA_IND_LENGTH_MAX-1));
      appMessage.MessageType = APP_MSG_TYPE_PPU_COMMAND_E;
      memcpy(appMessage.MessageBuffer, &ppuComand, sizeof(PPU_Message_t));
      SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
   }
}

/*************************************************************************************/
/**
* generatePpuResponse
* state handler for generating PPU response messages
*
* @param - pointer to event data
*
* @return - void

*/
void generatePpuResponse(const uint8_t * const pData)
{
   PPU_Message_t* pPpuMessage;
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status = SUCCESS_E;
   
   CO_PRINT_B_0(DBG_INFO_E,"generatePpuResponse+\r\n");
   
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   
   CO_Message_t* pMessage = (CO_Message_t*)pData;
   pPpuMessage = (PPU_Message_t*)pMessage->Payload.PhyDataReq.Data;
   
   /* pack into phy data req and put into RACH Q */
   pPhyDataReq = osPoolCAlloc(MeshPool);
   if (pPhyDataReq)
   {
      status = MC_PUP_PackPpuMessage(pPpuMessage, &pPhyDataReq->Payload.PhyDataReq);
      
      if( SUCCESS_E == status )
      {
         /* populate the Tx message */         
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = false;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         pPhyDataReq->Payload.PhyDataReq.Size = pPpuMessage->PacketLength;
         
         //Push the message into the RACHS queue
         status = MC_MACQ_Push(MAC_RACHS_Q_E, pPhyDataReq);
         if ( SUCCESS_E != status )
         {
            CO_PRINT_B_1(DBG_ERROR_E, "Failed to add PPU response to the send Queue - Error %d\r\n", status);
         }
         else 
         {
            //CO_PRINT_B_1(DBG_INFO_E, "generatePpuResponse sending response=%04X\r\n", pCommandData->Command);
            //Tell the MAC to send the message
            MC_MAC_SendPpuMessage();
         }
      }
      else 
      {
         CO_PRINT_B_1(DBG_ERROR_E, "Failed to pack PPU response - Error %d\r\n", status);
      }

      /* release the message memory */
      osPoolFree(MeshPool, pPhyDataReq);
   }
   else
   {
      CO_PRINT_A_0(DBG_ERROR_E,"generatePpuResponse - MeshPool failed to allocate\r\n");
   }
}

/*************************************************************************************/
/**
* SM_DuplicateMessageCallback
* Callback function called by the ACK Manager when a duplicate message overwites an older
* message in the send queue.
*
* @param DiscardedHandle - the transaction ID (appoined by the application) of the overwritten message.
* @param NewHandle       - the transaction ID (appoined by the application) of the replacement message.
* @param MsgType         - the message type of the duplicate message.
*
* @return - void

*/
void SM_DuplicateMessageCallback(const uint32_t DiscardedHandle, const uint32_t NewHandle, const ApplicationLayerMessageType_t MsgType)
{
   if ( 0 != DiscardedHandle )
   {
      switch( MsgType )
      {
         case APP_MSG_TYPE_FIRE_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_FIRE_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_ALARM_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_ALARM_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_OUTPUT_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_LOGON_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_LOGON_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_STATUS_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_STATUS_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_COMMAND_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_COMMAND_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_RESPONSE_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_RESPONSE_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_STATE_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_SET_STATE_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_RBU_DISABLE_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_RBU_DISABLE_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_ALARM_OUTPUT_STATE_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_BATTERY_STATUS_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_FAULT_SIGNAL_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_FAULT_SIGNAL_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_ZONE_ENABLE_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_ZONE_ENABLE_MESSAGE_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         case APP_MSG_TYPE_ROUTE_DROP_E:
            SM_SendConfirmationToApplication(DiscardedHandle, NewHandle, APP_CONF_ZONE_ENABLE_MESSAGE_E, ERR_MESSAGE_DUPLICATE_E);
            break;
         default:
            break;
      }
   }
}

/*************************************************************************************/
/**
* SM_SendEventToApplication
* Function to send network status information from the Mesh to the Application.
*
* @param MeshEventType  Event type, from enum CO_MessageType_t.
* @param pEventMessage  Pointer to the message structure
*
* @return - void
*/
void SM_SendEventToApplication( const CO_MessageType_t MeshEventType, const ApplicationMessage_t* pEventMessage )
{
   osStatus osStat = osErrorOS;
   CO_Message_t *pMessage = NULL;

   if ( pEventMessage )
   {
      /* create a status message and put into the App queue */
      pMessage = osPoolCAlloc(AppPool);
      if ( pMessage )
      {
         pMessage->Type = MeshEventType;
         memcpy(pMessage->Payload.PhyDataInd.Data, (uint8_t*)pEventMessage, sizeof(ApplicationMessage_t));
         osStat = osMessagePut(AppQ, (uint32_t)pMessage, 0);
         if (osOK != osStat)
         {
            /* failed to write */
            osPoolFree(AppPool, pMessage);
         }
      }
   }
}

/*************************************************************************************/
/**
* SM_SendConfirmationToApplication
* Function to send network status information from the Mesh to the Application.
*
* @param Handle               Transaction ID generated by the Application.
* @param ReplacementHandle    Replacement ID if a duplicate message was overwritten
* @param ConfirmationType     Message type being confirmed
* @param Error                Message success indicator for the Application
*
* @return - void
*/
void SM_SendConfirmationToApplication( const uint32_t Handle, const uint32_t ReplacementHandle, const AppConfirmationType_t ConfirmationType, const uint32_t Error )
{
   ApplicationMessage_t appMessage;
   appMessage.MessageType = CO_MESSAGE_CONFIRMATION_E;
   AppRequestConfirmation_t messageData;
   messageData.Handle = Handle;
   messageData.ReplacementHandle = ReplacementHandle;
   messageData.ConfirmationType = ConfirmationType;
   messageData.Error = Error;
   memcpy(appMessage.MessageBuffer, &messageData, sizeof(AppRequestConfirmation_t));
   SM_SendEventToApplication( CO_MESSAGE_CONFIRMATION_E, &appMessage );
}


/*************************************************************************************/
/**
* SM_TryToSendImmediately
* Function to send network status information from the Mesh to the Application.
*
* @param ackQueue             The queue in the Acknowledgement Mgr to read
* @param macQueue             The MAC Queue to write to
*
* @return - void
*/
void SM_TryToSendImmediately( const AcknowledgedQueueID_t ackQueue, const MacQueueType_t macQueue )
{
   ErrorCode_t status;
   //If there is an alarm signal ready to send, do it now so that we get 'next slot' response
   if ( MC_ACK_MessageReadyToSend(ackQueue) )
   {
      CO_Message_t *PMsg = ALLOCMESHPOOL;
      if (PMsg)
      {
         if ( true == MC_ACK_GetMessage(ackQueue, PMsg) )
         {
            CO_PRINT_B_2(DBG_INFO_E, "Fast sending message on RACH%d- type=%d\r\n", (ackQueue+1), PMsg->Type);
            status = MC_MACQ_Push(macQueue, PMsg);
            if ( SUCCESS_E != status )
            {
               CO_PRINT_B_2(DBG_ERROR_E, "Failed to add msg to RACH%d Queue - Error %d\r\n", (ackQueue+1), status);
            }
         }
         //The MAC Queue makes a copy so we can delete the original
         FREEMESHPOOL(PMsg);
      }
   }
}

/*************************************************************************************/
/**
* SM_TryToSendImmediately
* Function to process a PPU command from the application.
*
* @param pAppMessage    Structure containing the PPU command from the application
*
* @return - void
*/
void MC_SMGR_ProcessPpuMessage(ApplicationMessage_t* pAppMessage)
{
   uint32_t* pValue;
   uint32_t ppu_state;
   uint32_t command;
   if ( pAppMessage )
   {
      //Check that its a PPU message
      if ( APP_MSG_TYPE_PPU_MODE_E == pAppMessage->MessageType )
      {
         pValue = (uint32_t*)pAppMessage->MessageBuffer;
         ppu_state = *pValue;
         pValue++;
         command = *pValue;
         //Is it a PPU Disconnected mode message?
         if ( PPU_STATE_DISCONNECTED_E == ppu_state )
         {
            //Are we starting PPU Disconnected mode or is this an announcement trigger?
            if ( PPU_ANNOUNCEMENT_SEND == command )
            {
               //Send a PPU announcement message
               if ( MC_MAC_ProcessPpuModeRequest(command) )
               {
                  CO_PRINT_B_0(DBG_INFO_E,"PPU Announcement Sent\r\n");
               }
            }
            else if ( PPU_START_DISCONNECTED_MODE == command )
            {
               //Start PPU Diconnected Mode
               if ( MC_MAC_ProcessPpuModeRequest(command) )
               {
                  //The radio is set for PPU messages
                  //set the PPU mode in the config
                  MC_SetPpuMode(PPU_STATE_DISCONNECTED_E);
                  CO_PRINT_B_0(DBG_INFO_E,"PPU_STATE_DISCONNECTED_E 1\r\n");
               }
            }
         }
         else if ( PPU_STATE_CONNECTED_E == ppu_state )
         {
            if ( PPU_START_CONNECTED_MODE == command )
            {
               //Nothing to do for the 'PPU Connected' transition but store the state.
               MC_SetPpuMode(PPU_STATE_CONNECTED_E);
               CO_PRINT_B_0(DBG_INFO_E,"PPU_STATE_CONNECTED_E 2\r\n");
            }
            else if ( PPU_START_DISCONNECTED_MODE == command )
            {
               MC_SetPpuMode(PPU_STATE_DISCONNECTED_E);
               CO_PRINT_B_0(DBG_INFO_E,"PPU_STATE_DISCONNECTED_E 2\r\n");
            }
         }
         else if ( PPU_STATE_REQUESTED_E == ppu_state )
         {
            if ( PPU_SEND_ROUTE_DROP_E == command )
            {
               //Ask the session manager to send a ROUTE DROP signal
               MC_SMGR_SendRouteDrop(ADDRESS_GLOBAL, CO_RESET_REASON_PPU_MODE_REQUESTED_E, false, false, pAppMessage->Handle);
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* generateGlobalDelayMessage
* Function to send the global delays to a new child node.
*
* @param nodeId    The node ID of the child to send to.
*
* @return - bool     TRUE if the message was sent.
*/
bool generateGlobalDelayMessage(const uint16_t nodeId)
{
   CO_GlobalDelaysMessage_t globalDelayMessage;
   CO_Message_t *pPhyDataReq = NULL;
   ErrorCode_t status;
   bool result = false;
   
   if ( (MAX_DEVICES_PER_SYSTEM > nodeId) && (0 < nodeId) )
   {
      /*create a Global Delay message and send it to the specified node */
         /* pack into phy data req and put into RACHPQ */
      pPhyDataReq = ALLOCMESHPOOL;
      if (pPhyDataReq)
      {
         uint16_t delay1 = CFG_GetGlobalDelay1();
         uint16_t delay2 = CFG_GetGlobalDelay2();
         uint32_t delay_bits = CFG_GetLocalOrGlobalDelayBitmap();
         uint8_t delaybitmap = (uint8_t)(delay_bits & 0xff);
         
         /* populate delays signal */
         globalDelayMessage.Header.FrameType             = FRAME_TYPE_DATA_E;
         globalDelayMessage.Header.MACDestinationAddress = nodeId;
         globalDelayMessage.Header.MACSourceAddress      = gNetworkAddress;
         globalDelayMessage.Header.HopCount              = 0;
         globalDelayMessage.Header.DestinationAddress    = nodeId;
         globalDelayMessage.Header.SourceAddress         = gNetworkAddress;
         globalDelayMessage.Header.MessageType           = APP_MSG_TYPE_GLOBAL_DELAYS_E;
         globalDelayMessage.Delay1                       = delay1;
         globalDelayMessage.Delay2                       = delay2;
         globalDelayMessage.ChannelLocalOrGlobalDelay    = delaybitmap;
         globalDelayMessage.SystemId                     = SM_SystemId;
         
         pPhyDataReq->Type = CO_MESSAGE_PHY_DATA_REQ_E;
         pPhyDataReq->Payload.PhyDataReq.Downlink = true;
         pPhyDataReq->Payload.PhyDataReq.txPower = MC_SMGR_GetTxPower();
         status = MC_PUP_PackGlobalDelaysMessage(&globalDelayMessage, &pPhyDataReq->Payload.PhyDataReq);

         if (SUCCESS_E == status)
         {
               status = MC_MACQ_Push(MAC_DLCCHS_Q_E, pPhyDataReq);
               if ( SUCCESS_E == status )
               {
                  CO_PRINT_B_0(DBG_INFO_E, "Put Global Delays on DLCCHSQ Queue\r\n");
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E, "Failed to add Global Delays to DLCCHSQ Queue - Error %d\r\n", status);
               }
         }
         else 
         {
            CO_PRINT_B_1(DBG_INFO_E, "Global Delay message FAILED to pack. error=%d\r\n", status);
         }
         /* The Ack Manager and SACH Manager make a copy of the message so we can release the memory */
         FREEMESHPOOL(pPhyDataReq);
      }
      else
      {
         CO_PRINT_A_0(DBG_ERROR_E,"generateGlobalDelay - MeshPool failed to allocate\r\n");
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* globalDelayMessage
* state handler for the global delays message from the parent.
*
* @param - pointer to event data
*
* @return - void

*/
void globalDelayMessage(const uint8_t * const pData)
{
   CO_GlobalDelaysMessage_t globalDelayMessage = { 0 };
   CO_GlobalDelaysData_t globalDelaysData = {0};
   ErrorCode_t status = SUCCESS_E;
   
   /* check input data */
   if (NULL == pData)
   {
      return;
   }
   
   
   CO_Message_t *pMsg = (CO_Message_t *) pData;

   status = MC_PUP_UnpackGlobalDelaysMessage(&pMsg->Payload.PhyDataInd, &globalDelayMessage);
   
   if (SUCCESS_E == status)
   {
      CO_PRINT_B_1(DBG_INFO_E, "Received Global Delays msg from node %d\r\n", globalDelayMessage.Header.MACSourceAddress );
      
      if ( ((globalDelayMessage.Header.MACDestinationAddress == SM_address) || (ADDRESS_GLOBAL == globalDelayMessage.Header.MACDestinationAddress) ) 
            && (globalDelayMessage.SystemId == SM_SystemId) )
      {
         /* Send the new delay values to the Application*/
         globalDelaysData.Delay1 = globalDelayMessage.Delay1;
         globalDelaysData.Delay2 = globalDelayMessage.Delay2;
         globalDelaysData.ChannelLocalOrGlobalDelay = globalDelayMessage.ChannelLocalOrGlobalDelay;

         ApplicationMessage_t appMessage;
         appMessage.MessageType = APP_MSG_TYPE_GLOBAL_DELAYS_E;
         memcpy(appMessage.MessageBuffer, &globalDelaysData, sizeof(CO_GlobalDelaysData_t));
         SM_SendEventToApplication( CO_MESSAGE_PHY_DATA_IND_E, &appMessage );
      }
      else 
      {
         CO_PRINT_B_2(DBG_INFO_E, "globalDelayMessage FAIL.  dest=%d, sysid=%d\r\n",globalDelayMessage.Header.MACDestinationAddress, globalDelayMessage.SystemId );
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E, "FAILED to unpack Global Delays msg\r\n");
   }
   MC_SMGR_UpdateNetworkStatistics(SECONDARY_RACH, true);
}

