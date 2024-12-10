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
*  File         : DM_Device.c
*
*  Description  : Inplementation of Cygnus2 Devices
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include "stdlib.h"


/* User Include Files
*************************************************************************************/
#include "CO_Defines.h"
#include "DM_OutputManagement.h"
#include "CFG_Device_cfg.h"
#include "DM_Device.h"


/* Private Functions Prototypes
*************************************************************************************/
static DM_Enable_t channelEnabled[CO_CHANNEL_MAX_E];


/* Global Variables
*************************************************************************************/
DM_BaseType_t gBaseType;
bool gZoneEnabled = true;
bool gRbuEnabled = true;


/* Private Variables
*************************************************************************************/
/* Device Combination definitions from table 24 in Mesh Protocol Design (HKD-16-00150D_B) */

static uint32_t DM_DeviceCombination[] = 
{
   0,
   DEV_SMOKE_DETECTOR | DEV_INDICATOR_LED,
   DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_HEAT_DETECTOR_TYPE_B | DEV_INDICATOR_LED,
   DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_SMOKE_DETECTOR | DEV_PIR_DETECTOR | DEV_INDICATOR_LED,                                                                                                  // 5
   DEV_BEACON_W_2_4_7_5,
   DEV_BEACON_W_3_1_11_3,
   DEV_BEACON_C_3_8_9,
   DEV_BEACON_C_3_15,
   DEV_REMOTE_INDICATOR,                                                                                                                                        // 10
   DEV_SOUNDER_VI | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_SMOKE_DETECTOR | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_HEAT_DETECTOR_TYPE_B | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,                                                                           //15
   DEV_SOUNDER_VI | DEV_SMOKE_DETECTOR | DEV_PIR_DETECTOR | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_BEACON_W_2_4_7_5,
   DEV_SOUNDER_VI | DEV_BEACON_W_3_1_11_3,
   DEV_SOUNDER_VI | DEV_BEACON_C_3_8_9,
   DEV_SOUNDER_VI | DEV_BEACON_C_3_15,                                                                                                                        //20
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SMOKE_DETECTOR | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_HEAT_DETECTOR_TYPE_B | DEV_INDICATOR_LED,
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_INDICATOR_LED,                                                //25
   DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SMOKE_DETECTOR | DEV_PIR_DETECTOR | DEV_INDICATOR_LED,
   DEV_FIRE_CALL_POINT,
   DEV_IO_UNIT_INPUT_1 | DEV_IO_UNIT_OUTPUT_1 | DEV_IO_UNIT_INPUT_2 | DEV_IO_UNIT_OUTPUT_2,
   DEV_FIRE_CALL_POINT | DEV_SOUNDER_85DBA,                                                     // Construction Site Fire combination 1
   DEV_FIRE_CALL_POINT | DEV_SOUNDER_VI_CONSTR,                                                 // Construction Site Fire combination 2                        //30
   DEV_FIRE_CALL_POINT | DEV_PIR_DETECTOR | DEV_SOUNDER_VI_CONSTR,                              // Construction Site Fire combination 3
   DEV_FIRST_AID_CALL_POINT | DEV_SOUNDER_VI_CONSTR,                                            // Construction Site First Aid combination
   DEV_FIRE_CALL_POINT | DEV_FIRST_AID_CALL_POINT | DEV_SOUNDER_VI_CONSTR,                      // Construction Site Fire and First Aid combination 1
   DEV_FIRE_CALL_POINT | DEV_FIRST_AID_CALL_POINT | DEV_PIR_DETECTOR | DEV_SOUNDER_VI_CONSTR,   // Construction Site Fire and First Aid combination 2
   DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_SOUNDER_85DBA | DEV_INDICATOR_LED,                                                                    //35
   DEV_FIRE_CALL_POINT,
   DEV_FIRST_AID_CALL_POINT,
   DEV_FIRE_CALL_POINT | DEV_SOUNDER_85DBA | DEV_PIR_DETECTOR,
   DEV_FIRST_AID_CALL_POINT | DEV_SOUNDER_VI_CONSTR | DEV_PIR_DETECTOR,
   DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | DEV_SOUNDER_85DBA | DEV_PIR_DETECTOR | DEV_INDICATOR_LED,                                                 //40
   DEV_REPEATER
};


