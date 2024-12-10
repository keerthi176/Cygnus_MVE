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
*  File         : MM_PluginInterfaceTask.c
*
*  Description  : Handles wake-up interrupt and communications with head plug-in components
*                 such as detectors and beacon.
*
*************************************************************************************/

#ifdef USE_NEW_HEAD_INTERFACE

/* System Include Files
*************************************************************************************/
#include "stdio.h"
#include "string.h"
#include "stdarg.h"

/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "lptim.h"
#include "DM_SerialPort.h"
#include "DM_LED.h"
#include "MM_PluginInterfaceTask.h"
#include "delay.h"
#include "gpio.h"
#include "board.h"
#include "CO_Message.h"
#include "MM_MeshAPI.h"
#include "CO_HexString.h"
#include "DM_Device.h"
#include "DM_NVM.h"
#include "MM_TimedEventTask.h"
#include "utilities.h"
#include "MM_ApplicationCommon.h"
#include "MM_PluginQueueManager.h"

/* Definitions
*************************************************************************************/
#define HEADINTERFACE_POOL_SIZE   20
#define HEAD_Q_SIZE               10

#define HEAD_PERIODIC_TIMER_PERIOD_MS 10000U
#define MAX_QUEUED_TIMER_MESSAGES 2U
#define MIN_WAKEUP_PULSE_DURATION 295U
#define MAX_WAKEUP_PULSE_DURATION 360U
#define HEAD_INTERRUPTED 0xff

#define HEAD_SETTLING_TIME_INITIAL 30000
#define HEAD_SETTLING_TIME_MS 100U  //Time to wait for non-beacon types to power up, in milliseconds
#define HEAD_SETTLING_TIME_RETRY 15000
#define HEAD_RETRY_PERIOD_SECONDS 5U
#define HEAD_DETECTOR_POLL_PERIOD 10U
#define TAMPER_PERIOD_SECONDS 3U
#define BEACON_SELF_TEST_TIMEOUT 2U
#define BEACON_ACTIVE_FAULT_REQUEST_PERIOD 10U
#define BEACON_POWER_DOWN_DELAY 1U   //beacon heads are powered down BEACON_POWER_DOWN_DELAY seconds after the plugin queue is emptied
#define BEACON_SETTLING_TIME_MS 7U   //Time to wait for beacon to power up, in milliseconds

/* Type Definitions
*************************************************************************************/
typedef enum 
{
   HEAD_TASK_STATE_INITIALISE_E,
   HEAD_TASK_STATE_IDLE_E,
   HEAD_TASK_STATE_WAITING_RESPONSE_E,
   HEAD_TASK_STATE_SLEEP_E,
   HEAD_TASK_STATE_MAX_E
}HeadtaskState_t;


typedef struct
{
   PluginCmdType_t cmdType;
   char message[6];
}PluginCommandMap_t;

/* Private Functions Prototypes
*************************************************************************************/
static void MM_HeadInterfaceTaskMain(void const *argument);                   /* thread function */


static void MM_ResetHeadData(void);
static bool MM_ProcessPluginQueue(void);
static void MM_ProcessHeadQueue(void);
static void MM_QueuePluginMsg(const HeadMessageType_t cmdType, const PluginCmdType_t message, const PluginAccessType_t readWrite, const uint8_t value, const uint8_t transactionID);
static void MM_HeadWakeUpIntIrq(void);
static bool MM_HeadIsSerialNumberValid(const uint32_t SerialNumber);
static bool MM_CheckSmokeThresholds(void);
static bool MM_CheckHeatThresholds(void);
static bool MM_UpdateDetectorThresholds(void);
static void MM_PerformPeriodicTasks(void);
static uint32_t MM_HeadEncodeFirmwareVersion(const uint8_t *payload);
//static void HeadPeriodicTimerCallback(void const *id);
static bool MM_DetectPlugin(void);
static void MM_WaitForPluginInitialPeriod(const uint32_t settling_time);
static void MM_ActivatePlugin(void);
static void MM_DeactivatePlugin(void);
static void MM_ProcessHeadResponse(void);
static bool MM_ReadHeadResponse(PluginMessage_t* message);
static uint8_t MM_HeadGetParameterString(const uint8_t* string, const uint8_t string_len, uint8_t* buffer, 
                                        const uint8_t parameter_number, const uint8_t buffer_size, const uint8_t token);
static void MM_HeadSerialNumber(const PluginMessage_t* message);
static void MM_HeadStatus(const PluginMessage_t* message);
static void MM_HeadSmokeAnaValue(const PluginMessage_t* message);
static void MM_HeadHeatAnaValue(const PluginMessage_t* message);
static void MM_HeadPIRStatus(const PluginMessage_t* message);
static void MM_HeadSoftwareInfo(const PluginMessage_t* message);
static void MM_HeadLEDControl(const PluginMessage_t* message);
static void MM_HeadDeviceType(const PluginMessage_t* message);
static void MM_HeadDeviceClass(const PluginMessage_t* message);
static void MM_HeadDeviceTypeAndClass(const PluginMessage_t* message);
static void MM_HeadPIRReset(const PluginMessage_t* message);
static void MM_HeadEnablePIR(const PluginMessage_t* message);
static void MM_HeadEnableSmoke(const PluginMessage_t* message);
static void MM_HeadEnableHeat(const PluginMessage_t* message);
static void MM_HeadBeaconEnable(const PluginMessage_t* message);
static void MM_HeadOpticalChamber(const PluginMessage_t* message);
static void MM_HeadSwitchControl(const PluginMessage_t* message);
static void MM_HeadIdentificationNumber(const PluginMessage_t* message);
static void MM_HeadResetSource(const PluginMessage_t* message);
static void MM_HeadDisableCommunications(const PluginMessage_t* message);
static void MM_HeadBrandingID(const PluginMessage_t* message);
static void MM_HeadBeaconTestMode(const PluginMessage_t* message);
static void MM_HeadBeaconFlashRate(const PluginMessage_t* message);
static bool MM_HeadBeaconFaultStatus(const PluginMessage_t* message);
static void MM_HeadBeaconType(const PluginMessage_t* message);


static void MM_HeadDecodeOTACmd(const HeadMessage_t* const cmdMsg);
static void MM_SendOTAResponse(const HeadMessage_t *response);
static void MM_SendHeadResponse(const uint32_t value);
static void MM_HeadSend(const PluginMessage_t* const cmd_ref);
static void MM_ProcessHeadStateNormal(void);
static void MM_ProcessHeadStateThreshold(void);
static void MM_ProcessHeadStateFault(void);
static void MM_PerformTamperCheck(void);
static void MM_GenerateFaultSignal(const CO_ChannelIndex_t channelNumber);
static void MM_ReportSmokeSensorFault(void);
static void MM_ReportHeatSensorFault(void);
static void MM_ReportOpticalSensorFault(void);
static void MM_ReportPirSensorFault(void);
static void MM_ReportBeaconFault(void);
static void MM_StartBITTest(void);
static bool MM_BITTestResult(const bool headResponded);
static bool MM_PluginMismatch(void);
static void MM_ProcessApplicationRequest(const HeadMessage_t* const pMessage);
static void MM_SendSensorValueToApplication(const CO_RBUSensorData_t* sensorData);

/* Global Variables
*************************************************************************************/

/* IDs for Message Pools and Queues */
extern osPoolId AppPool;
extern osPoolId MeshPool;
extern osMessageQId(AppQ);

HeadInterfaceState_t HeadInterface_State = HEAD_STATE_STOPPED_E;

/*Head interface task */
osThreadId tid_HeadInterfaceTask;   /* thread id */
osThreadDef(MM_HeadInterfaceTaskMain, osPriorityNormal, 1, 512);   /* thread object */

/* Head Message Queue */
osMessageQDef(HeadQ, HEAD_Q_SIZE, &CO_Message_t);
osMessageQId(HeadQ);
int32_t gHeadQueueCount = 0;
/* Plugin Message Queue */
osMessageQDef(PluginQ, HEAD_Q_SIZE, &PluginMessage_t);
osMessageQId(PluginQ);


osTimerDef(HeadPeriodicTimer, HeadPeriodicTimerCallback);
static bool HeadPeriodicTimerActive = false;
static uint32_t gBeaconPowerDownCount = BEACON_POWER_DOWN_DELAY;
static uint32_t gPluginQueueMessageCount = 0;

/* Private Constants
*************************************************************************************/


/* Private Variables
*************************************************************************************/
uint32_t HeadPeriodicTimerArg = 2;
static osTimerId HeadPeriodicTimerID;

static bool HeadIsIdle = false;
static bool HeadIsEnabled = false;
static uint16_t gDeviceCombination;
static HeadDataObject_t HeadPlugIn;

static bool SerialPortInitialized = false;

static bool BitTestInProgress = false;
static bool gBeaconSelfTestActive = false;
static uint32_t gBeaconSelfTestCount = 0;
static uint32_t gBeaconFaultRequestCount = 0;
static bool DetectorThresholdsSet = false;
static uint32_t HeadDetectRetryCount = 0;
static bool gWakeupPulseActive = false;
/* GPIO pins for Head */
static Gpio_t HeadPowerOn;
static Gpio_t HeadWakeUpPin;
static Gpio_t HeadTxPin;
static bool gForceDetectHead;
static PluginMessage_t gFailedMessage;

const char HeadCommandString[30][48] = 
{
   {"HEAD_WRITE_ENABLED_E"},
   {"HEAD_WRITE_PREALARM_THRESHOLD_E"},
   {"HEAD_WRITE_ALARM_THRESHOLD_E"},
   {"HEAD_WRITE_FAULT_THRESHOLD_E"},
   {"HEAD_WRITE_FLASH_RATE_E"},
   {"HEAD_WRITE_TONE_SELECTION_E"},
   {"HEAD_WRITE_TEST_MODE_E"},
   {"HEAD_WRITE_OUTPUT_ACTIVATED_E"},
   {"HEAD_WRITE_INDICATOR_LED_E"},
   {"HEAD_WRITE_IDENTIFICATION_NUMBER_E"},
   {"HEAD_READ_ENABLED_E"},
   {"HEAD_READ_PREALARM_THRESHOLD_E"},
   {"HEAD_READ_ALARM_THRESHOLD_E"},
   {"HEAD_READ_FAULT_THRESHOLD_E"},
   {"HEAD_READ_FLASH_RATE_E"},
   {"HEAD_READ_TONE_SELECTION_E"},
   {"HEAD_READ_TEST_MODE_E"},
   {"HEAD_READ_ANALOGUE_VALUE_E"},
   {"HEAD_READ_SERIAL_NUMBER_E"},
   {"HEAD_READ_FIRMWARE_VERSION_E"},
   {"HEAD_READ_INDICATOR_LED_E"},
   {"HEAD_READ_FAULT_TYPE_E"},
   {"HEAD_READ_IDENTIFICATION_NUMBER_E"},
   {"HEAD_EVENT_PREALARM_THRESHOLD_EXCEEDED_E"},
   {"HEAD_EVENT_ALARM_THRESHOLD_EXCEEDED_E"},
   {"HEAD_EVENT_FAULT_THRESHOLD_EXCEEDED_E"},
   {"HEAD_RESET_PIR_E"},
   {"HEAD_DAY_NIGHT_SETTING_E"},
   {"HEAD_WAKE_UP_SIGNAL_E"},
   {"HEAD_INTERNAL_MESSAGE_E"}
};

PluginCommandMap_t PluginCommandMap[PARAM_TYPE_MAX_E] = 
{
   /* Command index               Command_string*/
   {HEAD_CMD_SERIAL_NUMBER_E,         ";SENU"},
   {HEAD_CMD_SOFTWARE_INFO_E,         ";I9SI"},
   {HEAD_CMD_DEVICE_TYPE_E,           ";DETY"},
   {HEAD_CMD_DEVICE_CLASS_E,          ";DECL"},
   {HEAD_CMD_SMOKE_UPPER_THRLD_E,     ";STH1"},
   {HEAD_CMD_SMOKE_LOWER_THRLD_E,     ";STH0"},
   {HEAD_CMD_HEAT_UPPER_THRLD_E,      ";HTH1"},
   {HEAD_CMD_HEAT_LOWER_THRLD_E,      ";HTH0"},
   {HEAD_CMD_SMOKE_ANA_VALUE_E,       ";SVAL"},
   {HEAD_CMD_HEAT_ANA_VALUE_E,        ";HVAL"},
   {HEAD_CMD_INDICATOR_CTRL_E,        ";ILED"},
   {HEAD_CMD_DETECTOR_STATUS_E,       ";STAT"},
   {HEAD_CMD_DEVICE_TYPE_CLASS_E,     ";DTAC"},
   {HEAD_CMD_PIR_STATUS_E,            ";PIRS"},
   {HEAD_CMD_RESET_PIR_E,             ";PIRC"},
   {HEAD_CMD_ENABLE_PIR_E,            ";PIRE"},
   {HEAD_CMD_ENABLE_SMOKE_E,          ";ED01"},
   {HEAD_CMD_ENABLE_HEAT_E,           ";ED02"},
   {HEAD_CMD_ENABLE_BEACON_E,         ";ENAB"},
   {HEAD_CMD_OPTICAL_CHBR_STATUS_E,   ";OCS1"},
   {HEAD_CMD_SWITCH_CONTROL_E,        ";OPEC"},
   {HEAD_CMD_ID_NUMBER_E,             ";IDNU"},
   {HEAD_CMD_RESET_SOURCE_E,          ";DRSO"},
   {HEAD_CMD_DISABLE_COMMS_E,         ";CXTO"},
   {HEAD_CMD_BRANDING_ID_E,           ";DBID"},
   {HEAD_CMD_BEACON_TEST_MODE_E,      ";TSTM"},
   {HEAD_CMD_BEACON_FLASH_RATE_E,     ";FLRT"},
   {HEAD_CMD_BEACON_FAULT_STATUS_E,   ";FLTS"},
   {HEAD_CMD_BEACON_TYPE_E,           ";TYPE"}
};


