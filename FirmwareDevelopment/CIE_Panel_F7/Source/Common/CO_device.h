/***************************************************************************
* File name: CO_device.h
* Project name: CIE_Panel
*
* Copyright 2018 Bull Products Ltd as an unpublished work.
* All Rights Reserved.
*
* The information contained herein is confidential property of Bull
* Products Ltd. The use, copying, transfer or disclosure of such
* information is prohibited except by express written agreement with Bull
* Products Ltd.
*
* First written on 18/05/18 by Matt Ault
*
* File History:
* Change History:
* Rev Date Description Author
* 1.0 18/05/18 Initial File
*
* Description:
* Panel settings class. Site specific data (Stores network, mesh, etc)
*
**************************************************************************/


#ifndef _CO_DEVICE_H_
#define _CO_DEVICE_H_


typedef enum
{
   DEV_SMOKE_DETECTOR_E,
   DEV_HEAT_DETECTOR_TYPE_A1R_E,
   DEV_HEAT_DETECTOR_TYPE_B_E,
   DEV_PIR_DETECTOR_E,
   DEV_FIRE_CALL_POINT_E,
   DEV_FIRST_AID_CALL_POINT_E,
   DEV_SOUNDER_E,
   DEV_BEACON_W_2_4_7_5_E,
   DEV_BEACON_W_3_1_11_3_E,
   DEV_BEACON_C_3_8_9_E,
   DEV_BEACON_C_3_15_E,
   DEV_VISUAL_INDICATOR_E,   /* Not EN54 */
   DEV_SOUNDER_VI_E,         /* Not EN54 */
   DEV_REMOTE_INDICATOR_E,
   DEV_SOUNDER_VI_CONSTR_E,
   DEV_INDICATOR_LED_E,      // Head Indicator LEDs
   DEV_IO_UNIT_INPUT_1_E,
   DEV_IO_UNIT_OUTPUT_1_E,
   DEV_IO_UNIT_INPUT_2_E,
   DEV_IO_UNIT_OUTPUT_2_E,
   DEV_SOUNDER_85DBA_E,
   DEV_MAX_DEV_TYPE_E,
} DM_DeviceType_t;

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
   DC_NUMBER_OF_DEVICE_COMBINATIONS_E
} DM_DeviceCombination_t;


#define DEVICE_TYPE_BUZZER							67
#define DEVICE_TYPE_EVACUATE						66
#define DEVICE_TYPE_FUNCTION_BUTTON				65
#define DEVICE_TYPE_PANEL_IO						64



#define DEV_DEVICE_BIT(X) (1U << DEV_##X##_E)
#define DEV_SMOKE_DETECTOR                 ( DEV_DEVICE_BIT(SMOKE_DETECTOR) )               //1
#define DEV_HEAT_DETECTOR_TYPE_A1R         ( DEV_DEVICE_BIT(HEAT_DETECTOR_TYPE_A1R) )       //2
#define DEV_HEAT_DETECTOR_TYPE_B           ( DEV_DEVICE_BIT(HEAT_DETECTOR_TYPE_B) )         //4
#define DEV_PIR_DETECTOR                   ( DEV_DEVICE_BIT(PIR_DETECTOR) )                 //8
#define DEV_FIRE_CALL_POINT                ( DEV_DEVICE_BIT(FIRE_CALL_POINT) )              //16
#define DEV_FIRST_AID_CALL_POINT           ( DEV_DEVICE_BIT(FIRST_AID_CALL_POINT) )         //32
#define DEV_SOUNDER                        ( DEV_DEVICE_BIT(SOUNDER) )                      //64
#define DEV_BEACON_W_2_4_7_5               ( DEV_DEVICE_BIT(BEACON_W_2_4_7_5) )             //128
#define DEV_BEACON_W_3_1_11_3              ( DEV_DEVICE_BIT(BEACON_W_3_1_11_3) )            //256
#define DEV_BEACON_C_3_8_9                 ( DEV_DEVICE_BIT(BEACON_C_3_8_9) )               //512
#define DEV_BEACON_C_3_15                  ( DEV_DEVICE_BIT(BEACON_C_3_15) )                //1024
#define DEV_VISUAL_INDICATOR               ( DEV_DEVICE_BIT(VISUAL_INDICATOR) )             //2048    /* Not EN54 */
#define DEV_SOUNDER_VI                     ( DEV_DEVICE_BIT(SOUNDER_VI) )                   //4096    /* Not EN54 */
#define DEV_REMOTE_INDICATOR               ( DEV_DEVICE_BIT(REMOTE_INDICATOR) )             //8192
#define DEV_SOUNDER_VI_CONSTR              ( DEV_DEVICE_BIT(SOUNDER_VI_CONSTR) )            //16384
#define DEV_INDICATOR_LED                  ( DEV_DEVICE_BIT(INDICATOR_LED) )                //32768
#define DEV_IO_UNIT_INPUT_1                ( DEV_DEVICE_BIT(IO_UNIT_INPUT_1) )              //65536
#define DEV_IO_UNIT_OUTPUT_1               ( DEV_DEVICE_BIT(IO_UNIT_OUTPUT_1) )             //131072
#define DEV_IO_UNIT_INPUT_2                ( DEV_DEVICE_BIT(IO_UNIT_INPUT_2) )              //262144
#define DEV_IO_UNIT_OUTPUT_2               ( DEV_DEVICE_BIT(IO_UNIT_OUTPUT_2) )             //524288
#define DEV_SOUNDER_85DBA                  ( DEV_DEVICE_BIT(SOUNDER_85DBA) )                //1048576
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


#define DEV_SVI_DEVICES                    ( DEV_SOUNDER | DEV_SOUNDER_VI | DEV_VISUAL_INDICATOR | DEV_SOUNDER_85DBA)

#define DEV_SVI_SOUNDERS                    ( DEV_SOUNDER | DEV_SOUNDER_VI | DEV_SOUNDER_85DBA)


#define DEV_IO_DEVICES                     ( DEV_IO_UNIT_INPUT_1 | DEV_IO_UNIT_OUTPUT_1 | \
                                             DEV_IO_UNIT_INPUT_2 | DEV_IO_UNIT_OUTPUT_2 )

#define DEV_STANDARD_RBU_DEVICES           ( DEV_SMOKE_DETECTOR | DEV_HEAT_DETECTOR_TYPE_A1R | \
                                             DEV_HEAT_DETECTOR_TYPE_B | DEV_PIR_DETECTOR )


#endif
