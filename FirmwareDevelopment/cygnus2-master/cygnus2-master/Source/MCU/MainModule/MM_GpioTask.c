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
*  File         : MM_GpioTask.c
*
*  Description  : GPIO Task implementation
*                 This is responsible for polling the GPIO inputs and handling debouncing
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "MM_GpioTask.h"
#include "MM_MeshAPI.h"
#include "board.h"
#include "utilities.h"
#include "DM_SerialPort.h"
#include "DM_Device.h"
#include "CFG_Device_cfg.h"
#include "MM_Interrupts.h"
#include "DM_InputMonitor.h"
#include "DM_NVM.h"

/* Private Functions Prototypes
*************************************************************************************/
static ErrorCode_t MM_Tamper1SwitchStateChange(DM_InputMonitorState_t newState);
static ErrorCode_t MM_Tamper2SwitchStateChange(DM_InputMonitorState_t newState);
static ErrorCode_t MM_MainBatteryStateChange(DM_InputMonitorState_t newState);
static ErrorCode_t MM_PluginTamperStateChange(DM_InputMonitorState_t newState);
static ErrorCode_t MM_PirStateChange(DM_InputMonitorState_t newState);

/* Global Variables
*************************************************************************************/
osThreadId tid_GpioTask;
osThreadDef (MM_GpioTaskMain, osPriorityNormal, 1, 512);

osSemaphoreDef (GpioSem);
osSemaphoreId  (GpioSemId);

extern osPoolId AppPool;
extern osMessageQId(AppQ);
extern DM_BaseType_t gBaseType;
extern Gpio_t TamperEnable;


/* Private Variables
*************************************************************************************/
static bool MM_IsFireCpEnabled = false;
static bool MM_IsFirstAidCpEnabled = false;
static DM_InputMonitorState_t gPIRState;

