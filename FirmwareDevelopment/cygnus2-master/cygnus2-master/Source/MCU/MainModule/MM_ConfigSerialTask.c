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
*  File         : MM_ConfigSerialTask.c
*
*  Description  : Config Serial task functions
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>


/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "board.h"
#include "DM_SerialPort.h"
#include "CO_Defines.h"
#include "MM_ConfigSerialTask.h"
#include "MM_ApplicationCommon.h"

/* Private Functions Prototypes
*************************************************************************************/
void MM_ConfigSerialTaskMain(void const *argument);                   // thread function


/* Global Variables
*************************************************************************************/
typedef enum
{
   CS_TASK_STATE_IDLE_E,
   CS_TASK_STATE_ACTIVE_E
} ConfigSerial_TaskState_t;

typedef enum
{
   CS_STATE_STOPPED_E,
   CS_STATE_IDLE_E,
   CS_STATE_RECEIVING_E,
   CS_STATE_INATMODE_E,
   CS_STATE_INTESTMODE_E,
   CS_STATE_DONE_E
} ConfigSerial_ChannelState_t;

typedef struct
{
   ConfigSerial_ChannelState_t channel_state;
   uint32_t timer_value;
}ConfigSerial_ChannelContext_t;

#define CONFIGSERIAL_POOL_SIZE	10
#define CONFIGSERIAL_Q_SIZE    	4

/* Delay in microseconds to abort the AT start Sequence detection */
#define AT_MODE_SEQ_TIMEOUT_US		((uint32_t)500e3)

/* Delay in microseconds to exit an active AT mode session */
#define AT_MODE_EXIT_TIMEOUT_US		((uint32_t)10e6)

/* Delay in milliseconds to authorise the system to go back to sleep after a serial wake up */
#define WAKE_UP_TIMEOUT_US          ((uint32_t)500e3)

/* Task period rates in milli seconds */
#define SLOW_TASK_PERIOD_MS 250u
#define FAST_TASK_PERIOD_MS 10u

ConfigSerial_TaskState_t ConfigSerial_TaskState = CS_TASK_STATE_IDLE_E;

ConfigSerial_ChannelContext_t ConfigSerial_ChannelContext[MAX_SUPPORTED_SERIAL_PORT_E];

osThreadId tid_ConfigSerialTask;												// thread id
osThreadDef(MM_ConfigSerialTaskMain, osPriorityNormal, 1, 512);	// thread object

// Pool Definition
osPoolDef (ConfigSerialPool, CONFIGSERIAL_POOL_SIZE, Cmd_Message_t);
osPoolId ConfigSerialPool;

// Queue Definition
osMessageQDef(ConfigSerialQ, CONFIGSERIAL_Q_SIZE, &Cmd_Message_t);
osMessageQId(ConfigSerialQ);

// Semaphore Definition
osSemaphoreDef(ConfigSerialSemaphore);										// Semaphore definition
osSemaphoreId ConfigSerialSemaphoreId;										// Semaphore ID

extern DM_BaseType_t gBaseType;
extern bool gDynamicPowerMode;

extern const bool ConfigSerial_ChannelDef[CS_MAX_CONFIGURATIONS_E][MAX_SUPPORTED_SERIAL_PORT_E];

/* Private Variables
*************************************************************************************/
#ifdef ENABLE_STOP_MODE
static uint32_t gSerialAwakeTimeout = 0;
#endif

/* This flag tells the idle thread not to enter stop mode. */
bool gSerialWakeupInterrupted = false;

/* Configuration to be used for this unit */
static bool (*ConfigSerial_my_config)[MAX_SUPPORTED_SERIAL_PORT_E];