/*************************************************************************************/
/**
* DM_DeviceInitialise
* Initialise local properties.
*
* @param - None
*
* @return - None
*/
void DM_DeviceInitialise(void)
{
   for ( uint32_t channel_index = 0; channel_index < CO_CHANNEL_MAX_E; channel_index++ )
   {
      channelEnabled[channel_index] = DM_ENABLE_DAY_AND_NIGHT_E;
   }
}
/*************************************************************************************/
/**
* DM_DeviceIsEnabled
* Routine to check if a device is enabled for a given device combination
* device type is a bit map. If this contains multiple enabled bits then the function
* returns true is one or more devices are enabled.
*
* @param - uint32_t deviceCombination - index into device combination table
* @param - uint32_t deviceType bit field where bits are set for the presence of each device type.
* @param - DM_DeviceMatch_t match.  Set to DC_MATCH_ANY_E or DC_MATCH_ALL
*
* @return - flag - true if enabled
*/
bool DM_DeviceIsEnabled(const uint32_t deviceCombinationIdx, const uint32_t deviceType, const DM_DeviceMatch_t match)
{
   bool isEnabled = false;

   /* check input parameters */
   CO_ASSERT_RET((sizeof(DM_DeviceCombination)/sizeof(uint32_t)) > deviceCombinationIdx, false);
   
   if ( DC_MATCH_ANY_E == match )
   {
      /* check if any device in deviceType is enabled in the selected device combination */
      if ((DM_DeviceCombination[deviceCombinationIdx] & deviceType) > 0)
      {
         isEnabled = true;
      }
   }
   else 
   {
      /* check if all of the devices in deviceType are enabled in the selected device combination */
      if ((DM_DeviceCombination[deviceCombinationIdx] & deviceType) == deviceType)
      {
         isEnabled = true;
      }
   }
   
   return isEnabled;
}


/*************************************************************************************/
/**
* DM_DeviceGetRUChannelIdx
* Routine to derive RU Channel Index from device combination and device type
*
* @param - uint32_t deviceCombination - index into device combination table
* @param - uint32_t deviceType
* @param - uint8_t *pRuChannelIdx - RU channel index (OUTPUT)
*
* @return - status - 0=Success
*/
ErrorCode_t DM_DeviceGetRUChannelIdx(const uint32_t deviceCombinationIdx, const uint32_t deviceType, uint8_t *pRuChannelIdx)
{
   ErrorCode_t status = ERR_OUT_OF_RANGE_E;
   int32_t x = 0;
   uint32_t testBit = 0;
   uint8_t ruChannelIdx = 0;

   /* check input parameters */
   CO_ASSERT_RET((sizeof(DM_DeviceCombination)/sizeof(uint32_t)) > deviceCombinationIdx, ERR_OUT_OF_RANGE_E);
   CO_ASSERT_RET(NULL != pRuChannelIdx, ERR_INVALID_POINTER_E)


   for (x = 0; x < DEV_MAX_DEV_TYPE_E; x++)
   {
      /* set test bit */
      testBit = 1U << x;

      /* check if this device is enabled in the selected device combination */
      if ((DM_DeviceCombination[deviceCombinationIdx] & testBit) > 0)
      {
         /* compare testBit with deviceType */
         if (testBit & deviceType)
         {
            /* pass out ru channel idx */
            *pRuChannelIdx = ruChannelIdx;
            status = SUCCESS_E;
            break;
         }

         /* increment the RU channel index */
         ruChannelIdx++;
      }
   }
   
   return status;
}


