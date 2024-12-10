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
*  File         : MM_CIEQueueManager.c
*
*  Description  : NCU ONLY.  Manages the four queues for CIE mesages, Fire, Alarm, Fault, Misc.
*
*************************************************************************************/



/* System Include Files
*************************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_Defines.h"
#include "CO_Message.h"
#include "MM_CIEQueueManager.h"


#define MESSAGE_ACCESS_MAX_COUNT 10

/* Private Functions Prototypes
*************************************************************************************/

/* Private Types
*************************************************************************************/

MessageStatusRegister_t CieMessageStatusRegister;

/* Global Variables
*************************************************************************************/
const char ParamCommandMap[PARAM_TYPE_MAX_E][6] = 
{
   [PARAM_TYPE_ANALOGUE_VALUE_E]             = {"ANA"},//0
   [PARAM_TYPE_NEIGHBOUR_INFO_E]             = {"DNI"},//1
   [PARAM_TYPE_STATUS_FLAGS_E]               = {"DSF"},//2
   [PARAM_TYPE_DEVICE_COMBINATION_E]         = {"DEC"},//3
   [PARAM_TYPE_ALARM_THRESHOLD_E]            = {"THU"},//4
   [PARAM_TYPE_PRE_ALARM_THRESHOLD_E]        = {"THL"},//5
   [PARAM_TYPE_FAULT_THRESHOLD_E]            = {"FTH"},//6 not in CIE protocol
   [PARAM_TYPE_FLASH_RATE_E]                 = {"BFR"},//7
   [PARAM_TYPE_TONE_SELECTION_E]             = {"STS"},//8
   [PARAM_TYPE_RBU_SERIAL_NUMBER_E]          = {"SNR"},//9
   [PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E]       = {"SNP"},//10
   [PARAM_TYPE_RBU_ENABLE_E]                 = {"EDR"},//11
   [PARAM_TYPE_PLUGIN_ENABLE_E]              = {"EDP"},//12
   [PARAM_TYPE_MODULATION_BANDWIDTH_E]       = {"MBW"},//13 not in CIE protocol
   [PARAM_TYPE_SPREADING_FACTOR_E]           = {"SPF"},//14 not in CIE protocol
   [PARAM_TYPE_FREQUENCY_E]                  = {"FRQ"},//15 not in CIE protocol
   [PARAM_TYPE_CODING_RATE_E]                = {"COD"},//16 not in CIE protocol
   [PARAM_TYPE_TX_POWER_E]                   = {"TXP"},//17 not in CIE protocol
   [PARAM_TYPE_TEST_MODE_E]                  = {"TMR"},//18
   [PARAM_TYPE_PLUGIN_TEST_MODE_E]           = {"TMP"},//19
   [PARAM_TYPE_FIRMWARE_INFO_E]              = {"FIR"},//20
   [PARAM_TYPE_FIRMWARE_ACTIVE_IMAGE_E]      = {"FIA"},//21 not in CIE protocol
   [PARAM_TYPE_REBOOT_E]                     = {"RRU"},//22
   [PARAM_TYPE_SVI_E]                        = {"SVI"},//23 not in CIE protocol
   [PARAM_TYPE_RBU_DISABLE_E]                = {"DDR"},//24 not in CIE protocol
   [PARAM_TYPE_TX_POWER_LOW_E]               = {"TXL"},//25 not in CIE protocol
   [PARAM_TYPE_TX_POWER_HIGH_E]              = {"TXH"},//26 not in CIE protocol
   [PARAM_TYPE_OUTPUT_STATE_E]               = {"TXO"},//27 not in CIE protocol
   [PARAM_TYPE_ZONE_E]                       = {"ZON"},//28 not in CIE protocol
   [PARAM_TYPE_CRC_INFORMATION_E]            = {"CRC"},//29 not in CIE protocol
   [PARAM_TYPE_PIR_RESET_E]                  = {"PIR"},//30
   [PARAM_TYPE_EEPROM_INFORMATION_E]         = {"EEI"},//31
   [PARAM_TYPE_FAULT_TYPE_E]                 = {"FLT"},//32
   [PARAM_TYPE_MESH_STATUS_E]                = {"MSR"},//33
   [PARAM_TYPE_INDICATOR_LED_E]              = {"LED"},//34
   [PARAM_TYPE_PLUGIN_ID_NUMBER_E]           = {"PID"},//35 not in CIE protocol
   [PARAM_TYPE_PLUGIN_FIRMWARE_INFO_E]       = {"FI"}, //36 not in CIE protocol
   [PARAM_TYPE_DAY_NIGHT_SETTING_E]          = {"SDN"},//37
   [PARAM_TYPE_ALARM_ACK_E]                  = {"ACK"},//38
   [PARAM_TYPE_EVACUATE_E]                   = {"EVAC"},//39
   [PARAM_TYPE_GLOBAL_DELAY_E]               = {"AGD"},//40
   [PARAM_TYPE_ENABLE_DISABLE_E]             = {"DISD"},//41
   [PARAM_TYPE_ALARM_RESET_E]                = {"RST"},//42
   [PARAM_TYPE_SOUND_LEVEL_E]                = {"SL"}, //43
   [PARAM_TYPE_TEST_ONE_SHOT_E]              = {"TOS"},//44
   [PARAM_TYPE_CONFIRMED_E]                  = {"CONF"},//45
   [PARAM_TYPE_ALARM_CONFIG_E]               = {"ACS"}, //46
   [PARAM_TYPE_ALARM_OPTION_FLAGS_E]         = {"AOF"}, //47
   [PARAM_TYPE_ALARM_DELAYS_E]               = {"ADC"}, //48
   [PARAM_TYPE_CHANNEL_FLAGS_E]              = {"CHF"}, //49
   [PARAM_TYPE_MAX_RANK_E]                   = {"RNK"}, //50
   [PARAM_TYPE_200_HOUR_TEST_E]              = {"T200"},//51
   [PARAM_TYPE_CHECK_FIRMWARE_E]             = {"CHFW"}, //52
   [PARAM_TYPE_SET_LOCAL_GLOBAL_DELAYS_E]    = {"LDLY"}, //53
   [PARAM_TYPE_GLOBAL_ALARM_DELAY_VALUES_E]  = {"GADC"}, //54
   [PARAM_TYPE_GLOBAL_ALARM_OVERRIDE_E]      = {"GDLY"}, //55
   [PARAM_TYPE_BATTERY_STATUS_REQUEST_E]     = {"BATS"}, //56
   [PARAM_TYPE_GLOBAL_DELAY_COMBINED_E]      = {"GSET"}, //57
   [PARAM_TYPE_PLUGIN_DETECTOR_TYPE_E]       = {"PTYPE"},//58
   [PARAM_TYPE_PLUGIN_TYPE_AND_CLASS_E]      = {"PTYPE"},//59
   [PARAM_TYPE_SCAN_FOR_DEVICES_E]           = {"SCAN"}, //60
   [PARAM_TYPE_SENSOR_VALUES_REQUEST_E]      = {"SENV"}, //61
   [PARAM_TYPE_DELAYED_OUTPUT_E]             = {"DLOUT"},//62
   [PARAM_TYPE_BATTERY_TEST_E]               = {"BTST"}, //63
   [PARAM_TYPE_PRODUCT_CODE_E]               = {"SERPX"}, //64
   [PARAM_TYPE_PPU_MODE_ENABLE_E]            = {"PPEN"}, //65
   [PARAM_TYPE_DEPASSIVATION_SETTINGS_E]     = {"DPASD"}, //66
   [PARAM_TYPE_ENTER_PPU_MODE_E]             = {"PPUMD"}  //67
};

