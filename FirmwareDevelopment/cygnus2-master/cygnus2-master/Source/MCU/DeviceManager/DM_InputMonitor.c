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
*  File         : DM_InputMonitor.c
*
*  Description  : Input Monitoring Device
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stddef.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "stm32l4xx.h"
#include "board.h"
#include "CO_SerialDebug.h"
#include "cmsis_os.h"
#include "DM_InputMonitor.h"
#include "DM_BatteryMonitor.h"


/* Private Functions Prototypes
*************************************************************************************/
static DM_InputMonitorState_t DM_InputMonitor_GetActualState(DM_InputMonitorIdx_t idx);
static void DM_InputMonitorCheckThresholds(DM_InputMonitorIdx_t idx);


/* Global Variables
*************************************************************************************/
DM_InputMonitorState_t gFireCallPointState;
DM_InputMonitorState_t gFirstAidCallPointState;

extern DM_BaseType_t gBaseType;

/* Private Variables
*************************************************************************************/

static DM_InputMonitorConfig_t InputMonitor_Config[IN_MON_MAX_IDX_E] = 
{
   [IN_MON_INSTALLATION_TAMPER_E] = 
   {
      .IsEnabled = false,
      .ActivateThreshold = TAMPER_SWITCH_ACTIVATE_DEBOUNCE_US,
      .DeactivateThreshold = TAMPER_SWITCH_DEACTIVATE_DEBOUNCE_US,
      .InputMonitor_time_us = 0,
      .pGpio_def = &InstallationTamper,
      .PinIdleState = GPIO_PIN_RESET,
      .ConfirmedState = IN_MON_UNKNOWN_STATE_E,
      .pfIrqHandler = NULL,
      .pStateChangeCallback = NULL,
   },

   [IN_MON_DISMANTLE_TAMPER_E] = 
   {
      .IsEnabled = false,
      .ActivateThreshold = TAMPER_SWITCH_ACTIVATE_DEBOUNCE_US,
      .DeactivateThreshold = TAMPER_SWITCH_DEACTIVATE_DEBOUNCE_US,
      .InputMonitor_time_us = 0,
      .pGpio_def = &DismantleTamper,
      .PinIdleState = GPIO_PIN_RESET,
      .ConfirmedState = IN_MON_UNKNOWN_STATE_E,
      .pfIrqHandler = NULL,
      .pStateChangeCallback = NULL,
   },

   [IN_MON_PIR_E] = 
   {
      .IsEnabled = false,
      .ActivateThreshold = PIR_ACTIVATE_DEBOUNCE_US,
      .DeactivateThreshold = PIR_DEACTIVATE_DEBOUNCE_US,
      .InputMonitor_time_us = 0,
      .pGpio_def = &Pir,
      .PinIdleState = GPIO_PIN_SET,
      .ConfirmedState = IN_MON_UNKNOWN_STATE_E,
      .pfIrqHandler = MM_PirInputIrq,
      .pStateChangeCallback = NULL,
		.StrikeCount = 100,
   },

   [IN_MON_MAIN_BATTERY_E] = 
   {
      .IsEnabled = false,
      .ActivateThreshold = MAIN_BATTERY_ACTIVATE_DEBOUNCE_US,
      .DeactivateThreshold = MAIN_BATTERY_DEACTIVATE_DEBOUNCE_US,
      .InputMonitor_time_us = 0,
      .pGpio_def = &PrimaryBatIrq,
      .PinIdleState = GPIO_PIN_SET,
      .ConfirmedState = IN_MON_UNKNOWN_STATE_E,
      .pfIrqHandler = MM_MainBatteryInputIrq,
      .pStateChangeCallback = NULL,
   },
   
   [IN_MON_HEAD_TAMPER_E] = 
   {
      .IsEnabled = false,
      .ActivateThreshold = TAMPER_SWITCH_ACTIVATE_DEBOUNCE_US,
      .DeactivateThreshold = TAMPER_SWITCH_DEACTIVATE_DEBOUNCE_US,
      .InputMonitor_time_us = 0,
      .pGpio_def = &HeadRx,
      .PinIdleState = GPIO_PIN_SET,
      .ConfirmedState = IN_MON_UNKNOWN_STATE_E,
      .pfIrqHandler = NULL,
      .pStateChangeCallback = NULL,
   },
};