/*************************************************************************************/
/**
* function name   : MM_HeadInterfaceTaskInit(const uint16_t deviceCombination)
* description     : Initialisation of the Head Interface task
*
* @param - const uint16_t deviceCombination
*
* @return - ErrorCode_t  // Returns an error code, else SUCCESS_E
*/
ErrorCode_t MM_HeadInterfaceTaskInit(const uint16_t deviceCombination)
{
   /* Capture device combination setting */
   gDeviceCombination = deviceCombination;
   
   /*create the head Queue*/
   HeadQ = osMessageCreate(osMessageQ(HeadQ), NULL);
   CO_ASSERT_RET_MSG(NULL != HeadQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create Head Queue");

   /*create the plug-in Queue*/
   PluginQ = osMessageCreate(osMessageQ(PluginQ), NULL);
   CO_ASSERT_RET_MSG(NULL != PluginQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create Plugin Queue");
   /*initialise plugin queue*/
   ErrorCode_t result = MM_PLUGINQ_Initialise();
   CO_ASSERT_RET(SUCCESS_E == result, ERR_INIT_FAIL_E);

   /*create the Head Timer*/
   HeadPeriodicTimerID = osTimerCreate(osTimer(HeadPeriodicTimer),osTimerOnce, (void*)HeadPeriodicTimerArg);
   CO_ASSERT_RET_MSG(NULL != HeadPeriodicTimerID, ERR_INIT_FAIL_E, "ERROR - Failed to create Head Timer");
   // Start the periodic timer for periodic checks
   //osTimerStart(HeadPeriodicTimerID, HEAD_PERIODIC_TIMER_PERIOD_MS);
   
   /* Configure TX output pin as normal IO and set to high */
   GpioInit(&HeadTxPin, CN_HEAD_TX, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 1);
   /* Configure HEAD_RX line as input */
   __HAL_GPIO_EXTI_CLEAR_IT(HEAD_WAKE_UP_Pin);
   GpioInit(&HeadWakeUpPin, CN_HEAD_RX, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0);
   
   /* Create thread (return on error) */
   tid_HeadInterfaceTask = osThreadCreate(osThread(MM_HeadInterfaceTaskMain), NULL);
   CO_ASSERT_RET_MSG(NULL != tid_HeadInterfaceTask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create Head Interface Task thread");
   
   CO_PRINT_B_0(DBG_INFO_E, "HeadInterfaceTask created\r\n");
   
   return SUCCESS_E;
}

/*************************************************************************************/
/**
* function name   : MM_ResetHeadData(void)
* description     : Initialise all of the head data
*
* @param    None.
*
* @return   None.
*/
void MM_ResetHeadData(void)
{
   HeadIsEnabled = true;
   gWakeupPulseActive = false;
   uint32_t nvm_value = 0;
   uint32_t channel_flags;
   
   /* Default thresholds */
   HeadPlugIn.HDayNight = CO_DAY_E;
   HeadPlugIn.HSmokeUpperThr[CO_DAY_E]   = HEAD_DEFAULT_SMOKE_UPPER_THRESHOLD_DAY;
   HeadPlugIn.HSmokeUpperThr[CO_NIGHT_E] = HEAD_DEFAULT_SMOKE_UPPER_THRESHOLD_NIGHT;
   HeadPlugIn.HSmokeLowerThr[CO_DAY_E]   = HEAD_DEFAULT_SMOKE_LOWER_THRESHOLD_DAY;
   HeadPlugIn.HSmokeLowerThr[CO_NIGHT_E] = HEAD_DEFAULT_SMOKE_LOWER_THRESHOLD_NIGHT;
   
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R, DC_MATCH_ANY_E) )
   {
      HeadPlugIn.HHeatUpperThr[CO_DAY_E]    = HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_A1R_DAY;
      HeadPlugIn.HHeatUpperThr[CO_NIGHT_E]  = HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_A1R_NIGHT;
      HeadPlugIn.HHeatLowerThr[CO_DAY_E]    = HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_A1R_DAY;
      HeadPlugIn.HHeatLowerThr[CO_NIGHT_E]  = HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_A1R_NIGHT;
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(CO_CHANNEL_HEAT_A1R_E);
      if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
      {
         channel_flags = (( nvm_value & 0xC0000000) >> 27);
         HeadPlugIn.HHeatEnable[CO_DAY_E] = (channel_flags & DM_CHANNEL_DAY_ENABLE_MASK) ? 1:0;
         HeadPlugIn.HHeatEnable[CO_NIGHT_E] = (channel_flags & DM_CHANNEL_NIGHT_ENABLE_MASK) ? 1:0;
      }
   }
   else if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
   {
      HeadPlugIn.HHeatUpperThr[CO_DAY_E]    = HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_B_DAY;
      HeadPlugIn.HHeatUpperThr[CO_NIGHT_E]  = HEAD_DEFAULT_HEAT_UPPER_THRESHOLD_B_NIGHT;
      HeadPlugIn.HHeatLowerThr[CO_DAY_E]    = HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_B_DAY;
      HeadPlugIn.HHeatLowerThr[CO_NIGHT_E]  = HEAD_DEFAULT_HEAT_LOWER_THRESHOLD_B_NIGHT;
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(CO_CHANNEL_HEAT_B_E);
      if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
      {
         channel_flags = (( nvm_value & 0xC0000000) >> 27);
         HeadPlugIn.HHeatEnable[CO_DAY_E] = (channel_flags & DM_CHANNEL_DAY_ENABLE_MASK) ? 1:0;
         HeadPlugIn.HHeatEnable[CO_NIGHT_E] = (channel_flags & DM_CHANNEL_NIGHT_ENABLE_MASK) ? 1:0;
         CO_PRINT_B_3(DBG_INFO_E,"Heat day-night, nvm=%d, enable[DAY]=%d, enable[NIGHT]=%d\r\n", channel_flags, HeadPlugIn.HHeatEnable[CO_DAY_E], HeadPlugIn.HHeatEnable[CO_NIGHT_E]);
      }
   }
   
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_PIR_DETECTOR, DC_MATCH_ANY_E) )
   {
      DM_NVMParamId_t nvm_param = DM_NVM_ChannelIndexToNVMParameter(CO_CHANNEL_PIR_E);
      if ( SUCCESS_E == DM_NVMRead(nvm_param, (void*)&nvm_value, sizeof(nvm_value)) )
      {
         channel_flags = (( nvm_value & 0xC0000000) >> 27);
         HeadPlugIn.HPIREnabled[CO_DAY_E] = (channel_flags & DM_CHANNEL_DAY_ENABLE_MASK) ? 1:0;
         HeadPlugIn.HPIREnabled[CO_NIGHT_E] = (channel_flags & DM_CHANNEL_NIGHT_ENABLE_MASK) ? 1:0;
         CO_PRINT_B_3(DBG_INFO_E,"PIR day-night, nvm=%d, enable[DAY]=%d, enable[NIGHT]=%d\r\n", channel_flags, HeadPlugIn.HPIREnabled[CO_DAY_E], HeadPlugIn.HPIREnabled[CO_NIGHT_E]);
      }
   }

   HeadPlugIn.HSmokeFaultThr             = HEAD_DEFAULT_FAULT_THRESHOLD;
   HeadPlugIn.HHeatFaultThr              = HEAD_DEFAULT_FAULT_THRESHOLD;
   HeadPlugIn.HSmokeAlarmState           = HEAD_ACTIVATION_NORMAL_E;
   HeadPlugIn.HHeatAlarmState            = HEAD_ACTIVATION_NORMAL_E;
   HeadPlugIn.HSmokeFaultStatus          = HEAD_FAULT_CLEAR;
   HeadPlugIn.HHeatFaultStatus           = HEAD_FAULT_CLEAR;
   HeadPlugIn.HPIRFaultStatus            = HEAD_FAULT_CLEAR;
   
   HeadPlugIn.HMismatchedType = false;
   HeadPlugIn.HResponding = true;
   gBeaconPowerDownCount = BEACON_POWER_DOWN_DELAY;
   gForceDetectHead = false;
   
}


/*************************************************************************************/
/**
* function name   : MM_HeadInterfaceTaskMain(void const *argument)
* description     : Main function for Head Interface task
*
* @param - argument is not used.
*
* @return - nothing
*/
void MM_HeadInterfaceTaskMain(void const *argument)
{
   HeadtaskState_t task_state = HEAD_TASK_STATE_INITIALISE_E;
   
   CO_PRINT_A_0(DBG_INFO_E, "Plugin task running\r\n");
   
   while(1)
   {
      switch ( task_state )
      {
         case HEAD_TASK_STATE_INITIALISE_E:
            MM_ResetHeadData();
            HeadIsEnabled = false;
            gForceDetectHead = false;
            SerialPortInitialized = false;
            if( MM_DetectPlugin() )
            {
               gForceDetectHead = false;
               MM_WaitForPluginInitialPeriod(HEAD_SETTLING_TIME_INITIAL);
               //Request device type and class
               MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_DEVICE_TYPE_CLASS_E, HEAD_CMD_TYPE_READ_E, 0, 0);
               // Start the BIT
               MM_HeadInterface_BuiltInTestReq();
            }
            else
            {
               //The head wsn't detected.  Beacon types won't be routinely retried, so set a flag to force retries.
               if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
               {
                  gForceDetectHead = true;
               }
            }
            
               
            task_state = HEAD_TASK_STATE_IDLE_E;
            break;
         case HEAD_TASK_STATE_IDLE_E:
//            CO_PRINT_B_0(DBG_INFO_E,"HEAD_STATE_IDLE_E\r\n");
         
            if ( MM_ProcessPluginQueue() )
            {
               task_state = HEAD_TASK_STATE_WAITING_RESPONSE_E;
            }
            else 
            {
               MM_ProcessHeadQueue();
            }
            break;
         case HEAD_TASK_STATE_WAITING_RESPONSE_E:
            MM_ProcessHeadResponse();
            task_state = HEAD_TASK_STATE_SLEEP_E;
         break;
         case HEAD_TASK_STATE_SLEEP_E:
            // close the plugin USART
            MM_DeactivatePlugin();
//            CO_PRINT_B_0(DBG_INFO_E,"HEAD_TASK_STATE_SLEEP_E\r\n");
            HeadIsIdle = true;
            osDelay(10);
            HeadIsIdle = false;
            task_state = HEAD_TASK_STATE_IDLE_E;
         break;
         default:
            task_state = HEAD_TASK_STATE_IDLE_E;
         break;
      }
   }
}


/*************************************************************************************/
/**
* function name   : MM_ProcessPluginQueue(void)
* description     : 
*
* @param    None.
*
* @return   True if the funtion initiated a head command that expects a response.
*/
bool MM_ProcessPluginQueue(void)
{
   bool response_expected = false;
   PluginMessage_t message;
   
   if ( SUCCESS_E == MM_PLUGINQ_Pop(&message) )
   {
      MM_HeadSend(&message);
      response_expected = true;
      
      if ( 0 < gPluginQueueMessageCount )
      {
         gPluginQueueMessageCount--;
      }
   }
   return response_expected;
}



