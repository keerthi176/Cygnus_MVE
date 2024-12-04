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

#ifndef _ApplicationLibClass_H
#define _ApplicationLibClass_H

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

/* Forward declaration of the class Application::Device */
#ifndef _ApplicationDevice_
  EW_DECLARE_CLASS( ApplicationDevice )
#define _ApplicationDevice_
#endif

/* Forward declaration of the class Application::DeviceConfig */
#ifndef _ApplicationDeviceConfig_
  EW_DECLARE_CLASS( ApplicationDeviceConfig )
#define _ApplicationDeviceConfig_
#endif

/* Forward declaration of the class Application::InputChannel */
#ifndef _ApplicationInputChannel_
  EW_DECLARE_CLASS( ApplicationInputChannel )
#define _ApplicationInputChannel_
#endif

/* Forward declaration of the class Application::LibClass */
#ifndef _ApplicationLibClass_
  EW_DECLARE_CLASS( ApplicationLibClass )
#define _ApplicationLibClass_
#endif

/* Forward declaration of the class Application::OutputChannel */
#ifndef _ApplicationOutputChannel_
  EW_DECLARE_CLASS( ApplicationOutputChannel )
#define _ApplicationOutputChannel_
#endif

/* Forward declaration of the class Application::Parameter */
#ifndef _ApplicationParameter_
  EW_DECLARE_CLASS( ApplicationParameter )
#define _ApplicationParameter_
#endif

/* Forward declaration of the class Resources::Bitmap */
#ifndef _ResourcesBitmap_
  EW_DECLARE_CLASS( ResourcesBitmap )
#define _ResourcesBitmap_
#endif


/* Deklaration of class : 'Application::LibClass' */
EW_DEFINE_FIELDS( ApplicationLibClass, XObject )
EW_END_OF_FIELDS( ApplicationLibClass )

/* Virtual Method Table (VMT) for the class : 'Application::LibClass' */
EW_DEFINE_METHODS( ApplicationLibClass, XObject )
EW_END_OF_METHODS( ApplicationLibClass )

/* 'C' function for method : 'Application::LibClass.GetConfig()' */
ApplicationDeviceConfig ApplicationLibClass_GetConfig( ApplicationLibClass _this, 
  XInt16 unit_num );

/* 'C' function for method : 'Application::LibClass.GetInput()' */
ApplicationInputChannel ApplicationLibClass_GetInput( ApplicationLibClass _this, 
  XInt16 unitnum, XUInt8 channel );

/* 'C' function for method : 'Application::LibClass.NumInputChannels()' */
XInt32 ApplicationLibClass_NumInputChannels( ApplicationLibClass _this, XInt16 unitnum );

/* 'C' function for method : 'Application::LibClass.NumOutputChannels()' */
XInt32 ApplicationLibClass_NumOutputChannels( ApplicationLibClass _this, XInt16 
  unitnum );

/* 'C' function for method : 'Application::LibClass.NumZones()' */
XInt32 ApplicationLibClass_NumZones( ApplicationLibClass _this );

/* 'C' function for method : 'Application::LibClass.GetOutput()' */
ApplicationOutputChannel ApplicationLibClass_GetOutput( ApplicationLibClass _this, 
  XInt16 unitnum, XUInt8 channel );

/* 'C' function for method : 'Application::LibClass.TypeName()' */
XString ApplicationLibClass_TypeName( ApplicationLibClass _this, XEnum type );

/* 'C' function for method : 'Application::LibClass.SetConfig()' */
XBool ApplicationLibClass_SetConfig( ApplicationLibClass _this, XInt16 unit_num, 
  ApplicationDeviceConfig dev );

/* 'C' function for method : 'Application::LibClass.GetParameter()' */
ApplicationParameter ApplicationLibClass_GetParameter( ApplicationLibClass _this, 
  XInt16 unitnum, XUInt8 index );

/* 'C' function for method : 'Application::LibClass.NumDevsInZone()' */
XInt32 ApplicationLibClass_NumDevsInZone( ApplicationLibClass _this, XUInt8 zone );

/* 'C' function for method : 'Application::LibClass.GetDevInZone()' */
XInt16 ApplicationLibClass_GetDevInZone( ApplicationLibClass _this, XInt32 zone, 
  XInt32 devno );

/* Wrapper function for the non virtual method : 'Application::LibClass.GetDevInZone()' */
XInt16 ApplicationLibClass__GetDevInZone( void* _this, XInt32 zone, XInt32 devno );

/* The following define announces the presence of the method Application::LibClass.GetDevInZone(). */
#define _ApplicationLibClass__GetDevInZone_

/* 'C' function for method : 'Application::LibClass.GetDevice()' */
ApplicationDevice ApplicationLibClass_GetDevice( ApplicationLibClass _this, XInt16 
  unit_num );

/* 'C' function for method : 'Application::LibClass.NumZonesWithDevices()' */
XInt32 ApplicationLibClass_NumZonesWithDevices( ApplicationLibClass _this );

/* 'C' function for method : 'Application::LibClass.GetZoneFlags()' */
XSet ApplicationLibClass_GetZoneFlags( ApplicationLibClass _this, XInt16 zone );

/* 'C' function for method : 'Application::LibClass.SetZoneFlags()' */
void ApplicationLibClass_SetZoneFlags( ApplicationLibClass _this, XInt16 zone, XSet 
  flags );

/* 'C' function for method : 'Application::LibClass.SetDevice()' */
void ApplicationLibClass_SetDevice( ApplicationLibClass _this, XInt16 unit_num, 
  ApplicationDevice dev );

/* 'C' function for method : 'Application::LibClass.SwitchOuput()' */
void ApplicationLibClass_SwitchOuput( ApplicationLibClass _this, XInt16 UnitNum, 
  XUInt8 ChanNum, XEnum profile, XBool state );

/* 'C' function for method : 'Application::LibClass.ProfileName()' */
XString ApplicationLibClass_ProfileName( ApplicationLibClass _this, XEnum action );

/* 'C' function for method : 'Application::LibClass.TypeBitmap()' */
ResourcesBitmap ApplicationLibClass_TypeBitmap( ApplicationLibClass _this, XEnum 
  type );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationLibClass_H */

/* Embedded Wizard */