/*************************************************************************************/
/**
* DM_InputMonitorInit
* Initialisation function for the Input Monitor device (Detection and debounce)
* call this function for each type of the monitored inputs
*
* @param - DM_InputMonitorIdx_t idx - index of the Input
* @param - pfInputMonitor_StateChangeCb_t - state change callback function pointer
*
* @return - ErrorCode_t - status 0=success else error
*/
ErrorCode_t DM_InputMonitorInit(DM_InputMonitorIdx_t idx, pfInputMonitor_StateChangeCb_t pfStateChangeCb, const uint32_t pullup)
{
   ErrorCode_t status = SUCCESS_E;
   
   /* check parameters */
   DM_IN_MON_ASSERT(IN_MON_MAX_IDX_E > idx);
   DM_IN_MON_ASSERT(NULL != pfStateChangeCb);

   InputMonitor_Config[idx].IsEnabled = true;
   InputMonitor_Config[idx].pStateChangeCallback = pfStateChangeCb;
   InputMonitor_Config[idx].ConfirmedState = IN_MON_UNKNOWN_STATE_E;
   InputMonitor_Config[idx].InputMonitor_time_us = osKernelSysTick();

   /* only set for interrupts if an interrupt hadler has been assigned */
   if ( NULL != InputMonitor_Config[idx].pfIrqHandler )
   {
      GpioSetInterrupt(InputMonitor_Config[idx].pGpio_def, 
                       IRQ_RISING_FALLING_EDGE_E, 
                       IRQ_LOW_PRIORITY_E, 
                       InputMonitor_Config[idx].pfIrqHandler, 
                       pullup );
   }

   return status;
}


/*************************************************************************************/
/**
* DM_InputMonitor_GetActualState
* Routine to extract state from an input value
* the value 0 is treated as idle and any other value is treated as active
*
* @param - DM_InputMonitorIdx_t idx - Monitored Input index
*
* @return - DM_InputMonitorState_t - Actual Input state
*/
static DM_InputMonitorState_t DM_InputMonitor_GetActualState(DM_InputMonitorIdx_t idx)
{
   uint32_t value;
   DM_InputMonitorState_t state = IN_MON_ACTIVE_E;

   /* check parameters */
   CO_ASSERT_RET_MSG(IN_MON_MAX_IDX_E > idx, IN_MON_UNKNOWN_STATE_E, "DM_InputMonitor_GetActualState: Index not valid");
   
   value = GpioRead(InputMonitor_Config[idx].pGpio_def);
   
   if (InputMonitor_Config[idx].PinIdleState == value)
   {
      state = IN_MON_IDLE_E;
   }

   return state;
}