/* Private Variables
*************************************************************************************/
#define MAX_FAILED_TXBUFFER_MESSAGES 3
#define BUFFER_EMPTY 0xFF
#define TX_BUFFER_TIMEOUT 20 /* short frames since message was sent */

osPoolDef(CieMsgPool, CIE_POOL_SIZE, CieBuffer_t);
osPoolId CieMsgPool;

osMessageQDef(CieFireQ, CIE_Q_SIZE, &CieBuffer_t);
osMessageQId(CieFireQ);

osMessageQDef(CieAlarmQ, CIE_Q_SIZE, &CieBuffer_t);
osMessageQId(CieAlarmQ);

osMessageQDef(CieFaultQ, CIE_Q_SIZE, &CieBuffer_t);
osMessageQId(CieFaultQ);

osMessageQDef(CieMiscQ, CIE_MISC_Q_SIZE, &CieBuffer_t);
osMessageQId(CieMiscQ);

static CieBuffer_t current_fire_message = {0};
static CieBuffer_t current_alarm_message = {0};
static CieBuffer_t current_fault_message = {0};
static CieBuffer_t current_misc_message = {0};
static bool tx_buffer_waiting_for_response = false;
static uint32_t tx_buffer_waiting_for_response_short_frame_count = 0;
static bool tx_output_signal_waiting_for_confirm = false;
static uint32_t tx_output_signal_waiting_for_confirm_short_frame_count = 0;
static uint8_t access_count_fire = 0;
static uint8_t access_count_first_aid = 0;
static uint8_t access_count_fault = 0;
static uint8_t access_count_misc = 0;


CO_QueueId_t current_output_signal_queue = CO_Q_MAX_QUEUES_E;

/* Public Functions
*************************************************************************************/