/*************************************************************************************/
/**
* function name   : MM_ProcessHeadQueue(void)
* description     : 
*
* @param    None.
*
* @return   None.
*/
void MM_ProcessHeadQueue(void)
{
   osEvent event;
   static CO_Message_t* pDataMessage;
   static HeadMessage_t commandMessage;
   HeadIsIdle = true;
   
   
   event = osMessageGet(HeadQ,osWaitForever);
   HeadIsIdle = false;
   if ( (osEventMessage == event.status) && (NULL != event.value.p) )
   {
      gHeadQueueCount--;
      pDataMessage = (CO_Message_t*)event.value.p;
      if (pDataMessage)
      {
         switch ( pDataMessage->Type )
         {
            case CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E:
               CO_PRINT_B_0(DBG_INFO_E, ">>>Head OTA received<<<\r\n\r\n");
               osDelay(5);
               
               memcpy(&commandMessage, pDataMessage->Payload.PhyDataReq.Data, sizeof(HeadMessage_t));
               CO_PRINT_B_5(DBG_INFO_E, "CMD %d,%d,%d,%d,%d\r\n", commandMessage.MessageType,commandMessage.TransactionID,commandMessage.ProfileIndex, commandMessage.ChannelNumber, commandMessage.Value);
               osDelay(5);
               /* Decode command */
               MM_HeadDecodeOTACmd(&commandMessage);
               break;
            case CO_MESSAGE_RUN_BUILT_IN_TESTS_E:
               MM_StartBITTest();
               break;
            case CO_MESSAGE_GENERATE_STATUS_INDICATION_E:
               if ( false == HeadIsEnabled )
               {
                  CO_PRINT_A_0(DBG_INFO_E, ">>>HEAD: Wake-up pulse detected while HeadIsEnabled=false<<<\r\n");
#ifdef SHOW_HEAD_TRAFFIC_ON_PPU_USART
                  SerialDebug_Print(PPU_UART_E, DBG_INFO_E, ">>>HEAD: Wake-up pulse detected while HeadIsEnabled=false<<<\r\n");
#endif
                  HeadIsEnabled = true;
               }
               else
               {
                  CO_PRINT_A_0(DBG_INFO_E, ">>>HEAD: Wake-up pulse detected<<<\r\n");
#ifdef SHOW_HEAD_TRAFFIC_ON_PPU_USART
                  SerialDebug_Print(PPU_UART_E, DBG_INFO_E, ">>>HEAD: Wake-up pulse detected<<<\r\n");
#endif
               }
//               osDelay(1);
               HeadPlugIn.HStatus_State = HEAD_INTERRUPTED;
               MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_DETECTOR_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, 0);
               break;
            case CO_MESSAGE_GENERATE_FAULT_SIGNAL_E:
               memcpy(&commandMessage, pDataMessage->Payload.PhyDataReq.Data, sizeof(HeadMessage_t));
               MM_GenerateFaultSignal((CO_ChannelIndex_t)commandMessage.ChannelNumber);
               break;
            case CO_MESSAGE_TIMER_EVENT_E:
               MM_PerformPeriodicTasks();
               break;
            case CO_MESSAGE_APPLICATION_REQUEST_E:
               memcpy(&commandMessage, pDataMessage->Payload.PhyDataReq.Data, sizeof(HeadMessage_t));
               MM_ProcessApplicationRequest(&commandMessage);
               break;
            default:
               CO_PRINT_A_1(DBG_ERROR_E, ">>>HEAD: Unknown Command = %d<<<\r\n", pDataMessage->Type);
               break;
         }
         
         /* Free this entry */
         osPoolFree(AppPool, pDataMessage);
      }
   }
   else
   {
      CO_PRINT_A_0(DBG_INFO_E, ">>>No Event<<<\r\n\r\n");
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadSend(const PluginMessage_t *pCommand)
* description     : 
*
* @param -  pCommand - Command object which contains data to be sent
*
* @return - None.
*/
static void MM_HeadSend(const PluginMessage_t *pCommand)
{
   const uint8_t crlf[] = "\r\n";
   char command[16];
   
   //Clear the command string
   memset(command,0,16);
   
   if ( HeadIsEnabled )
   {
      if ( pCommand )
      {
         /* Prepare the command */
         memcpy(command,PluginCommandMap[pCommand->PluginMessageType].message, 5);
         switch ( pCommand->readWrite )
         {
            case HEAD_CMD_TYPE_WRITE_E:
               command[5] = '=';
               if ( (HEAD_CMD_SMOKE_UPPER_THRLD_E == pCommand->PluginMessageType) ||
                    (HEAD_CMD_SMOKE_LOWER_THRLD_E == pCommand->PluginMessageType) ||
                    (HEAD_CMD_HEAT_UPPER_THRLD_E == pCommand->PluginMessageType) ||
                    (HEAD_CMD_HEAT_LOWER_THRLD_E == pCommand->PluginMessageType) )
               {
                  sprintf(&command[6],"%02d",pCommand->value);
               }
               else
               {
                  sprintf(&command[6],"%d",pCommand->value);
               }
               break;
            case HEAD_CMD_TYPE_SPECIAL_E:
               command[5] = '+';
               break;
            case HEAD_CMD_TYPE_READ_E:// Intentional drop-through
            default:
               //read
               command[5] = '?';
            break;
         }
         
         // add CR LF 
         strcat((char*)command,(const char*)crlf);
         
         CO_PRINT_B_1(DBG_INFO_E,"Sending to head - %s", command);// nor CR/LF because the head message contains it.
#ifdef SHOW_HEAD_TRAFFIC_ON_PPU_USART
         SerialDebug_Print(PPU_UART_E, DBG_INFO_E, "Sending to head - %s", command);
#endif
         // Wake up serial port
         MM_ActivatePlugin();
         /* Request a new transmission from the serial driver */
         SerialPortWriteTxBuffer(HEAD_UART_E, (uint8_t*)command, strlen(command));
      }
   }
}


/*************************************************************************************/
/**
* function name   : MM_HeadWakeUpIntIrq(void)
* description     : Head interrupt wake-up handler function
*
* @param - none
*
* @return - nothing
*/
static void MM_HeadWakeUpIntIrq(void)
{
   static uint32_t start_sync = 0;
   uint32_t time_now;
   uint32_t time_delta;
   
   time_now = LPTIM_ReadCounter(&hlptim1);
   
   /* Read pin level and send a StartSync or StopSync to the HeadQ */
   if(HAL_GPIO_ReadPin(HEAD_WAKE_UP_GPIO_Port, HEAD_WAKE_UP_Pin) == GPIO_PIN_RESET)
   {
      //the beacon doesn't send a wake-up pulse but we get here if it's unplugged
      //so don't do the pulse start if we are a beacon
      if ( false == DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
      {
         if ( 0 == start_sync )
         {
            //Start of a new interrupt pulse
            start_sync = time_now;
            gWakeupPulseActive = true;
         }
         else 
         {
            //The wake-up pulse was already active.
            //We shouldn't have received another falling edge.  The pulse is too short, cancel the wake-up check.
            start_sync = 0;
            gWakeupPulseActive = false;
         }
      }
   }
   else if ( (0 != start_sync) )
   {
      /* Disable Interrupt */
      //HAL_NVIC_DisableIRQ(EXTI0_IRQn);
      
      if (time_now < start_sync)
      {
         time_now += 0xffff;
      }
      time_delta = time_now - start_sync;
      if ( MIN_WAKEUP_PULSE_DURATION < time_delta && MAX_WAKEUP_PULSE_DURATION > time_delta )
      {
         CO_Message_t* pPhyDataReq = osPoolAlloc(AppPool);
         if (pPhyDataReq)
         {
            pPhyDataReq->Type = CO_MESSAGE_GENERATE_STATUS_INDICATION_E;
            pPhyDataReq->Payload.PhyDataReq.Size = 0;

            osStatus osStat = osMessagePut(HeadQ, (uint32_t)pPhyDataReq, 0);
            
            if (osOK != osStat)
            {
               /* failed to put message in the head queue */
               osPoolFree(AppPool, pPhyDataReq);
            }
            else
            {
               gHeadQueueCount++;
            }
         }
      }
      //reset start sync
      start_sync = 0;
      gWakeupPulseActive = false;
   }
}

/*************************************************************************************/
/**
* function name   : uint8_t MM_HeadGetParameterString(uint8_t* string, uint8_t string_len, uint8_t* buffer, 
*                                                     uint8_t parameter_number, uint8_t buffer_size)
* description     : Search for a string parameter within a coma separated string
*
* @param - uint8_t* string - Comma separated input string
* @param - uint8_t string_len - Length of the comma separated string
* @param - uint8_t* buffer - Buffer to be populated (output)
* @param - uint8_t parameter_number - Parameter index to be serached and returned in the output buffer
* @param - uint8_t buffer_size - Size of the output buffer
* @param - uint8_t token - descriminator character (i.e. comma)
*
* @return - Number of bytes written to buffer
*/
uint8_t MM_HeadGetParameterString(const uint8_t* string, const uint8_t string_len, uint8_t* buffer, 
                                        const uint8_t parameter_number, const uint8_t buffer_size, const uint8_t token) 
{
   uint8_t i = 0;
   uint8_t j = 0;
   uint8_t comma_counter = 0;
   
   if ( string && buffer )
   {
      /* Find out number of comas within the string */
      while (i < string_len && comma_counter != (parameter_number - 1))  
      {                   
         if (string[i] == token) 
         {
            comma_counter++;
         }
         i++;
      }
            
      if (i >= string_len)
      {
         return 0;
      }
      /* Populate buffer with string data requested */
      while ((i + j) < string_len && string[i + j] != token && string[i + j] != 0x00 && j < (buffer_size - 1)) 
      {
         buffer[j] = string[i + j];
         j++;
      }
      buffer[j] = 0;
   }
   return j;
}


/*************************************************************************************/
/**
* function name   : MM_HeadSerialNumber(const PluginMessage_t* message)
* description     : Captures serial number of the plug-in module
*
* @param -  message - data message containing serial number info from the plug-in
*
* @return - None.
*/
static void MM_HeadSerialNumber(const PluginMessage_t* message)
{
   if ( message )
   {
      uint8_t buffer[8];
      uint8_t yy, mm, bb;
      uint16_t sn;
      /* Extract first two parameters - YY and MM */
      MM_HeadGetParameterString((const uint8_t*)message->buffer, PLUGIN_MSG_BUFFER_SIZE, buffer, 1, sizeof(buffer), '-');
      mm = atoi((char*)buffer+2);
      buffer[2] = 0x00;
      yy = atoi((char*)buffer);
      /* Extract second parameter - BB */
      MM_HeadGetParameterString((const uint8_t*)message->buffer, PLUGIN_MSG_BUFFER_SIZE, buffer, 2, sizeof(buffer), '-');
      bb = atoi((char*)buffer);
      /* Extract third parameter - SN */
      MM_HeadGetParameterString((const uint8_t*)message->buffer, PLUGIN_MSG_BUFFER_SIZE, buffer, 3, sizeof(buffer), '-');
      sn = atoi((char*)buffer);
      /* Populate uint32 serial number */
      HeadPlugIn.HSerialNumber = 0;
      HeadPlugIn.HSerialNumber = (HeadPlugIn.HSerialNumber | yy) << 4; 
      HeadPlugIn.HSerialNumber = (HeadPlugIn.HSerialNumber | mm) << 5;
      HeadPlugIn.HSerialNumber = (HeadPlugIn.HSerialNumber | bb) << 16;
      HeadPlugIn.HSerialNumber = (HeadPlugIn.HSerialNumber | sn);
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadIsSerialNumberValid
* description     : checks if a serial number is valid
*
* @param - SerialNumber: Serial number to check
*
* @return - true if the serial number is valid
*/
static bool MM_HeadIsSerialNumberValid(const uint32_t SerialNumber)
{
   bool ValidSerialNumber = true;
   uint8_t yy, mm, bb;
   uint16_t sn;

   /* decode the uint32 serial number */
   yy = (HeadPlugIn.HSerialNumber >> 25) & 0x7Fu; 
   mm = (HeadPlugIn.HSerialNumber >> 21) & 0x0Fu; 
   bb = (HeadPlugIn.HSerialNumber >> 16) & 0x1Fu; 
   sn =  HeadPlugIn.HSerialNumber & 0xFFFFu; 

   CO_PRINT_B_4(DBG_INFO_E, "Head SRN is %02d%02d %02d %05d\r\n", yy,mm,bb,sn);

   /* Check the Year of Manufacture field */
   if (yy>99u || yy<17u)
   {
      /* Year of Manufacture is not valid */
      ValidSerialNumber = false;
   }
   else
   {
      /* Check the Month of Manufacture field */
      if (mm>12u || mm<1u)
      {
         /* Month of Manufacture is not valid */
         ValidSerialNumber = false;
      }
      else
      {
         /* Check the Batch field */
         if (bb>31u || bb<1u)
         {
            /* Batch number is not valid */
            ValidSerialNumber = false;
         }
         else
         {
            /* Check the actual serial number field */
            if (sn == 0u)
            {
               /* Serial Number is not valid */
               ValidSerialNumber = false;
            }
         }
      }
   }
   
   return ValidSerialNumber;
}

/*************************************************************************************/
/**
* function name   : MM_HeadStatus(const PluginMessage_t* message)
* description     : Captures head module status
*
* @param - message  The message containing the response from the head.
*
* @return - None.
*/
void MM_HeadStatus(const PluginMessage_t* message)
{
   uint8_t size;
   if ( message )
   {
      size = strlen((char*)message->buffer);
      uint8_t buffer[5];
      /* Extract information from reply */
      MM_HeadGetParameterString(message->buffer, size, buffer, 1, sizeof(buffer), ',');
      HeadPlugIn.HStatus_State = atoi((char*)buffer);
      MM_HeadGetParameterString(message->buffer, size, buffer, 2, sizeof(buffer), ',');
      HeadPlugIn.HStatus_Device = atoi((char*)buffer);
      CO_PRINT_B_2(DBG_INFO_E, "Head Status: %d,%d\r\n", HeadPlugIn.HStatus_State, HeadPlugIn.HStatus_Device);
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadSmokeAnaValue
* description     : Records the analogue value reported by the detector
*
* @param - message  The message containing the response from the head.
*
* @return - None.
*/
void MM_HeadSmokeAnaValue(const PluginMessage_t* message)
{
   if ( message )
   {
      int32_t analogueValue = atoi((char*)message->buffer);
      HeadPlugIn.HSmokeAnalogueValue = analogueValue;
      CO_PRINT_B_1(DBG_INFO_E, "Hd Smoke An val:%s\r\n", message->buffer);
      CO_RBUSensorData_t sensorData;
      sensorData.SensorType = CO_SMOKE_E;
      sensorData.RUChannelIndex = CO_CHANNEL_SMOKE_E;
      sensorData.SensorValue = analogueValue;
      MM_SendSensorValueToApplication(&sensorData);
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadHeatAnaValue
* description     : Records the analogue value reported by the detector
*
* @param - message  The message containing the response from the head.
*
* @return - None.
*/
void MM_HeadHeatAnaValue(const PluginMessage_t* message)
{
   if ( message )
   {
      int32_t analogueValue = atoi((char*)message->buffer);
      HeadPlugIn.HHeatAnalogueValue = analogueValue;
      CO_PRINT_B_1(DBG_INFO_E, "Hd Heat An val: %s\r\n", message->buffer);
      CO_RBUSensorData_t sensorData;
      if ( 1 == HeadPlugIn.HClass )
      {
         sensorData.SensorType = CO_HEAT_A1R_E;
         sensorData.RUChannelIndex = CO_CHANNEL_HEAT_A1R_E;
      }
      else  
      {
         sensorData.SensorType = CO_HEAT_B_E;
         sensorData.RUChannelIndex = CO_CHANNEL_HEAT_B_E;
      }
      sensorData.SensorValue = analogueValue;
      MM_SendSensorValueToApplication(&sensorData);
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadPIRStatus
* description     : Records the status reported by the PIR detector
*
* @param - message  The message containing the response from the head.
*
* @return - None.
*/
void MM_HeadPIRStatus(const PluginMessage_t* message)
{
   if ( message )
   {
      int32_t analogueValue = atoi((char*)message->buffer);
      if ( analogueValue != HeadPlugIn.HPIRStatus )
      {
         HeadPlugIn.HPIRStatus = analogueValue;
         CO_PRINT_B_1(DBG_INFO_E, "Head PIR Status: %s\r\n", message->buffer);
         MM_SendAlarmSignal(CO_CHANNEL_PIR_E, CO_PIR_E, HeadPlugIn.HPIRStatus, HeadPlugIn.HPIRStatus);
      }
      else 
      {
         //The alarm state hasn't changed.  Just forward the readin to the Application
         CO_RBUSensorData_t sensorData;
         sensorData.SensorType = CO_PIR_E;
         sensorData.RUChannelIndex = CO_CHANNEL_PIR_E;
         sensorData.SensorValue = analogueValue;
         MM_SendSensorValueToApplication(&sensorData);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadSoftwareInfo
* description     : Captures software information of the head
*
* @param -  message  The message containing the response from the head.
*
* @return - None
*/
void MM_HeadSoftwareInfo(const PluginMessage_t* message)
{
   if ( message )
   {
      CO_PRINT_B_1(DBG_INFO_E, "Head Software Version: %s\r\n", message->buffer);
      HeadPlugIn.HFirmwareVersion = MM_HeadEncodeFirmwareVersion(message->buffer);
   }
}  

/*************************************************************************************/
/**
* function name   : MM_HeadLEDControl
* description     : Head (Detectors-2 LEDs) LED control function
*
* @param - message  The message containing the response from the head.
*
* @return - None.
*/
void MM_HeadLEDControl(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HLEDStatus = message->value;
         CO_PRINT_B_1(DBG_INFO_E, "Head LED status: %d\r\n", HeadPlugIn.HLEDStatus);
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HLEDStatus = atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head LED status: %d\r\n", HeadPlugIn.HLEDStatus);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadDeviceType
* description     : Captures head device type information
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadDeviceType(const PluginMessage_t* message)
{
   if ( message )
   {
      HeadPlugIn.HType = atol((char*)message->buffer);
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadDeviceClass
* description     : Captures head device class information
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadDeviceClass(const PluginMessage_t* message)
{
   if ( message )
   {
      HeadPlugIn.HClass = atoi((char*)message->buffer);
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadDeviceTypeAndClass
* description     : Captures head device type and class information
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadDeviceTypeAndClass(const PluginMessage_t* message)
{
   uint8_t size;
   if ( message )
   {
      size = strlen((char*)message->buffer);
      uint8_t buffer[5];
      /* Extract information from reply */
      MM_HeadGetParameterString(message->buffer, size, buffer, 1, sizeof(buffer), ',');
      HeadPlugIn.HType = atoi((char*)buffer);
      MM_HeadGetParameterString(message->buffer, size, buffer, 2, sizeof(buffer), ',');
      HeadPlugIn.HClass = atoi((char*)buffer);
      CO_PRINT_B_2(DBG_INFO_E, "Head Type:%d, Class:%d\r\n", HeadPlugIn.HType, HeadPlugIn.HClass);
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadPIRReset
* description     : Captures head pir reset response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadPIRReset(const PluginMessage_t* message)
{
   if ( message )
   {
      CO_PRINT_B_1(DBG_INFO_E,"MM_HeadPIRReset %s\r\n", message->buffer);
      int32_t error = strncmp((char*)message->buffer, "OK", 2);
      if ( 0 == error )
      {
         /* The pir reset succeded.  Set the local cached status to 'not triggered' */
         HeadPlugIn.HPIRStatus = 0;
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadEnablePIR
* description     : Captures head pir enable response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadEnablePIR(const PluginMessage_t* message)
{
   if ( message )
   {
      int32_t error = strncmp((char*)message->buffer, "ERROR", 5);
      if ( 0 != error )
      {
         CO_PRINT_B_1(DBG_INFO_E,"MM_HeadEnablePIR %s\r\n", message->buffer);
         HeadPlugIn.HPIREnable = (uint8_t)atoi((char*)message->buffer);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadEnableSmoke
* description     : Captures head smoke enable response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadEnableSmoke(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HSmokeEnable = message->value;
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HSmokeEnable = atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Smoke Enable: %d\r\n", HeadPlugIn.HSmokeEnable);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadEnableHeat
* description     : Captures head heat enable response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadEnableHeat(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HHeatEnable[HeadPlugIn.HDayNight] = message->value;
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HHeatEnable[HeadPlugIn.HDayNight] = atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Heat Enable: %d\r\n", HeadPlugIn.HHeatEnable[HeadPlugIn.HDayNight]);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadBeaconEnable
* description     : Captures head beacon enable response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadBeaconEnable(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HBeaconEnable = message->value;
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HBeaconEnable = (uint8_t)atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Beacon Enable: %d\r\n", HeadPlugIn.HBeaconEnable);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadOpticalChamber
* description     : Captures head Optical Chamber response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadOpticalChamber(const PluginMessage_t* message)
{
   if ( message )
   {
      uint8_t previous_status = HeadPlugIn.HOptChamberStatus;
      
      HeadPlugIn.HOptChamberStatus = atoi((char*)message->buffer);
      CO_PRINT_B_1(DBG_INFO_E, "Head Optical Chamber Status: %s\r\n", message->buffer);
      
      if ( previous_status != HeadPlugIn.HOptChamberStatus )
      {
         MM_ReportOpticalSensorFault();
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadSwitchControl
* description     : Captures head switch control response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadSwitchControl(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HEnabled = message->value;
         CO_PRINT_B_1(DBG_INFO_E, "Head Enabled status set : %d\r\n", HeadPlugIn.HEnabled);
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head Switch Control reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HEnabled = atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Enabled status: %d\r\n", HeadPlugIn.HEnabled);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadIdentificationNumber
* description     : Captures head ID Number response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadIdentificationNumber(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HIdentificationNumber = message->value;
         CO_PRINT_B_1(DBG_INFO_E, "Head ID number set: %d\r\n", HeadPlugIn.HIdentificationNumber);
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head ID Number reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HIdentificationNumber = atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head ID Number: %x\r\n", HeadPlugIn.HIdentificationNumber);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadResetSource
* description     : Captures head Reset Source response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadResetSource(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HResetSource = message->value;
         CO_PRINT_B_1(DBG_INFO_E, "Head Reset source set: %d\r\n", HeadPlugIn.HResetSource);
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head Reset Source reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HResetSource = (uint8_t)atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Reset Source: %x\r\n", HeadPlugIn.HResetSource);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadDisableCommunications
* description     : Captures head Disable Comms response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadDisableCommunications(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         CO_PRINT_B_0(DBG_INFO_E, "Head Disable Communications reply: OK\r\n");
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head Disable Communications reply: ERROR\r\n");
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadBrandingID
* description     : Captures head Branding ID response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadBrandingID(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HBrandingID = message->value;
         CO_PRINT_B_1(DBG_INFO_E, "Head Branding ID set: %d\r\n", HeadPlugIn.HBrandingID);
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head Branding ID reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HResetSource = (uint8_t)atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Branding ID: %x\r\n", HeadPlugIn.HBrandingID);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadBeaconTestMode
* description     : Captures head beacon test mode response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadBeaconTestMode(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         if ( HEAD_BEACON_TEST_INACTIVE_E == message->value )
         {
            if ( HEAD_BEACON_SELF_TEST_ACTIVE_E == HeadPlugIn.HBeaconTestMode )
            {
               gBeaconSelfTestActive = false;
            }
            HeadPlugIn.HBeaconTestMode = HEAD_BEACON_TEST_INACTIVE_E;
         }
         else 
         {
            if ( gBeaconSelfTestActive )
            {
               //Self test initiated
               HeadPlugIn.HBeaconTestMode = HEAD_BEACON_SELF_TEST_ACTIVE_E;
            }
            else 
            {
               //Normal operating test while beacon is flashing
               HeadPlugIn.HBeaconTestMode = HEAD_BEACON_TEST_ACTIVE_E;
            }
         }
         gBeaconSelfTestCount = 0;
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HBeaconTestMode = (uint8_t)atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Beacon Test Mode: %d\r\n", HeadPlugIn.HBeaconTestMode);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadBeaconFlashRate
* description     : Captures head beacon flash rate response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadBeaconFlashRate(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HBeaconFlashRate = message->value;
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HBeaconFlashRate = (uint8_t)atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Beacon Enable: %d\r\n", HeadPlugIn.HBeaconFlashRate);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadBeaconFaultStatus
* description     : Captures head beacon fault status response
*
* @param - message  The message containing the response from the head.
*
* @return - bool : True if the fault status changed from its previous value
*/
bool MM_HeadBeaconFaultStatus(const PluginMessage_t* message)
{
   bool fault_status_changed = false;
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HBeaconFaultStatus = message->value;
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         uint8_t new_state = (uint8_t)atoi((char*)message->buffer);
         
         if ( HeadPlugIn.HBeaconFaultStatus != new_state )
         {
            fault_status_changed = true;
         }
         
         HeadPlugIn.HBeaconFaultStatus = new_state;
         CO_PRINT_B_1(DBG_INFO_E, "Head Beacon Fault Status: %d\r\n", HeadPlugIn.HBeaconFaultStatus);
      }
      
      //If the beacon is in self test, switch it off
      if ( HEAD_BEACON_SELF_TEST_ACTIVE_E == HeadPlugIn.HBeaconTestMode )
      {
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_BEACON_TEST_MODE_E, HEAD_CMD_TYPE_WRITE_E, HEAD_BEACON_TEST_INACTIVE_E, 0);
         //Stop the beacon
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_ENABLE_BEACON_E, HEAD_CMD_TYPE_WRITE_E, 0, 0);
         gBeaconSelfTestActive = false;
      }
   }
   
   return fault_status_changed;
}

/*************************************************************************************/
/**
* function name   : MM_HeadBeaconType
* description     : Captures head beacon type response
*
* @param - message  The message containing the response from the head.
*
* @return - nothing
*/
void MM_HeadBeaconType(const PluginMessage_t* message)
{
   if ( message )
   {
      if(!strncmp((const char*)"OK", (const char*)message->buffer, 2))
      {
         HeadPlugIn.HBeaconType = message->value;
      }
      else if(!strncmp((const char*)"ERROR", (const char*)message->buffer, 5))
      {
         CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
      }
      else
      {
         HeadPlugIn.HBeaconType = (uint8_t)atoi((char*)message->buffer);
         CO_PRINT_B_1(DBG_INFO_E, "Head Beacon Type: %d\r\n", HeadPlugIn.HBeaconType);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_HeadInterface_CanGoToSleep
* description     : Checks if this module allows the system to enter the sleep mode
*
* @param - none
*
* @return - true if conditions are met to enter sleep mode
*/
bool MM_HeadInterface_CanGoToSleep(void)
{
   if ( HeadIsEnabled )
   {
      return HeadIsIdle;
   }
   return true;
}

/*************************************************************************************/
/**
* function name   : MM_HeadInterface_BuiltInTestReq
* description     : Requests the built-in test of the Head Interface
*
* @param - none
*
* @return - Error code
*/
ErrorCode_t MM_HeadInterface_BuiltInTestReq(void)
{
   ErrorCode_t ErrorCode = ERR_BUILT_IN_TEST_FAIL_E;

   CO_Message_t* pPhyDataReq = osPoolAlloc(AppPool);
   if (pPhyDataReq)
   {
      pPhyDataReq->Type = CO_MESSAGE_RUN_BUILT_IN_TESTS_E;
      uint8_t* pMsgData = pPhyDataReq->Payload.PhyDataReq.Data;
      memset(pMsgData, 0u, sizeof(HeadMessage_t));
      pPhyDataReq->Payload.PhyDataReq.Size = sizeof(HeadMessage_t);
      
      osStatus osStat = osMessagePut(HeadQ, (uint32_t)pPhyDataReq, 0);
      
      if (osOK != osStat)
      {
         /* failed to put message in the head queue */
         osPoolFree(AppPool, pPhyDataReq);
      }
      else
      {
         gHeadQueueCount++;
         ErrorCode = SUCCESS_E;
      }
   }
   
   return ErrorCode;
}

/*************************************************************************************/
/**
* function name   : MM_HeadEncodeFirmwareVersion
* description     : Converts the version string from the head into a uint32_t
*                    The payload arrives in the format 01.01.01,02/03/19
*                    This function converts the character values to decimal representation.  
*                    They are then stored in the returned uint32_t with the following 
*                    breakdown, msb to lsb.
*                    major=2 bits, minor=7 bits, increment=7 bits, day=5 bits, month=4 bits, year=7 bits.
*
* @param - uint8_t *payload - data from head
*
* @return - uint32_t The converted version
*/
uint32_t MM_HeadEncodeFirmwareVersion(const uint8_t *payload)
{
   uint32_t version = 0;
   if ( payload )
   {
      /*extract the major revision*/
      char* ptr = (char*)payload;
      uint32_t major = atoi(ptr);
      /*extract the minor revision*/
      ptr += 3;
      uint32_t minor = atoi(ptr);
      /*extract the increment revision*/
      ptr += 3;
      uint32_t increment = atoi(ptr);
      /*extract the day*/
      ptr += 3;
      uint32_t day = atoi(ptr);
      /*extract the month*/
      ptr += 3;
      uint32_t month = atoi(ptr);
      /*extract the year*/
      ptr += 3;
      uint32_t year = atoi(ptr);
      /*do a sanity check on the figures */
      if ( ( 100 > major ) &&
           ( 100 > minor ) &&
           ( 100 > increment ) &&
           ( 32 > day ) &&
           ( 13 > month ) &&
           ( 99 > year ) )
      {
         /* encode the version number */
         version = year;
         version += (month << 7);
         version += (day << 11);
         version += (increment << 16);
         version += (minor << 23);
         version += (major << 30);
      }
   }
   
   return version;
}

/*************************************************************************************/
/**
* function name   : MM_HeadDecodeFirmwareVersion
* description     : Converts the stored uint32_t version number into a string.
*                   Details of the binary storage are described by MM_HeadEncodeFirmwareVersion.
*                   The returned format is "01.01.01,02/03/19".
*
* @param - uint32_t version         - binary format of the version number
* @param - uint8_t version_str      - [OUT] string format of the version number
* @param - uint32_t ver_str_length  - length of the version_str buffer in bytes.
*
* @return - bool - True if the conversion was successful
*/
bool MM_HeadDecodeFirmwareVersion(const uint32_t version, const uint8_t *version_str, const uint32_t ver_str_length)
{
   bool result = false;
   
   if ( version_str )
   {
      /* decode the major revision */
      uint32_t major = version >> 30;
      uint32_t minor = (version >> 23) & 0x7f;
      uint32_t increment = (version >> 16) & 0x7f;
      uint32_t day = (version >> 11) & 0x1f;
      uint32_t month = (version >> 7) & 0xf;
      uint32_t year = version & 0x7f;
      
      snprintf( (char*)version_str, ver_str_length, "%02d.%02d.%02d,%02d/%02d/%02d",
                                          major, minor, increment, day, month, year);
      result = true;
   }
   
   return result;
}



/*************************************************************************************/
/**
* function name   :  MM_CheckSmokeThresholds
* description     :  Test last recored analogue value from the smoke sensor against
*                    the upper and lower smoke thresholds.  If necessary, issue a 
*                    fire signal and move the thresholds in the sensor.
*
* @param - None..
*
* @return - bool     True if commands were prepared to update the head.
*/
bool MM_CheckSmokeThresholds(void)
{
   bool headUpdateRequired = false;
   
  /* Did the detector go over the upper threshold? */
   if( HeadPlugIn.HSmokeAnalogueValue >= HeadPlugIn.HSmokeUpperThr[HeadPlugIn.HDayNight] )
   {
      if ( HEAD_ACTIVATION_FAULT_E == HeadPlugIn.HSmokeAlarmState )
      {
         /* This is the head recovering from a fault condition to a normal condition */
         HeadPlugIn.HSmokeFaultStatus = HEAD_FAULT_CLEAR;
         HeadPlugIn.HSmokeAlarmState = HEAD_ACTIVATION_NORMAL_E;
         /* Send a fault cleared message to the application */
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E,  CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, 0, false, false );
         /* Set up a command to restore upper threshold from FAULT to the UPPER detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeUpperThr[HeadPlugIn.HDayNight], 0);
         headUpdateRequired = true;
#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 0);/* Switch off the head LEDs */
#endif
      }
      else if ( HEAD_ACTIVATION_NORMAL_E == HeadPlugIn.HSmokeAlarmState )
      {
         /* This is a transition from a normal condition to the alarm condition */
         HeadPlugIn.HSmokeAlarmState = HEAD_ACTIVATION_ALARM_E;
         /* Send a new fire value to the control panel */
         MM_SendFireSignal( CO_CHANNEL_SMOKE_E, CO_SMOKE_E, HeadPlugIn.HSmokeAnalogueValue, HeadPlugIn.HSmokeAlarmState );
         /* Set up a command to move the lower threshold from FAULT to the LOWER detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeLowerThr[HeadPlugIn.HDayNight], 0);
         headUpdateRequired = true;

#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 1);/* Switch on the head LEDs */
#endif
      }
   }
   /* Did the detector fall below the lower threshold? */
   else if( HeadPlugIn.HSmokeAnalogueValue < HeadPlugIn.HSmokeLowerThr[HeadPlugIn.HDayNight] )
   {
      /*There are two possible causes to bring us here.
         1) There is no fire condition and the sensor value has crossed below the fault threshold.
            Action = send a detector fault signal to the CIE.
         2) We are in alarm condition, heat is reducing and crossed below the lower threshold.
            Action = Move the lower threshold back to the FAULT level.  Send a 'fire no longer detected' signal to the CIE.
      */
      
      /* Fault condition */
      if ( HeadPlugIn.HSmokeAnalogueValue < HeadPlugIn.HSmokeFaultThr )
      {
         HeadPlugIn.HSmokeFaultStatus = HEAD_FAULT_INDICATED;
         HeadPlugIn.HSmokeAlarmState = HEAD_ACTIVATION_FAULT_E;
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E,  CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, 1, false, false );
         /* Set up a command to move the upper threshold from the UPPER to the FAULT detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeFaultThr, 0);
#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 1);/* Switch on the head LEDs */
#endif
         headUpdateRequired = true;
      }
      else  if ( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HSmokeAlarmState )
      {
         /* This is the head recovering from an alarm condition to a normal condition */
         HeadPlugIn.HSmokeFaultStatus = HEAD_FAULT_CLEAR;
         HeadPlugIn.HSmokeAlarmState = HEAD_ACTIVATION_NORMAL_E;
         /* Send a new fire value to the control panel */
         MM_SendFireSignal( CO_CHANNEL_SMOKE_E, CO_SMOKE_E, HeadPlugIn.HSmokeAnalogueValue, HeadPlugIn.HSmokeAlarmState );
         /* Set up a command to move the lower threshold from the LOWER to the FAULT detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeFaultThr, 0);
         headUpdateRequired = true;
#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 0);/* Switch off the head LEDs */
#endif
      }
      else if ( HEAD_ACTIVATION_NORMAL_E == HeadPlugIn.HSmokeAlarmState )
      {
         MM_UpdateDetectorThresholds();
      }
   }

   
   return headUpdateRequired;
}

/*************************************************************************************/
/**
* function name   :  MM_CheckHeatThresholds
* description     :  Test last recored analogue value from the heat sensor against
*                    the upper and lower heat thresholds.  If necessary, issue a 
*                    fire signal and move the thresholds in the sensor.
*
* @param - headData  A structure containing the sensor readings and channel number.
*
* @return - bool     True if commands were prepared to update the head.
*/
bool MM_CheckHeatThresholds(void)
{
   bool headUpdateRequired = false;
   
   /* Did the detector go over the upper threshold? */
   if( HeadPlugIn.HHeatAnalogueValue >= HeadPlugIn.HHeatUpperThr[HeadPlugIn.HDayNight] )
   {
      if ( HEAD_ACTIVATION_NORMAL_E == HeadPlugIn.HHeatAlarmState )
      {
         /* This is a transition from a normal condition to the alarm condition */
         HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_CLEAR;
         HeadPlugIn.HHeatAlarmState = HEAD_ACTIVATION_ALARM_E;
         /* Send a new fire value to the control panel */
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
         {
            MM_SendFireSignal( CO_CHANNEL_HEAT_B_E, CO_HEAT_B_E, HeadPlugIn.HHeatAnalogueValue, HeadPlugIn.HHeatAlarmState );
         }
         else if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R, DC_MATCH_ANY_E) )
         {
            MM_SendFireSignal( CO_CHANNEL_HEAT_A1R_E, CO_HEAT_A1R_E, HeadPlugIn.HHeatAnalogueValue, HeadPlugIn.HHeatAlarmState );
         }
         /* Set up a command to move the lower threshold from FAULT to the LOWER detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatLowerThr[HeadPlugIn.HDayNight],0);
         headUpdateRequired = true;
         
#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 1);/* Switch on the head LEDs */
#endif
      }
      else if ( HEAD_ACTIVATION_FAULT_E == HeadPlugIn.HHeatAlarmState )
      {
         /* This is the head recovering from a fault condition to a normal condition */
         HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_CLEAR;
         HeadPlugIn.HHeatAlarmState = HEAD_ACTIVATION_NORMAL_E;
         /* Send a fault cleared message to the application */
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
         {
            CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E,  CO_CHANNEL_HEAT_B_E, FAULT_SIGNAL_FAULTY_SENSOR_E, 0, false, false );
         }
         else if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R, DC_MATCH_ANY_E) )
         {
            CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E,  CO_CHANNEL_HEAT_A1R_E, FAULT_SIGNAL_FAULTY_SENSOR_E, 0, false, false );
         }
         /* Set up a command to move the upper threshold from FAULT to the UPPER detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatUpperThr[HeadPlugIn.HDayNight],0);
         headUpdateRequired = true;
#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 0);/* Switch off the head LEDs */
#endif
      }
   }
   /* Did the detector fall below the lower threshold? */
   else if( HeadPlugIn.HHeatAnalogueValue <= HeadPlugIn.HHeatLowerThr[HeadPlugIn.HDayNight] )
   {
      /*There are two possible causes to bring us here.
         1) There is no fire condition and the sensor value has crossed below the fault threshold.
            Action = send a detector fault signal to the CIE.
         2) We are in alarm condition, heat is reducing and crossed below the lower threshold.
            Action = Move the lower threshold back to the FAULT level.  Send a 'fire no longer detected' signal to the CIE.
      */
      
      /* Fault condition */
      if ( HeadPlugIn.HHeatAnalogueValue < HeadPlugIn.HHeatFaultThr )
      {
         HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_INDICATED;
         HeadPlugIn.HHeatAlarmState = HEAD_ACTIVATION_FAULT_E ;
         /* Send a fault message to the application */
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
         {
            CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E,  CO_CHANNEL_HEAT_B_E, FAULT_SIGNAL_FAULTY_SENSOR_E, 1, false, false );
         }
         else if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R, DC_MATCH_ANY_E) )
         {
            CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E,  CO_CHANNEL_HEAT_A1R_E, FAULT_SIGNAL_FAULTY_SENSOR_E, 1, false, false );
         }
         /* Set up a command to move the upper threshold from the UPPER to the FAULT detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatFaultThr,0);
         headUpdateRequired = true;
#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 1);/* Switch on the head LEDs */
#endif
      }
      else if ( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HHeatAlarmState )
      {
         /* This is the head recovering from an alarm condition to a normal condition */
         HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_CLEAR;
         HeadPlugIn.HHeatAlarmState = HEAD_ACTIVATION_NORMAL_E;
         /* Send a new fire value to the control panel */
         if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
         {
            MM_SendFireSignal( CO_CHANNEL_HEAT_B_E, CO_HEAT_B_E, HeadPlugIn.HHeatAnalogueValue, HeadPlugIn.HHeatAlarmState );
         }
         else if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R, DC_MATCH_ANY_E) )
         {
            MM_SendFireSignal( CO_CHANNEL_HEAT_A1R_E, CO_HEAT_A1R_E, HeadPlugIn.HHeatAnalogueValue, HeadPlugIn.HHeatAlarmState );
         }
         /* Set up a command to move the lower threshold from the LOWER to the FAULT detection threshold */
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatFaultThr,0);
         headUpdateRequired = true;
#ifdef AUTO_CONTROL_OF_HEAD_LEDS
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 0, 0);/* Switch off the head LEDs */
#endif
      }
      else if ( HEAD_ACTIVATION_NORMAL_E == HeadPlugIn.HHeatAlarmState )
      {
         MM_UpdateDetectorThresholds();
      }
   }

   return headUpdateRequired;
}

/*************************************************************************************/
/**
* function name   : MM_UpdateDetectorThresholds
* description     : Update the detector smoke/heat thresholds to day or night values.
*
* @param    None.
*
* @return   bool.    true if settings were queued for the head. 
*/
bool MM_UpdateDetectorThresholds(void)
{
   bool result = false;
   
   //Do we have smoke?
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SMOKE_DETECTOR, DC_MATCH_ANY_E) )
   {
      MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeUpperThr[HeadPlugIn.HDayNight], 0);
      MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeFaultThr, 0);
      result = true;
   }
   //Do we have heat?
   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R | DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
   {
      MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatUpperThr[HeadPlugIn.HDayNight], 0);
      MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatFaultThr, 0);
      result = true;
   }
   
   return result;
}



/*************************************************************************************/
/**
* function name   : MM_PerformPeriodicTasks
* description     : Performed any timed operations.  The RBU Application initiates this
*                   once per second.
*
* @param - None.
*
* @return - None.
*/
void MM_PerformPeriodicTasks( void )
{
   static uint32_t alarmActiveCount = 0;
   
   /* do periodic reading of the head analogue value if it is in alarm condition
      to ensure that the lower threshold callback from the head wasn't missed */
   if ( HeadIsEnabled &&
        (( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HHeatAlarmState ) ||
        ( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HSmokeAlarmState ) ) )
   {
      alarmActiveCount++;
      if ( HEAD_DETECTOR_POLL_PERIOD <= alarmActiveCount )
      {
         
         if ( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HSmokeAlarmState )
         {
            MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, 0);
         }
         
         if ( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HHeatAlarmState )
         {
            MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, 0);
         }

         /* reset time until next test */
         alarmActiveCount = 0;
      }
   }
   
   //If the plug-in is a beacon it is kept powered down.
   if ( false == DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) || gForceDetectHead)
   {
      //If the head isn't found, try to detect it
      if ( false == HeadIsEnabled )
      {
         HeadDetectRetryCount++;
         if ( HEAD_RETRY_PERIOD_SECONDS <= HeadDetectRetryCount )
         {
            CO_PRINT_B_0(DBG_INFO_E, "MM_PerformPeriodicTasks - Retry Head Detection\r\n");
            HeadDetectRetryCount = 0;
            if( MM_DetectPlugin() )
            {
               gForceDetectHead = false;
               CO_PRINT_B_0(DBG_INFO_E, "Head Detected. Powering up....\r\n");
               if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
               {
                  MM_WaitForPluginInitialPeriod(BEACON_SETTLING_TIME_MS);
               }
               else
               {
                  MM_WaitForPluginInitialPeriod(HEAD_SETTLING_TIME_RETRY);
               }
               // Start the BIT
               MM_HeadInterface_BuiltInTestReq();
               //Assume the head is in a normal state on returning
               HeadPlugIn.HSmokeAlarmState = HEAD_ACTIVATION_NORMAL_E;
            }
            else
            {
               //The head wsn't detected.  Beacon types won't be routinely retried, so set a flag to force retries.
               if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
               {
                  gForceDetectHead = true;
               }
            }
         }
      }
   }
   else 
   {
      //the head is a beacon type.  if it is powered up and there are no messages in the plugin queue,
      //increment the timeout count and power-down the plugin when the count expires.
      if ( HeadIsEnabled )
      {
         if ( 0 == gPluginQueueMessageCount )
         {
            if ( (BEACON_POWER_DOWN_DELAY > gBeaconPowerDownCount) &&
                  (HEAD_BEACON_SELF_TEST_ACTIVE_E != HeadPlugIn.HBeaconTestMode) &&
                  (HEAD_BEACON_TEST_ACTIVE_E != HeadPlugIn.HBeaconTestMode) )
            {
               gBeaconPowerDownCount++;
            }
         }
      }

   }
   
   if ( HEAD_BEACON_SELF_TEST_ACTIVE_E == HeadPlugIn.HBeaconTestMode )
   {
      gBeaconSelfTestCount++;
      if ( BEACON_SELF_TEST_TIMEOUT < gBeaconSelfTestCount )
      {
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_BEACON_FAULT_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, 0);
      }
   }   
   else if ( HEAD_BEACON_TEST_ACTIVE_E == HeadPlugIn.HBeaconTestMode )
   {
      gBeaconFaultRequestCount++;
      if ( BEACON_ACTIVE_FAULT_REQUEST_PERIOD <= gBeaconFaultRequestCount )
      {
         gBeaconFaultRequestCount = 0;
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_BEACON_FAULT_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, 0);
      }
   }
   
   //perform tamper check on head
   MM_PerformTamperCheck();
   
   HeadPeriodicTimerActive = false;
}
   

/*************************************************************************************/
/**
* HeadPeriodicTimerCallback
* Callback function for the periodic timer.  Triggers every RBU_PERIODIC_TIMER_PERIOD_MS
* milliseconds.
*
* @param    id.      The ID of the timer that triggered this function to be called.
*
* @return - None.
*/
void HeadPeriodicTimerCallback(void const *id)
{
   //HeadPeriodicTimerActive is incremented when a timer event has been placed in the
   //HeadQ.  It is decremented after the event has been pulled from the queue.  This count is designed to 
   //prevent the HeadQ from being filled with timer events, so we don't raise an event here
   //unless HeadPeriodicTimerActive is less than MAX_QUEUED_TIMER_MESSAGES.
   if ( id )
   {
      if ( false == HeadPeriodicTimerActive )
      {
         if ( HeadPeriodicTimerArg == (uint32_t) id )
         {
            /* defer the timer action to the main head task thread */
            osStatus osStat = osErrorOS;
            CO_Message_t *pCmdReq = NULL;
            pCmdReq = osPoolAlloc(AppPool);
            if (pCmdReq)
            {
               pCmdReq->Type = CO_MESSAGE_TIMER_EVENT_E;
               uint32_t* pData = (uint32_t*)pCmdReq->Payload.PhyDataReq.Data;
               *pData = (uint32_t) id;
               osStat = osMessagePut(HeadQ, (uint32_t)pCmdReq, 0);
               if (osOK == osStat)
               {
                  gHeadQueueCount++;
                  //Increment HeadPeriodicTimerActive to prevent multiple timer events in the HeadQ
                  HeadPeriodicTimerActive = true;
               }
               else 
               {
                  /* failed to write */
                  osPoolFree(AppPool, pCmdReq);
                  
               }
            }
         }
      }
      /* Restart the timer */
      if ( NULL != HeadPeriodicTimerID )
      {
         osTimerStart(HeadPeriodicTimerID, HEAD_PERIODIC_TIMER_PERIOD_MS);
   }
   }
}

/*************************************************************************************/
/**
* MM_QueuePluginMsg
* Prepare a message for the plug-in and drop it in the queue (PluginQ)
*
* @param    cmdType        The message command type
* @param    messageType    The message type
* @param    readWrite   Read or Write command
* @param    value          The value to pass
* @param    transactionID  The transaction number for matching responses
*
* @return - None.
*/
void MM_QueuePluginMsg(const HeadMessageType_t cmdType, const PluginCmdType_t messageType, const PluginAccessType_t readWrite, const uint8_t value, const uint8_t transactionID)
{
   if ( HeadIsEnabled )
   {
      PluginMessage_t message;
      
      message.CommandMessageType = cmdType;
      message.PluginMessageType = messageType;
      message.readWrite = readWrite;
      message.value = value;
      message.TransactionID = transactionID;
      memcpy(message.buffer, PluginCommandMap[messageType].message, 6);
      
      ErrorCode_t result = MM_PLUGINQ_Push(&message);
      if( SUCCESS_E != result)
      {
         CO_PRINT_A_2(DBG_ERROR_E,"Head failed to queue message for Plugin.  Type=%d, Error=%d\r\n", messageType, result);
      }
      else 
      {
         gPluginQueueMessageCount++;
         gBeaconPowerDownCount = 0;
      }
   }
   else 
   {
      CO_PRINT_A_0(DBG_ERROR_E,"Head failed to queue message for Plugin.  HeadIsEnabled=false\r\n");
      if ( HEAD_CMD_TYPE_READ_E == readWrite )
      {
         gFailedMessage.CommandMessageType = cmdType;
         gFailedMessage.PluginMessageType = messageType;
         gFailedMessage.readWrite = readWrite;
         gFailedMessage.TransactionID = transactionID;
         gFailedMessage.value = value;
         MM_SendHeadResponse( 0 );
      }
   }
}



/*************************************************************************************/
/**
* MM_DetectPlugin
* COnfigure the Plugin USART and send the initial sync pulse
*
* @param    None.
*
* @return   true if Plugin found.
*/
bool MM_DetectPlugin(void)
{
   bool plugin_found = false;
   
    /* DeInit Serial Port - only if it has been used */
   if(SerialPortInitialized)
   {
      SerialPortFlushRxBuffer(HEAD_UART_E);
      SerialPortInitialized = false;
      SerialPortDeInit(HEAD_UART_E);
      /* Configure TX output pin as normal IO and set to high */
      GpioInit(&HeadTxPin, CN_HEAD_TX, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 1);
      
      /* Configure HEAD_RX line as input */
      __HAL_GPIO_EXTI_CLEAR_IT(HEAD_WAKE_UP_Pin);
      GpioInit(&HeadWakeUpPin, CN_HEAD_RX, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0);
   }

   if (GpioRead(&HeadPowerOn) == GPIO_PIN_RESET)
   {
      /* Configure Head Device PSU control gpio line - set it to 1 to enable PSU */
      GpioInit(&HeadPowerOn, CN_HEAD_PWR_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 1);
//      CO_PRINT_B_0(DBG_INFO_E,"Beacon Head Power ON\r\n");
      /* Wait until PSU stable */
      if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
      {
         osDelay(BEACON_SETTLING_TIME_MS);
      }
      else
      {
         osDelay(HEAD_SETTLING_TIME_MS);
      }
   }

   /* Check status of HEAD_RX - If RX is low it means no head is present */
   if (GpioRead(&HeadWakeUpPin) == GPIO_PIN_RESET)
   {
      HeadIsEnabled = false;//This will cause the priodic checks to keep trying to detect a head.
   }
   else 
   {
      plugin_found = true;
   }

   return plugin_found;
}

/*************************************************************************************/
/**
* MM_WaitForPluginInitialPeriod
* Called on first detection of a plugin.  Waits for the head settling time.
*
* @param    settling_time_ms  The time to wait for the head to settle before contacting it.
*
* @return   None.
*/
void MM_WaitForPluginInitialPeriod(const uint32_t settling_time_ms)
{
   HeadIsEnabled = true;// allows the MCU to sleep while we wait

   /* Debug print */
//   CO_PRINT_B_0(DBG_INFO_E, ">>>Head first time power-on<<<\r\n");
   /* Allow sleep mode while we wait for the head */
   HeadIsIdle = true;
   /* Head is plugged - Wait first time power-on time */
   osDelay(settling_time_ms);
   HeadIsIdle = false;
}

/*************************************************************************************/
/**
* MM_ActivatePlugin
* COnfigure the Plugin USART and send the initial sync pulse
*
* @param    None.
*
* @return   None.
*/
void MM_ActivatePlugin(void)
{
   // Check that the head was found  and that it isn't already configured
   if ( HeadIsEnabled && !SerialPortInitialized )
   {
      //If the plug-in is a beacon it needs to be powered up.
      if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
      {
         GpioWrite(&HeadPowerOn,1);
         //give time for the plugin to stabilise
         osDelay(BEACON_SETTLING_TIME_MS);
         gBeaconPowerDownCount = 0;
      }
      else
      // we only use the sync pulse if we are initiating contact.  Skip it if the head interrupted the RBU.
      if ( HEAD_INTERRUPTED != HeadPlugIn.HStatus_State )
      {
         /* Wait Head PSU stable */
         CO_InlineDelay(10);
         /* Set RBU TX Line to Low to start wake-up sequence on head plug-in */
         GpioWrite(&HeadTxPin, 0);
         CO_InlineDelay(20);
         GpioWrite(&HeadTxPin, 1);
      }
      /* Interrupt is not needed */
      HAL_NVIC_DisableIRQ(EXTI0_IRQn);
      /* Configure LPUART to operate with HEAD */
      SerialPortInit(HEAD_UART_E);
      /* Start a new Reception Operation */
      SerialPortStartReception(HEAD_UART_E);
      SerialPortInitialized = true;
   }
}

/*************************************************************************************/
/**
* MM_DeactivatePlugin
* Close the Plugin USART and set the Rx pin for GPIO interrupt
*
* @param    None.
*
* @return   None.
*/
void MM_DeactivatePlugin(void)
{
   // Check that the head was found  and that it's configured
   if ( HeadIsEnabled )
   {
      SerialPortFlushRxBuffer(HEAD_UART_E);
       /* DeInit Serial Port - only if it has been used */
      if(SerialPortInitialized)
      {
         SerialPortInitialized = false;
         SerialPortDeInit(HEAD_UART_E);
      }
   
      /* Configure TX output pin as normal IO and set to high */
      GpioInit(&HeadTxPin, CN_HEAD_TX, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 1);
      
      /* Configure HEAD_RX line as input and enable interrupt on falling edge */
      __HAL_GPIO_EXTI_CLEAR_IT(HEAD_WAKE_UP_Pin);
      GpioInit(&HeadWakeUpPin, CN_HEAD_RX, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0);
      /* Configure Interrupt */
      GpioSetInterrupt(&HeadWakeUpPin, IRQ_RISING_FALLING_EDGE_E, IRQ_MEDIUM_PRIORITY_E, MM_HeadWakeUpIntIrq, GPIO_NOPULL);
      
      if ( HEAD_INTERRUPTED == HeadPlugIn.HStatus_State )
      {
         HeadPlugIn.HStatus_State = 0;
      }
      
   }
}

/*************************************************************************************/
/**
* MM_ProcessHeadResponse
* COnfigure the Plugin USART and send the initial sync pulse
*
* @param    None.
*
* @return   None.
*/
void MM_ProcessHeadResponse(void)
{
   static PluginMessage_t pluginMessage;
   
   //Get the last message sent to the head
   if ( SUCCESS_E == MM_PLUGINQ_Pop(&pluginMessage) )
   {
      if ( MM_ReadHeadResponse(&pluginMessage) )
      {
         switch ( pluginMessage.PluginMessageType )
         {
            case HEAD_CMD_SERIAL_NUMBER_E:
               if ( HEAD_CMD_TYPE_READ_E == pluginMessage.readWrite )
               {
                  MM_HeadSerialNumber(&pluginMessage);
                  if ( BitTestInProgress )
                  {
                     BitTestInProgress = false;
                     CO_PRINT_A_0(DBG_BIT_E, "Head Interface BIT Success\r\n");
                     if ( false == MM_HeadIsSerialNumberValid(HeadPlugIn.HSerialNumber) )
                     {
                        CO_PRINT_B_1(DBG_ERROR_E, "Head serial number has incorrect format : %s\r\n", pluginMessage.buffer);
                     }
                  }
                  else 
                  {
                     // Send a response back to the application
                     MM_SendHeadResponse(HeadPlugIn.HSerialNumber);
                  }
               }
               break;
            case HEAD_CMD_SMOKE_UPPER_THRLD_E:
               {
                  DetectorThresholdsSet = true;
                  if ( HEAD_CMD_TYPE_READ_E == pluginMessage.readWrite )
                  {
                     int32_t upperThreshold = atoi((char*)pluginMessage.buffer);
                     CO_PRINT_B_1(DBG_INFO_E, "Head Smoke Upper THR: %d\r\n", upperThreshold);
                  }
                  else 
                  {
                     if(!strncmp((const char*)"OK", (const char*)pluginMessage.buffer, 2))
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Head Smoke Upper THR set to %d\r\n", pluginMessage.value);
                     }
                     else if(!strncmp((const char*)"ERROR", (const char*)pluginMessage.buffer, 5))
                     {
                        CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
                     }
                  }
               }
               break;
            case HEAD_CMD_SMOKE_LOWER_THRLD_E:
               {
                  if ( HEAD_CMD_TYPE_READ_E == pluginMessage.readWrite )
                  {
                     int32_t lowerThreshold = atoi((char*)pluginMessage.buffer);
                     CO_PRINT_B_1(DBG_INFO_E, "Head Smoke Lower THR: %d\r\n", lowerThreshold);
                  }
                  else 
                  {
                     if(!strncmp((const char*)"OK", (const char*)pluginMessage.buffer, 2))
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Head Smoke Lower THR set to %d\r\n", pluginMessage.value);
                     }
                     else if(!strncmp((const char*)"ERROR", (const char*)pluginMessage.buffer, 5))
                     {
                        CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
                     }
                  }
               }
               break;
            case HEAD_CMD_HEAT_UPPER_THRLD_E:
               {
                  DetectorThresholdsSet = true;
                  int32_t upperThreshold = atoi((char*)pluginMessage.buffer);
                  if ( HEAD_CMD_TYPE_READ_E == pluginMessage.readWrite )
                  {
                     CO_PRINT_B_1(DBG_INFO_E, "Head Heat Upper THR: %d\r\n", upperThreshold);
                  }
                  else 
                  {
                     if(!strncmp((const char*)"OK", (const char*)pluginMessage.buffer, 2))
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Head Heat Upper THR set to %d\r\n", pluginMessage.value);
                     }
                     else if(!strncmp((const char*)"ERROR", (const char*)pluginMessage.buffer, 5))
                     {
                        CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
                     }
                  }
               }
               break;
            case HEAD_CMD_HEAT_LOWER_THRLD_E:
               {
                  int32_t lowerThreshold = atoi((char*)pluginMessage.buffer);
                  if ( HEAD_CMD_TYPE_READ_E == pluginMessage.readWrite )
                  {
                     CO_PRINT_B_1(DBG_INFO_E, "Head Heat Lower THR: %d\r\n", lowerThreshold);
                  }
                  else 
                  {
                     if(!strncmp((const char*)"OK", (const char*)pluginMessage.buffer, 2))
                     {
                        CO_PRINT_B_1(DBG_INFO_E, "Head Heat Lower THR set to %d\r\n", pluginMessage.value);
                     }
                     else if(!strncmp((const char*)"ERROR", (const char*)pluginMessage.buffer, 5))
                     {
                        CO_PRINT_B_0(DBG_ERROR_E, "Head reply: ERROR\r\n");
                     }
                  }
               }
               break;
            case HEAD_CMD_DETECTOR_STATUS_E:
            {
               MM_HeadStatus(&pluginMessage);
               
               switch ( HeadPlugIn.HStatus_State )
               {
                  case HEAD_ACTIVATION_ALARM_E:
                     MM_ProcessHeadStateThreshold();
                     break;
                  case HEAD_ACTIVATION_NORMAL_E:
                     MM_ProcessHeadStateNormal();
                     break;
                  case HEAD_ACTIVATION_FAULT_E:
                     MM_ProcessHeadStateFault();
                     break;
                  default:
                     CO_PRINT_A_2(DBG_ERROR_E,"Head returned unknown state.  Type=%d, State=%d\r\n", HeadPlugIn.HStatus_Device, HeadPlugIn.HStatus_State);
                  break;
               }
            }
               break;
            case HEAD_CMD_SMOKE_ANA_VALUE_E:
               MM_HeadSmokeAnaValue(&pluginMessage);
               MM_CheckSmokeThresholds();
               if ( HEAD_READ_ANALOGUE_VALUE_E == pluginMessage.CommandMessageType && HEAD_CMD_SMOKE_ANA_VALUE_E == pluginMessage.PluginMessageType )
               {
                  // Send a response back to the application
                  MM_SendHeadResponse( HeadPlugIn.HSmokeAnalogueValue );
               }
               break;
            case HEAD_CMD_HEAT_ANA_VALUE_E:
                  MM_HeadHeatAnaValue(&pluginMessage);
                  MM_CheckHeatThresholds();
                  if ( HEAD_READ_ANALOGUE_VALUE_E == pluginMessage.CommandMessageType && 
                     ( HEAD_CMD_HEAT_ANA_VALUE_E == pluginMessage.PluginMessageType))
                  {
                     // Send a response back to the application
                     MM_SendHeadResponse( HeadPlugIn.HHeatAnalogueValue );
                  }
               break;
            case HEAD_CMD_PIR_STATUS_E:
               MM_HeadPIRStatus(&pluginMessage);
               if ( HEAD_READ_ANALOGUE_VALUE_E == pluginMessage.CommandMessageType && HEAD_CMD_PIR_STATUS_E == pluginMessage.PluginMessageType )
               {
                  // Send a response back to the application
                  MM_SendHeadResponse( HeadPlugIn.HPIRStatus );
               }
               break;
            case HEAD_CMD_SOFTWARE_INFO_E:
               MM_HeadSoftwareInfo(&pluginMessage);
               // Send a response back to the application
               MM_SendHeadResponse( HeadPlugIn.HFirmwareVersion );
               break;
            case HEAD_CMD_INDICATOR_CTRL_E:
               MM_HeadLEDControl(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HLEDStatus );
               break;
            case HEAD_CMD_DEVICE_TYPE_E:
               MM_HeadDeviceType(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HType );
               break;
            case HEAD_CMD_DEVICE_CLASS_E:
               MM_HeadDeviceClass(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HClass );
               break;
            case HEAD_CMD_DEVICE_TYPE_CLASS_E:
               MM_HeadDeviceTypeAndClass(&pluginMessage);
               if ( BitTestInProgress )
               {
                  if ( MM_BITTestResult(true) )
                  {
                     // The head is OK.  Set the detector thresholds, if they haven't been done.
                     if ( false == DetectorThresholdsSet )
                     {
                        //MM_UpdateDetectorThresholds();
                        //ENsure that the sensor is enabled and do a read of the analogue detector value.  The response will set alarm conditions and thresholds.
                        if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_HEAT_DETECTOR_TYPE_A1R | DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
                        {
                           MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_HEAT_E, HEAD_CMD_TYPE_WRITE_E,HeadPlugIn.HHeatEnable[HeadPlugIn.HDayNight], 0);
                           MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, 0);
                        }
                        if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SMOKE_DETECTOR, DC_MATCH_ANY_E) )
                        {
                           MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_SMOKE_E, HEAD_CMD_TYPE_WRITE_E, 1, 0);
                           MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, 0);
                        }
                        if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_PIR_DETECTOR, DC_MATCH_ANY_E) )
                        {
                           if ( HeadPlugIn.HPIREnabled[HeadPlugIn.HDayNight] )
                           {
                              MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_PIR_E, HEAD_CMD_TYPE_WRITE_E, 1, 0);
                              MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_RESET_PIR_E, HEAD_CMD_TYPE_WRITE_E, 1, 0);
                           }
                        }
                     }
                  }
               }
               else 
               {
                  uint32_t type_class = ((uint32_t)HeadPlugIn.HType << 16) + HeadPlugIn.HClass;
                  MM_SendHeadResponse( type_class );
               }
               break;
            case HEAD_CMD_RESET_PIR_E:
               MM_HeadPIRReset(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HPIRStatus );
               break;
            case HEAD_CMD_ENABLE_PIR_E:
               MM_HeadEnablePIR(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HPIREnable );
               break;
            case HEAD_CMD_ENABLE_SMOKE_E:
               MM_HeadEnableSmoke(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HSmokeEnable );
               break;
            case HEAD_CMD_ENABLE_HEAT_E:
               MM_HeadEnableHeat(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HHeatEnable[HeadPlugIn.HDayNight] );
               break;
            case HEAD_CMD_ENABLE_BEACON_E:
               MM_HeadBeaconEnable(&pluginMessage);
               if ( HEAD_INTERNAL_MESSAGE_E != pluginMessage.CommandMessageType )
               {
                  MM_SendHeadResponse( HeadPlugIn.HBeaconEnable );
               }
               break;
            case HEAD_CMD_OPTICAL_CHBR_STATUS_E:
               MM_HeadOpticalChamber(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HOptChamberStatus );
               break;
            case HEAD_CMD_SWITCH_CONTROL_E:
               MM_HeadSwitchControl(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HEnabled );
               break;
            case HEAD_CMD_ID_NUMBER_E:
               MM_HeadIdentificationNumber(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HIdentificationNumber );
               break;
            case HEAD_CMD_RESET_SOURCE_E:
               MM_HeadResetSource(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HResetSource );
               break;
            case HEAD_CMD_DISABLE_COMMS_E:
               MM_HeadDisableCommunications(&pluginMessage);
               break;
            case HEAD_CMD_BRANDING_ID_E:
               MM_HeadBrandingID(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HBrandingID );
               break;
            case HEAD_CMD_BEACON_TEST_MODE_E:
               MM_HeadBeaconTestMode(&pluginMessage);
               if ( HEAD_INTERNAL_MESSAGE_E != pluginMessage.CommandMessageType )
               {
                  MM_SendHeadResponse( HeadPlugIn.HBeaconTestMode );
               }
               break;
            case HEAD_CMD_BEACON_FLASH_RATE_E:
               MM_HeadBeaconFlashRate(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HBeaconFlashRate );
               break;
            case HEAD_CMD_BEACON_FAULT_STATUS_E:
               if ( MM_HeadBeaconFaultStatus(&pluginMessage) )
               {
                  CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_BEACON_E, FAULT_SIGNAL_BEACON_LED_FAULT_E, HeadPlugIn.HBeaconFaultStatus, false, false);
               }
               
               if ( HEAD_INTERNAL_MESSAGE_E != pluginMessage.CommandMessageType )
               {
                  MM_SendHeadResponse( HeadPlugIn.HBeaconTestMode );
               }
               break;
            case HEAD_CMD_BEACON_TYPE_E:
               MM_HeadBeaconType(&pluginMessage);
               MM_SendHeadResponse( HeadPlugIn.HBeaconType );
               break;
            default:
               break;
         }
      }
      //Message processing complete.  Discard the message from the front of the queue.
      MM_PLUGINQ_Discard();
   }
   else 
   {
      //something went wrong.  Issue an error message
      CO_PRINT_B_0(DBG_ERROR_E,"ProcessHeadResponse called with no outstanding command\r\n");
   }
}

