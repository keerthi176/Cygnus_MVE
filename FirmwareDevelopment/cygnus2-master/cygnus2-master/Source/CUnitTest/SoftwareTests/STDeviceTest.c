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
*  File         : STNVMTest.c
*
*  Description  : Implementation for the Cygnus2 Device tests
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>



/* User Include Files
*************************************************************************************/
#include "CUnit.h"
#include "STDeviceTest.h"
#include "DM_Device.h"



/* Private Functions Prototypes
*************************************************************************************/
static void DEV_IsEnabledTest(void);
static void DEV_GetRUChannelIdxTest(void);
static void DEV_GetDeviceTypeTest(void);
static void DEV_InvalidParamsTest(void);
static void DEV_PinConfigurationTest(void);



/* Global Variables
*************************************************************************************/

/* Table containing the test settings */
CU_TestInfo ST_DeviceTests[] =
{
   { "IsEnabledTest",               DEV_IsEnabledTest },
   { "GetRUChannelIdxTest",         DEV_GetRUChannelIdxTest },
   { "GetDeviceTypeTest",           DEV_GetDeviceTypeTest },
   { "InvalidParamsTest",           DEV_InvalidParamsTest },
   { "PinConfigurationTest",        DEV_PinConfigurationTest },

   CU_TEST_INFO_NULL,
};



/* Private Variables
*************************************************************************************/



/*************************************************************************************/
/**
* DEV_IsEnabledTest
* test is enabled function
*
* no params
*
* @return - none

*/
static void DEV_IsEnabledTest(void)
{
   bool isEnabled = false;

   isEnabled = DM_DeviceIsEnabled(0, DEV_SMOKE_DETECTOR);
   CU_ASSERT_EQUAL(isEnabled, false);

   isEnabled = DM_DeviceIsEnabled(1, DEV_SMOKE_DETECTOR);
   CU_ASSERT_EQUAL(isEnabled, true);

   isEnabled = DM_DeviceIsEnabled(1, DEV_PIR_DETECTOR);
   CU_ASSERT_EQUAL(isEnabled, false);

   isEnabled = DM_DeviceIsEnabled(5, DEV_SMOKE_DETECTOR);
   CU_ASSERT_EQUAL(isEnabled, true);

   isEnabled = DM_DeviceIsEnabled(5, DEV_PIR_DETECTOR);
   CU_ASSERT_EQUAL(isEnabled, true);

   isEnabled = DM_DeviceIsEnabled(32, DEV_FIRST_AID_CALL_POINT);
   CU_ASSERT_EQUAL(isEnabled, true);

   isEnabled = DM_DeviceIsEnabled(32, DEV_SOUNDER);
   CU_ASSERT_EQUAL(isEnabled, true);

   isEnabled = DM_DeviceIsEnabled(32, DEV_SMOKE_DETECTOR);
   CU_ASSERT_EQUAL(isEnabled, false);
}


