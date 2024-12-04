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

#ifndef _AppLibClass_H
#define _AppLibClass_H

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

/* Forward declaration of the class App::Alert */
#ifndef _AppAlert_
  EW_DECLARE_CLASS( AppAlert )
#define _AppAlert_
#endif

/* Forward declaration of the class App::Device */
#ifndef _AppDevice_
  EW_DECLARE_CLASS( AppDevice )
#define _AppDevice_
#endif

/* Forward declaration of the class App::DeviceConfig */
#ifndef _AppDeviceConfig_
  EW_DECLARE_CLASS( AppDeviceConfig )
#define _AppDeviceConfig_
#endif

/* Forward declaration of the class App::InputChannel */
#ifndef _AppInputChannel_
  EW_DECLARE_CLASS( AppInputChannel )
#define _AppInputChannel_
#endif

/* Forward declaration of the class App::LibClass */
#ifndef _AppLibClass_
  EW_DECLARE_CLASS( AppLibClass )
#define _AppLibClass_
#endif

/* Forward declaration of the class App::OutputChannel */
#ifndef _AppOutputChannel_
  EW_DECLARE_CLASS( AppOutputChannel )
#define _AppOutputChannel_
#endif

/* Forward declaration of the class App::Parameter */
#ifndef _AppParameter_
  EW_DECLARE_CLASS( AppParameter )
#define _AppParameter_
#endif

/* Forward declaration of the class App::RadioUnit */
#ifndef _AppRadioUnit_
  EW_DECLARE_CLASS( AppRadioUnit )
#define _AppRadioUnit_
#endif

/* Forward declaration of the class App::ZoneAlert */
#ifndef _AppZoneAlert_
  EW_DECLARE_CLASS( AppZoneAlert )
#define _AppZoneAlert_
#endif

/* Forward declaration of the class Resources::Bitmap */
#ifndef _ResourcesBitmap_
  EW_DECLARE_CLASS( ResourcesBitmap )
#define _ResourcesBitmap_
#endif


/* Deklaration of class : 'App::LibClass' */
EW_DEFINE_FIELDS( AppLibClass, XObject )
EW_END_OF_FIELDS( AppLibClass )

/* Virtual Method Table (VMT) for the class : 'App::LibClass' */
EW_DEFINE_METHODS( AppLibClass, XObject )
EW_END_OF_METHODS( AppLibClass )

/* 'C' function for method : 'App::LibClass.GetConfig()' */
AppDeviceConfig AppLibClass_GetConfig( AppLibClass _this, XInt16 unit_num );

/* 'C' function for method : 'App::LibClass.NumZones()' */
XInt32 AppLibClass_NumZones( AppLibClass _this );

/* 'C' function for method : 'App::LibClass.TypeName()' */
XString AppLibClass_TypeName( AppLibClass _this, XEnum type );

/* 'C' function for method : 'App::LibClass.SetConfig()' */
XBool AppLibClass_SetConfig( AppLibClass _this, XInt16 unit_num, XBool transmit, 
  AppDeviceConfig dev );

/* 'C' function for method : 'App::LibClass.GetParameter()' */
AppParameter AppLibClass_GetParameter( AppLibClass _this, XInt16 unitnum, XUInt8 
  index );

/* 'C' function for method : 'App::LibClass.NumDevsInZone()' */
XInt32 AppLibClass_NumDevsInZone( AppLibClass _this, XInt32 zone );

/* 'C' function for method : 'App::LibClass.GetDevInZone()' */
XInt16 AppLibClass_GetDevInZone( AppLibClass _this, XInt32 zone, XInt32 devno );

/* Wrapper function for the non virtual method : 'App::LibClass.GetDevInZone()' */
XInt16 AppLibClass__GetDevInZone( void* _this, XInt32 zone, XInt32 devno );

/* The following define announces the presence of the method App::LibClass.GetDevInZone(). */
#define _AppLibClass__GetDevInZone_

/* 'C' function for method : 'App::LibClass.GetDevice()' */
AppDevice AppLibClass_GetDevice( AppLibClass _this, XInt16 unit_num );

/* 'C' function for method : 'App::LibClass.GetRadio()' */
AppRadioUnit AppLibClass_GetRadio( AppLibClass _this, XInt32 unit_num );

/* 'C' function for method : 'App::LibClass.NumZonesWithDevices()' */
XInt32 AppLibClass_NumZonesWithDevices( AppLibClass _this );

/* 'C' function for method : 'App::LibClass.GetZoneFlags()' */
XSet AppLibClass_GetZoneFlags( AppLibClass _this, XInt16 zone );

/* 'C' function for method : 'App::LibClass.SetZoneFlags()' */
void AppLibClass_SetZoneFlags( AppLibClass _this, XInt16 zone, XSet flags );

/* 'C' function for method : 'App::LibClass.SetDevice()' */
void AppLibClass_SetDevice( AppLibClass _this, XInt16 unit_num, AppDevice dev );