/*************************************************************************************/
/**
* DM_DeviceGetDeviceType
* Routine to derive device type from device combination and RU channel index
*
* @param - uint32_t deviceCombination - index into device combination table
* @param - uint8_t ruChannelIdx - RU channel index
* @param - uint32_t *pDeviceType - device type (OUTPUT)
*
* @return - status - 0=Success
*/
ErrorCode_t DM_DeviceGetDeviceType(const uint32_t deviceCombinationIdx, const uint8_t ruChannelIdx, uint32_t *pDeviceType)
{
   ErrorCode_t status = ERR_OUT_OF_RANGE_E;

   /* check input parameters */
   CO_ASSERT_RET((sizeof(DM_DeviceCombination)/sizeof(uint32_t)) > deviceCombinationIdx, ERR_OUT_OF_RANGE_E);
   CO_ASSERT_RET(NULL != pDeviceType, ERR_INVALID_POINTER_E);
   
   switch ( ruChannelIdx )
   {
      case CO_CHANNEL_SMOKE_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_SMOKE_DETECTOR, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_SMOKE_DETECTOR_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_HEAT_B_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_HEAT_DETECTOR_TYPE_B, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_HEAT_DETECTOR_TYPE_B_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_HEAT_A1R_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_HEAT_DETECTOR_TYPE_A1R, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_HEAT_DETECTOR_TYPE_A1R_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_CO_E:
         // optical chamber is part of the smoke sensor
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_SMOKE_DETECTOR, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_SMOKE_DETECTOR_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_PIR_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_PIR_DETECTOR, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_PIR_DETECTOR_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_SOUNDER_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_SOUNDER_VI, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_SOUNDER_VI_E;
            status = SUCCESS_E;
         }
         else if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_SOUNDER_85DBA;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_BEACON_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_BEACON_W_2_4_7_5, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_BEACON_W_2_4_7_5_E;
            status = SUCCESS_E;
         }
         else if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_BEACON_W_3_1_11_3, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_BEACON_W_3_1_11_3_E;
            status = SUCCESS_E;
         }
         else if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_BEACON_C_3_8_9, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_BEACON_C_3_8_9_E;
            status = SUCCESS_E;
         }
         else if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_BEACON_C_3_15, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_BEACON_C_3_15_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_FIRE_CALLPOINT_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_FIRE_CALL_POINT, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_FIRE_CALL_POINT_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_STATUS_INDICATOR_LED_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_INDICATOR_LED, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_INDICATOR_LED_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_VISUAL_INDICATOR_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_VISUAL_INDICATOR, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_VISUAL_INDICATOR_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_SOUNDER_VI_CONSTR, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_SOUNDER_VI_CONSTR_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_INPUT_1_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_IO_UNIT_INPUT_1, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_IO_UNIT_INPUT_1_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_INPUT_2_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_IO_UNIT_INPUT_2, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_IO_UNIT_INPUT_2_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_OUTPUT_1_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_IO_UNIT_OUTPUT_1, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_IO_UNIT_OUTPUT_1_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_OUTPUT_2_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_IO_UNIT_OUTPUT_2, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_IO_UNIT_OUTPUT_2_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_MEDICAL_CALLPOINT_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_FIRST_AID_CALL_POINT, DC_MATCH_ANY_E) )
         {
            *pDeviceType = DEV_FIRST_AID_CALL_POINT_E;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_EVAC_CALLPOINT_E:
         status = ERR_NOT_FOUND_E;
         break;
      default:
         status = ERR_NOT_FOUND_E;
         break;
   }
 
   return status;
}