/*************************************************************************************/
/**
* DM_InputMonitorCheckThresholds
* Routine to compare the counters to the appropriate detection threshold. If the threshold
* is exceeded then a state change occurs. At this stage the state change call back is called
*
* @param - DM_InputMonitorIdx_t idx - Monitored Input index
*
* @return - void
*/
static void DM_InputMonitorCheckThresholds(DM_InputMonitorIdx_t idx)
{
   ErrorCode_t status = SUCCESS_E;

   /* check parameters */
   CO_ASSERT_VOID_MSG(IN_MON_MAX_IDX_E > idx, "DM_InputMonitorCheckThresholds: Index not valid");
   
   if (IN_MON_UNKNOWN_STATE_E == InputMonitor_Config[idx].ConfirmedState)
   {
      if (IN_MON_ACTIVE_E == DM_InputMonitor_GetActualState(idx))
      {
         if ((osKernelSysTick() - InputMonitor_Config[idx].InputMonitor_time_us) > osKernelSysTickMicroSec(InputMonitor_Config[idx].ActivateThreshold))
         {
				//If the dismantle tamper has just beet restored on a construction unit or IOU, schedule a primary battery check.
				if ( (BASE_CONSTRUCTION_E == gBaseType) || (BASE_IOU_E == gBaseType) )
				{
					if ( idx == IN_MON_DISMANTLE_TAMPER_E )
					{
						//Remove the battery monitor inhibit
						DM_BAT_Inhibit(true);
					}
				}
				else 
				{
					//RBU and MCP units
					if ( idx == IN_MON_INSTALLATION_TAMPER_E )
					{
						//Remove the battery monitor inhibit
						DM_BAT_Inhibit(true);
					}
				}
				
            /* Post-Reset Active state confirmation
             * We need to notify the application about this
             */
            if (NULL != InputMonitor_Config[idx].pStateChangeCallback)
            {
               status = InputMonitor_Config[idx].pStateChangeCallback(IN_MON_ACTIVE_E);
            }
            
            if (status == SUCCESS_E)
            {
               InputMonitor_Config[idx].InputMonitor_time_us = osKernelSysTick();
               InputMonitor_Config[idx].ConfirmedState = IN_MON_ACTIVE_E;
            }
         }
      }		
      else
      {
         if ((osKernelSysTick() - InputMonitor_Config[idx].InputMonitor_time_us) > osKernelSysTickMicroSec(InputMonitor_Config[idx].DeactivateThreshold))
         {
            /* Post-Reset Idle state confirmation
             * No callback on the first IDLE state confirmation 
             */            
            InputMonitor_Config[idx].InputMonitor_time_us = osKernelSysTick();
            InputMonitor_Config[idx].ConfirmedState = IN_MON_IDLE_E;
         }
      }
   }
   else
   {
		if ((osKernelSysTick() - InputMonitor_Config[idx].InputMonitor_time_us) > osKernelSysTickMicroSec(InputMonitor_Config[idx].ActivateThreshold))
      {
			//If the dismantle tamper has just beet restored on a construction unit or IOU, schedule a primary battery check.
			if ( (BASE_CONSTRUCTION_E == gBaseType) || (BASE_IOU_E == gBaseType) )
			{
				if ( idx == IN_MON_DISMANTLE_TAMPER_E )
				{
					//Remove the battery monitor inhibit
					DM_BAT_Inhibit(true);
				}
			}
			else 
			{
				//RBU and MCP units
				if ( idx == IN_MON_INSTALLATION_TAMPER_E )
				{
					//Remove the battery monitor inhibit
					DM_BAT_Inhibit(true);
				}
			}
			
			if (IN_MON_IDLE_E == InputMonitor_Config[idx].ConfirmedState)
			{
            /* Input changes to ACTIVE state */
            if (NULL != InputMonitor_Config[idx].pStateChangeCallback)
            {
               status = InputMonitor_Config[idx].pStateChangeCallback(IN_MON_ACTIVE_E);
            }
            
            if (status == SUCCESS_E)
            {
               InputMonitor_Config[idx].InputMonitor_time_us = osKernelSysTick();
               InputMonitor_Config[idx].ConfirmedState = IN_MON_ACTIVE_E;
            }
         }
      }
		
     
      if ((osKernelSysTick() - InputMonitor_Config[idx].InputMonitor_time_us) > osKernelSysTickMicroSec(InputMonitor_Config[idx].DeactivateThreshold))
      {
			//If the dismantle tamper has just beet restored on a construction unit or IOU, schedule a primary battery check.
			if ( (BASE_CONSTRUCTION_E == gBaseType) || (BASE_IOU_E == gBaseType) )
			{
				if ( idx == IN_MON_DISMANTLE_TAMPER_E )
				{
					//Remove the battery monitor inhibit
					DM_BAT_Inhibit(false);
					//schedule battery check
					DM_BAT_RequestBatteryCheck(true);
					//Reset the battery restart flag in NVM
					DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E);
				}
			}
			else 
			{
				//RBU and MCP units
				if ( idx == IN_MON_INSTALLATION_TAMPER_E )
				{
					//Remove the battery monitor inhibit
					DM_BAT_Inhibit(false);
					//schedule battery check
					DM_BAT_RequestBatteryCheck(true);
					//Reset the battery restart flag in NVM
					DM_BAT_SetBatteryRestartFlag(DM_BAT_NORMAL_RESTART_E); 
				}
			}
					
			if (IN_MON_IDLE_E != InputMonitor_Config[idx].ConfirmedState)
			{
            /* Input changes to IDLE state */
            if (NULL != InputMonitor_Config[idx].pStateChangeCallback)
            {
               status = InputMonitor_Config[idx].pStateChangeCallback(IN_MON_IDLE_E);
            }
            
            if (status == SUCCESS_E)
            {
               InputMonitor_Config[idx].InputMonitor_time_us = osKernelSysTick();
               InputMonitor_Config[idx].ConfirmedState = IN_MON_IDLE_E;
            }
         }
      }
   }
}
  