/*************************************************************************************/
/**
* DEV_GetRUChannelIdxTest
* test get RU channel Idx function
*
* no params
*
* @return - none

*/
static void DEV_GetRUChannelIdxTest(void)
{
   ErrorCode_t ret = SUCCESS_E;
   uint8_t ruChannelIdx = 0;

   /* test device combination 1 = smoke only */
   ret = DM_DeviceGetRUChannelIdx(1, DEV_SMOKE_DETECTOR, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 0);

   /* test device combination 16 = sounder + smoke + PIR */
   ret = DM_DeviceGetRUChannelIdx(16, DEV_SMOKE_DETECTOR, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 0);

   ret = DM_DeviceGetRUChannelIdx(16, DEV_PIR_DETECTOR, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 1);

   ret = DM_DeviceGetRUChannelIdx(16, DEV_SOUNDER, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 2);

   /* test device combination 28 = IO unit with 2 channels in and 2 channels out */
   ret = DM_DeviceGetRUChannelIdx(28, DEV_IO_UNIT_INPUT_1, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 0);

   ret = DM_DeviceGetRUChannelIdx(28, DEV_IO_UNIT_OUTPUT_1, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 1);

   ret = DM_DeviceGetRUChannelIdx(28, DEV_IO_UNIT_INPUT_2, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 2);

   ret = DM_DeviceGetRUChannelIdx(28, DEV_IO_UNIT_OUTPUT_2, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 3);

   /* test device combination 34 = fire CP + first aid CP + sounder + visual indicator + PIR */
   ret = DM_DeviceGetRUChannelIdx(34, DEV_PIR_DETECTOR, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 0);

   ret = DM_DeviceGetRUChannelIdx(34, DEV_FIRE_CALL_POINT, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 1);

   ret = DM_DeviceGetRUChannelIdx(34, DEV_FIRST_AID_CALL_POINT, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 2);

   ret = DM_DeviceGetRUChannelIdx(34, DEV_SOUNDER, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 3);

   ret = DM_DeviceGetRUChannelIdx(34, DEV_VISUAL_INDICATOR, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(ruChannelIdx, 4);
}


/*************************************************************************************/
/**
* DEV_GetDeviceTypeTest
* test get Device type function
*
* no params
*
* @return - none

*/
static void DEV_GetDeviceTypeTest(void)
{
   ErrorCode_t ret = SUCCESS_E;
   uint32_t deviceType = DEV_MAX_DEV_TYPE_E;

   /* test device combination 1 = smoke only */
   ret = DM_DeviceGetDeviceType(1, 0, &deviceType);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(deviceType, DEV_SMOKE_DETECTOR);

   /* test device combination 34 = fire CP + first aid CP + sounder + visual indicator + PIR */
   ret = DM_DeviceGetDeviceType(34, 0, &deviceType);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(deviceType, DEV_PIR_DETECTOR);

   ret = DM_DeviceGetDeviceType(34, 1, &deviceType);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(deviceType, DEV_FIRE_CALL_POINT);

   ret = DM_DeviceGetDeviceType(34, 2, &deviceType);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(deviceType, DEV_FIRST_AID_CALL_POINT);

   ret = DM_DeviceGetDeviceType(34, 3, &deviceType);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(deviceType, DEV_SOUNDER);

   ret = DM_DeviceGetDeviceType(34, 4, &deviceType);
   CU_ASSERT_EQUAL(ret, SUCCESS_E);
   CU_ASSERT_EQUAL(deviceType, DEV_VISUAL_INDICATOR);
}


/*************************************************************************************/
/**
* DEV_InvalidParamsTest
* test invalid parameter handling
*
* no params
*
* @return - none

*/
static void DEV_InvalidParamsTest(void)
{
   ErrorCode_t ret = SUCCESS_E;
   bool isEnabled = false;
   uint8_t ruChannelIdx = 0;
   uint32_t deviceType = DEV_MAX_DEV_TYPE_E;

   /* test invalid params for IsEnabled */
   isEnabled = DM_DeviceIsEnabled(255, DEV_SMOKE_DETECTOR);
   CU_ASSERT_EQUAL(isEnabled, false);

   isEnabled = DM_DeviceIsEnabled(0, DEV_MAX_DEV_TYPE_E);
   CU_ASSERT_EQUAL(isEnabled, false);

   /* test invalid params for GetRUChannelIdx */
   ret = DM_DeviceGetRUChannelIdx(255, DEV_SMOKE_DETECTOR, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

   ret = DM_DeviceGetRUChannelIdx(1, DEV_SMOKE_DETECTOR, NULL);
   CU_ASSERT_EQUAL(ret, ERR_INVALID_POINTER_E);

   ret = DM_DeviceGetRUChannelIdx(0, DEV_SMOKE_DETECTOR, &ruChannelIdx);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

   /* test invalid params for GetDeviceType */
   ret = DM_DeviceGetDeviceType(255, 0, &deviceType);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

   ret = DM_DeviceGetDeviceType(1, 255, &deviceType);
   CU_ASSERT_EQUAL(ret, ERR_OUT_OF_RANGE_E);

   ret = DM_DeviceGetDeviceType(1, 0, NULL);
   CU_ASSERT_EQUAL(ret, ERR_INVALID_POINTER_E);
}


/*************************************************************************************/
/**
* DEV_PinConfigurationTest
* test pin configuration handling
*
* no params
*
* @return - none

*/
static void DEV_PinConfigurationTest(void)
{
   pc4Function_t pc4function;
   pc5Function_t pc5function;
   pb1Function_t pb1function;
   pb13Function_t pb13function;
   pb14Function_t pb14function;
   pb6Function_t pb6function;
   pb9Function_t pb9function;

   /* test configuration of PC4 */
   pc4function = DM_DeviceGetPC4Function(DC_SMOKE_DETECTOR_E, false);
   CU_ASSERT_EQUAL(pc4function, PC4_NOT_USED_E);

   pc4function = DM_DeviceGetPC4Function(DC_IO_UNIT_E, false);
   CU_ASSERT_EQUAL(pc4function, PC4_IO_IN1_E);

   pc4function = DM_DeviceGetPC4Function(DC_NO_DEVICES_FITTED_E, true);
   CU_ASSERT_EQUAL(pc4function, PC4_NCU_SBC_TX_E);

   pc4function = DM_DeviceGetPC4Function(DC_SOUNDER_VI_SMOKE_PIR_E, false);
   CU_ASSERT_EQUAL(pc4function, PC4_PIR_IN_E);

   /* test configuration of PC5 */
   pc5function = DM_DeviceGetPC5Function(DC_SMOKE_DETECTOR_E, false);
   CU_ASSERT_EQUAL(pc5function, PC5_NOT_USED_E);

   pc5function = DM_DeviceGetPC5Function(DC_IO_UNIT_E, false);
   CU_ASSERT_EQUAL(pc5function, PC5_IO_SHORT2_E);

   pc5function = DM_DeviceGetPC5Function(DC_NO_DEVICES_FITTED_E, true);
   CU_ASSERT_EQUAL(pc5function, PC5_NCU_SBC_RX_E);

   /* test configuration of PB1 */
   pb1function = DM_DeviceGetPB1Function(DC_FIRE_CALL_POINT_E);
   CU_ASSERT_EQUAL(pb1function, PB1_NOT_USED_E);

   pb1function = DM_DeviceGetPB1Function(DC_SMOKE_DETECTOR_E);
   CU_ASSERT_EQUAL(pb1function, PB1_HEAD_POWER_OUT_E);

   pb1function = DM_DeviceGetPB1Function(DC_REMOTE_INDICATOR_E);
   CU_ASSERT_EQUAL(pb1function, PB1_PWM3_E);

   /* test configuration of PB13 */
   pb13function = DM_DeviceGetPB13Function(DC_SMOKE_DETECTOR_E, false);
   CU_ASSERT_EQUAL(pb13function, PB13_NOT_USED_E);

   pb13function = DM_DeviceGetPB13Function(DC_IO_UNIT_E, false);
   CU_ASSERT_EQUAL(pb13function, PB13_IO_IN2_E);

   pb13function = DM_DeviceGetPB13Function(DC_NO_DEVICES_FITTED_E, true);
   CU_ASSERT_EQUAL(pb13function, PB13_NCU_SBC_CTS_E);

   pb13function = DM_DeviceGetPB13Function(DC_SOUNDER_SMOKE_PIR_E, false);
   CU_ASSERT_EQUAL(pb13function, PB13_PIR_POWER_E);

   /* test configuration of PB14 */
   pb14function = DM_DeviceGetPB14Function(DC_SMOKE_DETECTOR_E, false);
   CU_ASSERT_EQUAL(pb14function, PB14_NOT_USED_E);

   pb14function = DM_DeviceGetPB14Function(DC_IO_UNIT_E, false);
   CU_ASSERT_EQUAL(pb14function, PB14_IO_SHORT1_E);

   pb14function = DM_DeviceGetPB14Function(DC_NO_DEVICES_FITTED_E, true);
   CU_ASSERT_EQUAL(pb14function, PB14_NCU_SBC_RTS_E);

   pb14function = DM_DeviceGetPB14Function(DC_SOUNDER_VI_E, false);
   CU_ASSERT_EQUAL(pb14function, PB14_CON_VAD_POWER_E);

   /* test configuration of SVI power */
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_IO_UNIT_E, DEV_SOUNDER | DEV_VISUAL_INDICATOR), false);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_SOUNDER_E, DEV_SOUNDER | DEV_VISUAL_INDICATOR), true);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_SOUNDER_VI_E, DEV_SOUNDER | DEV_VISUAL_INDICATOR), true);

   /* test configuration of First Aid call point */
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_SOUNDER_E, DEV_FIRST_AID_CALL_POINT), false);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_CONSTRUCTION_SITE_FIRST_AID_COMBINATION_E, DEV_FIRST_AID_CALL_POINT), true);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_CONSTRUCTION_SITE_FIRE_AND_FIRST_AID_COMBINATION_1_E, DEV_FIRST_AID_CALL_POINT), true);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_CONSTRUCTION_SITE_FIRE_AND_FIRST_AID_COMBINATION_2_E, DEV_FIRST_AID_CALL_POINT), true);

   /* test configuration of First Aid call point */
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_SOUNDER_E, DEV_FIRE_CALL_POINT), false);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_FIRE_CALL_POINT_E, DEV_FIRE_CALL_POINT), true);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_CONSTRUCTION_SITE_FIRE_COMBINATION_1_E, DEV_FIRE_CALL_POINT), true);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_CONSTRUCTION_SITE_FIRE_COMBINATION_2_E, DEV_FIRE_CALL_POINT), true);
   CU_ASSERT_EQUAL(DM_DeviceIsEnabled(DC_CONSTRUCTION_SITE_FIRE_COMBINATION_3_E, DEV_FIRE_CALL_POINT), true);

   /* test configuration of PB6 */
   pb6function = DM_DeviceGetPB6Function(DC_SMOKE_DETECTOR_E);
   CU_ASSERT_EQUAL(pb6function, PB6_NOT_USED_E);

   pb6function = DM_DeviceGetPB6Function(DC_SOUNDER_VI_E);
   CU_ASSERT_EQUAL(pb6function, PB6_I2C1_SCL_E);

   pb6function = DM_DeviceGetPB6Function(DC_IO_UNIT_E);
   CU_ASSERT_EQUAL(pb6function, PB6_IO_OUT1_E);

   /* test configuration of PB9 */
   pb9function = DM_DeviceGetPB9Function(DC_SMOKE_DETECTOR_E);
   CU_ASSERT_EQUAL(pb9function, PB9_NOT_USED_E);

   pb9function = DM_DeviceGetPB9Function(DC_SOUNDER_VI_E);
   CU_ASSERT_EQUAL(pb9function, PB9_I2C1_SDA_E);

   pb9function = DM_DeviceGetPB9Function(DC_IO_UNIT_E);
   CU_ASSERT_EQUAL(pb9function, PB9_IO_OUT2_E);
}