/*************************************************************************************/
/**
* DM_DeviceGetDeviceCode
* Routine to derive device bitmap value from device combination and RU channel index
*
* @param - uint32_t deviceCombination - index into device combination table
* @param - uint8_t ruChannelIdx - RU channel index
* @param - uint32_t *pDeviceCode - device type (OUTPUT)
*
* @return - status - 0=Success
*/
ErrorCode_t DM_DeviceGetDeviceCode(const uint32_t deviceCombinationIdx, const uint8_t ruChannelIdx, uint32_t *pDeviceCode)
{
   ErrorCode_t status = ERR_OUT_OF_RANGE_E;

   /* check input parameters */
   CO_ASSERT_RET((sizeof(DM_DeviceCombination)/sizeof(uint32_t)) > deviceCombinationIdx, ERR_OUT_OF_RANGE_E);
   CO_ASSERT_RET(NULL != pDeviceCode, ERR_INVALID_POINTER_E);
   
   switch ( ruChannelIdx )
   {
      case CO_CHANNEL_SMOKE_E:
            *pDeviceCode = DEV_SMOKE_DETECTOR;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_HEAT_B_E:
            *pDeviceCode = DEV_HEAT_DETECTOR_TYPE_B;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_HEAT_A1R_E:
            *pDeviceCode = DEV_HEAT_DETECTOR_TYPE_A1R;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_CO_E:
         // optical chamber is part of the smoke sensor
            *pDeviceCode = DEV_SMOKE_DETECTOR;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_PIR_E:
            *pDeviceCode = DEV_PIR_DETECTOR;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_SOUNDER_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_SOUNDER_VI, DC_MATCH_ANY_E) )
         {
            *pDeviceCode = DEV_SOUNDER_VI;
            status = SUCCESS_E;
         }
         else if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_SOUNDER_85DBA, DC_MATCH_ANY_E) )
         {
            *pDeviceCode = DEV_SOUNDER_85DBA;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_BEACON_E:
         if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_BEACON_W_2_4_7_5, DC_MATCH_ANY_E) )
         {
            *pDeviceCode = DEV_BEACON_W_2_4_7_5;
            status = SUCCESS_E;
         }
         else if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_BEACON_W_3_1_11_3, DC_MATCH_ANY_E) )
         {
            *pDeviceCode = DEV_BEACON_W_3_1_11_3;
            status = SUCCESS_E;
         }
         else if ( DM_DeviceIsEnabled(deviceCombinationIdx, DEV_BEACON_C_3_8_9, DC_MATCH_ANY_E) )
         {
            *pDeviceCode = DEV_BEACON_C_3_8_9;
            status = SUCCESS_E;
         }
         else
         {
            *pDeviceCode = DEV_BEACON_C_3_15;
            status = SUCCESS_E;
         }
         break;
      case CO_CHANNEL_FIRE_CALLPOINT_E:
            *pDeviceCode = DEV_FIRE_CALL_POINT;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_STATUS_INDICATOR_LED_E:
            *pDeviceCode = DEV_INDICATOR_LED;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_VISUAL_INDICATOR_E:
            *pDeviceCode = DEV_VISUAL_INDICATOR;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E:
            *pDeviceCode = DEV_SOUNDER_VI_CONSTR;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_INPUT_1_E:
            *pDeviceCode = DEV_IO_UNIT_INPUT_1;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_INPUT_2_E:
            *pDeviceCode = DEV_IO_UNIT_INPUT_2;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_OUTPUT_1_E:
            *pDeviceCode = DEV_IO_UNIT_OUTPUT_1;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_OUTPUT_2_E:
            *pDeviceCode = DEV_IO_UNIT_OUTPUT_2;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_MEDICAL_CALLPOINT_E:
            *pDeviceCode = DEV_FIRST_AID_CALL_POINT;
            status = SUCCESS_E;
         break;
      case CO_CHANNEL_EVAC_CALLPOINT_E:
         status = ERR_NOT_FOUND_E;
         break;
      default:
         status = ERR_NOT_FOUND_E;
         break;
   }
 
   return status;
}

/*****************************************************************************
*  Function:   DM_MapRUChannelIndexToHeadDevice
*  Description:      Converts the RU Channel Index that is defined in the Mesh protocol 
*                    into the device type that is defined in the Head Interface Spec.
*
* @param - uint32_t deviceCombination        - index into device combination table
* @param - uint8_t  RUChannelIndex           - The RU Channel Index, read from the command message.
* @param - MM_HeadDeviceType_t*  pHeadDevice - [OUT] the device type for the RU Channel Index.
*
* @return            SUCCESS_E or error code if something went wrong.
*
*  Notes:            [OUT] parameter pHeadDevice is not modified unless the function succeeds.
*****************************************************************************/
ErrorCode_t DM_MapRUChannelIndexToHeadDevice(const uint32_t deviceCombinationIdx, const uint8_t ruChannelIdx, MM_HeadDeviceType_t* const pHeadDevice)
{
   uint32_t ru_channel_device_type;
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pHeadDevice )
   {
      
      if ( DEV_ALL_DEVICES == ruChannelIdx || CO_CHANNEL_NONE_E == ruChannelIdx)
      {
         *pHeadDevice = DEV_NO_DEVICE_TYPE_E;
         result = SUCCESS_E;
      }
      else 
      {
         /* use the device combination and the RU Channel Index to find the configuration device type */
         result = DM_DeviceGetDeviceType(deviceCombinationIdx, ruChannelIdx, &ru_channel_device_type);
         
         if ( SUCCESS_E == result )
         {
            /* convert the configuration device type to a Head device type */
            switch ( ru_channel_device_type )
            {
               case DEV_SMOKE_DETECTOR_E:
                  *pHeadDevice = DEV_SMOKE_DETECTOR_HEAD_E;
                  break;
               case DEV_HEAT_DETECTOR_TYPE_A1R_E:
                  *pHeadDevice = DEV_HEAT_DETECTOR_HEAD_E;
                  break;
              case DEV_HEAT_DETECTOR_TYPE_B_E:
                  *pHeadDevice = DEV_HEAT_DETECTOR_HEAD_E;
                  break;
               case DEV_PIR_DETECTOR_E:
                  *pHeadDevice = DEV_PIR_DETECTOR_HEAD_E;
                  break;
               case DEV_FIRE_CALL_POINT_E:
                  *pHeadDevice = DEV_CALL_POINT_E;
                  break;
               case DEV_FIRST_AID_CALL_POINT_E:
                  *pHeadDevice = DEV_CALL_POINT_E;
                  break;
               case DEV_BEACON_W_2_4_7_5_E:
                  *pHeadDevice = DEV_BEACON_HEAD_E;
                  break;
               case DEV_BEACON_W_3_1_11_3_E:
                  *pHeadDevice = DEV_BEACON_HEAD_E;
                  break;
               case DEV_BEACON_C_3_8_9_E:
                  *pHeadDevice = DEV_BEACON_HEAD_E;
                  break;
               case DEV_BEACON_C_3_15_E:
                  *pHeadDevice = DEV_BEACON_HEAD_E;
                  break;
               case DEV_INDICATOR_LED_E:
                  *pHeadDevice = DEV_INDICATOR_LED_HEAD_E;
                  break;
               case DEV_IO_UNIT_INPUT_1_E:
                  *pHeadDevice = DEV_IO_UNIT_E;
               break;
               case DEV_IO_UNIT_OUTPUT_1_E:
                  *pHeadDevice = DEV_IO_UNIT_E;
               break;
               case DEV_IO_UNIT_INPUT_2_E:
                  *pHeadDevice = DEV_IO_UNIT_E;
               break;
                case DEV_IO_UNIT_OUTPUT_2_E:
                  *pHeadDevice = DEV_IO_UNIT_E;
               break;
               default:
                  result = ERR_NOT_FOUND_E;
                  break;
            }
         }
      }
   }
   
   return result;
}