/*************************************************************************************/
/**
* MM_CIEQ_Initialise
* Function to clear the CIE queues.
*
* @param - None.
*
* @return - Error code or SUCCESS_E.
*/
ErrorCode_t MM_CIEQ_Initialise(void)
{
   CieMessageStatusRegister.CieAlarmQCount = 0;
   CieMessageStatusRegister.CieAlarmQLostMessageCount = 0;
   CieMessageStatusRegister.CieFaultQCount = 0;
   CieMessageStatusRegister.CieFaultQLostMessageCount = 0;
   CieMessageStatusRegister.CieFireQCount = 0;
   CieMessageStatusRegister.CieFireQLostMessageCount = 0;
   CieMessageStatusRegister.CieMiscQCount = 0;
   CieMessageStatusRegister.CieMiscQLostMessageCount = 0;
   CieMessageStatusRegister.CieTxBufferQLostMessageCount = 0;
   CieMessageStatusRegister.CieOutputSignalQLostMessageCount = 0;
   
   current_output_signal_queue = CO_Q_MAX_QUEUES_E;
   
   CieMsgPool = osPoolCreate(osPool(CieMsgPool));
   CO_ASSERT_RET_MSG(NULL != CieMsgPool, ERR_FAILED_TO_CREATE_MEMPOOL_E, "ERROR - Failed to create AT Pool");
   CieFireQ = osMessageCreate(osMessageQ(CieFireQ), NULL);
   CO_ASSERT_RET_MSG(NULL != CieFireQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create CIE Fire Queue");
   CieAlarmQ = osMessageCreate(osMessageQ(CieAlarmQ), NULL);
   CO_ASSERT_RET_MSG(NULL != CieAlarmQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create CIE Alarm Queue");
   CieFaultQ = osMessageCreate(osMessageQ(CieFaultQ), NULL);
   CO_ASSERT_RET_MSG(NULL != CieFaultQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create CIE Fault Queue");
   CieMiscQ = osMessageCreate(osMessageQ(CieMiscQ), NULL);
   CO_ASSERT_RET_MSG(NULL != CieMiscQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create CIE Misc Queue");
   
   MM_CIEQ_ResetMessageQueue(CIE_Q_FIRE_E);
   MM_CIEQ_ResetMessageQueue(CIE_Q_ALARM_E);
   MM_CIEQ_ResetMessageQueue(CIE_Q_FAULT_E);
   MM_CIEQ_ResetMessageQueue(CIE_Q_MISC_E);
   //Initialise the output downlink queues
   CO_QueueInitialise();
   
return SUCCESS_E;
}


/*************************************************************************************/
/**
* MM_CIEQ_Push
* Function to add a new message to one of the CIE queues.
*
* @param - queue     The ID of the queue to add to.
* @param - pMessage  The message to be added
*
* @return - error code,
*/
ErrorCode_t MM_CIEQ_Push(const CIEQueueType_t queue, const CieBuffer_t* pMessage)
{
   ErrorCode_t error = ERR_INVALID_POINTER_E;
   osStatus osStat = osErrorOS;
   
   if ( pMessage )
   {
         
      switch (queue)
      {
         case CIE_Q_FIRE_E:
         {
            CieBuffer_t* pMsg = osPoolAlloc(CieMsgPool);
            if ( pMsg )
            {
               memcpy(pMsg, pMessage, sizeof(CieBuffer_t));
               osStat = osMessagePut(CieFireQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(CieMsgPool, pMsg);
                  CieMessageStatusRegister.CieFireQLostMessageCount++;
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  CieMessageStatusRegister.CieFireQCount++;
                  error = SUCCESS_E;
               }
            }
            else 
            {
               error = ERR_NO_MEMORY_E;
            }
         }
            break;
         case CIE_Q_ALARM_E:
         {
            CieBuffer_t* pMsg = osPoolAlloc(CieMsgPool);
            if ( pMsg )
            {
               memcpy(pMsg, pMessage, sizeof(CieBuffer_t));
               osStat = osMessagePut(CieAlarmQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(CieMsgPool, pMsg);
                  CieMessageStatusRegister.CieAlarmQLostMessageCount++;
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  CieMessageStatusRegister.CieAlarmQCount++;
                  error = SUCCESS_E;
               }
            }
            else 
            {
               error = ERR_NO_MEMORY_E;
            }
         }
            break;
         case CIE_Q_FAULT_E:
         {
            CieBuffer_t* pMsg = osPoolAlloc(CieMsgPool);
            if ( pMsg )
            {
               memcpy(pMsg, pMessage, sizeof(CieBuffer_t));
               osStat = osMessagePut(CieFaultQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(CieMsgPool, pMsg);
                  CieMessageStatusRegister.CieFaultQLostMessageCount++;
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  CieMessageStatusRegister.CieFaultQCount++;
                  error = SUCCESS_E;
               }
            }
            else 
            {
               error = ERR_NO_MEMORY_E;
            }
         }
            break;
         case CIE_Q_MISC_E:
         {
            CieBuffer_t* pMsg = osPoolAlloc(CieMsgPool);
            if ( pMsg )
            {
               memcpy(pMsg, pMessage, sizeof(CieBuffer_t));
               osStat = osMessagePut(CieMiscQ, (uint32_t)pMsg, 0);
               if (osOK != osStat)
               {
                  /* failed to write */
                  osPoolFree(CieMsgPool, pMsg);
                  CieMessageStatusRegister.CieMiscQLostMessageCount++;
                  error = ERR_QUEUE_OVERFLOW_E;
               }
               else
               {
                  CieMessageStatusRegister.CieMiscQCount++;
                  error = SUCCESS_E;
               }
            }
            else 
            {
               error = ERR_NO_MEMORY_E;
            }
         }
            break;
         case CIE_Q_TX_BUFFER_E:
         {
            error = CO_QueuePush(CO_Q_LOW_PRIORITY_E, pMessage, true);
            if( SUCCESS_E != error )
            {
               CieMessageStatusRegister.CieTxBufferQLostMessageCount++;
            }
         }
            break;
         case CIE_Q_OUTPUT_SIGNAL_E:
         {
            ErrorCode_t error1;
            ErrorCode_t error2;
            ErrorCode_t error3;
            error1 = CO_QueuePush(CO_Q_PRIORITY_1_E, pMessage, true);
            error2 = CO_QueuePush(CO_Q_PRIORITY_2_E, pMessage, true);
            error3 = CO_QueuePush(CO_Q_PRIORITY_3_E, pMessage, true);
            
            error = SUCCESS_E;
            if ( SUCCESS_E != error1 )
            {
               error = error1;
            }
            else if ( SUCCESS_E != error2 )
            {
               error = error2;
            }
            else if ( SUCCESS_E != error3 )
            {
               error = error3;
            }
             
            if( SUCCESS_E != error )
            {
               CieMessageStatusRegister.CieOutputSignalQLostMessageCount++;
            }
         }
            break;
         case CIE_Q_OUTPUT_SIGNAL_PRIORITY_E:
         {
            error = CO_QueuePush(CO_Q_PRIORITY_1_E, pMessage, true);
            
            if( SUCCESS_E != error )
            {
               CieMessageStatusRegister.CieOutputSignalQLostMessageCount++;
            }
         }
            break;
         default:
            error = ERR_OUT_OF_RANGE_E;
            break;
      }
   }
   return error;
}

/*************************************************************************************/
/**
* MM_CIEQ_Pop
* Function to get the first message from one of the CIE queues.
*
* @param - queue           The ID of the queue to add to.
* @param - pMessage [OUT]  Pointer to buffer that accepts the popped message.
*
* @return - void
*/
ErrorCode_t MM_CIEQ_Pop(const CIEQueueType_t queue, CieBuffer_t* pMessage)
{
   osEvent event;
   CieBuffer_t *pMsg = NULL;
   ErrorCode_t error = ERR_INVALID_POINTER_E;
   
   if ( pMessage )
   {
      switch (queue)
      {
         case CIE_Q_FIRE_E:
         {
            if ( BUFFER_EMPTY == current_fire_message.Buffer[0] )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(CieFireQ, 1);
               if (osEventMessage == event.status)
               {
                  pMsg = (CieBuffer_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_fire_message;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     osPoolFree(CieMsgPool, pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_fire_message, sizeof(CieBuffer_t));
               error = SUCCESS_E;
               access_count_fire++;
            }
            
            if ( MESSAGE_ACCESS_MAX_COUNT <= access_count_fire )
            {
               MM_CIEQ_Discard(CIE_Q_FIRE_E);
               current_fire_message.Buffer[0] = BUFFER_EMPTY;
               error = ERR_NOT_FOUND_E;
            }
            break;
         }
         case CIE_Q_ALARM_E:
         {
            if ( BUFFER_EMPTY == current_alarm_message.Buffer[0] )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(CieAlarmQ, 1);
               if (osEventMessage == event.status)
               {
                  pMsg = (CieBuffer_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_alarm_message;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     osPoolFree(CieMsgPool, pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_alarm_message, sizeof(CieBuffer_t));
               error = SUCCESS_E;
               access_count_first_aid++;
            }
            
            if ( MESSAGE_ACCESS_MAX_COUNT <= access_count_first_aid )
            {
               MM_CIEQ_Discard(CIE_Q_ALARM_E);
               current_alarm_message.Buffer[0] = BUFFER_EMPTY;
               error = ERR_NOT_FOUND_E;
            }
            break;
         }
         case CIE_Q_FAULT_E:
         {
            if ( BUFFER_EMPTY == current_fault_message.Buffer[0] )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(CieFaultQ, 1);
               if (osEventMessage == event.status)
               {
                  pMsg = (CieBuffer_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_fault_message;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     osPoolFree(CieMsgPool, pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_fault_message, sizeof(CieBuffer_t));
               error = SUCCESS_E;
               access_count_fault++;
            }
            
            if ( MESSAGE_ACCESS_MAX_COUNT <= access_count_fault )
            {
               MM_CIEQ_Discard(CIE_Q_FAULT_E);
               current_fault_message.Buffer[0] = BUFFER_EMPTY;
               error = ERR_NOT_FOUND_E;
            }
            break;
         }
         case CIE_Q_MISC_E:
         {
            if ( BUFFER_EMPTY == current_misc_message.Buffer[0] )
            {
               /* the current message is empty so pop one out of the queue */  
               error = ERR_NOT_FOUND_E;/*assume failure*/
               event = osMessageGet(CieMiscQ, 1);
               if (osEventMessage == event.status)
               {
                  pMsg = (CieBuffer_t *)event.value.p;
                  if (pMsg)
                  {
                     uint8_t* pOutMsg = (uint8_t*)&current_misc_message;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     pOutMsg = (uint8_t*)pMessage;
                     memcpy(pOutMsg, pMsg, sizeof(CieBuffer_t));
                     osPoolFree(CieMsgPool, pMsg);
                     error = SUCCESS_E;
                  }
               }
            }
            else 
            {
               /* the current message is populated so use that */
               uint8_t* pOutMsg = (uint8_t*)pMessage;
               memcpy(pOutMsg, &current_misc_message, sizeof(CieBuffer_t));
               error = SUCCESS_E;
               access_count_misc++;
            }
            
            if ( MESSAGE_ACCESS_MAX_COUNT <= access_count_misc )
            {
               MM_CIEQ_Discard(CIE_Q_MISC_E);
               current_misc_message.Buffer[0] = BUFFER_EMPTY;
               error = ERR_NOT_FOUND_E;
            }
            break;
         }
         case CIE_Q_TX_BUFFER_E:
         {
            error = CO_QueuePeek( CO_Q_LOW_PRIORITY_E, pMessage);
            
            if ( SUCCESS_E != error )
            {
               tx_buffer_waiting_for_response = false;
               tx_buffer_waiting_for_response_short_frame_count = 0;
            }
         }
            break;
         case CIE_Q_OUTPUT_SIGNAL_E:
         {
            if ( CO_Q_MAX_QUEUES_E == current_output_signal_queue )
            {
               //Read the front message in the priority 1 queue
               current_output_signal_queue = CO_Q_PRIORITY_1_E;
               error = CO_QueuePeek( CO_Q_PRIORITY_1_E, pMessage);
               
               if ( SUCCESS_E != error )
               {
                  //No message from the priority 1 queue, make sure it's unlocked and try priority 2
                  CO_QueueSetLock(CO_Q_PRIORITY_1_E, CO_Q_LOCK_UNLOCKED_E);
                  current_output_signal_queue = CO_Q_PRIORITY_2_E;
                  error = CO_QueuePeek( CO_Q_PRIORITY_2_E, pMessage);
                  
                  if ( SUCCESS_E != error )
                  {
                     //No message from the priority 2 queue, make sure it's unlocked and try priority 3
                     CO_QueueSetLock(CO_Q_PRIORITY_2_E, CO_Q_LOCK_UNLOCKED_E);
                     current_output_signal_queue = CO_Q_PRIORITY_3_E;
                     error = CO_QueuePeek( CO_Q_PRIORITY_3_E, pMessage);
                     
                     if ( SUCCESS_E != error )
                     {
                        //No message from the priority 3 queue, make sure it's unlocked.
                        CO_QueueSetLock(CO_Q_PRIORITY_3_E, CO_Q_LOCK_UNLOCKED_E);
                     }
                  }
               }
            }
            else 
            {
               //Read the front message from current_output_signal_queue
               error = CO_QueuePeek( current_output_signal_queue, pMessage);
            }
            
            if ( SUCCESS_E == error )
            {
               CO_QueueSetLock(current_output_signal_queue, CO_Q_LOCK_FIRST_ENTRY_E);
            }
            else 
            {
               current_output_signal_queue = CO_Q_MAX_QUEUES_E;
               tx_output_signal_waiting_for_confirm = false;
            }
         }
            break;
         case CIE_Q_OUTPUT_SIGNAL_PRIORITY_E:
         {
            if ( CO_Q_MAX_QUEUES_E == current_output_signal_queue )
            {
               //Read the front message in the priority 1 queue
               current_output_signal_queue = CO_Q_PRIORITY_1_E;
            }
            
            if ( CO_Q_PRIORITY_1_E == current_output_signal_queue )
            {
               error = CO_QueuePeek( CO_Q_PRIORITY_1_E, pMessage);
               
               if ( SUCCESS_E == error )
               {
                  CO_QueueSetLock(current_output_signal_queue, CO_Q_LOCK_FIRST_ENTRY_E);
                  current_output_signal_queue = CO_Q_PRIORITY_1_E;
               }
               else 
               {
                  current_output_signal_queue = CO_Q_MAX_QUEUES_E;
                  tx_output_signal_waiting_for_confirm = false;
               }
            }
         }
            break;
         default:
            error = ERR_OUT_OF_RANGE_E;
            break;
      }
   }
   return error;
}

/*************************************************************************************/
/**
* MM_CIEQ_Discard
* Function to remove the first message from one of the CIE queues.
*
* @param - queue           The ID of the queue to remove from.
*
* @return - void
*/
ErrorCode_t MM_CIEQ_Discard(const CIEQueueType_t queue)
{
   osEvent event;
   ErrorCode_t error = ERR_NOT_FOUND_E;
   
   switch (queue)
   {
      case CIE_Q_FIRE_E:
      {
         access_count_fire = 0;
         if ( BUFFER_EMPTY != current_fire_message.Buffer[0] )
         {
            current_fire_message.Buffer[0] = BUFFER_EMPTY;
            CieMessageStatusRegister.CieFireQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(CieFireQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t *pMessage = (CieBuffer_t*)event.value.p;
               if(pMessage != NULL)
               {
                  osPoolFree(CieMsgPool, pMessage);
                  CieMessageStatusRegister.CieFireQCount--;
               }
               error = SUCCESS_E;
            }                  
         }
         break;
      }
      case CIE_Q_ALARM_E:
      {
         access_count_first_aid = 0;
         if ( BUFFER_EMPTY != current_alarm_message.Buffer[0] )
         {
            current_alarm_message.Buffer[0] = BUFFER_EMPTY;
            CieMessageStatusRegister.CieAlarmQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(CieAlarmQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t *pMessage = (CieBuffer_t*)event.value.p;
               if(pMessage != NULL)
               {
                  osPoolFree(CieMsgPool, pMessage);
                  CieMessageStatusRegister.CieAlarmQCount--;
               }
               error = SUCCESS_E;
            }                  
         }
         break;
      }
      case CIE_Q_FAULT_E:
      {
         access_count_fault = 0;
         if ( BUFFER_EMPTY != current_fault_message.Buffer[0] )
         {
            current_fault_message.Buffer[0] = BUFFER_EMPTY;
            CieMessageStatusRegister.CieFaultQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(CieFaultQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t *pMessage = (CieBuffer_t*)event.value.p;
               if(pMessage != NULL)
               {
                  osPoolFree(CieMsgPool, pMessage);
                  CieMessageStatusRegister.CieFaultQCount--;
               }
               error = SUCCESS_E;
            }                  
         }
         break;
      }
      case CIE_Q_MISC_E:
      {
         access_count_misc = 0;
         if ( BUFFER_EMPTY != current_misc_message.Buffer[0] )
         {
            current_misc_message.Buffer[0] = BUFFER_EMPTY;
            CieMessageStatusRegister.CieMiscQCount--;
            error = SUCCESS_E;
         }
         else 
         {
            /* the current message is empty so pop one out of the queue */  
            error = ERR_NOT_FOUND_E;/*assume failure*/
            event = osMessageGet(CieMiscQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t *pMessage = (CieBuffer_t*)event.value.p;
               if(pMessage != NULL)
               {
                  osPoolFree(CieMsgPool, pMessage);
                  CieMessageStatusRegister.CieMiscQCount--;
               }
               error = SUCCESS_E;
            }                  
         }
         break;
      }
      case CIE_Q_TX_BUFFER_E:
      {
         error = CO_QueueDiscard( CO_Q_LOW_PRIORITY_E );
         tx_buffer_waiting_for_response = false;
         tx_buffer_waiting_for_response_short_frame_count = 0;
         
         break;
      }
      case CIE_Q_OUTPUT_SIGNAL_E:
      {
         if ( CO_Q_MAX_QUEUES_E != current_output_signal_queue )
         {
            CO_QueueSetLock(current_output_signal_queue, CO_Q_LOCK_UNLOCKED_E);
            error = CO_QueueDiscard( current_output_signal_queue );
            tx_output_signal_waiting_for_confirm = false;
            tx_output_signal_waiting_for_confirm_short_frame_count = 0;
         }
         else 
         {
            //No queues expecting a response
            error = ERR_NOT_FOUND_E;
         }
         
         break;
      }
      case CIE_Q_OUTPUT_SIGNAL_PRIORITY_E:
      {
         if ( CO_Q_PRIORITY_1_E == current_output_signal_queue )
         {
            CO_QueueSetLock(current_output_signal_queue, CO_Q_LOCK_UNLOCKED_E);
            error = CO_QueueDiscard( current_output_signal_queue );
            tx_output_signal_waiting_for_confirm = false;
            tx_output_signal_waiting_for_confirm_short_frame_count = 0;
         }
         else 
         {
            //No queues expecting a response
            error = ERR_NOT_FOUND_E;
         }
         
         break;
      }
      default:
         error = ERR_OUT_OF_RANGE_E;
         break;
   }
   return error;
}

/*************************************************************************************/
/**
* MM_CIEQ_MessageCount
* Function to add a new message to one of the CIE queues.
*
* @param - queue           The ID of the queue.
*
* @return - The message count or 0 if the queue type is unknown.
*/
uint32_t MM_CIEQ_MessageCount(const CIEQueueType_t queue)
{
   uint32_t msg_count = 0;
   switch (queue)
   {
      case CIE_Q_FIRE_E:
         msg_count = CieMessageStatusRegister.CieFireQCount;
         break;
      case CIE_Q_ALARM_E:
         msg_count = CieMessageStatusRegister.CieAlarmQCount;
         break;
      case CIE_Q_FAULT_E:
         msg_count = CieMessageStatusRegister.CieFaultQCount;
         break;
      case CIE_Q_MISC_E:
         msg_count = CieMessageStatusRegister.CieMiscQCount;
         break;
      case CIE_Q_TX_BUFFER_E:
           msg_count = CO_QueueMessageCount(CO_Q_LOW_PRIORITY_E);
         break;
      case CIE_Q_OUTPUT_SIGNAL_E:            //Intentional drop-through
      case CIE_Q_OUTPUT_SIGNAL_PRIORITY_E:
         {
            uint32_t count1 = CO_QueueMessageCount(CO_Q_PRIORITY_1_E);
            uint32_t count2 = CO_QueueMessageCount(CO_Q_PRIORITY_2_E);
            uint32_t count3 = CO_QueueMessageCount(CO_Q_PRIORITY_3_E);
            msg_count = count1 + count2 + count3;
         }
         break;
      default:
         break;
   }
   
   return msg_count;
}

/*************************************************************************************/
/**
* MM_CIEQ_LostMessageCount
* Function to return the lost message count of one of the CIE queues.
*
* @param - queue           The ID of the queue.
*
* @return - The lost message count or 0 if the queue type is unknown.
*/
uint32_t MM_CIEQ_LostMessageCount(const CIEQueueType_t queue)
{
   uint32_t msg_count = 0;
   switch (queue)
   {
      case CIE_Q_FIRE_E:
         msg_count = CieMessageStatusRegister.CieFireQLostMessageCount;
         break;
      case CIE_Q_ALARM_E:
         msg_count = CieMessageStatusRegister.CieAlarmQLostMessageCount;
         break;
      case CIE_Q_FAULT_E:
         msg_count = CieMessageStatusRegister.CieFaultQLostMessageCount;
         break;
      case CIE_Q_MISC_E:
         msg_count = CieMessageStatusRegister.CieMiscQLostMessageCount;
         break;
      case CIE_Q_TX_BUFFER_E:
         msg_count = CieMessageStatusRegister.CieTxBufferQLostMessageCount;
         break;
      case CIE_Q_OUTPUT_SIGNAL_E:            //Intentional drop-through
      case CIE_Q_OUTPUT_SIGNAL_PRIORITY_E:
         msg_count = CieMessageStatusRegister.CieOutputSignalQLostMessageCount;
         break;
      default:
         break;
   }
   
   return msg_count;
}

/*************************************************************************************/
/**
* MM_CIEQ_ResetLostMessageCount
* Function to reset the lost message count of one of the CIE queues.
*
* @param - queue  The ID of the queue.
*
* @return - SUCCESS_E if the count was reset, ERR_NOT_FOUND_E otherwise.
*/
ErrorCode_t MM_CIEQ_ResetLostMessageCount(const CIEQueueType_t queue)
{
   ErrorCode_t result = SUCCESS_E;
   switch (queue)
   {
      case CIE_Q_FIRE_E:
         CieMessageStatusRegister.CieFireQLostMessageCount = 0;
         break;
      case CIE_Q_ALARM_E:
         CieMessageStatusRegister.CieAlarmQLostMessageCount = 0;
         break;
      case CIE_Q_FAULT_E:
         CieMessageStatusRegister.CieFaultQLostMessageCount = 0;
         break;
      case CIE_Q_MISC_E:
         CieMessageStatusRegister.CieMiscQLostMessageCount = 0;
         break;
      case CIE_Q_TX_BUFFER_E:
         CieMessageStatusRegister.CieTxBufferQLostMessageCount = 0;
         break;
      case CIE_Q_OUTPUT_SIGNAL_E:            //Intentioanl drop-through
      case CIE_Q_OUTPUT_SIGNAL_PRIORITY_E:
         CieMessageStatusRegister.CieOutputSignalQLostMessageCount = 0;
         break;
      default:
         result = ERR_NOT_FOUND_E;
         break;
   }
   
   return result;
}

/*************************************************************************************/
/**
* MM_CIEQ_ResetMessageQueue
* Function to flush all messages from one of the CIE queues.
*
* @param - queue  The ID of the queue.
*
* @return - SUCCESS_E if the queue was flushed, ERR_NOT_FOUND_E otherwise.
*/
ErrorCode_t MM_CIEQ_ResetMessageQueue(const CIEQueueType_t queue)
{
   osEvent event;
   ErrorCode_t result = SUCCESS_E;
   switch (queue)
   {
      case CIE_Q_FIRE_E:
      {
         do
         {
            event = osMessageGet(CieFireQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t* pMsg = (CieBuffer_t *)event.value.p;
               if (pMsg)
               {
                  osPoolFree(CieMsgPool, pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         CieMessageStatusRegister.CieFireQCount = 0;
         current_fire_message.Buffer[0] = BUFFER_EMPTY;
      }
         break;
      case CIE_Q_ALARM_E:
      {
         do
         {
            event = osMessageGet(CieAlarmQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t* pMsg = (CieBuffer_t *)event.value.p;
               if (pMsg)
               {
                  osPoolFree(CieMsgPool, pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         CieMessageStatusRegister.CieAlarmQCount = 0;
         current_alarm_message.Buffer[0] = BUFFER_EMPTY;
      }
         break;
      case CIE_Q_FAULT_E:
      {
         do
         {
            event = osMessageGet(CieFaultQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t* pMsg = (CieBuffer_t *)event.value.p;
               if (pMsg)
               {
                  osPoolFree(CieMsgPool, pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         CieMessageStatusRegister.CieFaultQCount = 0;
         current_fault_message.Buffer[0] = BUFFER_EMPTY;
      }
         break;
      case CIE_Q_MISC_E:
      {
         do
         {
            event = osMessageGet(CieMiscQ, 1);
            if (osEventMessage == event.status)
            {
               CieBuffer_t* pMsg = (CieBuffer_t *)event.value.p;
               if (pMsg)
               {
                  osPoolFree(CieMsgPool, pMsg);
               }            
            }

         } while ( osEventMessage == event.status );
         CieMessageStatusRegister.CieMiscQCount = 0;
         current_misc_message.Buffer[0] = BUFFER_EMPTY;
      }
         break;
      case CIE_Q_TX_BUFFER_E:       //Intentional drop-through
      case CIE_Q_OUTPUT_SIGNAL_E:
      case CIE_Q_OUTPUT_SIGNAL_PRIORITY_E:
      {
         ErrorCode_t tx_q_result;
         ErrorCode_t op_q1_result;
         ErrorCode_t op_q2_result;
         ErrorCode_t op_q3_result;

         tx_q_result = CO_QueueReset(CO_Q_LOW_PRIORITY_E);
         tx_buffer_waiting_for_response = false;
         tx_buffer_waiting_for_response_short_frame_count = 0;
         
         //Clear the three output message queues
         op_q1_result = CO_QueueReset(CO_Q_PRIORITY_1_E);
         op_q2_result = CO_QueueReset(CO_Q_PRIORITY_2_E);
         op_q3_result = CO_QueueReset(CO_Q_PRIORITY_3_E);
         tx_output_signal_waiting_for_confirm = false;
         tx_output_signal_waiting_for_confirm_short_frame_count = 0;
         
         if ( (SUCCESS_E == tx_q_result) &&
               (SUCCESS_E == op_q1_result) &&
               (SUCCESS_E == op_q2_result) &&
               (SUCCESS_E == op_q3_result) )
         {
            result = SUCCESS_E;
         }
         else 
         {
            result = ERR_OPERATION_FAIL_E;
         }
      }
      break;
      default:
         result = ERR_OUT_OF_RANGE_E;
         break;
   }

   return result;
}

/*************************************************************************************/
/**
* MM_CIEQ_TxBufferConfirmation
* Function to report when a command has been sent to the Mesh but no response has been 
* received yet.
*
* @param - TransactionID   The transaction ID that the message was sent with.
* @param - MsgSent         Is true if the confirmation callback indicates success.
*
* @return - true if we are waiting for an RBU to respond to the confirmed message.
*/
bool MM_CIEQ_TxBufferConfirmation(const uint8_t TransactionID, const bool MsgSent)
{
   static uint32_t failed_message_count = 0;
   
   if ( MM_CIEQ_MessageCount(CIE_Q_TX_BUFFER_E) )
   {
      CieBuffer_t current_message;
      if ( SUCCESS_E == MM_CIEQ_Pop( CIE_Q_TX_BUFFER_E, &current_message) )
      {
         if ( APP_MSG_TYPE_COMMAND_E == current_message.MessageType )
         {
            CO_CommandData_t* pCommand;
            pCommand = (CO_CommandData_t*)current_message.Buffer;
            if ( TransactionID == pCommand->TransactionID )
            {
               CO_PRINT_B_1(DBG_INFO_E,"MM_CIEQ_TxBufferConfirmation matched msg ID %d\r\n",TransactionID);
               /* the confirmation matches the message at the front of the queue */
               tx_buffer_waiting_for_response = MsgSent;
               tx_buffer_waiting_for_response_short_frame_count = 0;
               
               /* prevent permanent blockage by messages that countinously fail 
                  Discard the message after a fixed number of failures */
               if ( false == MsgSent )
               {
                  failed_message_count++;
                  if( MAX_FAILED_TXBUFFER_MESSAGES <= failed_message_count )
                  {
                     /* discard the failing message and place an error message 
                        into the Misc Queue */
                     if ( PARAM_TYPE_MAX_E > pCommand->CommandType )
                     {
                        CieBuffer_t message;
                        snprintf((char*)&message.Buffer,sizeof(CieBuffer_t),"%s:%d,%d,%d,1,ERROR\r\n", ParamCommandMap[pCommand->CommandType],
                                 pCommand->Destination, pCommand->Parameter1,pCommand->Parameter2);
                        
                        MM_CIEQ_Push(CIE_Q_MISC_E, &message);
                     }
                     /* discard the message */
                     MM_CIEQ_Discard(CIE_Q_TX_BUFFER_E);
                     failed_message_count = 0;
                  }
               }
               else
               {
                  /* The message was sent. Broadcast commands don't expect a response.  Check the message type remove
                     these types from the Tx Queue */
                  CO_CommandData_t* pCurrentCommand = (CO_CommandData_t*)&current_message.Buffer;
                  if ( pCurrentCommand )
                  {
                     if ( PARAM_TYPE_DAY_NIGHT_SETTING_E == pCurrentCommand->CommandType )
                     {
                        /* discard the message */
                        MM_CIEQ_Discard(CIE_Q_TX_BUFFER_E);
                        failed_message_count = 0;
                        tx_buffer_waiting_for_response = false;
                     }
                  }
               }
            }
            else 
            {
               CO_PRINT_B_2(DBG_INFO_E,"MM_CIEQ_TxBufferConfirmation miss-matched msg ID %d, expected %d\r\n",TransactionID, pCommand->TransactionID);
            }
         }
      }
   }
   return tx_buffer_waiting_for_response;
}

/*************************************************************************************/
/**
* MM_CIEQ_TxBufferWaitingForResponse
* Function to report when a command has been sent to the Mesh but no response has been 
* received yet.
*
* @param - None.
*
* @return - true if there is an outstanding message.
*/
bool MM_CIEQ_TxBufferWaitingForResponse(void)
{
   return tx_buffer_waiting_for_response;
}

/*************************************************************************************/
/**
* MM_CIEQ_TxBufferTimedOut
* Called once per short frame, this function decides whether the message at the from of
* the TxBuffer queue has timed out.
*
* @param - None.
*
* @return - true if the message has timed out.
*/
bool MM_CIEQ_TxBufferTimedOut(void)
{
   bool timed_out = false;
   
   tx_buffer_waiting_for_response_short_frame_count++;
   if ( TX_BUFFER_TIMEOUT <= tx_buffer_waiting_for_response_short_frame_count )
   {
      tx_buffer_waiting_for_response_short_frame_count = 0;
      timed_out = true;
   }
   return timed_out;
}

/*************************************************************************************/
/**
* MM_CIEQ_OutputSignalConfirmation
* Function to report when an output signal has been queued for transmission.
*
* @param - Handle    The message ID that the message was sent with.
* @param - MsgSent   Is true if the confirmation callback indicates success.
*
* @return - true if we are waiting for a confirmation message.
*/
bool MM_CIEQ_OutputSignalConfirmation(const uint32_t Handle, const bool MsgSent)
{
   if ( MM_CIEQ_MessageCount(CIE_Q_OUTPUT_SIGNAL_E) )
   {
      CieBuffer_t current_message;
      ErrorCode_t result = MM_CIEQ_Pop( CIE_Q_OUTPUT_SIGNAL_E, &current_message);
      if ( SUCCESS_E == result )
      {
         if ( APP_MSG_TYPE_OUTPUT_SIGNAL_E == current_message.MessageType )
         {
            CO_OutputData_t* pOutData;
            pOutData = (CO_OutputData_t*)current_message.Buffer;
            CO_PRINT_B_2(DBG_INFO_E,"OUTPUT_SIGNAL confirmation Rxd. Handle=%d current msg handle=%d\r\n", Handle, pOutData->Handle);
            if ( Handle == pOutData->Handle )
            {
               CO_PRINT_B_0(DBG_INFO_E,"Confirmation matched output signal\r\n");
               /* the confirmation matches the message at the front of the queue */
               CO_QueueSetLock(current_output_signal_queue, CO_Q_LOCK_UNLOCKED_E);
               ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_OUTPUT_SIGNAL_E);
               current_output_signal_queue = CO_Q_MAX_QUEUES_E;
               if ( SUCCESS_E == error )
               {
                  if ( MsgSent )
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"On Confirm : OUTPUT SIGNAL sent\r\n");
                  }
                  else 
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"On Confirm : OUTPUT SIGNAL failed to send\r\n");
                  }
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E,"On Confirm : Failed to remove OUTPUT SIGNAL from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
               }
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E,"On Confirm : Mismatch in message handle.  Expected %d\r\n", pOutData->Handle);
            }
         }
         else 
         {
            CO_PRINT_B_1(DBG_ERROR_E,"On Confirm : Mismatch in message type.  Expected %d\r\n", current_message.MessageType);
         }
      }
      else 
      {
         CO_PRINT_B_1(DBG_ERROR_E,"On Confirm : Failed to find message.  error %d\r\n", result);
      }
   }
   else 
   {
      CO_PRINT_B_0(DBG_ERROR_E,"On Confirm : Failed to find message.  Queue empty.\r\n");
   }

   return tx_output_signal_waiting_for_confirm;
}

/*************************************************************************************/
/**
* MM_CIEQ_AlarmOutputSignalConfirmation
* Function to report when an alarm output signal (AOS) has been queued for transmission.
*
* @param - Handle    The message ID that the message was sent with.
* @param - MsgSent   Is true if the confirmation callback indicates success.
*
* @return - true if we are waiting for a confirmation message.
*/
bool MM_CIEQ_AlarmOutputSignalConfirmation(const uint32_t Handle, const bool MsgSent)
{
   if ( MM_CIEQ_MessageCount(CIE_Q_OUTPUT_SIGNAL_E) )
   {
      CieBuffer_t current_message;
      if ( SUCCESS_E == MM_CIEQ_Pop( CIE_Q_OUTPUT_SIGNAL_E, &current_message) )
      {
         if ( APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E == current_message.MessageType )
         {
            CO_OutputData_t* pOutData;
            pOutData = (CO_OutputData_t*)current_message.Buffer;
            CO_PRINT_B_2(DBG_INFO_E,"ALARM OUTPUT_SIGNAL confirmation Rxd. Handle=%d current msg handle=%d\r\n", Handle, pOutData->Handle);
            if ( Handle == pOutData->Handle )
            {
               CO_PRINT_B_0(DBG_INFO_E,"Confirmation matched output signal\r\n");
               /* the confirmation matches the message at the front of the queue */
               CO_QueueSetLock(current_output_signal_queue, CO_Q_LOCK_UNLOCKED_E);
               tx_output_signal_waiting_for_confirm = false;
               ErrorCode_t error = MM_CIEQ_Discard(CIE_Q_OUTPUT_SIGNAL_E);
               current_output_signal_queue = CO_Q_MAX_QUEUES_E;
               if ( SUCCESS_E == error )
               {
                  if ( MsgSent )
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"On Confirm : ALARM OUTPUT SIGNAL sent\r\n");
                  }
                  else 
                  {
                     CO_PRINT_B_0(DBG_INFO_E,"On Confirm : ALARM OUTPUT SIGNAL failed to send\r\n");
                  }
               }
               else 
               {
                  CO_PRINT_B_1(DBG_ERROR_E,"On Confirm : Failed to remove ALARM OUTPUT SIGNAL from the TxBuffer Queue. Error=%d\r\n", (int32_t)error);
               }
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E,"On Confirm : Mismatch in message handle.  Expected %d\r\n", pOutData->Handle);
            }
         }
         else 
         {
            CO_PRINT_B_1(DBG_ERROR_E,"On Confirm : Mismatch in message type.  Expected %d\r\n", current_message.MessageType);
         }
      }
   }
   
   return tx_output_signal_waiting_for_confirm;
}


/*************************************************************************************/
/**
* MM_CIEQ_OutputSignalWaitingForConfirm
* Function to report when an output signal has been sent to the Mesh but no confirmation 
* has been received yet.
*
* @param - None.
*
* @return - true if there is an outstanding message.
*/
bool MM_CIEQ_OutputSignalWaitingForConfirm(void)
{
   return tx_output_signal_waiting_for_confirm;
}

/*************************************************************************************/
/**
* MM_CIEQ_OutputSignalTimedOut
* Called once per short frame, this function decides whether the message at the front of
* the OutputSignal queue has timed out.
*
* @param - None.
*
* @return - true if the message has timed out.
*/
bool MM_CIEQ_OutputSignalTimedOut(void)
{
   bool timed_out = false;
   
   tx_output_signal_waiting_for_confirm_short_frame_count++;
   if ( TX_BUFFER_TIMEOUT <= tx_output_signal_waiting_for_confirm_short_frame_count )
   {
      tx_output_signal_waiting_for_confirm_short_frame_count = 0;
      timed_out = true;
   }
   return timed_out;
}

/*************************************************************************************/
/**
* MM_CIEQ_GetNextOutputSignal
* Get the first message from the Output Signal queue.
*
* @param - pMessage [OUT] Destination structure for the message
* @param - pNewSignal   [OUT] set to true if the mesage is new, or false if it's been read before
*
* @return - true if a message was returned.
*/
bool MM_CIEQ_GetNextOutputSignal(CieBuffer_t* pMessage, bool* pNewSignal)
{
   bool result = false;
   
   if ( pMessage && pNewSignal )
   {
      if ( 0 < MM_CIEQ_MessageCount(CIE_Q_OUTPUT_SIGNAL_E) )
      {
         //Get the front message from the queue
         ErrorCode_t error = MM_CIEQ_Pop(CIE_Q_OUTPUT_SIGNAL_E, pMessage);
         if ( SUCCESS_E == error )
         {
            if ( false == tx_output_signal_waiting_for_confirm )
            {
               *pNewSignal = true;
               tx_output_signal_waiting_for_confirm = true;
#ifdef _DEBUG
               CO_AlarmOutputStateData_t* pAlarmData = (CO_AlarmOutputStateData_t*)pMessage->Buffer;
               CO_PRINT_B_2(DBG_INFO_E,"MM_CIEQ_Pop -New msg read. Hndl=%d, New Sig=%d\r\n", pAlarmData->Handle, *pNewSignal);
#endif
            }
            
            result = true;
         }
         else 
         {
            *pNewSignal = false;
            if ( ERR_NOT_FOUND_E == error )
            {
               CO_PRINT_B_0(DBG_INFO_E,"MM_CIEQ_Pop FAILED- no messge found\r\n");
            }
            else 
            {
               CO_PRINT_B_1(DBG_INFO_E,"MM_CIEQ_Pop - Error %d\r\n", error);
            }
         }
      }
   }
   
   return result;
}