/*************************************************************************************/
/**
* function name	: MM_ConfigSerialTaskInit(void)
* description		: Initialisation of the ConfigSerial task
*
* @param - none
*
* @return - ErrorCode_t  // Returns an error code, else SUCCESS_E
*/
ErrorCode_t MM_ConfigSerialTaskInit(CS_Configurations_t serial_config)
{
   ErrorCode_t result = SUCCESS_E;

   CO_ASSERT_RET_MSG(serial_config < CS_MAX_CONFIGURATIONS_E, ERR_FAILED_TO_CREATE_THREAD_E, "Serial Configuration not valid");

   /* Initialise the state of the channel */
   for(uint8_t idx = 0; idx < MAX_SUPPORTED_SERIAL_PORT_E; idx++)
   {      
      ConfigSerial_ChannelContext[idx].channel_state = CS_STATE_STOPPED_E;

      if ( (false != ConfigSerial_ChannelDef[serial_config][idx]) && (DEBUG_UART_E != idx))
      {
         if ( UART_SUCCESS_E != SerialPortInit(idx))
         {
            result = ERR_INIT_FAIL_E;
         }
      }
   }
   
   if (SUCCESS_E == result )
   {
      /* Initialise the task to Idle */
      ConfigSerial_TaskState = CS_TASK_STATE_IDLE_E;
      
      // create pool
      ConfigSerialPool = osPoolCreate(osPool(ConfigSerialPool));
      
      if (ConfigSerialPool != NULL)
      {
         // create queues
         ConfigSerialQ = osMessageCreate(osMessageQ(ConfigSerialQ), NULL);

         if (ConfigSerialQ != NULL)
         {
            // Create thread (return on error)
            tid_ConfigSerialTask = osThreadCreate(osThread(MM_ConfigSerialTaskMain), (void*)&ConfigSerial_ChannelDef[serial_config]);
               
            if (tid_ConfigSerialTask)
            {
               // Create Semapore
               ConfigSerialSemaphoreId = osSemaphoreCreate(osSemaphore(ConfigSerialSemaphore), 0);
               
               if (ConfigSerialSemaphoreId)
               {
                  result = SUCCESS_E;
               }
               else 
               {
                  result = ERR_INIT_FAIL_E;
               }
            }
            else 
            {
               result = ERR_FAILED_TO_CREATE_THREAD_E;
            }
         }
         else 
         {
            result = ERR_FAILED_TO_CREATE_QUEUE_E;
         }
      }
      else 
      {
         result = ERR_FAILED_TO_CREATE_MEMPOOL_E;
      }
   }
   
   if (result == SUCCESS_E)
   {
      CO_PRINT_B_0(DBG_INFO_E, "ConfigSerialTask created\r\n");
   }
   else
   {
      CO_PRINT_B_1(DBG_ERROR_E, "ConfigSerialTask creation Failed. Error=%d\r\n", result);
   }

   return result;
}