/*************************************************************************************/
/**
* MM_GpioTaskInit
* Initialisation function for the GPIO Task
*
* @param - bool deviceCombination - Device combination configuration
* @param - bool isFireCpEnabled - flag true if Fire call point is enabled
* @param - bool isFirstAidCpEnabled - flag true if First Aid call point is enabled
*
* @return - ErrorCode_t  // Returns an error code, else SUCCESS_E

*/
ErrorCode_t MM_GpioTaskInit(bool isFireCpEnabled, bool isFirstAidCpEnabled)
{     
   CO_PRINT_B_2(DBG_INFO_E,"GPIO Task Init. FireMCP=%d, FirstAidMCP=%d\r\n", isFireCpEnabled, isFirstAidCpEnabled);
   /* take local copy of configuration */
   MM_IsFireCpEnabled     = isFireCpEnabled;
   MM_IsFirstAidCpEnabled = isFirstAidCpEnabled;

   /* create semaphore */
   GpioSemId = osSemaphoreCreate(osSemaphore(GpioSem), 1);
   CO_ASSERT_RET_MSG(NULL != GpioSemId, ERR_INIT_FAIL_E, "ERROR - Failed to create GPIO semaphore");
      
	/* Get PIR debounce values from NVM */
	DM_InputMonitorConfig_t* config = DM_GetPirConfig( );	
	DM_NVMRead( NV_PIR_ACTIVATE_PERIOD_E,   &config->ActivateThreshold,   sizeof(config->ActivateThreshold) );
	DM_NVMRead( NV_PIR_DEACTIVATE_PERIOD_E, &config->DeactivateThreshold, sizeof(config->DeactivateThreshold) );
	DM_NVMRead( NV_PIR_STRIKE_COUNT_E, 		 &config->StrikeCount, 			 sizeof(config->StrikeCount) );
	
		
   DM_SetFireCallPointState(IN_MON_IDLE_E);
   DM_SetFirstAidCallPointState(IN_MON_IDLE_E);
   
   if (true == MM_IsFireCpEnabled)
   {
      GpioSetInterrupt(&FireMCP, 
                 IRQ_RISING_FALLING_EDGE_E, 
                 IRQ_HIGH_PRIORITY_E, 
                 MM_FireMCPInputIrq, 
                 GPIO_PULLUP );
   }
   
   if (true == MM_IsFirstAidCpEnabled)
   {
      GpioSetInterrupt(&FirstAidMCP, 
                 IRQ_RISING_FALLING_EDGE_E, 
                 IRQ_HIGH_PRIORITY_E, 
                 MM_FirstAidMCPInputIrq, 
                 GPIO_PULLUP );
   }
   
   switch ( gBaseType )
   {
      case BASE_CONSTRUCTION_E:
         /* Dismantle tamper */
         DM_InputMonitorInit(IN_MON_DISMANTLE_TAMPER_E, MM_Tamper2SwitchStateChange, GPIO_NOPULL);
         /* Installation tamper */
         DM_InputMonitorInit(IN_MON_INSTALLATION_TAMPER_E, MM_Tamper1SwitchStateChange, GPIO_NOPULL);
         /* Monitor PIR from SiteNet */
         DM_InputMonitorInit(IN_MON_PIR_E, MM_PirStateChange, GPIO_PULLUP);
         break;
      case BASE_IOU_E:
         /* Dismantle tamper */
         DM_InputMonitorInit(IN_MON_DISMANTLE_TAMPER_E, MM_Tamper2SwitchStateChange, GPIO_NOPULL);
         break;
      case BASE_MCP_E:
         /* Installation tamper */
         DM_InputMonitorInit(IN_MON_INSTALLATION_TAMPER_E, MM_Tamper1SwitchStateChange, GPIO_NOPULL);
         break;
      case BASE_NCU_E:
         /* Monitor Low Battery indication line */
         DM_InputMonitorInit(IN_MON_MAIN_BATTERY_E, MM_MainBatteryStateChange, GPIO_NOPULL);
         break;
      case BASE_RBU_E:
         /* Installation tamper */
         DM_InputMonitorInit(IN_MON_INSTALLATION_TAMPER_E, MM_Tamper1SwitchStateChange, GPIO_NOPULL);
         /* Monitor Low Battery indication line */
         DM_InputMonitorInit(IN_MON_MAIN_BATTERY_E, MM_MainBatteryStateChange, GPIO_NOPULL);
         if ( CFG_GetBlankingPlateFitted() )
         {
            CO_PRINT_B_0(DBG_INFO_E,"Blanking Plate Fitted\r\n");
            /*Plug-in Dismantle tamper */
            DM_InputMonitorInit(IN_MON_HEAD_TAMPER_E, MM_PluginTamperStateChange, GPIO_NOPULL);
         }
         else 
         {
            CO_PRINT_B_0(DBG_INFO_E,"Blanking Plate NOT Fitted\r\n");
         }
         break;
      case BASE_REPEATER_E:
         /* Installation tamper */
         DM_InputMonitorInit(IN_MON_INSTALLATION_TAMPER_E, MM_Tamper1SwitchStateChange, GPIO_NOPULL);
         if ( CFG_GetBlankingPlateFitted() )
         {
            /*Plug-in Dismantle tamper */
            DM_InputMonitorInit(IN_MON_HEAD_TAMPER_E, MM_PluginTamperStateChange, GPIO_NOPULL);
         }
         break;
      default:
         break;
   }

   /* Set the Tamper Enable */
   GpioWrite(&TamperEnable, 0);

   osDelay(1000);
   
   // Create thread (return on error)
   tid_GpioTask = osThreadCreate(osThread(MM_GpioTaskMain), NULL);
   CO_ASSERT_RET_MSG(NULL != tid_GpioTask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create GPIO Task thread");

   return(SUCCESS_E);
}

/*************************************************************************************/
/**
* MM_GpioTaskMain
* Main function for GPIO Task
*
* @param - arguments
*
* @return - void

*/
void MM_GpioTaskMain(void const *argument)
{
   while(true)
   {
      int32_t available_tokens = 0;
      bool pollingEnabled = true;
      /* On the first entry, wait forever for the semaphore to be released */
      uint32_t timeout = osWaitForever;

      do
      {
         available_tokens = osSemaphoreWait(GpioSemId, timeout);
      
         // Flush all the tokens after the first one
         timeout = 0;
      } while ( available_tokens > 0);
      
      //CO_PRINT_B_0(DBG_INFO_E,"GPIO Poll\r\n");
      
      /* Set the Tamper Enable */
      GpioWrite(&TamperEnable, 1);
      
      if ( false == CFG_GetPollingDisabled() )
      {
         //If a blanking plate is fitted set the plugin Tx high
         if ( CFG_GetBlankingPlateFitted() )
         {
            //Wait for level to settle
            osDelay(395);
            GpioWrite(&HeadPowerOn, 1);
            GpioWrite(&HeadTx, 1);
            osDelay(5);
         }
         else 
         {
            //Wait for level to settle
            osDelay(400);
         }
         
         /* Start the Polling round */
         DM_InputMonitorPollingStart();
         do
         {
            switch ( gBaseType )
            {
               case BASE_CONSTRUCTION_E:
                  /* Dismantle tamper */
                  DM_InputMonitorPoll(IN_MON_DISMANTLE_TAMPER_E);
                  /* Installation tamper */
                  DM_InputMonitorPoll(IN_MON_INSTALLATION_TAMPER_E);
                  /* Monitor PIR from SiteNet */
                  DM_InputMonitorPoll(IN_MON_PIR_E);
                  /* Monitor Low Battery indication line */
                  DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
                  break;
               case BASE_IOU_E:
                  /* Dismantle tamper */
                  DM_InputMonitorPoll(IN_MON_DISMANTLE_TAMPER_E);
                  break;
               case BASE_MCP_E:
                  /* Dismantle tamper */
                  DM_InputMonitorPoll(IN_MON_DISMANTLE_TAMPER_E);
                  /* Installation tamper */
                  DM_InputMonitorPoll(IN_MON_INSTALLATION_TAMPER_E);
                  break;
               case BASE_NCU_E:
                  /* Monitor Low Battery indication line */
                  DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
                  break;
               case BASE_RBU_E:
                  /* Dismantle tamper */
                  DM_InputMonitorPoll(IN_MON_DISMANTLE_TAMPER_E);
                  /* Installation tamper */
                  DM_InputMonitorPoll(IN_MON_INSTALLATION_TAMPER_E);
                  /* Monitor Low Battery indication line */
                  DM_InputMonitorPoll(IN_MON_MAIN_BATTERY_E);
                  if ( CFG_GetBlankingPlateFitted() )
                  {
                     //Blanking plate tamper
                     DM_InputMonitorPoll(IN_MON_HEAD_TAMPER_E);
                  }
                  break;
               case BASE_REPEATER_E:
                  /* Dismantle tamper */
                  DM_InputMonitorPoll(IN_MON_DISMANTLE_TAMPER_E);
                  /* Installation tamper */
                  DM_InputMonitorPoll(IN_MON_INSTALLATION_TAMPER_E);
                  if ( CFG_GetBlankingPlateFitted() )
                  {
                     //Blanking plate tamper
                     DM_InputMonitorPoll(IN_MON_HEAD_TAMPER_E);
                  }
                  break;
               default:
                  break;
            }
            
            pollingEnabled = DM_InputMonitorGetPollingEnabled();
            if ( pollingEnabled )
            {
               //Allow a short sleep to enable the watchdog to be kicked.
               osDelay(10);
            }
         } while( pollingEnabled );
         

         /* Exit the polling mode */
         DM_InputMonitorPollingEnd();

         /* Reset the Tamper Enable */
         GpioWrite(&TamperEnable, 0);
         //If a blanking plate is fitted reset the plugin Tx low
         if ( CFG_GetBlankingPlateFitted() )
         {
            GpioWrite(&HeadPowerOn, 0);
            GpioWrite(&HeadTx, 0);
         }
      }
   }
}


/*************************************************************************************/
/**
* MM_MainBatteryStateChange
* Report a change in the state of the Main battery status
*
* @param - DM_InputMonitorState_t newState - new state of the main Battery status
*
* @return - ErrorCode_t - status 0=success
*/
static ErrorCode_t MM_MainBatteryStateChange(DM_InputMonitorState_t newState)
{
   ErrorCode_t status = SUCCESS_E;
   uint16_t isActive = newState == IN_MON_ACTIVE_E ? 0 : 1;
 
   status = CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_LOW_BATTERY_E, isActive, false, false);
      
   return status;
}

