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
*  File         : MM_SviTask.c
*
*  Description  : SVI (Sounder and Visual Indicator) Task implementation
*                 This is responsible for the communication with the SVI unit
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>



/* User Include Files
*************************************************************************************/
#include "cmsis_os.h"
#include "CO_Defines.h"
#include "CO_Message.h"
#include "board.h"
#include "DM_LED.h"
#include "DM_Device.h"
#include "DM_OutputManagement.h"
#include "DM_svi.h"
#include "MM_SviTask.h"

#ifndef I2C_SVI_BOARD_SIMULATION

/* Private Functions Prototypes
*************************************************************************************/
static ErrorCode_t MM_SviGetSerialNumber(uint32_t *SerialNumber);
static bool MM_SviIsSerialNumberValid(void);


/* Global Variables
*************************************************************************************/
extern osPoolId AppPool;
extern Gpio_t SndrBeaconOn;
extern uint16_t gDeviceCombination;

/* Private Variables
*************************************************************************************/
static void MM_SVI_TaskMain(void const *argument);
osThreadId tid_SVITask;
osThreadDef(MM_SVI_TaskMain, osPriorityNormal, 1, 1024);
osMessageQDef(SviQ, 3u, uint32_t);
osMessageQId(SviQ);

/*************************************************************************************/
/**
* MM_SVI_Initialise
* initialise the SVI interface
*
* @param - None.
*
* @return - ErrorCode_t   error code.
*/
ErrorCode_t MM_SVI_Initialise(void)
{
   /* Create the Message Queue */
   SviQ = osMessageCreate(osMessageQ(SviQ), NULL);
   CO_ASSERT_RET_MSG(NULL != SviQ, ERR_FAILED_TO_CREATE_QUEUE_E, "ERROR - Failed to create SVI Queue");
   /* start the SVI Task */
   tid_SVITask = osThreadCreate (osThread(MM_SVI_TaskMain), NULL);
   CO_ASSERT_RET_MSG(NULL != tid_SVITask, ERR_FAILED_TO_CREATE_THREAD_E, "ERROR - Failed to create SVI Task thread");

   return SUCCESS_E;
}

/*************************************************************************************/
/**
* function name   : MM_SVI_BuiltInTestReq
* description     : Requests the built-in test of the Head Interface
*
* @param - none
*
* @return - Error code
*/
ErrorCode_t MM_SVI_BuiltInTestReq(void)
{
   ErrorCode_t ErrorCode = ERR_BUILT_IN_TEST_FAIL_E;

   CO_Message_t* pPhyDataReq = osPoolAlloc(AppPool);
   if (pPhyDataReq)
   {
      pPhyDataReq->Type = CO_MESSAGE_RUN_BUILT_IN_TESTS_E;
      uint8_t* pMsgData = pPhyDataReq->Payload.PhyDataReq.Data;
      memset(pMsgData, 0u, sizeof(HeadMessage_t));
      pPhyDataReq->Payload.PhyDataReq.Size = sizeof(HeadMessage_t);
      
      osStatus osStat = osMessagePut(SviQ, (uint32_t)pPhyDataReq, 0);      
      
      if (osOK != osStat)
      {
         /* failed to put message in the head queue */
         osPoolFree(AppPool, pPhyDataReq);
      }
      else
      {         
         /* Message was queued OK */
         ErrorCode = SUCCESS_E;
      }
   }
   
   return ErrorCode;
}

/*************************************************************************************/
/**
* MM_SVI_TaskMain
* Main function for the Sound and Visual Indicator (SVI) emulation task.
*
* @param - argument  Pointer to passed in data.
*
* @return - void
*/
static void MM_SVI_TaskMain (void const *argument)
{
   osEvent event;
   CO_Message_t* pDataMessage;
   
   uint8_t retries = 0;

   osDelay(5000);
   
   /* Before we start the actual Task, we need to do the Power-Up Built-In Test */
   while ( (false == MM_SviIsSerialNumberValid()) && (3 > retries) )
   {
      retries++;
   }
   if ( 3 <= retries )
   {
      DM_LedPatternRequest(LED_BUILT_IN_TEST_FAIL_E);
      CO_PRINT_A_0(DBG_ERROR_E, "SVI Not Found\r\n");
   }
   else
   {
      CO_PRINT_A_0(DBG_INFO_E, "SVI Found\r\n");
   }
               
   /* Task Core */
   while(1)
   {
      event = osMessageGet(SviQ,osWaitForever);

      if ( (osEventMessage == event.status) && (NULL != event.value.p) )
      {
         pDataMessage = (CO_Message_t*)event.value.p;
         if (pDataMessage)
         {
            if (pDataMessage->Type == CO_MESSAGE_RUN_BUILT_IN_TESTS_E)
            {
               /* the struct commandMessage contains the comand data */
               osPoolFree(AppPool, pDataMessage);

               /* Check if the serial number is valid */
               if ( false == MM_SviIsSerialNumberValid())
               {
                  DM_LedPatternRequest(LED_BUILT_IN_TEST_FAIL_E);
                  CO_PRINT_A_0(DBG_BIT_E, "SVI Fail\r\n");
               }
               else
               {
                  CO_PRINT_B_0(DBG_BIT_E, "SVI Success\r\n");
               }
            }
         }
      }
   }
}

/*************************************************************************************/
/**
* function name   : MM_SviGetSerialNumber
* description     : Read the serial Number from the SVI device
*
* @param - none
*
* @return - true if the serial number is valid
*/
static ErrorCode_t MM_SviGetSerialNumber(uint32_t *SerialNumber)
{
   ErrorCode_t result;
   union
   {
      uint8_t bytes[4];
      uint32_t word;
   }serial_number;
   
//   if ( DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
//   {
//      //Enable the I2C to the sounder board via the sounder/beacon power line
//      GpioWrite(&SndrBeaconOn, 1);
//      osDelay(10);
//   }


   result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_0, &serial_number.bytes[0]);
   if (SUCCESS_E == result)
   {
      result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_1, &serial_number.bytes[1]);
      if (SUCCESS_E == result)
      {
         result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_2, &serial_number.bytes[2]);
         if (SUCCESS_E == result)
         {
            result = DM_SVI_ReadRegister(DM_I2C_SVI_REG_SERIAL_NUMBER_3, &serial_number.bytes[3]);
            if (SUCCESS_E == result)
            {
               /* copy the read value */
               *SerialNumber = serial_number.word;
            }
         }
      }
   }

//   //Only remove power from the 85dB SVI if it is not in alarm state
//   if (  DM_DeviceIsEnabled(gDeviceCombination, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) &&
//         DM_OP_IsActivated(DM_OP_CHANNEL_SOUNDER_E) )
//   {
//      //Disable the I2C to the sounder board via the sounder/beacon power line
//      GpioWrite(&SndrBeaconOn, 0);
//   }

   return result;
}

/*************************************************************************************/
/**
* function name   : MM_SviIsSerialNumberValid
* description     : checks if a serial number is valid
*
* @param - none
*
* @return - true if the serial number is valid
*/
static bool MM_SviIsSerialNumberValid(void)
{
   bool validSerialNumber = false;
   ErrorCode_t result = ERR_BUILT_IN_TEST_FAIL_E;
   uint32_t SerialNumber;


   /* Read the Serial Number */
   result = MM_SviGetSerialNumber(&SerialNumber);

   if (SUCCESS_E == result)
   {
      if ( 0 != SerialNumber )
      {
         validSerialNumber = true;
         CO_PRINT_B_1(DBG_INFO_E, "SVI SRN is %d\r\n", SerialNumber);
      }
   }

   return validSerialNumber;
}
#endif
