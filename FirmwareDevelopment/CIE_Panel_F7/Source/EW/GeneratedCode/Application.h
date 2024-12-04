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

#ifndef Application_H
#define Application_H

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

#include "_ApplicationAccess.h"
#include "_ApplicationAccesssPopup.h"
#include "_ApplicationAlarmRowItem.h"
#include "_ApplicationAlarms.h"
#include "_ApplicationApplication.h"
#include "_ApplicationButton.h"
#include "_ApplicationCAEAction.h"
#include "_ApplicationCAECompare.h"
#include "_ApplicationCAEComponent.h"
#include "_ApplicationCAEDayNight.h"
#include "_ApplicationCAEDevice.h"
#include "_ApplicationCAEGate.h"
#include "_ApplicationCAEGateAND.h"
#include "_ApplicationCAEGateOR.h"
#include "_ApplicationCAEModule.h"
#include "_ApplicationCAEName.h"
#include "_ApplicationCAEZone.h"
#include "_ApplicationCAEZoneFireAlarm.h"
#include "_ApplicationCAEZoneSmokeDetectors.h"
#include "_ApplicationCauseAndEffects.h"
#include "_ApplicationConfirmPopup.h"
#include "_ApplicationDFUPopup.h"
#include "_ApplicationDatePicker.h"
#include "_ApplicationDevice.h"
#include "_ApplicationDeviceClass.h"
#include "_ApplicationDeviceConfig.h"
#include "_ApplicationDeviceInfo.h"
#include "_ApplicationDeviceItem.h"
#include "_ApplicationDisable.h"
#include "_ApplicationDisableChannelItem.h"
#include "_ApplicationDisableItem.h"
#include "_ApplicationDisablePopup.h"
#include "_ApplicationDisableUnitItem.h"
#include "_ApplicationDisableZoneItem.h"
#include "_ApplicationDisablementItem.h"
#include "_ApplicationDisablements.h"
#include "_ApplicationDisplayPopup.h"
#include "_ApplicationDropDownMenu.h"
#include "_ApplicationDropMenuItem.h"
#include "_ApplicationDynamicAreaHead.h"
#include "_ApplicationEnableDisable.h"
#include "_ApplicationEventItem.h"
#include "_ApplicationEvents.h"
#include "_ApplicationExportConfigPopup.h"
#include "_ApplicationFaultItem.h"
#include "_ApplicationFaults.h"
#include "_ApplicationFlatCheckBox.h"
#include "_ApplicationImportConfigPopup.h"
#include "_ApplicationInputAction.h"
#include "_ApplicationInputChannel.h"
#include "_ApplicationInputChannels.h"
#include "_ApplicationInputDelay.h"
#include "_ApplicationInputItem.h"
#include "_ApplicationInputSignal.h"
#include "_ApplicationKey.h"
#include "_ApplicationKeyBmp.h"
#include "_ApplicationKeyLetter.h"
#include "_ApplicationKeySwitch.h"
#include "_ApplicationKeyboard.h"
#include "_ApplicationKeypadKey.h"
#include "_ApplicationKeypadPopup.h"
#include "_ApplicationLEDPopup.h"
#include "_ApplicationLevelAccess.h"
#include "_ApplicationLibClass.h"
#include "_ApplicationLogItem.h"
#include "_ApplicationLogPopup.h"
#include "_ApplicationMattsKeyboard.h"
#include "_ApplicationMenuItem.h"
#include "_ApplicationMeshNCU.h"
#include "_ApplicationMeshTree.h"
#include "_ApplicationMeshUnit.h"
#include "_ApplicationMeshUnitItem.h"
#include "_ApplicationModify.h"
#include "_ApplicationModifyPopup.h"
#include "_ApplicationNumboard.h"
#include "_ApplicationOnTest.h"
#include "_ApplicationOnTestItem.h"
#include "_ApplicationOutputAction.h"
#include "_ApplicationOutputChannel.h"
#include "_ApplicationOutputChannels.h"
#include "_ApplicationOutputDelays.h"
#include "_ApplicationOutputItem.h"
#include "_ApplicationOutputSignal.h"
#include "_ApplicationParamSlider.h"
#include "_ApplicationParamSpecific.h"
#include "_ApplicationParamToggle.h"
#include "_ApplicationParameter.h"
#include "_ApplicationPhoneNumber.h"
#include "_ApplicationPopup.h"
#include "_ApplicationProgress.h"
#include "_ApplicationProgressArrow.h"
#include "_ApplicationProgressPopup.h"
#include "_ApplicationRoundBox.h"
#include "_ApplicationSaveSitePopup.h"
#include "_ApplicationSetTimePopup.h"
#include "_ApplicationSettings.h"
#include "_ApplicationSmallDropDownMenu.h"
#include "_ApplicationSmallDropDownMenuList.h"
#include "_ApplicationSmallDropMenuItem.h"
#include "_ApplicationSystemPage.h"
#include "_ApplicationTabButton.h"
#include "_ApplicationTabButtonPull.h"
#include "_ApplicationTest.h"
#include "_ApplicationTestChannelItem.h"
#include "_ApplicationTestItem.h"
#include "_ApplicationTestPopup.h"
#include "_ApplicationTestUnitItem.h"
#include "_ApplicationTestZoneItem.h"
#include "_ApplicationTextEditor.h"
#include "_ApplicationTextPopup.h"
#include "_ApplicationTimePicker.h"
#include "_ApplicationTreeView.h"
#include "_ApplicationWritable.h"
#include "_ApplicationZoneToggle.h"