/*************************************************************************************/
/**
* DM_InputMonitorPoll
* Periodic poll function for Inputs.
*
* @param - DM_InputMonitorIdx_t idx - index of the Monitored Input
*
* @return - ErrorCode_t - status 0=success
*/
ErrorCode_t DM_InputMonitorPoll(DM_InputMonitorIdx_t idx)
{
   ErrorCode_t status = SUCCESS_E;
   DM_InputMonitorState_t ActualState;

   /* check parameters */
   DM_IN_MON_ASSERT(IN_MON_MAX_IDX_E > idx);
   
   if ( InputMonitor_Config[idx].IsEnabled )
   {
      ActualState = DM_InputMonitor_GetActualState(idx);
      
		if ( idx == IN_MON_PIR_E )
		{
			static DM_InputMonitorState_t prevState = IN_MON_UNKNOWN_STATE_E;
			static DM_InputMonitorState_t currentState = IN_MON_IDLE_E;
		
			// If new state
			if ( ActualState != prevState )
			{
				// time stamp state change
				InputMonitor_Config[idx].InputMonitor_time_us = osKernelSysTick();
				prevState = ActualState;
			//	CO_PRINT_A_1( DBG_INFO_E, "PIR State changed to %d\r\n", ActualState );
			}
			else 
			{
				uint32_t now = osKernelSysTick( );
				
				// if still active
				if ( ActualState == IN_MON_ACTIVE_E )
				{
					// and active for longer than threshold
					if ( now - InputMonitor_Config[idx].InputMonitor_time_us > osKernelSysTickMicroSec(InputMonitor_Config[idx].ActivateThreshold ) )
					{
						//char buff[30];
						//sprintf( buff, "Count: %d\n", InputMonitor_Config[idx].Count );
						
						//CO_PRINT_A_1( DBG_INFO_E, "PIR Count: %d\r\n",InputMonitor_Config[idx].Count );
						
						// reset timestamp
						InputMonitor_Config[idx].InputMonitor_time_us = now;
						
						// if strike count limit
						if ( ++InputMonitor_Config[idx].Count > InputMonitor_Config[idx].StrikeCount )
						{
							// if current state idle
							if ( currentState != IN_MON_ACTIVE_E )
							{
								//CO_PRINT_A_0( DBG_INFO_E, "PIR set to active\r\n" );
								// set current state to active;
								currentState = IN_MON_ACTIVE_E;
								
								// If callback
								if (NULL != InputMonitor_Config[idx].pStateChangeCallback)
								{
									// call
									status = InputMonitor_Config[idx].pStateChangeCallback(IN_MON_ACTIVE_E);
								}	
							}								
						}	
					}
				}
				else	// state = IDLE
				{
					// and active for longer than threshold
					if ((osKernelSysTick() - InputMonitor_Config[idx].InputMonitor_time_us) > osKernelSysTickMicroSec(InputMonitor_Config[idx].DeactivateThreshold ) )
					{
						// zero strike count 
						InputMonitor_Config[idx].Count = 0;
					 
						if ( currentState == IN_MON_ACTIVE_E )
						{
							// set current state to idle;
							currentState = IN_MON_IDLE_E;
						
							// If callback
							if ( NULL != InputMonitor_Config[idx].pStateChangeCallback)
							{
								// call
								status = InputMonitor_Config[idx].pStateChangeCallback( IN_MON_IDLE_E );
							}							
						}	
					}
				}
			}
		}
		else
		{			
			// Use original debounce routine
			
			if (ActualState == InputMonitor_Config[idx].ConfirmedState)
			{
				InputMonitor_Config[idx].InputMonitor_time_us = osKernelSysTick();
			}
			else
			{
				DM_InputMonitorCheckThresholds(idx);
			}
		}
   }
   
   return status;
}