/* 'C' function for method : 'App::LibClass.SwitchOutput()' */
void AppLibClass_SwitchOutput( AppLibClass _this, XInt16 UnitNum, XInt8 ChanNum, 
  XEnum profile, XBool state );

/* 'C' function for method : 'App::LibClass.ProfileName()' */
XString AppLibClass_ProfileName( AppLibClass _this, XEnum profile );

/* 'C' function for method : 'App::LibClass.GetConfigNum()' */
AppDeviceConfig AppLibClass_GetConfigNum( AppLibClass _this, XInt16 dev_num );

/* 'C' function for method : 'App::LibClass.GetNumSiteDevices()' */
XInt32 AppLibClass_GetNumSiteDevices( AppLibClass _this );

/* 'C' function for method : 'App::LibClass.GetNumRadiosFound()' */
XInt32 AppLibClass_GetNumRadiosFound( AppLibClass _this );

/* 'C' function for method : 'App::LibClass.GetRadioNum()' */
AppRadioUnit AppLibClass_GetRadioNum( AppLibClass _this, XInt16 num );

/* 'C' function for method : 'App::LibClass.GetAlertNum()' */
AppAlert AppLibClass_GetAlertNum( AppLibClass _this, XSet event, XInt32 num );

/* 'C' function for method : 'App::LibClass.GetRadioFromPtr()' */
AppRadioUnit AppLibClass_GetRadioFromPtr( AppLibClass _this, XInt32 unit_ptr );

/* 'C' function for method : 'App::LibClass.GetConfigFromPtr()' */
AppDeviceConfig AppLibClass_GetConfigFromPtr( AppLibClass _this, XInt32 ptr );

/* 'C' function for method : 'App::LibClass.GetConfigNumInZone()' */
AppDeviceConfig AppLibClass_GetConfigNumInZone( AppLibClass _this, XInt16 zone, 
  XInt16 unitnum );

/* 'C' function for method : 'App::LibClass.GetInputFromPtr()' */
AppInputChannel AppLibClass_GetInputFromPtr( AppLibClass _this, XInt32 devptr, XUInt8 
  channel );

/* 'C' function for method : 'App::LibClass.GetOutputFromPtr()' */
AppOutputChannel AppLibClass_GetOutputFromPtr( AppLibClass _this, XInt32 devptr, 
  XUInt8 channel );

/* 'C' function for method : 'App::LibClass.GetAlertNumInZone()' */
AppAlert AppLibClass_GetAlertNumInZone( AppLibClass _this, XSet event, XInt32 num, 
  XInt32 zone, XInt32 panel );

/* 'C' function for method : 'App::LibClass.GetZoneAlert()' */
AppZoneAlert AppLibClass_GetZoneAlert( AppLibClass _this, XInt32 num );

/* 'C' function for method : 'App::LibClass.GetNumAlertsInZone()' */
XInt32 AppLibClass_GetNumAlertsInZone( AppLibClass _this, XInt32 zone, XInt32 panel );

/* 'C' function for method : 'App::LibClass.GetPanelName()' */
XString AppLibClass_GetPanelName( AppLibClass _this, XInt32 index );

/* 'C' function for method : 'App::LibClass.GetPanelZoneFlags()' */
XSet AppLibClass_GetPanelZoneFlags( AppLibClass _this, XInt16 zone, XInt16 panel );

/* 'C' function for method : 'App::LibClass.SetPanelZoneFlags()' */
void AppLibClass_SetPanelZoneFlags( AppLibClass _this, XInt16 zone, XInt16 panel, 
  XSet flags );

/* 'C' function for method : 'App::LibClass.GetZoneDisabledState()' */
XBool AppLibClass_GetZoneDisabledState( AppLibClass _this, XInt32 zone );

/* 'C' function for method : 'App::LibClass.SetZoneDisabledState()' */
void AppLibClass_SetZoneDisabledState( AppLibClass _this, XInt32 zone, XBool state );

/* 'C' function for method : 'App::LibClass.OnTree()' */
XBool AppLibClass_OnTree( AppLibClass _this, XInt32 unitnum );

/* 'C' function for method : 'App::LibClass.Order()' */
XString AppLibClass_Order( AppLibClass _this, XInt32 order );

/* 'C' function for method : 'App::LibClass.GetCombiName()' */
XString AppLibClass_GetCombiName( AppLibClass _this, XInt32 type );

/* 'C' function for method : 'App::LibClass.TypeBitmap()' */
ResourcesBitmap AppLibClass_TypeBitmap( AppLibClass _this, XEnum Bitmap );

/* 'C' function for method : 'App::LibClass.Unit()' */
XString AppLibClass_Unit( AppLibClass _this, XInt32 u );

/* 'C' function for method : 'App::LibClass.TypeSmallBitmap()' */
ResourcesBitmap AppLibClass_TypeSmallBitmap( AppLibClass _this, XEnum Bitmap );

#ifdef __cplusplus
  }
#endif

#endif /* _AppLibClass_H */

/* Embedded Wizard */