/*****************************************************************************
*  Function:   DM_MapHeadDeviceToRUChannelIndex
*  Description:      Converts a head interface device type into a RU Channel Index.
*
* @param - uint32_t deviceCombination - index into device combination table
* @param - MM_HeadDeviceType_t head_device - The head device type.
* @param - uint8_t* pRUChannelIdx - [OUT]The corresponding RU Channel Index.
*
* @return            SUCCESS_E or error code if something went wrong.
*
*  Notes:            [OUT] parameter pRUChannelIdx is not modified unless the function succeeds.
*****************************************************************************/
ErrorCode_t DM_MapHeadDeviceToRUChannelIndex(const uint32_t deviceCombinationIdx, const MM_HeadDeviceType_t headDevice,  uint8_t* const pRUChannelIdx)
{
   uint32_t device_type = 0;
   ErrorCode_t result = ERR_INVALID_POINTER_E;
   
   if ( pRUChannelIdx )
   {
      /* convert the Head device type to a  configuration device type*/
      switch ( headDevice )
      {
         case DEV_SMOKE_DETECTOR_HEAD_E:
            device_type = DEV_SMOKE_DETECTOR;
            break;
         case DEV_HEAT_DETECTOR_HEAD_E:
            device_type = DEV_HEAT_DETECTOR_TYPE_A1R | DEV_HEAT_DETECTOR_TYPE_B;
            break;
         case DEV_PIR_DETECTOR_HEAD_E:
            device_type = DEV_PIR_DETECTOR;
            break;
         case DEV_BEACON_HEAD_E:
            device_type = DEV_BEACON_W_2_4_7_5 | DEV_BEACON_W_3_1_11_3 | DEV_BEACON_C_3_8_9 | DEV_BEACON_C_3_15;
            break;
         case DEV_CALL_POINT_E:
            device_type = DEV_FIRE_CALL_POINT | DEV_FIRST_AID_CALL_POINT;
            break;
         case DEV_IO_UNIT_E:
            device_type = DEV_IO_UNIT_INPUT_1 | DEV_IO_UNIT_OUTPUT_1 | DEV_IO_UNIT_INPUT_2 | DEV_IO_UNIT_OUTPUT_2;
            break;
         case DEV_CO_DETECTOR_HEAD_E:
            /* we have no mapping for CO Detector */
            device_type = 0;
            break;
         case DEV_INDICATOR_LED_HEAD_E:
            device_type = DEV_INDICATOR_LED;
            break;
         case DEV_SPARE_1_E:
            device_type = 0;
            break;
         case DEV_SPARE_2_E:
            device_type = 0;
            break;
         default:
            device_type = 0;
            break;
      }
      
      /* use the device combination and the head device to get the RU Channel Index */
      uint8_t channel;
      result = DM_DeviceGetRUChannelIdx(deviceCombinationIdx, device_type, &channel);
      
      if ( SUCCESS_E == result )
      {
         *pRUChannelIdx = channel;
      }
   }
   
   return result;
}

