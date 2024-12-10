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
*  File         : DM_Device.h
*
*  Description  : Header for Cygnus2 Devices
*
*************************************************************************************/

#ifndef DM_DEVICE_H
#define DM_DEVICE_H


/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include <stdbool.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "board.h"
#ifdef USE_NEW_HEAD_INTERFACE
#include "MM_PluginInterfaceTask.h"
#else
#include "MM_HeadInterfaceTask.h"
#endif



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/
/* Device Type definitions from table 23 in Mesh Protocol Design (HKD-16-00150D_B) */
typedef enum
{
   DEV_SMOKE_DETECTOR_E,
   DEV_HEAT_DETECTOR_TYPE_A1R_E,
   DEV_HEAT_DETECTOR_TYPE_B_E,
   DEV_PIR_DETECTOR_E,
   DEV_FIRE_CALL_POINT_E,
   DEV_FIRST_AID_CALL_POINT_E,
   DEV_BEACON_W_2_4_7_5_E,
   DEV_BEACON_W_3_1_11_3_E,
   DEV_BEACON_C_3_8_9_E,
   DEV_BEACON_C_3_15_E,
   DEV_VISUAL_INDICATOR_E,
   DEV_SOUNDER_VI_E,
   DEV_REMOTE_INDICATOR_E,
   DEV_SOUNDER_VI_CONSTR_E,
   DEV_INDICATOR_LED_E,      // Head Indicator LEDs
   DEV_IO_UNIT_INPUT_1_E,
   DEV_IO_UNIT_OUTPUT_1_E,
   DEV_IO_UNIT_INPUT_2_E,
   DEV_IO_UNIT_OUTPUT_2_E,
   DEV_SOUNDER_85DBA_E,
   DEV_REPEATER_E,
   DEV_MAX_DEV_TYPE_E
} DM_DeviceType_t;


/* Each device type is assigned a bit in a uint32_t */
#define DEV_DEVICE_BIT(X) (1U << DEV_##X##_E)
#define DEV_SMOKE_DETECTOR                 ( DEV_DEVICE_BIT(SMOKE_DETECTOR) )               //1
#define DEV_HEAT_DETECTOR_TYPE_A1R         ( DEV_DEVICE_BIT(HEAT_DETECTOR_TYPE_A1R) )       //2
#define DEV_HEAT_DETECTOR_TYPE_B           ( DEV_DEVICE_BIT(HEAT_DETECTOR_TYPE_B) )         //4
#define DEV_PIR_DETECTOR                   ( DEV_DEVICE_BIT(PIR_DETECTOR) )                 //8
#define DEV_FIRE_CALL_POINT                ( DEV_DEVICE_BIT(FIRE_CALL_POINT) )              //16
#define DEV_FIRST_AID_CALL_POINT           ( DEV_DEVICE_BIT(FIRST_AID_CALL_POINT) )         //32
#define DEV_BEACON_W_2_4_7_5               ( DEV_DEVICE_BIT(BEACON_W_2_4_7_5) )             //64
#define DEV_BEACON_W_3_1_11_3              ( DEV_DEVICE_BIT(BEACON_W_3_1_11_3) )            //128
#define DEV_BEACON_C_3_8_9                 ( DEV_DEVICE_BIT(BEACON_C_3_8_9) )               //256
#define DEV_BEACON_C_3_15                  ( DEV_DEVICE_BIT(BEACON_C_3_15) )                //512
#define DEV_VISUAL_INDICATOR               ( DEV_DEVICE_BIT(VISUAL_INDICATOR) )             //1024
#define DEV_SOUNDER_VI                     ( DEV_DEVICE_BIT(SOUNDER_VI) )                   //2048
#define DEV_REMOTE_INDICATOR               ( DEV_DEVICE_BIT(REMOTE_INDICATOR) )             //4096
#define DEV_SOUNDER_VI_CONSTR              ( DEV_DEVICE_BIT(SOUNDER_VI_CONSTR) )            //8192
#define DEV_INDICATOR_LED                  ( DEV_DEVICE_BIT(INDICATOR_LED) )                //16384
#define DEV_IO_UNIT_INPUT_1                ( DEV_DEVICE_BIT(IO_UNIT_INPUT_1) )              //32768
#define DEV_IO_UNIT_OUTPUT_1               ( DEV_DEVICE_BIT(IO_UNIT_OUTPUT_1) )             //65536
#define DEV_IO_UNIT_INPUT_2                ( DEV_DEVICE_BIT(IO_UNIT_INPUT_2) )              //131072
#define DEV_IO_UNIT_OUTPUT_2               ( DEV_DEVICE_BIT(IO_UNIT_OUTPUT_2) )             //262144
#define DEV_SOUNDER_85DBA                  ( DEV_DEVICE_BIT(SOUNDER_85DBA) )                //524288
#define DEV_REPEATER                       ( DEV_DEVICE_BIT(REPEATER) )                     //1048576
#define DEV_ALL_DEVICES                    0xffU