/*************************************************************************************/
/**
* MM_ReadHeadResponse
* Wait for a reponse from the head
*
* @param    None.
*
* @return   None.
*/
bool MM_ReadHeadResponse(PluginMessage_t* message)
{
   bool response_received = false;
   uint32_t length;
   uint32_t to_counter = 0;
   
   if ( message )
   {
      while (HEAD_COMMS_TIMEOUT > to_counter && !response_received)
      {
         length = SerialPortFindCRLFRxBuffer(HEAD_UART_E);
         if( 0 < length )
         {
            /* Extract data received from Head */
            memset(message->buffer, 0, PLUGIN_MSG_BUFFER_SIZE);
            SerialPortReadRxBuffer(HEAD_UART_E, (uint8_t*)message->buffer, length);
            CO_PRINT_B_1(DBG_INFO_E,"Rx'd from head:%s", message->buffer);
#ifdef SHOW_HEAD_TRAFFIC_ON_PPU_USART
            SerialDebug_Print(PPU_UART_E, DBG_INFO_E, "Rx'd from head:%s", message->buffer);
#endif
            /* Remove <CR><LF> from the packet received */
            length -= 2;
            message->buffer[length] = 0;
            message->buffer[length+1] = 0;
            response_received = true;
            
            //Clear outstanding internal fault in the application
            if ( false == HeadPlugIn.HResponding )
            {
               CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_INTERNAL_FAULT_E, HEAD_FAULT_CLEAR, false, false);
            }
            
            HeadPlugIn.HResponding = true;
         
         }
         else
         {
            osDelay(10);
            to_counter++;
            if(HEAD_COMMS_TIMEOUT <= to_counter)
            {
               CO_PRINT_A_0(DBG_ERROR_E, "Head Interface Failed to respond\r\n");
               
               //If we were BIT testing, treat as a BIT failure.
               if ( BitTestInProgress )
               {
                  MM_BITTestResult(false);
               }

               //No response from Head.  Raise internal fault in the application if not already raised.
               if ( HeadPlugIn.HResponding )
               {
                  CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_INTERNAL_FAULT_E, HEAD_FAULT_INDICATED, false, false);
               }
               
               HeadPlugIn.HResponding = false;
            }
         }
      }
   }

   return response_received;
}