/*************************************************************************************/
/**
* DM_DeviceGetBaseType
* Function to figure out the role of the radio board e.g. NCU, RBU, CONSRUCTION etc
*
* @param - uint32_t deviceCombination - index into device combination table
* @param - bool isNCU - flag - true for NCU
*
* @return - enum identifying the RBU role.
*/
DM_BaseType_t DM_DeviceGetBaseType(const uint32_t deviceCombinationIdx, bool isNCU)
{
   DM_BaseType_t base_type = BASE_UNKNOWN_E;
   
   if ( isNCU )
   {
      base_type = BASE_NCU_E;
   }
   else
   {
      switch (deviceCombinationIdx)
      {
         case DC_NO_DEVICES_FITTED_E://intentional drop through
         case DC_REPEATER_E:
            base_type = BASE_REPEATER_E;
         break;
         case DC_SMOKE_DETECTOR_E:  //intentional drop through
         case DC_HEAT_DETECTOR_TYPE_A1R_E:
         case DC_HEAT_DETECTOR_TYPE_B_E:
         case DC_SMOKE_AND_HEAT_DETECTOR_TYPE_A1R_E:
         case DC_SMOKE_AND_PIR_DETECTOR_E:
         case DC_BEACON_W_2_4_7_5_E:
         case DC_BEACON_W_3_1_11_3_E:
         case DC_BEACON_C_3_8_9_E:
         case DC_BEACON_C_3_15_E:
         case DC_REMOTE_INDICATOR_E:
         case DC_SOUNDER_E:
         case DC_SOUNDER_SMOKE_DETECTOR_E:
         case DC_SOUNDER_HEAT_DETECTOR_TYPE_A1R_E:
         case DC_SOUNDER_HEAT_DETECTOR_TYPE_B_E:
         case DC_SOUNDER_SMOKE_HEAT_DETECTOR_TYPE_A1R_E:
         case DC_SOUNDER_SMOKE_PIR_E:
         case DC_SOUNDER_BEACON_W_2_4_7_5_E:
         case DC_SOUNDER_BEACON_W_3_1_11_3_E:
         case DC_SOUNDER_BEACON_C_3_8_9_E:
         case DC_SOUNDER_BEACON_C_3_15_E:
         case DC_SOUNDER_VI_E:
         case DC_SOUNDER_VI_SMOKE_DETECTOR_E:
         case DC_SOUNDER_VI_HEAT_DETECTOR_TYPE_A1R_E:
         case DC_SOUNDER_VI_HEAT_DETECTOR_TYPE_B_E:
         case DC_SOUNDER_VI_SMOKE_HEAT_DETECTOR_TYPE_A1R_E:
         case DC_SOUNDER_VI_SMOKE_PIR_E:
            base_type = BASE_RBU_E;
            break;
         
         case DC_FIRE_CALL_POINT_RADIO_BASE_E:
            base_type = BASE_MCP_E;
            break;
         
         case DC_IO_UNIT_E:
            base_type = BASE_IOU_E;
            break;
         
         case DC_CONSTRUCTION_SITE_FIRE_COMBINATION_1_E:  //intentional drop through
         case DC_CONSTRUCTION_SITE_FIRE_COMBINATION_2_E:
         case DC_CONSTRUCTION_SITE_FIRE_COMBINATION_3_E:
         case DC_CONSTRUCTION_SITE_FIRST_AID_COMBINATION_E:
         case DC_CONSTRUCTION_SITE_FIRE_AND_FIRST_AID_COMBINATION_1_E:
         case DC_CONSTRUCTION_SITE_FIRE_AND_FIRST_AID_COMBINATION_2_E:
         case DC_CONSTRUCTION_SITE_SMOKE_HEAT_DETECTOR_TYPE_A1R_SOUNDER_E:
         case DC_FIRE_CALL_POINT_E:
         case DC_FIRST_AID_CALL_POINT_E:
         case DC_CONSTRUCTION_SITE_FIRE_CALL_POINT_SOUNDER_E:
         case DC_CONSTRUCTION_SITE_FIRST_AID_CALL_POINT_SOUNDER_PIR_E:
         case DC_CONSTRUCTION_SITE_SMOKE_AND_HEAT_DETECTOR_TYPE_A1R_SOUNDER_PIR_E:
            base_type = BASE_CONSTRUCTION_E;
            break;
         
         default:
            break;
      }
   }

   return base_type;
}