/* combinations of devices */
#define DEV_HEAD_DEVICES                   ( DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | \
                                             DEV_HEAT_DETECTOR_TYPE_B | \
                                             DEV_BEACON_W_2_4_7_5 | DEV_BEACON_W_3_1_11_3 | \
                                             DEV_BEACON_C_3_8_9 | DEV_BEACON_C_3_15 )

#define DEV_HEAD_BEACONS                   ( DEV_BEACON_W_2_4_7_5 | DEV_BEACON_W_3_1_11_3 | \
                                             DEV_BEACON_C_3_8_9 | DEV_BEACON_C_3_15 )

#define DEV_HEAD_OUTPUTS                   ( DEV_BEACON_W_2_4_7_5 | DEV_BEACON_W_3_1_11_3 | \
                                             DEV_BEACON_C_3_8_9 | DEV_BEACON_C_3_15 | DEV_INDICATOR_LED )


#define DEV_SVI_DEVICES                    ( DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SOUNDER_85DBA )

#define DEV_SVI_SOUNDERS                   ( DEV_SOUNDER_VI | DEV_SOUNDER_85DBA )


#define DEV_IO_DEVICES                     ( DEV_IO_UNIT_INPUT_1 | DEV_IO_UNIT_OUTPUT_1 | \
                                             DEV_IO_UNIT_INPUT_2 | DEV_IO_UNIT_OUTPUT_2 )

#define DEV_STANDARD_RBU_DEVICES           ( DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | \
                                             DEV_HEAT_DETECTOR_TYPE_B | DEV_PIR_DETECTOR )


/* Device Combination definitions from table 24 in Mesh Protocol Design (HKD-16-00150D_B) */
typedef enum
{
   DC_NO_DEVICES_FITTED_E,                            // 0
   DC_SMOKE_DETECTOR_E,
   DC_HEAT_DETECTOR_TYPE_A1R_E,
   DC_HEAT_DETECTOR_TYPE_B_E,
   DC_SMOKE_AND_HEAT_DETECTOR_TYPE_A1R_E,
   DC_SMOKE_AND_PIR_DETECTOR_E,                       // 5
   DC_BEACON_W_2_4_7_5_E,
   DC_BEACON_W_3_1_11_3_E,
   DC_BEACON_C_3_8_9_E,
   DC_BEACON_C_3_15_E,
   DC_REMOTE_INDICATOR_E,                             // 10
   DC_SOUNDER_E,
   DC_SOUNDER_SMOKE_DETECTOR_E,
   DC_SOUNDER_HEAT_DETECTOR_TYPE_A1R_E,
   DC_SOUNDER_HEAT_DETECTOR_TYPE_B_E,
   DC_SOUNDER_SMOKE_HEAT_DETECTOR_TYPE_A1R_E,         // 15
   DC_SOUNDER_SMOKE_PIR_E,
   DC_SOUNDER_BEACON_W_2_4_7_5_E,
   DC_SOUNDER_BEACON_W_3_1_11_3_E,
   DC_SOUNDER_BEACON_C_3_8_9_E,
   DC_SOUNDER_BEACON_C_3_15_E,                        //20
   DC_SOUNDER_VI_E,
   DC_SOUNDER_VI_SMOKE_DETECTOR_E,
   DC_SOUNDER_VI_HEAT_DETECTOR_TYPE_A1R_E,
   DC_SOUNDER_VI_HEAT_DETECTOR_TYPE_B_E,
   DC_SOUNDER_VI_SMOKE_HEAT_DETECTOR_TYPE_A1R_E,      //25
   DC_SOUNDER_VI_SMOKE_PIR_E,
   DC_FIRE_CALL_POINT_E,
   DC_IO_UNIT_E,
   DC_CONSTRUCTION_SITE_FIRE_COMBINATION_1_E,
   DC_CONSTRUCTION_SITE_FIRE_COMBINATION_2_E,         // 30
   DC_CONSTRUCTION_SITE_FIRE_COMBINATION_3_E,
   DC_CONSTRUCTION_SITE_FIRST_AID_COMBINATION_E,
   DC_CONSTRUCTION_SITE_FIRE_AND_FIRST_AID_COMBINATION_1_E,
   DC_CONSTRUCTION_SITE_FIRE_AND_FIRST_AID_COMBINATION_2_E,
   DC_CONSTRUCTION_SITE_SMOKE_HEAT_DETECTOR_TYPE_A1R_SOUNDER_E,  //35
   DC_FIRE_CALL_POINT_RADIO_BASE_E,
   DC_FIRST_AID_CALL_POINT_E,
   DC_CONSTRUCTION_SITE_FIRE_CALL_POINT_SOUNDER_E,
   DC_CONSTRUCTION_SITE_FIRST_AID_CALL_POINT_SOUNDER_PIR_E,
   DC_CONSTRUCTION_SITE_SMOKE_AND_HEAT_DETECTOR_TYPE_A1R_SOUNDER_PIR_E,  // 40
   DC_REPEATER_E,
   DC_NUMBER_OF_DEVICE_COMBINATIONS_E
} DM_DeviceCombination_t;