/*************************************************************************************/
/**
* function name   : MM_HeadDecodeOTACmd(const HeadMessage_t* const cmdMsg)
* description     : Decodes OTA command received by RBU and populates HeadCommandList
*                   container with command string to be passed to head device.
*
* @param -  cmdMsg   The message to be decoded.
*
* @return - None.
*/
void MM_HeadDecodeOTACmd(const HeadMessage_t* const cmdMsg)
{
   HeadMessage_t responseMessage;
   
   if ( cmdMsg )
   {
      
      CO_PRINT_B_1(DBG_INFO_E, "Head OTA decoded : %d\r\n\r\n", cmdMsg->MessageType);
      //Build a response
      responseMessage.MessageType = cmdMsg->MessageType;
      responseMessage.ChannelNumber = cmdMsg->ChannelNumber;
      responseMessage.TransactionID = cmdMsg->TransactionID;
      responseMessage.ProfileIndex = cmdMsg->ProfileIndex;
      responseMessage.Duration = cmdMsg->Duration;
      responseMessage.Value = cmdMsg->Value;

      switch(cmdMsg->MessageType)
      {
         /* Write Commands */
         case HEAD_WRITE_ENABLED_E:
            if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_SMOKE_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
            }
            else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_HEAT_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
            }
            else if(DEV_PIR_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_PIR_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
            }
            else if(DEV_BEACON_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_BEACON_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
            }
            else if(DEV_NO_DEVICE_TYPE_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_SWITCH_CONTROL_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
            }
         break;
            
         case HEAD_WRITE_OUTPUT_ACTIVATED_E:
         {
            if(DEV_BEACON_HEAD_E == cmdMsg->ChannelNumber)
            {
               //Set the flash rate
               MM_QueuePluginMsg(HEAD_WRITE_OUTPUT_ACTIVATED_E, HEAD_CMD_BEACON_FLASH_RATE_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->FlashRate, cmdMsg->TransactionID);
               
               //Activate the beacon
               if ( 0 != cmdMsg->Value )
               {
                  MM_QueuePluginMsg(HEAD_WRITE_OUTPUT_ACTIVATED_E, HEAD_CMD_ENABLE_BEACON_E, HEAD_CMD_TYPE_WRITE_E, 0x01, cmdMsg->TransactionID);
                  //switch on beacon self test
                  MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_BEACON_TEST_MODE_E, HEAD_CMD_TYPE_WRITE_E, 0x01, 0);
                  //not a self test of the beacon
                  gBeaconSelfTestActive = false;
               }
               else
               {
                  MM_QueuePluginMsg(HEAD_WRITE_OUTPUT_ACTIVATED_E, HEAD_CMD_ENABLE_BEACON_E, HEAD_CMD_TYPE_WRITE_E, 0x00, cmdMsg->TransactionID);
                  //switch off beacon self test
                  MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_BEACON_TEST_MODE_E, HEAD_CMD_TYPE_WRITE_E, HEAD_BEACON_TEST_INACTIVE_E, 0);
               }
            }
            // Head LED
            if (DEV_INDICATOR_LED_HEAD_E == cmdMsg->ChannelNumber)
            {
               if ( 0 != cmdMsg->Value )
               {
                  // Turn the head LEDs on
                  MM_QueuePluginMsg(HEAD_WRITE_OUTPUT_ACTIVATED_E, HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 0x01, cmdMsg->TransactionID);
               }
               else
               {
                  // Turn the head LEDs off
                  MM_QueuePluginMsg(HEAD_WRITE_OUTPUT_ACTIVATED_E, HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, 0x00, cmdMsg->TransactionID);
               }
            }
         }
         break;
            
         case HEAD_WRITE_ALARM_THRESHOLD_E:
         {
            /* separate the day and night values and save them */
            uint8_t day_setting = (uint8_t)(cmdMsg->Value & 0xff);
            uint8_t night_setting = (uint8_t)((cmdMsg->Value >> 8) & 0xff);
            
            if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               HeadPlugIn.HSmokeUpperThr[CO_DAY_E] = day_setting;
               HeadPlugIn.HSmokeUpperThr[CO_NIGHT_E] = night_setting;
               MM_QueuePluginMsg(HEAD_WRITE_ALARM_THRESHOLD_E, HEAD_CMD_SMOKE_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeUpperThr[HeadPlugIn.HDayNight], cmdMsg->TransactionID);
               CO_PRINT_B_2(DBG_INFO_E,"Day/Night SMOKE Upper Threshold updated to %d/%d\r\n", HeadPlugIn.HSmokeUpperThr[CO_DAY_E], HeadPlugIn.HSmokeUpperThr[CO_NIGHT_E]);
            }
            else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               HeadPlugIn.HHeatUpperThr[CO_DAY_E] = day_setting;
               HeadPlugIn.HHeatUpperThr[CO_NIGHT_E] = night_setting;
               MM_QueuePluginMsg(HEAD_WRITE_ALARM_THRESHOLD_E, HEAD_CMD_HEAT_UPPER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatUpperThr[HeadPlugIn.HDayNight], cmdMsg->TransactionID);
               CO_PRINT_B_2(DBG_INFO_E,"Day/Night HEAT Upper Threshold updated to %d/%d\r\n", HeadPlugIn.HHeatUpperThr[CO_DAY_E], HeadPlugIn.HHeatUpperThr[CO_NIGHT_E]);
            }

            //Send a response back to the application
            MM_SendOTAResponse(&responseMessage);
         }
         break;
         
         case HEAD_WRITE_PREALARM_THRESHOLD_E:
         {
            /* separate the day and night values and save them */
            uint8_t day_setting = (uint8_t)(cmdMsg->Value & 0xff);
            uint8_t night_setting = (uint8_t)((cmdMsg->Value >> 8) & 0xff);

            if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               HeadPlugIn.HSmokeLowerThr[CO_DAY_E] = day_setting;
               HeadPlugIn.HSmokeLowerThr[CO_NIGHT_E] = night_setting;
               CO_PRINT_B_2(DBG_INFO_E,"Day/Night SMOKE Lower Threshold updated to %d/%d\r\n", HeadPlugIn.HSmokeLowerThr[CO_DAY_E], HeadPlugIn.HSmokeLowerThr[CO_NIGHT_E]);
               
               /* only update the head if the lower threshold is in use (i.e. alarm condition) */
               if ( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HSmokeAlarmState )
               {
                  MM_QueuePluginMsg(HEAD_WRITE_PREALARM_THRESHOLD_E, HEAD_CMD_SMOKE_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HSmokeLowerThr[HeadPlugIn.HDayNight], cmdMsg->TransactionID);
               }
            }
            else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               HeadPlugIn.HHeatLowerThr[CO_DAY_E] = day_setting;
               HeadPlugIn.HHeatLowerThr[CO_NIGHT_E] = night_setting;
               CO_PRINT_B_2(DBG_INFO_E,"Day/Night HEAT Lower Threshold updated to %d/%d\r\n", HeadPlugIn.HHeatLowerThr[CO_DAY_E], HeadPlugIn.HHeatLowerThr[CO_NIGHT_E]);
               
               /* only update the head if the lower threshold is in use (i.e. alarm condition) */
               if ( HEAD_ACTIVATION_ALARM_E == HeadPlugIn.HHeatAlarmState )
               {
                  MM_QueuePluginMsg(HEAD_WRITE_PREALARM_THRESHOLD_E, HEAD_CMD_HEAT_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, HeadPlugIn.HHeatLowerThr[HeadPlugIn.HDayNight], cmdMsg->TransactionID);
               }
            }

            //Send a response back to the application
            MM_SendOTAResponse(&responseMessage);

         }
         break;
         case HEAD_WRITE_FAULT_THRESHOLD_E:
         {
            if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               HeadPlugIn.HSmokeFaultThr = (uint8_t)cmdMsg->Value;
               /* only update the head if the lower threshold is  being used for fault (i.e. non-alarm condition) */
               if ( HEAD_ACTIVATION_ALARM_E != HeadPlugIn.HSmokeAlarmState )
               {
                  MM_QueuePluginMsg(HEAD_WRITE_FAULT_THRESHOLD_E, HEAD_CMD_HEAT_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
               }
            }
            else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               HeadPlugIn.HHeatFaultThr = (uint8_t)cmdMsg->Value;
               /* only update the head if the lower threshold is being used for fault (i.e. non-alarm condition) */
               if ( HEAD_ACTIVATION_ALARM_E != HeadPlugIn.HHeatAlarmState )
               {
                  MM_QueuePluginMsg(HEAD_WRITE_FAULT_THRESHOLD_E, HEAD_CMD_HEAT_LOWER_THRLD_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
               }
            }

            //Send a response back to the application
            MM_SendOTAResponse(&responseMessage);
         }
         break;
         case HEAD_WRITE_FLASH_RATE_E:
            if (DEV_BEACON_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_WRITE_FLASH_RATE_E, HEAD_CMD_BEACON_FLASH_RATE_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->FlashRate, cmdMsg->TransactionID);
            }
         break;
         
         case HEAD_WRITE_TONE_SELECTION_E:
            //TODO
         break;
         
         case HEAD_WRITE_TEST_MODE_E:
            if (DEV_BEACON_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_WRITE_TEST_MODE_E, HEAD_CMD_BEACON_TEST_MODE_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
            }
         break;
         
         case HEAD_WRITE_INDICATOR_LED_E:
            MM_QueuePluginMsg(HEAD_WRITE_INDICATOR_LED_E, HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
         break;
         
         case HEAD_WRITE_IDENTIFICATION_NUMBER_E:
            MM_QueuePluginMsg(HEAD_WRITE_IDENTIFICATION_NUMBER_E, HEAD_CMD_ID_NUMBER_E, HEAD_CMD_TYPE_WRITE_E, cmdMsg->Value, cmdMsg->TransactionID);
         break;
         
         /* Read Commands */
         case HEAD_READ_ENABLED_E:
            if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ENABLED_E, HEAD_CMD_ENABLE_SMOKE_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
            else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ENABLED_E, HEAD_CMD_ENABLE_HEAT_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
            else if(DEV_PIR_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ENABLED_E, HEAD_CMD_ENABLE_PIR_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
            else if(DEV_NO_DEVICE_TYPE_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ENABLED_E, HEAD_CMD_SWITCH_CONTROL_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
         break;
         
         case HEAD_READ_ALARM_THRESHOLD_E:
            {
               HeadMessage_t response;
               response.ChannelNumber = cmdMsg->ChannelNumber;
               response.Duration = cmdMsg->Duration;
               response.MessageType = cmdMsg->MessageType;
               response.ProfileIndex = cmdMsg->ProfileIndex;
               response.TransactionID = cmdMsg->TransactionID;
               if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
               {
                  response.Value = HeadPlugIn.HSmokeUpperThr[HeadPlugIn.HDayNight];
                  CO_PRINT_B_2(DBG_INFO_E,"Read SMOKE Upper Threshold %d.  Day/Night=%d\r\n", HeadPlugIn.HSmokeUpperThr[HeadPlugIn.HDayNight], HeadPlugIn.HDayNight);
                  MM_SendOTAResponse( &response );
               }
               else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
               {
                  response.Value = HeadPlugIn.HHeatUpperThr[HeadPlugIn.HDayNight];
                  MM_SendOTAResponse( &response );
               }
            }
         break;
         
         case HEAD_READ_PREALARM_THRESHOLD_E:
            {
               HeadMessage_t response;
               response.ChannelNumber = cmdMsg->ChannelNumber;
               response.Duration = cmdMsg->Duration;
               response.MessageType = cmdMsg->MessageType;
               response.ProfileIndex = cmdMsg->ProfileIndex;
               response.TransactionID = cmdMsg->TransactionID;
               if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
               {
                  response.Value = HeadPlugIn.HSmokeLowerThr[HeadPlugIn.HDayNight];
                  MM_SendOTAResponse( &response );
               }
               else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
               {
                  response.Value = HeadPlugIn.HHeatLowerThr[HeadPlugIn.HDayNight];
                  MM_SendOTAResponse( &response );
               }
            }
         break;
            
         case HEAD_READ_FAULT_THRESHOLD_E:
            {
               HeadMessage_t response;
               response.ChannelNumber = cmdMsg->ChannelNumber;
               response.Duration = cmdMsg->Duration;
               response.MessageType = cmdMsg->MessageType;
               response.ProfileIndex = cmdMsg->ProfileIndex;
               response.TransactionID = cmdMsg->TransactionID;
               if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
               {
                  response.Value = HeadPlugIn.HSmokeFaultThr;
                  MM_SendOTAResponse( &response );
               }
               else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
               {
                  response.Value = HeadPlugIn.HHeatFaultThr;
                  MM_SendOTAResponse( &response );
               }
            }
         break;
         
         case HEAD_READ_FLASH_RATE_E:
            if (DEV_BEACON_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_FLASH_RATE_E, HEAD_CMD_BEACON_FLASH_RATE_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
         break;
         
         case HEAD_READ_TONE_SELECTION_E:
            //TODO
         break;
         
         case HEAD_READ_TEST_MODE_E:
            if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_TEST_MODE_E, HEAD_CMD_OPTICAL_CHBR_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
            else if (DEV_BEACON_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_TEST_MODE_E, HEAD_CMD_BEACON_TEST_MODE_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
         break;
         
         case HEAD_READ_ANALOGUE_VALUE_E:
            if(DEV_SMOKE_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ANALOGUE_VALUE_E, HEAD_CMD_SMOKE_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
            else if(DEV_HEAT_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ANALOGUE_VALUE_E, HEAD_CMD_HEAT_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
            else if(DEV_PIR_DETECTOR_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ANALOGUE_VALUE_E, HEAD_CMD_PIR_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
         break;
         
         case HEAD_READ_SERIAL_NUMBER_E:
            MM_QueuePluginMsg(HEAD_READ_SERIAL_NUMBER_E, HEAD_CMD_SERIAL_NUMBER_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
         break;
         
         case HEAD_READ_INDICATOR_LED_E:
            MM_QueuePluginMsg(HEAD_READ_INDICATOR_LED_E, HEAD_CMD_INDICATOR_CTRL_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
         break;
         
         case HEAD_EVENT_PREALARM_THRESHOLD_EXCEEDED_E:
            ;
         break;
         
         case HEAD_EVENT_ALARM_THRESHOLD_EXCEEDED_E:
            ;
         break;
         
         case HEAD_EVENT_FAULT_THRESHOLD_EXCEEDED_E:
            ;
         break;
         
         case HEAD_RESET_PIR_E:
            MM_QueuePluginMsg(HEAD_RESET_PIR_E, HEAD_CMD_RESET_PIR_E, HEAD_CMD_TYPE_SPECIAL_E, 0, cmdMsg->TransactionID);
         break;
         
         case HEAD_READ_IDENTIFICATION_NUMBER_E:
            MM_QueuePluginMsg(HEAD_READ_IDENTIFICATION_NUMBER_E, HEAD_CMD_ID_NUMBER_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
         break;
         
         case HEAD_READ_FIRMWARE_VERSION_E:
            MM_QueuePluginMsg(HEAD_READ_FIRMWARE_VERSION_E, HEAD_CMD_SOFTWARE_INFO_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
         break;
         
         case HEAD_READ_FAULT_TYPE_E:
            if (DEV_BEACON_HEAD_E == cmdMsg->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_FAULT_TYPE_E, HEAD_CMD_BEACON_FAULT_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            }
         break;
            
         case HEAD_DAY_NIGHT_SETTING_E:
            if ( CO_DAY_E == cmdMsg->Value )
            {
               HeadPlugIn.HDayNight = CO_DAY_E;
               CO_PRINT_B_0(DBG_INFO_E,"Day/Night set to DAY\r\n");
            }
            else if ( CO_NIGHT_E == cmdMsg->Value )
            {
               HeadPlugIn.HDayNight = CO_NIGHT_E;
               CO_PRINT_B_0(DBG_INFO_E,"Day/Night set to NIGHT\r\n");
            }
            else 
            {
               CO_PRINT_B_1(DBG_ERROR_E,"Head received bad Day/Night setting (%d)\r\n", cmdMsg->Value);
            }
            //Send the setting to the plugin.
            if (true == DM_DeviceIsEnabled(gDeviceCombination, (DEV_HEAT_DETECTOR_TYPE_A1R | DEV_HEAT_DETECTOR_TYPE_B), DC_MATCH_ANY_E ))
            {
               MM_QueuePluginMsg(HEAD_WRITE_ENABLED_E, HEAD_CMD_ENABLE_HEAT_E, HEAD_CMD_TYPE_WRITE_E,HeadPlugIn.HHeatEnable[HeadPlugIn.HDayNight], 0);
            }
            break;
            
         case HEAD_READ_DEVICE_TYPE_E:
            MM_QueuePluginMsg(HEAD_READ_DEVICE_TYPE_E, HEAD_CMD_DEVICE_TYPE_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            break;
         
         case HEAD_READ_DEVICE_TYPE_AND_CLASS_E:
            MM_QueuePluginMsg(HEAD_READ_DEVICE_TYPE_AND_CLASS_E, HEAD_CMD_DEVICE_TYPE_CLASS_E, HEAD_CMD_TYPE_READ_E, 0, cmdMsg->TransactionID);
            break;
         
         default:
         break;
      }
   }
}

/*************************************************************************************/
/**
* MM_SendHeadResponse
* Send a response message back to the application based on the current head command
*
* @param    value.   The value to put into the response message
*
* @return   None.
*/
void MM_SendHeadResponse(const uint32_t value)
{
   PluginMessage_t pluginMessage;
   HeadMessage_t Response;
   
   //Get the last message sent to the head
   if ( SUCCESS_E == MM_PLUGINQ_Pop(&pluginMessage) )
   {
      /* copy the details from the command into a response and populate the Value field */
      Response.MessageType = pluginMessage.CommandMessageType;
      Response.TransactionID = pluginMessage.TransactionID;
      Response.ChannelNumber = pluginMessage.ChannelNumber;
      Response.ProfileIndex = pluginMessage.ProfileIndex;
      Response.Value = value;
      
      CO_PRINT_B_2(DBG_INFO_E,"SendHeadResponse sending msgType=%d, value=%d\r\n", Response.MessageType, value);
      
      /* Send the response to the Application */
      MM_SendOTAResponse(&Response);
   }
   else 
   {
      CO_PRINT_B_0(DBG_INFO_E,"SendHeadResponse failed.  No message in head queue\r\n");
      Response.MessageType = gFailedMessage.CommandMessageType;
      Response.TransactionID = gFailedMessage.TransactionID;
      Response.ChannelNumber = gFailedMessage.ChannelNumber;
      Response.ProfileIndex = gFailedMessage.ProfileIndex;
      Response.Value =gFailedMessage.value;
      MM_SendOTAResponse(&Response);
   }
}

/*************************************************************************************/
/**
* MM_SendOTAResponse
* Send the supplied response message back to the application.
*
* @param    response.   The response message to be sent.
*
* @return   None.
*/
void MM_SendOTAResponse(const HeadMessage_t *response)
{
   if ( response )
   {
      /* Send the response to the Application */
      CO_Message_t* pResponseMessage = osPoolAlloc(AppPool);
      if (pResponseMessage)
      {
         pResponseMessage->Type = CO_MESSAGE_GENERATE_RESPONSE_SIGNAL_E;
         memcpy(pResponseMessage->Payload.PhyDataReq.Data, response, sizeof(HeadMessage_t));
         pResponseMessage->Payload.PhyDataReq.Size = sizeof(HeadMessage_t);
         
         osStatus osStat = osMessagePut(AppQ, (uint32_t)pResponseMessage, 0);
         
         if (osOK != osStat)
         {
            /* failed to put message in the head queue */
            osPoolFree(AppPool, pResponseMessage);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_SendSensorValueToApplication
* Send the supplied sensor data to the application.
*
* @param    sensorData.   The sensor information to be sent.
*
* @return   None.
*/
void MM_SendSensorValueToApplication(const CO_RBUSensorData_t* sensorData)
{
   if ( sensorData )
   {
      CO_PRINT_B_2(DBG_INFO_E,"Head sending sensor reading to application.  Channel=%d, value=%d\r\n", sensorData->RUChannelIndex, sensorData->SensorValue);
      /* Send the response to the Application */
      CO_Message_t* pSensorMessage = osPoolAlloc(AppPool);
      if (pSensorMessage)
      {
         pSensorMessage->Type = CO_MESSAGE_SENSOR_DATA_E;
         memcpy(pSensorMessage->Payload.PhyDataReq.Data, sensorData, sizeof(CO_RBUSensorData_t));
         pSensorMessage->Payload.PhyDataReq.Size = sizeof(CO_RBUSensorData_t);
         
         osStatus osStat = osMessagePut(AppQ, (uint32_t)pSensorMessage, 0);
         
         if (osOK != osStat)
         {
            /* failed to put message in the head queue */
            osPoolFree(AppPool, pSensorMessage);
         }
      }
   }
}

/*************************************************************************************/
/**
* MM_ProcessHeadStateNormal
* Process a report of normal operation from the head
*
* @param    None.
*
* @return   None.
*/
void MM_ProcessHeadStateNormal(void)
{
   uint8_t previous_status = MM_SensorFaultStatus();
   
   if ( HEAD_FAULT_DEVICE_SMOKE == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HSmokeFaultStatus = HEAD_FAULT_CLEAR;
      //This update indicates no smoke sensor faults.
      //if there was previously a fault send an update to the Application.
      if ( previous_status & HEAD_FAULT_DEVICE_SMOKE )
      {
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_CLEAR, false, false);
      }
   }
   else if ( HEAD_FAULT_DEVICE_HEAT == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_CLEAR;
      //This update indicates no smoke sensor faults.
      //if there was previously a fault send an update to the Application.
      if ( previous_status & HEAD_FAULT_DEVICE_HEAT )
      {
         CO_ChannelIndex_t channel;
         if ( DEV_HEAT_CLASS_A1R_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_A1R_E;
         }
         else if ( DEV_HEAT_CLASS_B_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_B_E;
         }
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, channel, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_CLEAR, false, false);
      }
   } 
   else if ( HEAD_FAULT_DEVICE_SMOKE_AND_HEAT == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HSmokeFaultStatus = HEAD_FAULT_CLEAR;
      HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_CLEAR;
      //This update indicates no smoke or heat sensor faults.
      //if there was previously a fault send an update to the Application.
      if ( previous_status & HEAD_FAULT_DEVICE_HEAT )
      {
         CO_ChannelIndex_t channel;
         if ( DEV_HEAT_CLASS_A1R_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_A1R_E;
         }
         else if ( DEV_HEAT_CLASS_B_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_B_E;
         }
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, channel, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_CLEAR, false, false);
      }
      
      if ( previous_status & HEAD_FAULT_DEVICE_SMOKE )
      {
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_CLEAR, false, false);
      }
   }
   else if ( HEAD_FAULT_DEVICE_PIR == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HPIRFaultStatus = HEAD_FAULT_CLEAR;
      //This update indicates no PIR sensor faults.
      //if there was previously a fault send an update to the Application.
      if ( previous_status & HEAD_FAULT_DEVICE_PIR )
      {
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_CLEAR, false, false);
      }
   }
}


/*************************************************************************************/
/**
* MM_ProcessHeadStateThreshold
* Query the detector head in response to a threshold being crossed
*
* @param    None.
*
* @return   None.
*/
void MM_ProcessHeadStateThreshold(void)
{
   if ( (DEV_SMOKE_DETECTOR_HEAD & HeadPlugIn.HStatus_Device) == DEV_SMOKE_DETECTOR_HEAD )
   {
      //the Smoke detector raised an event
      if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SMOKE_DETECTOR_HEAD, DC_MATCH_ANY_E) )
      {
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_SMOKE_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, 0);
      }
   }
   if ( (DEV_HEAT_DETECTOR_HEAD & HeadPlugIn.HStatus_Device) == DEV_HEAT_DETECTOR_HEAD )
   {
      //the Heat detector raised an event
      uint32_t HeatDetectorType = DEV_HEAT_DETECTOR_TYPE_A1R;
      if ( DEV_HEAT_CLASS_B_E ==  HeadPlugIn.HClass )
      {
         HeatDetectorType = DEV_HEAT_DETECTOR_TYPE_B;
      }
      if ( DM_DeviceIsEnabled(gDeviceCombination, HeatDetectorType, DC_MATCH_ANY_E) )
      {
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_HEAT_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, 0);
      }
   }
   if ( (DEV_PIR_DETECTOR_HEAD & HeadPlugIn.HStatus_Device) == DEV_PIR_DETECTOR_HEAD )
   {
      //the PIR raised an event
      if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_PIR_DETECTOR_HEAD, DC_MATCH_ANY_E) )
      {
         MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_PIR_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, 0);
      }
   }
}


/*************************************************************************************/
/**
* MM_ProcessHeadStateFault
* Process a fault report from the head
*
* @param    None.
*
* @return   None.
*/
void MM_ProcessHeadStateFault(void)
{
   uint8_t previous_status = MM_SensorFaultStatus();
   
   if ( HEAD_FAULT_DEVICE_SMOKE == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HSmokeFaultStatus = HEAD_FAULT_INDICATED;
      //This update indicates a smoke sensor fault.
      //if there was previously no fault send an update to the Application.
      if ( HEAD_FAULT_CLEAR == (previous_status & HEAD_FAULT_DEVICE_SMOKE) )
      {
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, false);
      }
   }
   else if ( HEAD_FAULT_DEVICE_HEAT == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_INDICATED;
      //This update indicates a smoke sensor fault.
      //if there was previously no fault send an update to the Application.
      if ( HEAD_FAULT_CLEAR == (previous_status & HEAD_FAULT_INDICATED) )
      {
         CO_ChannelIndex_t channel;
         if ( DEV_HEAT_CLASS_A1R_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_A1R_E;
         }
         else if ( DEV_HEAT_CLASS_B_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_B_E;
         }
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, channel, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, false);
      }
   } 
   else if ( HEAD_FAULT_DEVICE_SMOKE_AND_HEAT == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HSmokeFaultStatus = HEAD_FAULT_INDICATED;
      HeadPlugIn.HHeatFaultStatus = HEAD_FAULT_INDICATED;
      //This update indicates a smoke or heat sensor fault.
      //if there was previously no fault send an update to the Application.
      if ( HEAD_FAULT_CLEAR == (previous_status & HEAD_FAULT_DEVICE_HEAT) )
      {
         CO_ChannelIndex_t channel;
         if ( DEV_HEAT_CLASS_A1R_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_A1R_E;
         }
         else if ( DEV_HEAT_CLASS_B_E == HeadPlugIn.HClass )
         {
            channel = CO_CHANNEL_HEAT_B_E;
         }
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, channel, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, false);
      }
      
      if ( HEAD_FAULT_CLEAR == (previous_status & HEAD_FAULT_DEVICE_SMOKE) )
      {
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, false);
      }
   }
   else if ( HEAD_FAULT_DEVICE_PIR == HeadPlugIn.HStatus_Device  )
   {
      HeadPlugIn.HPIRFaultStatus = HEAD_FAULT_INDICATED;
      //This update indicates a PIR sensor fault.
      //if there was previously no fault send an update to the Application.
      if ( HEAD_FAULT_CLEAR == (previous_status & HEAD_FAULT_DEVICE_PIR) )
      {
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, false);
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_PerformTamperCheck
* description     : Checks that the head is still plugged in and raises a fault if it 
*                   is not found.  Initiates periodic checks for head plug in.
*
* @param - None.
*
* @return - None.
*/
void MM_PerformTamperCheck( void )
{
   static uint32_t fail_count = 0;
   static bool fault_raised = false;
   bool send_fault_message = false;
   bool fault_active = false;
   
   
   // only perform tamper check if the head serial port is not in operation 
   // and we are not receiving a wake-up pulse from the head
   if ( !SerialPortInitialized && !gWakeupPulseActive)
   {
      if ( MM_DetectPlugin() )
      {
         gForceDetectHead = false;
         fail_count = 0;
         if ( fault_raised )
         {
            fault_raised = false;
            fault_active = false;
            send_fault_message = true;
         }
         
         if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
         {
            //Beacon heads need to be powered back down if their hold-off delay has expired
            if ( BEACON_POWER_DOWN_DELAY <= gBeaconPowerDownCount )
            {
               GpioWrite(&HeadPowerOn,0);
            }
         }
      }
      else 
      {
         fail_count++;
         if ( TAMPER_PERIOD_SECONDS <= fail_count && !fault_raised )
         {
            fault_raised = true;
            fault_active = true;
            send_fault_message = true;
            //The head has been removed.  Make sure that the detector thresholds are set when the head is re-fitted
            DetectorThresholdsSet = false;
         }

         //The head wsn't detected.  Beacon types won't be routinely retried, so set a flag to force retries.
         if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
         {
            gForceDetectHead = true;
         }
      }
      
      if ( send_fault_message )
      {
         //We have seen a tamper fault.  Put a message in the head queue.
         uint16_t isActive = (fault_active ? 1 : 0);
         
         if ( SUCCESS_E == CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_HEAD_REMOVED_E, isActive, false, false) )
         {
            gHeadQueueCount++;
            CO_PRINT_A_1(DBG_INFO_E, "Head Tamper State=%d\r\n", isActive);
            // On tamper change, initiate check for head
            HeadInterface_State = HEAD_STATE_IDLE_E;
            if ( isActive )
            {
               HeadIsEnabled = false;// This will cause the periodic checks to keep trying to detect a head.
               HeadPlugIn.HBeaconTestMode = HEAD_BEACON_TEST_INACTIVE_E;
            }
            else 
            {
               if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
               {
                  HeadIsEnabled = true;
               }
            }
            HeadDetectRetryCount = HEAD_RETRY_PERIOD_SECONDS; // This will force an immediate head check.
         }
         
         //On head removal, cancel any alarm conditions raised by the head.
         if ( isActive )
         {
            HeadPlugIn.HSmokeAlarmState = HEAD_ACTIVATION_NORMAL_E;
            HeadPlugIn.HHeatAlarmState = HEAD_ACTIVATION_NORMAL_E;
            HeadPlugIn.HPIRStatus = HEAD_ACTIVATION_NORMAL_E;
         }
      }
   }
   else 
   {
      //CO_PRINT_B_2(DBG_INFO_E,"Head tamper check disabled : SerialPortInitialized=%d, gWakeupPulseActive=%d\r\n", SerialPortInitialized, gWakeupPulseActive);
      fault_raised = false;
      //Cancel gWakeupPulseActive It will never remain set until the next check unless the head has been removed.
      gWakeupPulseActive = false;
   }
}