/*************************************************************************************/
/**
* DM_DeviceGetPA2Function
* Routine to derive the function for pin PA_2 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pa2Function_t DM_DeviceGetPA2Function(const DM_BaseType_t basetype)
{
  pa2Function_t function = PA2_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PA2_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_RBU_E: /* intentional drop-through */
      case BASE_MCP_E:
      case BASE_IOU_E:
         function = PA2_BAT_I1_MON_E;
         break;
      case BASE_CONSTRUCTION_E:
         function = PA2_AN_BKP_CELL_1_MON_E;
      break;
      default:
         function = PA2_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPA3Function
* Routine to derive the function for pin PA_3 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pa3Function_t DM_DeviceGetPA3Function(const DM_BaseType_t basetype)
{
  pa3Function_t function = PA3_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PA3_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_RBU_E: /* intentional drop-through */
      case BASE_MCP_E:
      case BASE_IOU_E:
         function = PA3_BAT_I2_MON_E;
         break;
      case BASE_CONSTRUCTION_E:
         function = PA3_AN_BKP_BAT_MON_E;
      break;
      default:
         function = PA3_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPA11Function
* Routine to derive the function for pin PA_11 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pa11Function_t DM_DeviceGetPA11Function(const DM_BaseType_t basetype)
{
  pa11Function_t function = PA11_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PA11_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_RBU_E:
         function = PA11_SVI_PWR_ON_E;
         break;
      case BASE_IOU_E:
         function = PA11_IOD_SET_E;
         break;
      case BASE_CONSTRUCTION_E:
         function = PA11_SNDR_BEACON_ON_E;
      break;
      default:
         function = PA11_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPB1Function
* Routine to derive the function for pin PB_1 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pb1Function_t DM_DeviceGetPB1Function(const DM_BaseType_t basetype)
{
  pb1Function_t function = PB1_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PB1_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_RBU_E:
         function = PB1_HEAD_PWR_ON_E;
         break;
      case BASE_IOU_E:
         function = PB1_EN_DET_1_E;
         break;
      default:
         function = PB1_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPB14Function
* Routine to derive the function for pin PB_14 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pb14Function_t DM_DeviceGetPB14Function(const DM_BaseType_t basetype)
{
  pb14Function_t function = PB14_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PB14_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_CONSTRUCTION_E:
         function = PB14_VBAT_MON_EN_E;
         break;
      case BASE_IOU_E:
         function = PB14_DET_R1_E;
         break;
      default:
         function = PB14_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPC0Function
* Routine to derive the function for pin PC_0 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pc0Function_t DM_DeviceGetPC0Function(const DM_BaseType_t basetype)
{
  pc0Function_t function = PC0_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PC0_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_RBU_E: /* intentional drop-through */
      case BASE_CONSTRUCTION_E:
      case BASE_MCP_E:
         function = PC0_HEAD_RX_E;
         break;
      case BASE_IOU_E:
         function = PC0_FAULT_DET_1_E;
      break;
      default:
         function = PC0_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPC1Function
* Routine to derive the function for pin PC_1 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pc1Function_t DM_DeviceGetPC1Function(const DM_BaseType_t basetype)
{
  pc1Function_t function = PC1_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PC1_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_CONSTRUCTION_E: /* intentional drop-through */
      case BASE_MCP_E:
      case BASE_RBU_E:
         function = PC1_HEAD_TX_E;
         break;
      case BASE_IOU_E:
         function = PC1_FAULT_DET_2_E;
      break;
      default:
         function = PC1_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPC2Function
* Routine to derive the function for pin PC_2 from the base type of the device.
*
* @param - DM_BaseType_t basetype - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pc2Function_t DM_DeviceGetPC2Function(const DM_BaseType_t basetype)
{
  pc2Function_t function = PC2_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  basetype), PC2_NOT_USED_E);

   switch ( basetype )
   {
      case BASE_MCP_E:
      case BASE_IOU_E:
         function = PC2_BAT_I3_MON_E;
         break;
      case BASE_CONSTRUCTION_E:
         function = PC2_AN_VBAT_MON_E;
      break;
      default:
         function = PC2_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_DeviceGetPC13Function
* Routine to derive the function for pin PC_13 from the base type of the device.
*
* @param - DM_BaseType_t baseType - The role of the radio board (NCU, RBU, MCP etc)
*
* @return - function
*/
pc13Function_t DM_DeviceGetPC13Function(const DM_BaseType_t baseType)
{
  pc13Function_t function = PC13_NOT_USED_E;

   /* check input parameters */
   CO_ASSERT_RET((BASE_UNKNOWN_E !=  baseType), PC13_NOT_USED_E);

   switch ( baseType )
   {
      case BASE_RBU_E://Intentional drop-through
      case BASE_IOU_E:
      case BASE_MCP_E:
      case BASE_CONSTRUCTION_E:
         function = PC13_BKP_BATTERY_EN_E;
      break;
      case BASE_NCU_E:
         function = PC13_NOT_USED_E;
         break;
      default:
         function = PC13_NOT_USED_E;
         break;
   }
   
   return function;
}

/*************************************************************************************/
/**
* DM_SetDeviceEnabled
* Set the global enable for alarm delays.
*
* @param  - bool  True to enable all outputs, false to disable all outputs
*
* @return - None.
*/
void DM_SetRbuEnabled(const bool enable)
{
   gRbuEnabled = enable;
}

/*************************************************************************************/
/**
* DM_DeviceEnabled
* Returns the enabled state of the RBU.
*
* @param  - None.
*
* @return - True if the RBU outputs are enabled, false if disabled.
*/
bool DM_RbuEnabled(void)
{
   return gRbuEnabled;
}

///*************************************************************************************/
///**
//* DM_OP_SetDeviceEnabled
//* Set the global enable for alarm delays.
//*
//* @param  - bool  True to enable all outputs, false to disable all outputs
//*
//* @return - None.
//*/
//void DM_SetZoneEnabled(const bool enable)
//{
//   gZoneEnabled = enable;
//}

///*************************************************************************************/
///**
//* DM_ZoneEnabled
//* Returns the enabled state of the zone.
//*
//* @param  - None.
//*
//* @return - True if the zone is enabled, false if disabled.
//*/
//bool DM_ZoneEnabled(void)
//{
//   return gZoneEnabled;
//}

/*************************************************************************************/
/**
* DM_SetChannelEnabled
* Set the channel enable to allow alarms to be issued or blocked.
*
* @param  - channelIndex   The index of the channel to enable/disable.
* @param  - enable         True to enable, false to disable.
*
* @return - None.
*/
void DM_SetChannelEnabled(const CO_ChannelIndex_t channelIndex, const DM_Enable_t enable)
{
   if ( CO_CHANNEL_MAX_E > channelIndex )
   {
      channelEnabled[channelIndex] = enable;
   }
}

/*************************************************************************************/
/**
* DM_ChannelEnabled
* Returns the enabled state of the channel.
*
* @param  - channelIndex   The index of the channel to be checked.
*
* @return - True if the channel is enabled, false if disabled.
*/
bool DM_ChannelEnabled(const CO_ChannelIndex_t channelIndex)
{
   bool enable_state = false;
   
   if ( (CO_CHANNEL_MAX_E > channelIndex) &&
        (DM_RbuEnabled()) &&
        (CFG_ZoneEnabled()) )
   {
      if ( CO_DAY_E == CFG_GetDayNight() )
      {
         //day time
         if ( (DM_ENABLE_DAY_E == channelEnabled[channelIndex]) || (DM_ENABLE_DAY_AND_NIGHT_E == channelEnabled[channelIndex]) )
         {
            enable_state = true;
         }
         else
         {
            enable_state = false;
         }
      }
      else 
      {
         //night time
         if ( (DM_ENABLE_NIGHT_E == channelEnabled[channelIndex]) || (DM_ENABLE_DAY_AND_NIGHT_E == channelEnabled[channelIndex]) )
         {
            enable_state = true;
         }
         else
         {
            enable_state = false;
         }
      }
   }
   return enable_state;
}