/*************************************************************************************/
/**
* function name	: ConfigSerial_channelHandler(void)
* description		: Handler of a serial communication interface
*
* @param - none
*
* @return - ConfigSerial_ChannelState_t  Actual state of the channel
*/
static ConfigSerial_ChannelState_t ConfigSerial_channelHandler(uint8_t channel_id)
{
	uint8_t aux_reg;
	osStatus status;
      
   ConfigSerial_ChannelContext_t *const p_channel_context = &ConfigSerial_ChannelContext[channel_id];
   
   UartComm_t port_id = (UartComm_t)channel_id;
   
   switch(p_channel_context->channel_state)
   {
      case CS_STATE_STOPPED_E:
      {
         /* Start a new Reception Operation */
         SerialPortStartReception(port_id);
      
         /* Move to the Idle State */
         p_channel_context->channel_state = CS_STATE_IDLE_E;
         break;
      }
      
      case CS_STATE_IDLE_E:
      {
         if(SerialPortBytesInRxBuffer(port_id) > 0)
         {
            /* Something is received */
            p_channel_context->channel_state = CS_STATE_RECEIVING_E;

            /* Capture the now time */
            p_channel_context->timer_value = osKernelSysTick();
         }
         break;
      }
   
      case CS_STATE_RECEIVING_E:
      {
         uint8_t data_buffer[32];
         aux_reg = SerialPortFindCRLFRxBuffer(port_id);         

         /* We expect 3 '+' characters, a carriage return and new line special characters */
         if(aux_reg < 5u)
         {
            /* Read the non valid bytes to be removed from the buffer */
            SerialPortReadRxBuffer(port_id, data_buffer, aux_reg);
         }
         else
         {
            SerialPortReadRxBuffer(port_id, data_buffer, aux_reg);
            
            /* Check for +++ sequence in RX buffer of the Serial Port */
            data_buffer[aux_reg-2] = '\0';
            if (strcmp((char*)&data_buffer[aux_reg - 5],"+++") == 0) 
            {
               p_channel_context->channel_state = CS_STATE_INATMODE_E;
            
               /* Capture the now time */
               p_channel_context->timer_value = osKernelSysTick();
               
               // Allocate Pool
               Cmd_Message_t *pCmdMessage = NULL;
               pCmdMessage = osPoolAlloc(ConfigSerialPool);
               if (pCmdMessage)
               {
                  pCmdMessage->port_id = port_id;
                  memcpy(pCmdMessage->data_buffer, "+++",3);

                  pCmdMessage->length = 3u;                                   
                  status = osMessagePut(ConfigSerialQ, (uint32_t)pCmdMessage, 0);
                  if (osOK != status)
                  {
                     osPoolFree(ConfigSerialPool, pCmdMessage);
                  }
               }
            }
         }

         if ((osKernelSysTick() - p_channel_context->timer_value) > osKernelSysTickMicroSec(AT_MODE_SEQ_TIMEOUT_US))
         {
            SerialPortFlushRxBuffer(port_id);
            p_channel_context->channel_state = CS_STATE_STOPPED_E;
         }
         break;
      }
      
      case CS_STATE_INATMODE_E:
      {
         aux_reg = SerialPortFindCRLFRxBuffer(port_id);
         if(aux_reg > 2)
         {
            /* Capture the now time */
            p_channel_context->timer_value = osKernelSysTick();
            
            // Allocate Pool
            Cmd_Message_t *pCmdMessage = NULL;
            pCmdMessage = osPoolAlloc(ConfigSerialPool);
            if (pCmdMessage)
            {
               pCmdMessage->port_id = port_id;
               SerialPortReadRxBuffer(port_id, pCmdMessage->data_buffer, aux_reg);

               // Subtract 2 to remove <CR><LF>
               pCmdMessage->length = aux_reg - 2u;                                   
               status = osMessagePut(ConfigSerialQ, (uint32_t)pCmdMessage, 0);
               if (osOK != status)
               {
                  // failed to write
                  ;
                  // RM: TODO - Error handling
                  osPoolFree(ConfigSerialPool, pCmdMessage);
               }
            }
         }
         else
         {         
            // Timeout to exit AT Mode if no command is received within AT_MODE_EXIT_TIMEOUT milliseconds
            if ((osKernelSysTick() - p_channel_context->timer_value) > osKernelSysTickMicroSec(AT_MODE_EXIT_TIMEOUT_US))
            {
               CO_PRINT_B_1(DBG_INFO_E, "PORT %d: AT Mode Timed-out\r\n", port_id);
               SerialPortFlushRxBuffer(port_id);
               p_channel_context->channel_state = CS_STATE_STOPPED_E;
            }
         }
         break;
      }
      case CS_STATE_INTESTMODE_E:
      {
         uint32_t bytes_in_buffer = 0;

         bytes_in_buffer = SerialPortBytesInRxBuffer(port_id);
         if( bytes_in_buffer >= TEST_PAYLOAD_SIZE)
         {
            Cmd_Message_t *pCmdMessage = osPoolAlloc(ConfigSerialPool);
            if (pCmdMessage)
            {
               strcpy((char*)pCmdMessage->data_buffer, "ATTST=");
               uint8_t* pPayload = &pCmdMessage->data_buffer[6];/*point to the next byte after ATTST= */
               pCmdMessage->port_id = port_id;
               pCmdMessage->length = TEST_PAYLOAD_SIZE + 6;/* test payload length plus 6 bytes for ATTST= */
               SerialPortReadRxBuffer(port_id, pPayload, TEST_PAYLOAD_SIZE);
               status = osMessagePut(ConfigSerialQ, (uint32_t)pCmdMessage, 0);
               if (osOK != status)
               {
                  osPoolFree(ConfigSerialPool, pCmdMessage);
               }
            }
            bytes_in_buffer = 0;
         }

         break;
      }
         
      default:
      {
         ;
         break;
      }
   }
   
   return(p_channel_context->channel_state);
}