/*************************************************************************************/
/**
* function name   : MM_GetActivationState
* description     : Gets the activation state of the specified channel.
*
* @param - None.
*
* @return - The activation state.
*/
HeadActivationState_t MM_GetActivationState(const CO_ChannelIndex_t channel)
{
   HeadActivationState_t state = HEAD_ACTIVATION_NORMAL_E;
   
   switch ( channel )
   {
      case CO_CHANNEL_SMOKE_E:
         state = HeadPlugIn.HSmokeAlarmState;
         break;
      case CO_CHANNEL_HEAT_B_E:  //intentional drop-through
      case CO_CHANNEL_HEAT_A1R_E:
         state = HeadPlugIn.HHeatAlarmState;
         break;
      case CO_CHANNEL_PIR_E:
         if( HeadPlugIn.HPIRStatus )
         {
            state = HEAD_ACTIVATION_ALARM_E;
         }
         break;
      default:
         break;
   }
   
   return state;
}

/*************************************************************************************/
/**
* function name   : MM_GetSmokeSensorValue
* description     : Gets the analogue value of the smoke channel.
*
* @param - None.
*
* @return - uint8_t  The analogue value.
*/
uint8_t MM_GetSmokeSensorValue(void)
{
   return HeadPlugIn.HSmokeAnalogueValue;
}