/* Bitmap resource : 'Application::Circle55Bitmap' */
EW_DECLARE_BITMAP_RES( ApplicationCircle55Bitmap )

/* Bitmap resource : 'Application::FireBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationFireBitmap )

/* Bitmap resource : 'Application::AlertBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationAlertBitmap )

/* Bitmap resource : 'Application::FireAlarmBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationFireAlarmBitmap )

/* Bitmap resource : 'Application::KeyBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationKeyBitmap )

/* Bitmap resource : 'Application::SmokeBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationSmokeBitmap )

/* Bitmap resource : 'Application::MuteBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationMuteBitmap )

/* Bitmap resource : 'Application::FaultBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationFaultBitmap )

/* Bitmap resource : 'Application::DisableBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationDisableBitmap )

/* Bitmap resource : 'Application::OnTestBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationOnTestBitmap )

/* Bitmap resource : 'Application::AccessBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationAccessBitmap )

/* Bitmap resource : 'Application::DropdownBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationDropdownBitmap )

/* Bitmap resource : 'Application::EvacuateBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationEvacuateBitmap )

/* Bitmap resource : 'Application::ResetBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationResetBitmap )

/* Font resource : 'Application::FontVerdana16' */
EW_DECLARE_FONT_RES( ApplicationFontVerdana16 )

/* Bitmap resource : 'Application::ExpandBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationExpandBitmap )

/* Bitmap resource : 'Application::CollapseBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationCollapseBitmap )

/* Font resource : 'Application::VerdanaProSemiBold16' */
EW_DECLARE_FONT_RES( ApplicationVerdanaProSemiBold16 )

/* Bitmap resource : 'Application::ViewLogBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationViewLogBitmap )

/* Bitmap resource : 'Application::HeatSensorBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationHeatSensorBitmap )

/* Bitmap resource : 'Application::GreenCallpointBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationGreenCallpointBitmap )

/* Bitmap resource : 'Application::PIRBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationPIRBitmap )

/* Font resource : 'Application::FontVerdanaProCondSemiBold18' */
EW_DECLARE_FONT_RES( ApplicationFontVerdanaProCondSemiBold18 )

/* Bitmap resource : 'Application::ActionBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationActionBitmap )

/* Bitmap resource : 'Application::OrBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationOrBitmap )

/* Bitmap resource : 'Application::AndBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationAndBitmap )

/* Font resource : 'Application::FontVerdana14' */
EW_DECLARE_FONT_RES( ApplicationFontVerdana14 )

/* Bitmap resource : 'Application::RadioBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationRadioBitmap )

/* Bitmap resource : 'Application::ZoneBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationZoneBitmap )

/* Bitmap resource : 'Application::PanelBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationPanelBitmap )

/* Bitmap resource : 'Application::PullSwitchBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationPullSwitchBitmap )

/* Bitmap resource : 'Application::Key2Bitmap' */
EW_DECLARE_BITMAP_RES( ApplicationKey2Bitmap )

/* Font resource : 'Application::FontVerdanaProSemiBold18' */
EW_DECLARE_FONT_RES( ApplicationFontVerdanaProSemiBold18 )

/* Font resource : 'Application::FontVerdanaProSemiBold20' */
EW_DECLARE_FONT_RES( ApplicationFontVerdanaProSemiBold20 )

/* Bitmap resource : 'Application::LogAccess' */
EW_DECLARE_BITMAP_RES( ApplicationLogAccess )

/* Bitmap resource : 'Application::LogMisc' */
EW_DECLARE_BITMAP_RES( ApplicationLogMisc )

/* Bitmap resource : 'Application::LogFaults' */
EW_DECLARE_BITMAP_RES( ApplicationLogFaults )

/* Bitmap resource : 'Application::LogEvents' */
EW_DECLARE_BITMAP_RES( ApplicationLogEvents )