/*************************************************************************************/
/**
* function name   : MM_ConfigSerialTaskMain(void const *argument)
* description     : Main function for ConfigSerial task
*
* @param - 
*
* @return - nothing
*/
void MM_ConfigSerialTaskMain(void const *argument)
{
   CO_ASSERT_VOID_MSG(argument, "MM_ConfigSerialTaskMain argument=NULL");
   
   /* Set the configuration we should be using */
   ConfigSerial_my_config = (bool (*)[MAX_SUPPORTED_SERIAL_PORT_E])argument;
      
   while (1)
   {
      switch(ConfigSerial_TaskState)
      {
         case CS_TASK_STATE_IDLE_E:
         {
            uint8_t channel_id;

            #ifdef ENABLE_STOP_MODE
            if ( (STATE_IDLE_E != GetCurrentDeviceState())  && 
               ( (BASE_NCU_E != gBaseType || ((BASE_NCU_E == gBaseType) && 
                 (true == gDynamicPowerMode))) ) )
            {
               /* Check if we woke up from sleep */
               if ( false == gSerialWakeupInterrupted )
               {
                  uint32_t waiting_time;
                  
                  
                  #ifdef RESTORE_SERIAL_PORTS_AFTER_WAKEUP
                  waiting_time = SLOW_TASK_PERIOD_MS;
                  #else
                  //Make sure we have configured for stop mode before we hang on the semaphore
                  if ( MM_ConfigSerialEnterStopMode() )
                  {
                     waiting_time = osWaitForever;
                  }
                  else 
                  {
                     waiting_time = SLOW_TASK_PERIOD_MS;
                  }
                  #endif

                  /* Wait for something to be received or timeout */
                  if (osSemaphoreWait(ConfigSerialSemaphoreId, waiting_time) > 0)
                  {
                     for(channel_id = 0; 
                         channel_id < MAX_SUPPORTED_SERIAL_PORT_E;
                         channel_id++)        
                     {
                        #ifndef RESTORE_SERIAL_PORTS_AFTER_WAKEUP
                        if (false != (*ConfigSerial_my_config)[channel_id])
                        {
                           SerialPortLeaveStopMode(channel_id);
                           ConfigSerial_ChannelContext[channel_id].channel_state = CS_STATE_STOPPED_E;
                        }
                        #endif
                     }
                     /* Keep track that we had a serial wake-up */
                     gSerialWakeupInterrupted = true;

                     /* Read the current time as timestamp to give the AT commands 
                      * time to operate. 
                      */
                     gSerialAwakeTimeout = osKernelSysTick();
                  }
               }
               else
               {
                  /* We woke up from sleep because of an activity detection on a serial port.
                   * We need to run the task faster
                   */
                  osDelay(FAST_TASK_PERIOD_MS);
                  
                  /* We need to keep the system awake for a short period to give AT commands a chance to be accepted. */      
                  if ((osKernelSysTick() - gSerialAwakeTimeout) > osKernelSysTickMicroSec(WAKE_UP_TIMEOUT_US))
                  {
                     /* Timed out since the interrupt */
                     gSerialWakeupInterrupted = false;
                  }
               }
            }
            else 
            {
               osDelay(FAST_TASK_PERIOD_MS);
            }
            #else
            /* No stop mode, just schdeule the task */
            osDelay(FAST_TASK_PERIOD_MS);
            #endif

            for(channel_id = 0; 
                channel_id < MAX_SUPPORTED_SERIAL_PORT_E ;
                channel_id++)
            {
               const bool is_channel_enabled = (*ConfigSerial_my_config)[channel_id];
               if (false != is_channel_enabled)
               {
                  ConfigSerial_ChannelState_t channel_state;

                  channel_state = ConfigSerial_channelHandler(channel_id);
                  
                  if (CS_STATE_IDLE_E != channel_state)
                  {
                     /* At least one channel has changed state, move to the Active state */
                     ConfigSerial_TaskState = CS_TASK_STATE_ACTIVE_E;
                  }
               }
            }
            break;
         }

         case CS_TASK_STATE_ACTIVE_E:
         {
            bool all_channel_inactive = true;
            uint8_t channel_id;
            
            for(channel_id = 0; 
                channel_id < MAX_SUPPORTED_SERIAL_PORT_E;
                channel_id++)        
            {
               const bool is_channel_enabled = (*ConfigSerial_my_config)[channel_id];

               if (false != is_channel_enabled)
               {
                  ConfigSerial_ChannelState_t channel_state;
                  
                  channel_state = ConfigSerial_channelHandler(channel_id);
                  
                  if (CS_STATE_IDLE_E != channel_state)
                  {
                     /* At least one channel has changed state, move to the Active state */
                     all_channel_inactive = false;
                  }
               }
            }
            
            if (all_channel_inactive == false)
            {            
               /* At least a channel is active, keep the processing going */
               osDelay(FAST_TASK_PERIOD_MS);
            }
            else
            {
               /* All serial channels are inactive */
               ConfigSerial_TaskState = CS_TASK_STATE_IDLE_E;
            }
            break;
         }
         default:
         {
            ConfigSerial_TaskState = CS_TASK_STATE_IDLE_E;
            break;
         }
      }
   }
}