/*************************************************************************************/
/**
* function name   : MM_GetHeatSensorValue
* description     : Gets the analogue value of the heat channel.
*
* @param - None.
*
* @return - uint8_t  The analogue value.
*/
uint8_t MM_GetHeatSensorValue(void)
{
   return HeadPlugIn.HHeatAnalogueValue;
}

/*************************************************************************************/
/**
* function name   : MM_GetPIRState
* description     : Gets the trigger state of the PIR channel.
*
* @param - None.
*
* @return - uint8_t  The PIR state.
*/
uint8_t MM_GetPIRState(void)
{
   return HeadPlugIn.HPIRStatus;
}

/*************************************************************************************/
/**
* function name   :  MM_GenerateFaultSignal
* description     :  Directs the supplied channel index to the appropriate fault generator
*                    Channel CO_CHANNEL_MAX_E requires all channels to be checked
*
* @param - channelNumber   The channel to generate a fault for
*
* @return - None.
*/
void MM_GenerateFaultSignal(const CO_ChannelIndex_t channelNumber)
{
   switch (channelNumber)
   {
      case CO_CHANNEL_SMOKE_E:
         MM_ReportSmokeSensorFault();
         break;
      
      case CO_CHANNEL_HEAT_A1R_E: // intentional drop-through
      case CO_CHANNEL_HEAT_B_E:
         MM_ReportHeatSensorFault();
         break;
      
      case CO_CHANNEL_CO_E:
         MM_ReportOpticalSensorFault();
         break;
      
      case CO_CHANNEL_PIR_E:
         MM_ReportPirSensorFault();
         break;
      
      case CO_CHANNEL_BEACON_E:
         MM_ReportBeaconFault();
         break;
      
      case CO_CHANNEL_MAX_E:
         //Report all channel faults
         MM_ReportSmokeSensorFault();
         MM_ReportHeatSensorFault();
         MM_ReportOpticalSensorFault();
         MM_ReportPirSensorFault();
         MM_ReportBeaconFault();
         break;
      
      default:
         break;
   }
}