/* Bitmap resource : 'Application::LogFire' */
EW_DECLARE_BITMAP_RES( ApplicationLogFire )

/* Bitmap resource : 'Application::LogSettings' */
EW_DECLARE_BITMAP_RES( ApplicationLogSettings )

/* Bitmap resource : 'Application::LogSystem' */
EW_DECLARE_BITMAP_RES( ApplicationLogSystem )

/* Bitmap resource : 'Application::LogTest' */
EW_DECLARE_BITMAP_RES( ApplicationLogTest )

/* Bitmap resource : 'Application::Fire4Bitmap' */
EW_DECLARE_BITMAP_RES( ApplicationFire4Bitmap )

/* Bitmap resource : 'Application::USBStick' */
EW_DECLARE_BITMAP_RES( ApplicationUSBStick )

/* Bitmap resource : 'Application::LaptopBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationLaptopBitmap )

/* Bitmap resource : 'Application::DropDownSimpleBitmap' */
EW_DECLARE_BITMAP_RES( ApplicationDropDownSimpleBitmap )

/* Bitmap resource : 'Application::EvacEvent' */
EW_DECLARE_BITMAP_RES( ApplicationEvacEvent )

/* Bitmap resource : 'Application::CheckBoxLarge' */
EW_DECLARE_BITMAP_RES( ApplicationCheckBoxLarge )

/* Bitmap resource : 'Application::CheckBoxSmall' */
EW_DECLARE_BITMAP_RES( ApplicationCheckBoxSmall )

/* Bitmap resource : 'Application::SiteNet' */
EW_DECLARE_BITMAP_RES( ApplicationSiteNet )

/* Bitmap resource : 'Application::FireRouting' */
EW_DECLARE_BITMAP_RES( ApplicationFireRouting )

/* Bitmap resource : 'Application::GeneralOutput' */
EW_DECLARE_BITMAP_RES( ApplicationGeneralOutput )

/* Bitmap resource : 'Application::GeneralInput' */
EW_DECLARE_BITMAP_RES( ApplicationGeneralInput )

/* Bitmap resource : 'Application::Sounder' */
EW_DECLARE_BITMAP_RES( ApplicationSounder )

/* Bitmap resource : 'Application::SounderVI' */
EW_DECLARE_BITMAP_RES( ApplicationSounderVI )

/* Bitmap resource : 'Application::VI' */
EW_DECLARE_BITMAP_RES( ApplicationVI )

/* Bitmap resource : 'Application::Beacon' */
EW_DECLARE_BITMAP_RES( ApplicationBeacon )

/* Bitmap resource : 'Application::CO' */
EW_DECLARE_BITMAP_RES( ApplicationCO )

/* Bitmap resource : 'Application::evacuate' */
EW_DECLARE_BITMAP_RES( Applicationevacuate )

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

/* User defined auto object: 'Application::NormalPushButton' */
EW_DECLARE_AUTOOBJECT( ApplicationNormalPushButton, WidgetSetPushButtonConfig )

/* User defined auto object: 'Application::Devices' */
EW_DECLARE_AUTOOBJECT( ApplicationDevices, ApplicationDeviceClass )

/* This autoobject provides the default customization for the 'radio button' widget 
   (WidgetSet::RadioButton) in its medium size variant. */