/*************************************************************************************/
/**
* DM_InputMonitorGetPollingEnabled
* Routine to check if any polling is in progress
*
* @param - void
*
* @return -  true if a polling operation is in progress
*/
bool DM_InputMonitorGetPollingEnabled(void)
{
   /* check that there is no polling in progress */
   bool IsPollingInProgress = false;
   
   for(uint8_t idx = 0u; idx < IN_MON_MAX_IDX_E; idx++)
   {
      if (   InputMonitor_Config[idx].IsEnabled /* Check if this input is initialised/monitored */
          && (InputMonitor_Config[idx].ConfirmedState != DM_InputMonitor_GetActualState((DM_InputMonitorIdx_t)idx)))
      {
         /* At least one item is in a transcient state */
         IsPollingInProgress = true;
      }
   }

   return IsPollingInProgress;
}

/*************************************************************************************/
/**
* DM_InputMonitorPollingEnd
* Routine to stop the polling round and go back ro the interrupt mode
*
* @param - void
*
* @return - void
*/
void DM_InputMonitorPollingEnd(void)
{
   for(uint8_t idx = 0u; idx < IN_MON_MAX_IDX_E; idx++)
   {
      if ( InputMonitor_Config[idx].IsEnabled ) /* Check if this input is initialised/monitored */
      {
         /* At least one item is in a transcient state */
         uint32_t temp = EXTI->IMR1;
         temp |= ((uint32_t)InputMonitor_Config[idx].pGpio_def->pinIndex);
         EXTI->IMR1 = temp;
      }
   }
}

/*************************************************************************************/
/**
* DM_InputMonitorPollingStart
* Routine to start a polling round
*
* @param - void
*
* @return - void
*/
void DM_InputMonitorPollingStart(void)
{   
   for(uint8_t index = 0u; index < IN_MON_MAX_IDX_E; index++)
   {
      InputMonitor_Config[index].InputMonitor_time_us = osKernelSysTick();
   }
}


/*************************************************************************************/
/**
* DM_SetFireCallPointState
* Set the last read value for the fire call point
*
* @param - state  The latest reading
*
* @return - None.
*/
void DM_SetFireCallPointState(const DM_InputMonitorState_t state)
{
   gFireCallPointState = state;
}

/*************************************************************************************/
/**
* DM_GetFireCallPointState
* Return the last read value for the fire call point
*
* @param - None.
*
* @return - The latest reading
*/
DM_InputMonitorState_t DM_GetFireCallPointState(void)
{
   return gFireCallPointState;
}

/*************************************************************************************/
/**
* DM_SetFirstAidCallPointState
* Set the last read value for the first aid call point
*
* @param - state  The latest reading
*
* @return - None.
*/
void DM_SetFirstAidCallPointState(const DM_InputMonitorState_t state)
{
   gFirstAidCallPointState = state;
}

/*************************************************************************************/
/**
* GetFirstAidCallPointState
* Return the last read value for the first aid call point
*
* @param - None.
*
* @return - The latest reading
*/
DM_InputMonitorState_t DM_GetFirstAidCallPointState(void)
{
   return gFirstAidCallPointState;
}

/*************************************************************************************/
/**
* GetPirState
* Return the last read value for the first aid call point
*
* @param - None.
*
* @return - The latest reading
*/
DM_InputMonitorState_t DM_GetPirState(void)
{
   return InputMonitor_Config[IN_MON_PIR_E].ConfirmedState;
}


/*************************************************************************************/
/**
* GetPirState
* Return the PIR confifg
*
* @param - None.
*
* @return - The latest reading
*/
DM_InputMonitorConfig_t* DM_GetPirConfig(void)
{
   return InputMonitor_Config + IN_MON_PIR_E;
}


/*************************************************************************************/
/**
* DM_InputMonitorSleep
* Stop all polling for sleep mode
*
* @param - None.
*
* @return - None.
*/
void DM_InputMonitorSleep(void)
{
   for(uint8_t index = 0u; index < IN_MON_MAX_IDX_E; index++)
   {
      InputMonitor_Config[index].IsEnabled = false;
   }
}