typedef enum
{
   DC_MATCH_ANY_E,
   DC_MATCH_ALL_E,
} DM_DeviceMatch_t;

typedef enum{
   DM_ENABLE_NONE_E = 0,
   DM_ENABLE_DAY_E = 1,
   DM_ENABLE_NIGHT_E = 2,
   DM_ENABLE_DAY_AND_NIGHT_E = 3,
   DM_ENABLE_MAX_E = 4
} DM_Enable_t;

/* Public Functions Prototypes
*************************************************************************************/
bool DM_DeviceIsEnabled(const uint32_t deviceCombinationIdx, const uint32_t deviceType, const DM_DeviceMatch_t match);
ErrorCode_t DM_DeviceGetRUChannelIdx(const uint32_t deviceCombinationIdx, const uint32_t deviceType, uint8_t *pRuChannelIdx);
ErrorCode_t DM_DeviceGetDeviceType(const uint32_t deviceCombinationIdx, const uint8_t ruChannelIdx, uint32_t *pDeviceType);
ErrorCode_t DM_DeviceGetDeviceCode(const uint32_t deviceCombinationIdx, const uint8_t ruChannelIdx, uint32_t *pDeviceCode);
ErrorCode_t DM_MapRUChannelIndexToHeadDevice(const uint32_t deviceCombinationIdx, const uint8_t ruChannelIdx, MM_HeadDeviceType_t* const pHeadDevice);
ErrorCode_t DM_MapHeadDeviceToRUChannelIndex(const uint32_t deviceCombinationIdx, const MM_HeadDeviceType_t headDevice,  uint8_t* const pRUChannelIdx);
DM_BaseType_t DM_DeviceGetBaseType(const uint32_t deviceCombinationIdx, bool isNCU);

void DM_DeviceInitialise(void);
void DM_SetRbuEnabled(const bool enabled);
bool DM_RbuEnabled(void);
//void DM_SetZoneEnabled(const bool enabled);
//bool DM_ZoneEnabled(void);
void DM_SetChannelEnabled(const CO_ChannelIndex_t channelIndex, const DM_Enable_t enable);
bool DM_ChannelEnabled(const CO_ChannelIndex_t channelIndex);

pa2Function_t DM_DeviceGetPA2Function(const DM_BaseType_t basetype);
pa3Function_t DM_DeviceGetPA3Function(const DM_BaseType_t basetype);
pa11Function_t DM_DeviceGetPA11Function(const DM_BaseType_t basetype);
pb1Function_t DM_DeviceGetPB1Function(const DM_BaseType_t basetype);
pb14Function_t DM_DeviceGetPB14Function(const DM_BaseType_t basetype);
pc0Function_t DM_DeviceGetPC0Function(const DM_BaseType_t basetype);
pc1Function_t DM_DeviceGetPC1Function(const DM_BaseType_t basetype);
pc2Function_t DM_DeviceGetPC2Function(const DM_BaseType_t basetype);
pc13Function_t DM_DeviceGetPC13Function(const DM_BaseType_t basetype);

/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/




#endif // DM_DEVICE_H