/*************************************************************************************/
/**
* MM_Tamper1SwitchStateChange
* Handler for the InstallationTamper Switch state change
*
* @param - DM_InputMonitorState_t newState - new state of the InstallationTamper switch
*
* @return - ErrorCode_t - status 0=success
*/
static ErrorCode_t MM_Tamper1SwitchStateChange(DM_InputMonitorState_t newState)
{
   ErrorCode_t status = SUCCESS_E;
   uint16_t isActive = newState == IN_MON_ACTIVE_E ? 1 : 0;
 
   CO_PRINT_B_1(DBG_INFO_E,"Installation Tamper state changed : %d\r\n", isActive);
   
   status = CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_INSTALLATION_TAMPER_E, isActive, false, false);
      
   return status;
}

/*************************************************************************************/
/**
* MM_Tamper2SwitchStateChange
* Handler for the DismantleTamper Switch state change
*
* @param - DM_InputMonitorState_t newState - new state of the DismantleTamper switch
*
* @return - ErrorCode_t - status 0=success
*/
static ErrorCode_t MM_Tamper2SwitchStateChange(DM_InputMonitorState_t newState)
{
   ErrorCode_t status = SUCCESS_E;
   uint16_t isActive = newState == IN_MON_ACTIVE_E ? 1 : 0;
   
   CO_PRINT_B_1(DBG_INFO_E,"Dismantle Tamper state changed : %d\r\n", isActive);
 
   status = CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_DISMANTLE_TAMPER_E, isActive, false, false);
      
   return status;
}