/*************************************************************************************/
/**
* function name	: MM_SetTransparentMode(void)
* description		: Called when an AT command has set the NCU/RBU into the  TRANSPARENT
*                   test mode.  Move the local state machine to CS_STATE_INTESTMODE_E.
*                   This is a permanent state so the NCU/RBU must be reset to recover
*                   from it.
*
* @param  - uart_port: UART channel index
*
* @return - none
*/
void MM_SetTransparentMode(UartComm_t uart_port)
{   
   /*Should only ever be called in state CS_STATE_INATMODE_E */
   if ( CS_STATE_INATMODE_E == ConfigSerial_ChannelContext[uart_port].channel_state )
   {
      ConfigSerial_ChannelContext[uart_port].channel_state = CS_STATE_INTESTMODE_E;
   }
}

/*************************************************************************************/
/**
* function name   : ConfigSerial_WakeupInterrupt
* description     : ISR for the debug serial port pin when configured for
*                   Stop Mode.
*
* @param - None
*
* @return -  none
*/
void ConfigSerial_WakeupInterrupt(void)
{      
   /* Release the Semaphore to notify the task of a wakeup from sleep due to a
    * serial port Rx activity
    */
   osSemaphoreRelease(ConfigSerialSemaphoreId);
}

/*************************************************************************************/
/**
* function name   : MM_ConfigSerialEnterStopMode()
* description     : 
*
* @param - None
*
* @return -  true if all ports entered stop mode
*/
bool MM_ConfigSerialEnterStopMode()
{
   bool result = true;
   uint8_t channel_id;
      
   /* first decomission the debug usart */
   for(channel_id = 0; 
       channel_id < MAX_SUPPORTED_SERIAL_PORT_E;
       channel_id++)        
   {   
      if (false != (*ConfigSerial_my_config)[channel_id])
      {
         if ( false == SerialPortEnterStopMode(channel_id, &ConfigSerial_WakeupInterrupt) )
         {
            result = false;
         }
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* function name   : MM_ConfigSerialLeaveStopMode()
* description     : 
*
* @param - None
*
* @return -  none
*/
void MM_ConfigSerialLeaveStopMode()
{
   uint8_t channel_id;
   
   /* set the usart up */
      for(channel_id = 0; 
          channel_id < MAX_SUPPORTED_SERIAL_PORT_E;
          channel_id++)        
   {
      if (false != (*ConfigSerial_my_config)[channel_id])
      {
         SerialPortLeaveStopMode(channel_id);          
         ConfigSerial_ChannelContext[channel_id].channel_state = CS_STATE_STOPPED_E;
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_ConfigSerial_CanGoToSleep
* description     : Checks if this module allows the system to enter the sleep mode
*
* @param - none
*
* @return - true if conditions are met to enter sleep mode
*/
bool MM_ConfigSerial_CanGoToSleep(void)
{
   bool can_sleep = true;

   if ( (CS_TASK_STATE_IDLE_E != ConfigSerial_TaskState) || ( false != gSerialWakeupInterrupted ))
   {
      /* Task is not idle, keep awake */
      can_sleep = false;
   }
   
   return can_sleep;
}

/*************************************************************************************/
/**
* function name   : MM_ConfigSerial_ErrorCallback
* description     : Callback to notify this task of any error at the low level
*
* @param - none
*
* @return - none
*/
void MM_ConfigSerial_ErrorCallback(uint8_t channel_id)
{
   if (MAX_SUPPORTED_SERIAL_PORT_E > channel_id)
   {
      /* Restart the channel */
      ConfigSerial_ChannelContext[channel_id].channel_state = CS_STATE_STOPPED_E;
   }   
}
