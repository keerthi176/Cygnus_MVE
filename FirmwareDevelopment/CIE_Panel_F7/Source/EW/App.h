/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This file was generated automatically by Embedded Wizard Studio.
*
* Please do not make any modifications of this file! The modifications are lost
* when the file is generated again by Embedded Wizard Studio!
*
* The template of this heading text can be found in the file 'head.ewt' in the
* directory 'Platforms' of your Embedded Wizard installation directory. If you
* wish to adapt this text, please copy the template file 'head.ewt' into your
* project directory and edit the copy only. Please avoid any modifications of
* the original template file!
*
* Version  : 9.00
* Profile  : STM32F746
* Platform : STM.STM32.RGB565
*
*******************************************************************************/

#ifndef App_H
#define App_H

#ifdef __cplusplus
  extern "C"
  {
#endif

#include "ewrte.h"
#if EW_RTE_VERSION != 0x00090000
  #error Wrong version of Embedded Wizard Runtime Environment.
#endif

#include "ewgfx.h"
#if EW_GFX_VERSION != 0x00090000
  #error Wrong version of Embedded Wizard Graphics Engine.
#endif

#include "_AppAccess.h"
#include "_AppAccesssPopup.h"
#include "_AppAddDevicePopup.h"
#include "_AppAdditionalPopup.h"
#include "_AppAdditionalUnit.h"
#include "_AppAlarmRowItem.h"
#include "_AppAlert.h"
#include "_AppAppSupportPopup.h"
#include "_AppApplication.h"
#include "_AppBoolean.h"
#include "_AppButton.h"
#include "_AppButtonState.h"
#include "_AppChannelImage.h"
#include "_AppConfirmPopup.h"
#include "_AppConfirmTimerPopup.h"
#include "_AppCountView.h"
#include "_AppDFUPopup.h"
#include "_AppDatePicker.h"
#include "_AppDayTimeRow.h"
#include "_AppDayTimesPopup.h"
#include "_AppDevice.h"
#include "_AppDeviceClass.h"
#include "_AppDeviceConfig.h"
#include "_AppDeviceInfo.h"
#include "_AppDisable.h"
#include "_AppDisableChannelItem.h"
#include "_AppDisableItem.h"
#include "_AppDisableUnitItem.h"
#include "_AppDisableZoneItem.h"
#include "_AppDisablementItem.h"
#include "_AppDisablements.h"
#include "_AppDisablementsTabButton.h"
#include "_AppDisplayPopup.h"
#include "_AppDropDownMenu.h"
#include "_AppDropMenuItem.h"
#include "_AppEvents.h"
#include "_AppFaultItem.h"
#include "_AppFaults.h"
#include "_AppFire.h"
#include "_AppFireChannel.h"
#include "_AppFireContainer.h"
#include "_AppFireZone.h"
#include "_AppInputAction.h"
#include "_AppInputChannel.h"
#include "_AppInputChannels.h"
#include "_AppInputDelay.h"
#include "_AppInputSignal.h"
#include "_AppKey.h"
#include "_AppKeyBmp.h"
#include "_AppKeyLetter.h"
#include "_AppKeySwitch.h"
#include "_AppKeyboard.h"
#include "_AppKeypadChangePopup.h"
#include "_AppKeypadKey.h"
#include "_AppKeypadPopup.h"
#include "_AppLEDPopup.h"
#include "_AppLevelAccess.h"
#include "_AppLibClass.h"
#include "_AppListViewer.h"
#include "_AppLogChoice.h"
#include "_AppLogItem.h"
#include "_AppLogPopup.h"
#include "_AppMattsKeyboard.h"
#include "_AppMenuItem.h"
#include "_AppMeshLogPopup2.h"
#include "_AppMeshStage.h"
#include "_AppMeshStatus.h"
#include "_AppMeshTree.h"
#include "_AppMeshUnit.h"
#include "_AppMeshUnitItem.h"
#include "_AppMeshVector.h"
#include "_AppModify.h"
#include "_AppModifyPopup.h"
#include "_AppNewNumberPopup.h"
#include "_AppNumboard.h"
#include "_AppOnTest.h"
#include "_AppOnTestItem.h"
#include "_AppOutputAction.h"
#include "_AppOutputChannel.h"
#include "_AppOutputChannels.h"
#include "_AppOutputDelays.h"
#include "_AppOutputSignal.h"
#include "_AppParamSlider.h"
#include "_AppParamSpecific.h"
#include "_AppParamToggle.h"
#include "_AppParameter.h"
#include "_AppPhoneNumber.h"
#include "_AppPopup.h"
#include "_AppProgress.h"
#include "_AppProgressPopup.h"
#include "_AppPtrClass.h"
#include "_AppRBUProgress.h"
#include "_AppRadioSetup.h"
#include "_AppRadioUnit.h"
#include "_AppRoundBox.h"
#include "_AppSetTimePopup.h"
#include "_AppSettings.h"
#include "_AppSmallChannelImage.h"
#include "_AppSmallDropDownMenu.h"
#include "_AppSmallDropDownMenuList.h"
#include "_AppSmallDropMenuItem.h"
#include "_AppSystemPage.h"
#include "_AppTabButton.h"
#include "_AppTabButtonPull.h"
#include "_AppTest.h"
#include "_AppTestChannelItem.h"
#include "_AppTestItem.h"
#include "_AppTestUnitItem.h"
#include "_AppTestZoneItem.h"
#include "_AppTextEditor.h"
#include "_AppTextEditor1.h"
#include "_AppTextPopup.h"
#include "_AppTimePicker.h"
#include "_AppTimePicker1.h"
#include "_AppToggleItem.h"
#include "_AppTreeView.h"
#include "_AppUnitNumber.h"
#include "_AppUpdateDevicePopup.h"
#include "_AppUserButton.h"
#include "_AppWritable.h"
#include "_AppWritable1.h"
#include "_AppZoneAlert.h"
#include "_AppZoneAndCombi.h"
#include "_AppZoneSpinner.h"
#include "_AppZoneToggle.h"

/* Bitmap resource : 'App::SmokeBitmap' */
EW_DECLARE_BITMAP_RES( AppSmokeBitmap )

/* Bitmap resource : 'App::AlertBitmap' */
EW_DECLARE_BITMAP_RES( AppAlertBitmap )

/* Bitmap resource : 'App::FireAlarmBitmap' */
EW_DECLARE_BITMAP_RES( AppFireAlarmBitmap )

/* Bitmap resource : 'App::KeyBitmap' */
EW_DECLARE_BITMAP_RES( AppKeyBitmap )

/* Bitmap resource : 'App::MuteBitmap' */
EW_DECLARE_BITMAP_RES( AppMuteBitmap )

/* Bitmap resource : 'App::FaultBitmap' */
EW_DECLARE_BITMAP_RES( AppFaultBitmap )

/* Bitmap resource : 'App::DisableBitmap' */
EW_DECLARE_BITMAP_RES( AppDisableBitmap )

/* Bitmap resource : 'App::AccessBitmap' */
EW_DECLARE_BITMAP_RES( AppAccessBitmap )

/* Bitmap resource : 'App::DropdownBitmap' */
EW_DECLARE_BITMAP_RES( AppDropdownBitmap )

/* Bitmap resource : 'App::EvacuateBitmap' */
EW_DECLARE_BITMAP_RES( AppEvacuateBitmap )

/* Bitmap resource : 'App::ResetBitmap' */
EW_DECLARE_BITMAP_RES( AppResetBitmap )

/* Font resource : 'App::FontVerdana16' */
EW_DECLARE_FONT_RES( AppFontVerdana16 )

/* Bitmap resource : 'App::ExpandBitmap' */
EW_DECLARE_BITMAP_RES( AppExpandBitmap )

/* Bitmap resource : 'App::CollapseBitmap' */
EW_DECLARE_BITMAP_RES( AppCollapseBitmap )

/* Font resource : 'App::VerdanaProSemiBold16' */
EW_DECLARE_FONT_RES( AppVerdanaProSemiBold16 )

/* Bitmap resource : 'App::SilenceBitmap' */
EW_DECLARE_BITMAP_RES( AppSilenceBitmap )

/* Bitmap resource : 'App::HeatBSensorBitmap' */
EW_DECLARE_BITMAP_RES( AppHeatBSensorBitmap )

/* Bitmap resource : 'App::GreenCallpointBitmap' */
EW_DECLARE_BITMAP_RES( AppGreenCallpointBitmap )

/* Bitmap resource : 'App::PIRBitmap' */
EW_DECLARE_BITMAP_RES( AppPIRBitmap )

/* Font resource : 'App::FontVerdanaProCondSemiBold18' */
EW_DECLARE_FONT_RES( AppFontVerdanaProCondSemiBold18 )

/* Font resource : 'App::FontVerdana14' */
EW_DECLARE_FONT_RES( AppFontVerdana14 )

/* Bitmap resource : 'App::Key2Bitmap' */
EW_DECLARE_BITMAP_RES( AppKey2Bitmap )

/* Font resource : 'App::FontVerdanaProSemiBold18' */
EW_DECLARE_FONT_RES( AppFontVerdanaProSemiBold18 )

/* Font resource : 'App::FontVerdanaProSemiBold20' */
EW_DECLARE_FONT_RES( AppFontVerdanaProSemiBold20 )

/* Bitmap resource : 'App::LogAccess' */
EW_DECLARE_BITMAP_RES( AppLogAccess )

/* Bitmap resource : 'App::LogMisc' */
EW_DECLARE_BITMAP_RES( AppLogMisc )

/* Bitmap resource : 'App::LogFaults' */
EW_DECLARE_BITMAP_RES( AppLogFaults )

/* Bitmap resource : 'App::LogEvents' */
EW_DECLARE_BITMAP_RES( AppLogEvents )

/* Bitmap resource : 'App::LogFire' */
EW_DECLARE_BITMAP_RES( AppLogFire )

/* Bitmap resource : 'App::LogSettings' */
EW_DECLARE_BITMAP_RES( AppLogSettings )

/* Bitmap resource : 'App::LogSystem' */
EW_DECLARE_BITMAP_RES( AppLogSystem )

/* Bitmap resource : 'App::LogTest' */
EW_DECLARE_BITMAP_RES( AppLogTest )

/* Bitmap resource : 'App::Fire4Bitmap' */
EW_DECLARE_BITMAP_RES( AppFire4Bitmap )

/* Bitmap resource : 'App::DropDownSimpleBitmap' */
EW_DECLARE_BITMAP_RES( AppDropDownSimpleBitmap )

/* Bitmap resource : 'App::CheckBoxLarge' */
EW_DECLARE_BITMAP_RES( AppCheckBoxLarge )

/* Bitmap resource : 'App::CheckBoxSmall' */
EW_DECLARE_BITMAP_RES( AppCheckBoxSmall )

/* Bitmap resource : 'App::SiteNetBitmap' */
EW_DECLARE_BITMAP_RES( AppSiteNetBitmap )

/* Bitmap resource : 'App::FireRouting' */
EW_DECLARE_BITMAP_RES( AppFireRouting )

/* Bitmap resource : 'App::GeneralOutputBitmap' */
EW_DECLARE_BITMAP_RES( AppGeneralOutputBitmap )

/* Bitmap resource : 'App::GeneralInputBitmap' */
EW_DECLARE_BITMAP_RES( AppGeneralInputBitmap )

/* Bitmap resource : 'App::SounderBitmap' */
EW_DECLARE_BITMAP_RES( AppSounderBitmap )

/* Bitmap resource : 'App::SounderVIBitmap' */
EW_DECLARE_BITMAP_RES( AppSounderVIBitmap )

/* Bitmap resource : 'App::VIBitmap' */
EW_DECLARE_BITMAP_RES( AppVIBitmap )

/* Bitmap resource : 'App::BeaconBitmap' */
EW_DECLARE_BITMAP_RES( AppBeaconBitmap )

/* Bitmap resource : 'App::COBitmap' */
EW_DECLARE_BITMAP_RES( AppCOBitmap )

/* Bitmap resource : 'App::evacuate' */
EW_DECLARE_BITMAP_RES( Appevacuate )

/* Bitmap resource : 'App::SilencedBitmap' */
EW_DECLARE_BITMAP_RES( AppSilencedBitmap )

/* Bitmap resource : 'App::HeatA1RBitmap' */
EW_DECLARE_BITMAP_RES( AppHeatA1RBitmap )

/* Bitmap resource : 'App::CauseEffect2' */
EW_DECLARE_BITMAP_RES( AppCauseEffect2 )

/* Bitmap resource : 'App::battery1' */
EW_DECLARE_BITMAP_RES( Appbattery1 )

/* Bitmap resource : 'App::battery2' */
EW_DECLARE_BITMAP_RES( Appbattery2 )

/* Bitmap resource : 'App::battery3' */
EW_DECLARE_BITMAP_RES( Appbattery3 )

/* Bitmap resource : 'App::battery0' */
EW_DECLARE_BITMAP_RES( Appbattery0 )

/* Bitmap resource : 'App::input_alarm' */
EW_DECLARE_BITMAP_RES( Appinput_alarm )

/* Bitmap resource : 'App::input_callpoint' */
EW_DECLARE_BITMAP_RES( Appinput_callpoint )

/* Bitmap resource : 'App::input_fault' */
EW_DECLARE_BITMAP_RES( Appinput_fault )

/* Bitmap resource : 'App::input_routing_ack' */
EW_DECLARE_BITMAP_RES( Appinput_routing_ack )

/* Bitmap resource : 'App::input_silence' */
EW_DECLARE_BITMAP_RES( Appinput_silence )

/* Bitmap resource : 'App::output_alarm' */
EW_DECLARE_BITMAP_RES( Appoutput_alarm )

/* Bitmap resource : 'App::output_fault' */
EW_DECLARE_BITMAP_RES( Appoutput_fault )

/* Bitmap resource : 'App::output_fire' */
EW_DECLARE_BITMAP_RES( Appoutput_fire )

/* Bitmap resource : 'App::output_routing' */
EW_DECLARE_BITMAP_RES( Appoutput_routing )

/* Bitmap resource : 'App::output_sounders' */
EW_DECLARE_BITMAP_RES( Appoutput_sounders )

/* Bitmap resource : 'App::lorry' */
EW_DECLARE_BITMAP_RES( Applorry )

/* Font resource : 'App::FontMediumBold' */
EW_DECLARE_FONT_RES( AppFontMediumBold )

/* Bitmap resource : 'App::hazard' */
EW_DECLARE_BITMAP_RES( Apphazard )

/* Bitmap resource : 'App::hazardstripe' */
EW_DECLARE_BITMAP_RES( Apphazardstripe )

/* Bitmap resource : 'App::usbsmall' */
EW_DECLARE_BITMAP_RES( Appusbsmall )

/* Bitmap resource : 'App::jackplugsm' */
EW_DECLARE_BITMAP_RES( Appjackplugsm )

/* Bitmap resource : 'App::DisableCheckboxBitmap' */
EW_DECLARE_BITMAP_RES( AppDisableCheckboxBitmap )

/* Bitmap resource : 'App::DisableCheckboxSmallBitmap' */
EW_DECLARE_BITMAP_RES( AppDisableCheckboxSmallBitmap )

/* Bitmap resource : 'App::log_broadcast' */
EW_DECLARE_BITMAP_RES( Applog_broadcast )

/* Bitmap resource : 'App::log_link' */
EW_DECLARE_BITMAP_RES( Applog_link )

/* Bitmap resource : 'App::log_status' */
EW_DECLARE_BITMAP_RES( Applog_status )

/* Bitmap resource : 'App::log_drop' */
EW_DECLARE_BITMAP_RES( Applog_drop )

/* Bitmap resource : 'App::cygnusblue2' */
EW_DECLARE_BITMAP_RES( Appcygnusblue2 )

/* Bitmap resource : 'App::function' */
EW_DECLARE_BITMAP_RES( Appfunction )

/* Bitmap resource : 'App::lillybutton' */
EW_DECLARE_BITMAP_RES( Applillybutton )

/* Bitmap resource : 'App::goodbyeelise' */
EW_DECLARE_BITMAP_RES( Appgoodbyeelise )

/* Font resource : 'App::TerminalFont' */
EW_DECLARE_FONT_RES( AppTerminalFont )

/* Bitmap resource : 'App::outputs_off' */
EW_DECLARE_BITMAP_RES( Appoutputs_off )

/* Bitmap resource : 'App::ontest' */
EW_DECLARE_BITMAP_RES( Appontest )

/* Bitmap resource : 'App::cosmall' */
EW_DECLARE_BITMAP_RES( Appcosmall )

/* Bitmap resource : 'App::evacuate40csmall' */
EW_DECLARE_BITMAP_RES( Appevacuate40csmall )

/* Bitmap resource : 'App::evacuatesmall' */
EW_DECLARE_BITMAP_RES( Appevacuatesmall )

/* Bitmap resource : 'App::fire_alarmsmall' */
EW_DECLARE_BITMAP_RES( Appfire_alarmsmall )

/* Bitmap resource : 'App::fire_routingsmall' */
EW_DECLARE_BITMAP_RES( Appfire_routingsmall )

/* Bitmap resource : 'App::functionsmall' */
EW_DECLARE_BITMAP_RES( Appfunctionsmall )

/* Bitmap resource : 'App::general_inputsmall' */
EW_DECLARE_BITMAP_RES( Appgeneral_inputsmall )

/* Bitmap resource : 'App::greencall2small' */
EW_DECLARE_BITMAP_RES( Appgreencall2small )

/* Bitmap resource : 'App::heatA1Rsmall' */
EW_DECLARE_BITMAP_RES( AppheatA1Rsmall )

/* Bitmap resource : 'App::heatsmall' */
EW_DECLARE_BITMAP_RES( Appheatsmall )

/* Bitmap resource : 'App::input_alarmsmall' */
EW_DECLARE_BITMAP_RES( Appinput_alarmsmall )

/* Bitmap resource : 'App::input_callpointsmall' */
EW_DECLARE_BITMAP_RES( Appinput_callpointsmall )

/* Bitmap resource : 'App::input_faultsmall' */
EW_DECLARE_BITMAP_RES( Appinput_faultsmall )

/* Bitmap resource : 'App::input_routing_acksmall' */
EW_DECLARE_BITMAP_RES( Appinput_routing_acksmall )

/* Bitmap resource : 'App::input_silencesmall' */
EW_DECLARE_BITMAP_RES( Appinput_silencesmall )

/* Bitmap resource : 'App::pirsmall' */
EW_DECLARE_BITMAP_RES( Apppirsmall )

/* Bitmap resource : 'App::silenced_small' */
EW_DECLARE_BITMAP_RES( Appsilenced_small )

/* Bitmap resource : 'App::smoke2small' */
EW_DECLARE_BITMAP_RES( Appsmoke2small )

/* Bitmap resource : 'App::meshunit_fill' */
EW_DECLARE_BITMAP_RES( Appmeshunit_fill )

/* Bitmap resource : 'App::meshunit_outline' */
EW_DECLARE_BITMAP_RES( Appmeshunit_outline )

/* Bitmap resource : 'App::meshunit_outline_thin' */
EW_DECLARE_BITMAP_RES( Appmeshunit_outline_thin )

/* Bitmap resource : 'App::Bmp_1bar' */
EW_DECLARE_BITMAP_RES( AppBmp_1bar )

/* Bitmap resource : 'App::Bmp_2bar' */
EW_DECLARE_BITMAP_RES( AppBmp_2bar )

/* Bitmap resource : 'App::Bmp_3bar' */
EW_DECLARE_BITMAP_RES( AppBmp_3bar )

/* Bitmap resource : 'App::Bmp_4bar' */
EW_DECLARE_BITMAP_RES( AppBmp_4bar )

/* Bitmap resource : 'App::Bmp_0bar' */
EW_DECLARE_BITMAP_RES( AppBmp_0bar )

/* Bitmap resource : 'App::WaterLeakBitmap' */
EW_DECLARE_BITMAP_RES( AppWaterLeakBitmap )

/* Forward declaration of the class Graphics::AttrSet */
#ifndef _GraphicsAttrSet_
  EW_DECLARE_CLASS( GraphicsAttrSet )
#define _GraphicsAttrSet_
#endif

/* Forward declaration of the class Resources::ExternBitmap */
#ifndef _ResourcesExternBitmap_
  EW_DECLARE_CLASS( ResourcesExternBitmap )
#define _ResourcesExternBitmap_
#endif

/* Forward declaration of the class WidgetSet::HorizontalSliderConfig */
#ifndef _WidgetSetHorizontalSliderConfig_
  EW_DECLARE_CLASS( WidgetSetHorizontalSliderConfig )
#define _WidgetSetHorizontalSliderConfig_
#endif

/* Forward declaration of the class WidgetSet::PushButtonConfig */
#ifndef _WidgetSetPushButtonConfig_
  EW_DECLARE_CLASS( WidgetSetPushButtonConfig )
#define _WidgetSetPushButtonConfig_
#endif

/* Forward declaration of the class WidgetSet::RadioButtonConfig */
#ifndef _WidgetSetRadioButtonConfig_
  EW_DECLARE_CLASS( WidgetSetRadioButtonConfig )
#define _WidgetSetRadioButtonConfig_
#endif

/* Forward declaration of the class WidgetSet::ToggleButtonConfig */
#ifndef _WidgetSetToggleButtonConfig_
  EW_DECLARE_CLASS( WidgetSetToggleButtonConfig )
#define _WidgetSetToggleButtonConfig_
#endif

/* User defined auto object: 'App::NormalPushButton' */
EW_DECLARE_AUTOOBJECT( AppNormalPushButton, WidgetSetPushButtonConfig )

/* User defined auto object: 'App::Devices' */
EW_DECLARE_AUTOOBJECT( AppDevices, AppDeviceClass )

/* This autoobject provides the default customization for the 'radio button' widget 
   (WidgetSet::RadioButton) in its medium size variant. */
EW_DECLARE_AUTOOBJECT( AppNormalRadioButton, WidgetSetRadioButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( AppNormalCheckBox, WidgetSetToggleButtonConfig )

/* This autoobject provides the default customization for the 'horizontal slider' 
   widget (WidgetSet::HorizontalSlider) in its medium size variant. */
EW_DECLARE_AUTOOBJECT( AppNormalHorizontalSlider, WidgetSetHorizontalSliderConfig )

/* User defined auto object: 'App::NotmalToggleButton' */
EW_DECLARE_AUTOOBJECT( AppNotmalToggleButton, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'switch'. */
EW_DECLARE_AUTOOBJECT( AppSwitch_Medium, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( AppLargeCheckBox, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( AppSmallCheckBox, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( AppSmallCheckBoxCaption, WidgetSetToggleButtonConfig )

/* This autoobject provides the default customization for the 'push button' widget 
   (WidgetSet::PushButton) in its small size variant. */
EW_DECLARE_AUTOOBJECT( AppSmallPushButton, WidgetSetPushButtonConfig )

/* User defined set: 'App::ActionBits' */
typedef enum
{
  AppActionBitsFire                     = 0x00000001,
  AppActionBitsFirstAid                 = 0x00000002,
  AppActionBitsEvacuation               = 0x00000004,
  AppActionBitsSecurity                 = 0x00000008,
  AppActionBitsGeneral                  = 0x00000010,
  AppActionBitsFault                    = 0x00000020,
  AppActionBitsRoutingAck               = 0x00000040,
  AppActionBitsTest                     = 0x00000080,
  AppActionBitsSilentTest               = 0x00000100,
  AppActionBitsReset                    = 0x00000200,
  AppActionBitsSilence                  = 0x00000400,
  AppActionBitsUndefined                = 0x00000800
} AppActionBits;

/* User defined auto object: 'App::Lib' */
EW_DECLARE_AUTOOBJECT( AppLib, AppLibClass )

/* User defined set: 'App::Options' */
typedef enum
{
  AppOptionsLatching                    = 0x00000001,
  AppOptionsDisabled                    = 0x00000002,
  AppOptionsInverted                    = 0x00000004,
  AppOptionsDisabledDay                 = 0x00000008,
  AppOptionsDisabledNight               = 0x00000010,
  AppOptionsIgnoreDelay                 = 0x00000020,
  AppOptionsSilenceable                 = 0x00000040,
  AppOptionsCoincidence                 = 0x00000080,
  AppOptionsUseGlobalDelays             = 0x00000100,
  AppOptionsNoNightDelays               = 0x00000200
} AppOptions;

/* User defined enumeration: 'App::Action' */
typedef enum
{
  AppActionFire                         = 0,
  AppActionFirstAid                     = 1,
  AppActionEvacuation                   = 2,
  AppActionSecurity                     = 3,
  AppActionGeneral                      = 4,
  AppActionFault                        = 5,
  AppActionRoutingAck                   = 6,
  AppActionResound                      = 8,
  AppActionReset                        = 9,
  AppActionSilence                      = 10,
  AppActionUndefined                    = 11
} AppAction;

/* User defined enumeration: 'App::ChannelType' */
typedef enum
{
  AppChannelTypeNone                    = 0,
  AppChannelTypeSmoke                   = 1,
  AppChannelTypeHeatB                   = 2,
  AppChannelTypeCO                      = 3,
  AppChannelTypePIR                     = 4,
  AppChannelTypeSounder                 = 5,
  AppChannelTypeBeacon                  = 6,
  AppChannelTypeFireCallpoint           = 7,
  AppChannelTypeStatus_indicator_led    = 8,
  AppChannelTypeVisualIndicator         = 9,
  AppChannelTypeSounderVisualIndicatorCombined = 10,
  AppChannelTypeMedicalCallpoint        = 11,
  AppChannelTypeEvacCallpoint           = 12,
  AppChannelTypeOutputRouting           = 13,
  AppChannelTypeInput1                  = 14,
  AppChannelTypeInput32                 = 45,
  AppChannelTypeOutput1                 = 46,
  AppChannelTypeOutput32                = 61,
  AppChannelTypeHeatA1R                 = 62,
  AppChannelTypeWaterLeak               = 63,
  AppChannelTypePanelRoutingAck         = 64,
  AppChannelTypePanelCallpoint          = 65,
  AppChannelTypePanelAlarmInput         = 66,
  AppChannelTypePanelSilenceInput       = 67,
  AppChannelTypePanelFaultInput         = 68,
  AppChannelTypePanelAlarmRelay         = 69,
  AppChannelTypePanelFaultRelay         = 70,
  AppChannelTypePanelFireRelay          = 71,
  AppChannelTypePanelRoutingOutput      = 72,
  AppChannelTypePanelSoundersRelay      = 73,
  AppChannelTypePanelFunction1          = 74,
  AppChannelTypePanelFunction2          = 75,
  AppChannelTypePanelEvacuate           = 76
} AppChannelType;

/* User defined enumeration: 'App::ParamCode' */
typedef enum
{
  AppParamCodeUnused                    = 0,
  AppParamCodeVolume                    = 1,
  AppParamCodeToggle                    = 2
} AppParamCode;

/* User defined set: 'App::ChannelFlags' */
typedef enum
{
  AppChannelFlagsAsserted               = 0x00000001,
  AppChannelFlagsSilenced               = 0x00000002,
  AppChannelFlagsLatched                = 0x00000004,
  AppChannelFlagsInputRuledDisabled     = 0x00000008,
  AppChannelFlagsInputRuledOnTest       = 0x00000010,
  AppChannelFlagsInputOnTest            = 0x00000020,
  AppChannelFlagsInFault                = 0x00000040,
  AppChannelFlagsInputActiveOnTest      = 0x00000080,
  AppChannelFlagsInputActive            = 0x00000100,
  AppChannelFlagsInputNoticedOnTest     = 0x00000200,
  AppChannelFlagsInputNotFound          = 0x00000400,
  AppChannelFlagsOutputDelayed          = 0x00000800,
  AppChannelFlagsOutputSkipped          = 0x00001000,
  AppChannelFlagsOutputRuledDisabled    = 0x00002000,
  AppChannelFlagsOutputOnTest           = 0x00004000,
  AppChannelFlagsOutputAsserted         = 0x00008000
} AppChannelFlags;

/* User defined auto object: 'App::DummyConfig' */
EW_DECLARE_AUTOOBJECT( AppDummyConfig, AppDeviceConfig )

/* User defined set: 'App::ZoneFlags' */
typedef enum
{
  AppZoneFlagsInFire                    = 0x00000001,
  AppZoneFlagsSilenced                  = 0x00000002,
  AppZoneFlagsModifyExpanded            = 0x00000004,
  AppZoneFlagsOnTest                    = 0x00000008,
  AppZoneFlagsDisabled                  = 0x00000010,
  AppZoneFlagsTestExpanded              = 0x00000020,
  AppZoneFlagsDisableExpanded           = 0x00000040,
  AppZoneFlagsFireExpanded              = 0x00000080
} AppZoneFlags;

/* User defined set: 'App::DeviceSetting' */
typedef enum
{
  AppDeviceSettingDisableFireExpanded   = 0x00000001,
  AppDeviceSettingDisableExpanded       = 0x00000002,
  AppDeviceSettingOnTest                = 0x00000004,
  AppDeviceSettingTestExpanded          = 0x00000008,
  AppDeviceSettingDisableRoutingExpanded = 0x00000010,
  AppDeviceSettingRuledOnTest           = 0x00000020,
  AppDeviceSettingRuledDisabled         = 0x00000040
} AppDeviceSetting;

/* User defined enumeration: 'App::RadioStatus' */
typedef enum
{
  AppRadioStatusNotFound                = 0,
  AppRadioStatusFound                   = 1,
  AppRadioStatusAdditional              = 2,
  AppRadioStatusDifferent               = 3,
  AppRadioStatusNCU                     = 4
} AppRadioStatus;

/* User defined set: 'App::AlertFlags' */
typedef enum
{
  AppAlertFlagsSilenced                 = 0x00000001,
  AppAlertFlagsNetDevice                = 0x00000010,
  AppAlertFlagsDevice                   = 0x00000020,
  AppAlertFlagsNetCAE                   = 0x00000040,
  AppAlertFlagsCAE                      = 0x00000080
} AppAlertFlags;

/* User defined enumeration: 'App::NCUStage' */
typedef enum
{
  AppNCUStageUnknnown                   = 0,
  AppNCUStageMeshing                    = 1,
  AppNCUStageActivating                 = 2,
  AppNCUStageDropped                    = 10
} AppNCUStage;

/* User defined auto object: 'App::DummyParam' */
EW_DECLARE_AUTOOBJECT( AppDummyParam, AppParameter )

/* User defined auto object: 'App::DummyInputChan' */
EW_DECLARE_AUTOOBJECT( AppDummyInputChan, AppInputChannel )

/* User defined auto object: 'App::DummyOutputChan' */
EW_DECLARE_AUTOOBJECT( AppDummyOutputChan, AppOutputChannel )

/* User defined enumeration: 'App::Profile' */
typedef enum
{
  AppProfileFire                        = 0,
  AppProfileFirstAid                    = 1,
  AppProfileEvacuation                  = 2,
  AppProfileSecurity                    = 3,
  AppProfileGeneral                     = 4,
  AppProfileFault                       = 5,
  AppProfileRoutingAck                  = 6,
  AppProfileTestTone                    = 7,
  AppProfileSilent                      = 8
} AppProfile;

/* User defined auto object: 'App::DummyParam1' */
EW_DECLARE_AUTOOBJECT( AppDummyParam1, AppParameter )

/* User defined auto object: 'App::LogoBitmap' */
EW_DECLARE_AUTOOBJECT( AppLogoBitmap, ResourcesExternBitmap )

/* User defined set: 'App::SiteSetting' */
typedef enum
{
  AppSiteSettingResoundNewZone          = 0x00000001,
  AppSiteSettingDisableOutputDelays     = 0x00000002,
  AppSiteSettingDisableFireRouting      = 0x00000004,
  AppSiteSettingDisableFireAlarms       = 0x00000008,
  AppSiteSettingIgnoreSecurityAtNight   = 0x00000010,
  AppSiteSettingIgnoreSecurityInDay     = 0x00000020,
  AppSiteSettingOverrideOutputDelays    = 0x00000040
} AppSiteSetting;

/* User defined set: 'App::DeviceConfigFlags' */
typedef enum
{
  AppDeviceConfigFlagsDisabled          = 0x00000001
} AppDeviceConfigFlags;

/* User defined auto object: 'App::FocusedPushButton' */
EW_DECLARE_AUTOOBJECT( AppFocusedPushButton, WidgetSetPushButtonConfig )

/* User defined auto object: 'App::AttrSetFireZone' */
EW_DECLARE_AUTOOBJECT( AppAttrSetFireZone, GraphicsAttrSet )

/* User defined auto object: 'App::AttrSetFireChannel' */
EW_DECLARE_AUTOOBJECT( AppAttrSetFireChannel, GraphicsAttrSet )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( AppLargeDisableCheckBox, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( AppDisableSmallCheckBox, WidgetSetToggleButtonConfig )

/* User defined auto object: 'App::DummyOutputChan1' */
EW_DECLARE_AUTOOBJECT( AppDummyOutputChan1, AppOutputChannel )

#ifdef __cplusplus
  }
#endif

#endif /* App_H */

/* Embedded Wizard */