/*************************************************************************************/
/**
* function name   :  MM_SensorFaultStatus
* description     :  Returns a bitmap indicating the sensor fault status where
*                    bit 0 = no fault.
*                    bit 1 = smoke sensor fault.
*                    bit 2 = heat sensor fault.
*                    bit 3 = Optical Chamber fault.
*                    bit 4 = PIR sensor fault.
*
* @param    -        None.
*
* @return   -        bitmap of sensor fault states.
*/
uint8_t MM_SensorFaultStatus(void)
{
   uint8_t status = HEAD_FAULT_CLEAR;
   
   if ( HEAD_FAULT_INDICATED == HeadPlugIn.HSmokeFaultStatus )
   {
      status |= HEAD_FAULT_DEVICE_SMOKE;
   }
   
   if ( HEAD_FAULT_INDICATED == HeadPlugIn.HHeatFaultStatus )
   {
      status |= HEAD_FAULT_DEVICE_HEAT;
   }
   
   if ( HEAD_FAULT_INDICATED == HeadPlugIn.HPIRFaultStatus )
   {
      status |= HEAD_FAULT_DEVICE_PIR;
   }
   
   if ( HEAD_OPTICAL_CHAMBER_FAULT_E == HeadPlugIn.HOptChamberStatus )
   {
      status |= HEAD_FAULT_DEVICE_OPTICAL_CHAMBER;
   }
   
   return status;
}

/*************************************************************************************/
/**
* function name   :  MM_ReportSmokeSensorFault
* description     :  Checks the smoke sensor for fault conditions and issues
*                    fault signal if the channel has a fault.
*
* @param    - None.
*
* @return   - None.
*/
void MM_ReportSmokeSensorFault(void)
{
   uint8_t fault_status = MM_SensorFaultStatus();
   
   if ( fault_status & HEAD_FAULT_DEVICE_SMOKE )
   {
      CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_SMOKE_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, true);
   }
}

/*************************************************************************************/
/**
* function name   :  MM_ReportHeatSensorFault
* description     :  Checks the heat sensor for fault conditions and issues
*                    fault signal if the channel has a fault.
*
* @param    - None.
*
* @return   - None.
*/
void MM_ReportHeatSensorFault(void)
{
   uint8_t fault_status = MM_SensorFaultStatus();

   if ( fault_status & HEAD_FAULT_DEVICE_HEAT )
   {
      CO_ChannelIndex_t channel;
      if ( DEV_HEAT_CLASS_A1R_E == HeadPlugIn.HClass )
      {
         channel = CO_CHANNEL_HEAT_A1R_E;
      }
      else if ( DEV_HEAT_CLASS_B_E == HeadPlugIn.HClass )
      {
         channel = CO_CHANNEL_HEAT_B_E;
      }
      CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, channel, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, true);
   }
}

/*************************************************************************************/
/**
* function name   :  MM_ReportOpticalSensorFault
* description     :  Checks the optical sensor for fault conditions and issues
*                    fault signal if the channel has a fault.
*
* @param    - None.
*
* @return   - None.
*/
void MM_ReportOpticalSensorFault(void)
{
   uint8_t fault_status = MM_SensorFaultStatus();

   if ( fault_status & HEAD_FAULT_DEVICE_OPTICAL_CHAMBER )
   {
      CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_CO_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, true);
   }
}

/*************************************************************************************/
/**
* function name   :  MM_ReportPirSensorFault
* description     :  Checks the PIR sensor for fault conditions and issues
*                    fault signal if the channel has a fault.
*
* @param    - None.
*
* @return   - None.
*/
void MM_ReportPirSensorFault(void)
{
   uint8_t fault_status = MM_SensorFaultStatus();

   if ( fault_status & HEAD_FAULT_DEVICE_PIR )
   {
      CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_PIR_E, FAULT_SIGNAL_FAULTY_SENSOR_E, HEAD_FAULT_INDICATED, false, true);
   }
}

/*************************************************************************************/
/**
* function name   :  MM_ReportBeaconFault
* description     :  Checks the beacon for fault conditions and issues
*                    fault signal if the channel has a fault.
*
* @param    - None.
*
* @return   - None.
*/
void MM_ReportBeaconFault(void)
{
   if ( HEAD_FAULT_CLEAR != HeadPlugIn.HBeaconFaultStatus)
   {
      CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_BEACON_E, FAULT_SIGNAL_BEACON_LED_FAULT_E, HEAD_FAULT_INDICATED, false, false);
   }
}


/*************************************************************************************/
/**
* function name   :  MM_StartBITTest
* description     :  Starts the BIT test.  Head type and class are requested for comparison
*                    with the device combination setting.
*
* @param    - None.
*
* @return   - None.
*/
void MM_StartBITTest(void)
{
   CO_PRINT_B_0(DBG_INFO_E, "Start BIT Test\r\n");
   //Build Type and Class request and push it onto the plugin queue
   BitTestInProgress = true;
   MM_QueuePluginMsg(HEAD_INTERNAL_MESSAGE_E, HEAD_CMD_DEVICE_TYPE_CLASS_E, HEAD_CMD_TYPE_READ_E, 0, 0);
}

/*************************************************************************************/
/**
* function name   :  MM_BITTestResult
* description     :  Handles BIT Test result.
*
* @param    headResponded  TRUE if the head responded to the BIT request.
*
* @return   None.
*/
bool MM_BITTestResult(const bool headResponded)
{
   bool result = false;
   
   if ( headResponded )
   {
      HeadIsEnabled = true;
      
      if ( MM_PluginMismatch() )
      {
         //Head doesn't match the device combination. Send a 'head mismatch' fault
         CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_TYPE_MISMATCH_E, HEAD_FAULT_INDICATED, false, false);
         CO_PRINT_A_0(DBG_BIT_E, "Head Interface BIT Fail\r\n");
         HeadPlugIn.HMismatchedType = true;
         //This will prompt retries to contact head
         MM_DeactivatePlugin();
         HeadIsEnabled = false;
         HeadDetectRetryCount = 0;
      }
      else 
      {
         CO_PRINT_B_0(DBG_BIT_E, "Head Interface BIT OK\r\n");
         //Clear channel faults
         MM_ProcessHeadStateNormal();
         //Cancel the mismatch fault on in the application, if required.
         if ( HeadPlugIn.HMismatchedType )
         {
            CO_SendFaultSignal(BIT_SOURCE_PLUGIN_E, CO_CHANNEL_NONE_E, FAULT_SIGNAL_TYPE_MISMATCH_E, HEAD_FAULT_CLEAR, false, false);
            CO_PRINT_A_0(DBG_BIT_E, "Head mismatch fault cleared\r\n");
            HeadPlugIn.HMismatchedType = false;
         }
         result = true;
      }
   }
   else 
   {
      //This will prompt retries to contact head
      if ( DM_DeviceIsEnabled(gDeviceCombination,DEV_HEAD_BEACONS, DC_MATCH_ANY_E) )
      {
         gForceDetectHead = true;
      }
      else 
      {
         MM_DeactivatePlugin();
      }
      HeadIsEnabled = false;
      HeadDetectRetryCount = 0;
   }
   
   BitTestInProgress = false;
   
   return result;
}

/*************************************************************************************/
/**
* function name   :  MM_ProcessApplicationRequest
* description     :  Handles requests from the RBU Application.
*
* @param    - None.
*
* @return   - None.
*/
void MM_ProcessApplicationRequest(const HeadMessage_t* const pMessage)
{
   osEvent event;
   CO_Message_t* pDataMessage;
   bool read_message = true;
   uint32_t queue_timeout = 0;

   if ( pMessage )
   {
      switch ( pMessage->MessageType )
      {
         case HEAD_SLEEP_MODE_E:
            //Stop all activity for sleep mode
         
            //Stop the periodic timer
            osTimerStop(HeadPeriodicTimerID);
            //flush the queue from the App.
            HeadIsIdle = false;
            while ( read_message )
            {
               event = osMessageGet(HeadQ,queue_timeout);
               if ( (osEventMessage == event.status) && (NULL != event.value.p) )
               {
                  gHeadQueueCount--;
                  pDataMessage = (CO_Message_t*)event.value.p;
                  osPoolFree(AppPool, pDataMessage);
               }
               else 
               {
                  read_message = false;
               }
            }
            //flush the head queue
            MM_PLUGINQ_ResetMessageQueue();
            // close the plugin USART
            MM_DeactivatePlugin();
            break;
         case HEAD_READ_ANALOGUE_VALUE_E:
            if(DEV_SMOKE_DETECTOR_HEAD_E == pMessage->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ANALOGUE_VALUE_E, HEAD_CMD_SMOKE_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, pMessage->TransactionID);
            }
            else if(DEV_HEAT_DETECTOR_HEAD_E == pMessage->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ANALOGUE_VALUE_E, HEAD_CMD_HEAT_ANA_VALUE_E, HEAD_CMD_TYPE_READ_E, 0, pMessage->TransactionID);
            }
            else if(DEV_PIR_DETECTOR_HEAD_E == pMessage->ChannelNumber)
            {
               MM_QueuePluginMsg(HEAD_READ_ANALOGUE_VALUE_E, HEAD_CMD_PIR_STATUS_E, HEAD_CMD_TYPE_READ_E, 0, pMessage->TransactionID);
            }
            break;
         default:
            break;
      }
   }
}

/*************************************************************************************/
/**
* function name   :  MM_PluginMismatch
* description     :  Compares the reported head type and class to the device
*                    configuration and returns TRUE if they are mismatched.
*
* @param    - None.
*
* @return   bool     TRUE if there is a mismatch, FALSE if the head matched the DC.
*/
bool MM_PluginMismatch(void)
{
   bool mismatch_detected = false;
   uint32_t expected_type = 0;
   uint32_t expected_class = 0;
   
   switch ( gDeviceCombination )
   {
      case DC_SMOKE_DETECTOR_E:
         expected_type = 1;
         expected_class = 1;
         break;
      case DC_HEAT_DETECTOR_TYPE_A1R_E:
         expected_type = 2;
         expected_class = 1;
         break;
      case DC_HEAT_DETECTOR_TYPE_B_E:
         expected_type = 2;
         expected_class = 2;
         break;
      case DC_SMOKE_AND_HEAT_DETECTOR_TYPE_A1R_E:
         expected_type = 3;
         expected_class = 1;
         break;
      case DC_SMOKE_AND_PIR_DETECTOR_E:
         expected_type = 9;
         expected_class = 1;
         break;
      case DC_SOUNDER_SMOKE_DETECTOR_E:
         expected_type = 1;
         expected_class = 1;
         break;
      case DC_SOUNDER_HEAT_DETECTOR_TYPE_A1R_E:
         expected_type = 2;
         expected_class = 1;
         break;
      case DC_SOUNDER_HEAT_DETECTOR_TYPE_B_E:
         expected_type = 2;
         expected_class = 2;
         break;
      case DC_SOUNDER_SMOKE_HEAT_DETECTOR_TYPE_A1R_E:
         expected_type = 3;
         expected_class = 1;
         break;
      case DC_SOUNDER_SMOKE_PIR_E:
         expected_type = 9;
         expected_class = 1;
         break;
      case DC_SOUNDER_VI_SMOKE_DETECTOR_E:
         expected_type = 1;
         expected_class = 1;
         break;
      case DC_SOUNDER_VI_HEAT_DETECTOR_TYPE_A1R_E:
         expected_type = 2;
         expected_class = 1;
         break;
      case DC_SOUNDER_VI_HEAT_DETECTOR_TYPE_B_E:
         expected_type = 2;
         expected_class = 2;
         break;
      case DC_SOUNDER_VI_SMOKE_HEAT_DETECTOR_TYPE_A1R_E:
         expected_type = 3;
         expected_class = 1;
         break;
      case DC_SOUNDER_VI_SMOKE_PIR_E:
         expected_type = 9;
         expected_class = 1;
         break;
      case DC_CONSTRUCTION_SITE_SMOKE_HEAT_DETECTOR_TYPE_A1R_SOUNDER_E:
         expected_type = 3;
         expected_class = 1;
         break;
      case DC_CONSTRUCTION_SITE_SMOKE_AND_HEAT_DETECTOR_TYPE_A1R_SOUNDER_PIR_E:
         expected_type = 3;
         expected_class = 1;
         break;
      default:
         expected_type = 0;
         expected_class = 0;
      break;
   }
   
   if ( HeadPlugIn.HType != expected_type )
   {
      mismatch_detected = true;
      CO_PRINT_A_2(DBG_ERROR_E,"Head Type Mismatch expected=%d, reported=%d\r\n", expected_type, HeadPlugIn.HType);
   }

   if ( HeadPlugIn.HClass != expected_class )
   {
      mismatch_detected = true;
      CO_PRINT_A_2(DBG_ERROR_E,"Head Class Mismatch expected=%d, reported=%d\r\n", expected_class, HeadPlugIn.HClass);
   }

   return mismatch_detected;
}
#endif