/*************************************************************************************/
/**
* MM_PluginTamperStateChange
* Handler for the Plug-in tamper state change
*
* @param - DM_InputMonitorState_t newState - new state of the tamper
*
* @return - ErrorCode_t - status 0=success
*/
static ErrorCode_t MM_PluginTamperStateChange(DM_InputMonitorState_t newState)
{
   ErrorCode_t status = SUCCESS_E;
   uint16_t isActive = newState == IN_MON_ACTIVE_E ? 1 : 0;
 
   CO_PRINT_B_1(DBG_INFO_E,"Blanking Plate Tamper state changed : %d\r\n", isActive);
   
   status = CO_SendFaultSignal(BIT_SOURCE_NONE_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_HEAD_REMOVED_E, isActive, false, false);
      
   return status;
}


/*************************************************************************************/
/**
* MM_PirStateChange
* Handler for the SiteNet PIR state change
*
* @param - DM_InputMonitorState_t newState - new state for the PIR
*
* @return - ErrorCode_t - status 0=success
*/
static ErrorCode_t MM_PirStateChange(DM_InputMonitorState_t newState)
{
   ErrorCode_t status = SUCCESS_E;
   osStatus osStat = osErrorOS;
   CO_Message_t *pFireSignalReq = NULL;
   CO_RBUSensorData_t sensorData;
   
   gPIRState = newState;
      
   /* create FireSignalReq message and put into mesh queue */
   pFireSignalReq = osPoolAlloc(AppPool);
   if (pFireSignalReq)
   {
      sensorData.SensorType = CO_PIR_E;
      sensorData.SensorValue = IN_MON_ACTIVE_E == newState ? GPIO_ACTIVE:GPIO_IDLE;
      sensorData.AlarmState = IN_MON_ACTIVE_E == newState ? 1:0;
      sensorData.RUChannelIndex = CO_CHANNEL_PIR_E;
      
      pFireSignalReq->Type = CO_MESSAGE_GENERATE_ALARM_SIGNAL_E;
      memcpy(pFireSignalReq->Payload.PhyDataReq.Data, &sensorData, sizeof(CO_RBUSensorData_t));
      osStat = osMessagePut(AppQ, (uint32_t)pFireSignalReq, 0);
      if (osOK != osStat)
      {
         /* failed to write */
         osPoolFree(AppPool, pFireSignalReq);

         /* Return error code */
         status = ERR_MESSAGE_FAIL_E;
      }
   }
   
   return status;
}

/*************************************************************************************/
/**
* GetConstructionPIRState
* Return the last read value for the construction PIR
*
* @param - None.
*
* @return - The latest reading
*/
DM_InputMonitorState_t GetConstructionPIRState(void)
{
   return gPIRState;
}