EW_DECLARE_AUTOOBJECT( ApplicationNormalRadioButton, WidgetSetRadioButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( ApplicationNormalCheckBox, WidgetSetToggleButtonConfig )

/* This autoobject provides the default customization for the 'horizontal slider' 
   widget (WidgetSet::HorizontalSlider) in its medium size variant. */
EW_DECLARE_AUTOOBJECT( ApplicationNormalHorizontalSlider, WidgetSetHorizontalSliderConfig )

/* User defined auto object: 'Application::NotmalToggleButton' */
EW_DECLARE_AUTOOBJECT( ApplicationNotmalToggleButton, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( ApplicationLargeCheckBox, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( ApplicationSmallCheckBox, WidgetSetToggleButtonConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'check box'. */
EW_DECLARE_AUTOOBJECT( ApplicationSmallCheckBoxCaption, WidgetSetToggleButtonConfig )

/* This autoobject provides the default customization for the 'push button' widget 
   (WidgetSet::PushButton) in its small size variant. */
EW_DECLARE_AUTOOBJECT( ApplicationSmallPushButton, WidgetSetPushButtonConfig )

/* User defined set: 'Application::ActionBits' */
typedef enum
{
  ApplicationActionBitsFire             = 0x00000001,
  ApplicationActionBitsFirstAid         = 0x00000002,
  ApplicationActionBitsEvacuation       = 0x00000004,
  ApplicationActionBitsSecurity         = 0x00000008,
  ApplicationActionBitsGeneral          = 0x00000010,
  ApplicationActionBitsFault            = 0x00000020,
  ApplicationActionBitsTest             = 0x00000040,
  ApplicationActionBitsSilentTest       = 0x00000080,
  ApplicationActionBitsRoutingAck       = 0x00000100,
  ApplicationActionBitsReset            = 0x00000200,
  ApplicationActionBitsSilence          = 0x00000400,
  ApplicationActionBitsUndefined        = 0x00000800
} ApplicationActionBits;

/* User defined auto object: 'Application::Lib' */
EW_DECLARE_AUTOOBJECT( ApplicationLib, ApplicationLibClass )

/* User defined set: 'Application::Options' */
typedef enum
{
  ApplicationOptionsLatching            = 0x00000001,
  ApplicationOptionsSustained           = 0x00000002,
  ApplicationOptionsInverted            = 0x00000004,
  ApplicationOptionsDoubleKnock         = 0x00000008,
  ApplicationOptionsDisabled            = 0x00000010,
  ApplicationOptionsIgnoreDelay         = 0x00000020,
  ApplicationOptionsSilenceable         = 0x00000040
} ApplicationOptions;

/* User defined enumeration: 'Application::Action' */
typedef enum
{
  ApplicationActionFire                 = 0,
  ApplicationActionFirstAid             = 1,
  ApplicationActionEvacuation           = 2,
  ApplicationActionSecurity             = 3,
  ApplicationActionGeneral              = 4,
  ApplicationActionFault                = 5,
  ApplicationActionTest                 = 6,
  ApplicationActionSilentTest           = 7,
  ApplicationActionRoutingAck           = 8,
  ApplicationActionReset                = 9,
  ApplicationActionSilence              = 10,
  ApplicationActionUndefined            = 11
} ApplicationAction;

/* User defined enumeration: 'Application::ChannelType' */
typedef enum
{
  ApplicationChannelTypeNone            = 0,
  ApplicationChannelTypeSmoke           = 1,
  ApplicationChannelTypeHeat            = 2,
  ApplicationChannelTypeCo              = 3,
  ApplicationChannelTypePir             = 4,
  ApplicationChannelTypeSounder         = 5,
  ApplicationChannelTypeBeacon          = 6,
  ApplicationChannelTypeFire_callpoint  = 7,
  ApplicationChannelTypeStatus_indicator_led = 8,
  ApplicationChannelTypeVisual_indicator = 9,
  ApplicationChannelTypeSounder_visual_indicator_combined = 10,
  ApplicationChannelTypeMedical_callpoint = 11,
  ApplicationChannelTypeEvac_callpoint  = 12,
  ApplicationChannelTypeOutput_routing  = 13,
  ApplicationChannelTypeInput1          = 14,
  ApplicationChannelTypeInput32         = 45,
  ApplicationChannelTypeOutput1         = 46,
  ApplicationChannelTypeOutput32        = 77
} ApplicationChannelType;

/* User defined enumeration: 'Application::ParamCode' */
typedef enum
{
  ApplicationParamCodeUnused            = 0,
  ApplicationParamCodeVolume            = 1,
  ApplicationParamCodeFlashRate         = 2
} ApplicationParamCode;

/* User defined set: 'Application::ChannelFlags' */
typedef enum
{
  ApplicationChannelFlagsAsserted       = 0x00000001,
  ApplicationChannelFlagsSilenced       = 0x00000002,
  ApplicationChannelFlagsLatched        = 0x00000004,
  ApplicationChannelFlagsInputDisabled  = 0x00000008,
  ApplicationChannelFlagsActive         = 0x00000010,
  ApplicationChannelFlagsOnTest         = 0x00000020,
  ApplicationChannelFlagsInFault        = 0x00000040,
  ApplicationChannelFlagsOutputDisabled = 0x00000080
} ApplicationChannelFlags;

/* User defined auto object: 'Application::DummyConfig' */
EW_DECLARE_AUTOOBJECT( ApplicationDummyConfig, ApplicationDeviceConfig )

/* User defined set: 'Application::ZoneFlags' */
typedef enum
{
  ApplicationZoneFlagsOnTest            = 0x00000008,
  ApplicationZoneFlagsSecondItem        = 0x00000010,
  ApplicationZoneFlagsThirdItem         = 0x00000020
} ApplicationZoneFlags;

/* User defined set: 'Application::DeviceSetting' */
typedef enum
{
  ApplicationDeviceSettingDisabled      = 0x00000001,
  ApplicationDeviceSettingSecondItem    = 0x00000002,
  ApplicationDeviceSettingOnTest        = 0x00000004
} ApplicationDeviceSetting;

#ifdef __cplusplus
  }
#endif

#endif /* Application_H */

/* Embedded Wizard */
