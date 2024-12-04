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

#ifndef _AppDeviceConfig_H
#define _AppDeviceConfig_H

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


/* Deklaration of class : 'App::DeviceConfig' */
EW_DEFINE_FIELDS( AppDeviceConfig, XObject )
  EW_ARRAY   ( inputs,          AppInputChannel, [32])
  EW_ARRAY   ( outputs,         AppOutputChannel, [32])
  EW_ARRAY   ( params,          AppParameter, [4])
  EW_PROPERTY( location,        XString )
  EW_PROPERTY( combiname,       XString )
  EW_PROPERTY( flags,           XSet )
  EW_PROPERTY( unitnum,         XInt16 )
  EW_PROPERTY( zonenum,         XInt16 )
  EW_PROPERTY( numoutputs,      XUInt8 )
  EW_PROPERTY( numinputs,       XUInt8 )
  EW_PROPERTY( numparams,       XUInt8 )
  EW_PROPERTY( combination,     XUInt8 )
EW_END_OF_FIELDS( AppDeviceConfig )

/* Virtual Method Table (VMT) for the class : 'App::DeviceConfig' */
EW_DEFINE_METHODS( AppDeviceConfig, XObject )
EW_END_OF_METHODS( AppDeviceConfig )

/* 'C' function for method : 'App::DeviceConfig.IsAlarmDevice()' */
XBool AppDeviceConfig_IsAlarmDevice( AppDeviceConfig _this, XInt32 channum );

/* Default onget method for the property 'combination' */
XUInt8 AppDeviceConfig_OnGetcombination( AppDeviceConfig _this );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnGetcombination()' */
XUInt8 AppDeviceConfig__OnGetcombination( void* _this );

/* The following define announces the presence of the method App::DeviceConfig.OnGetcombination(). */
#define _AppDeviceConfig__OnGetcombination_

/* Default onset method for the property 'combination' */
void AppDeviceConfig_OnSetcombination( AppDeviceConfig _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnSetcombination()' */
void AppDeviceConfig__OnSetcombination( void* _this, XUInt8 value );

/* The following define announces the presence of the method App::DeviceConfig.OnSetcombination(). */
#define _AppDeviceConfig__OnSetcombination_

/* Default onget method for the property 'numparams' */
XUInt8 AppDeviceConfig_OnGetnumparams( AppDeviceConfig _this );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnGetnumparams()' */
XUInt8 AppDeviceConfig__OnGetnumparams( void* _this );

/* The following define announces the presence of the method App::DeviceConfig.OnGetnumparams(). */
#define _AppDeviceConfig__OnGetnumparams_

/* Default onset method for the property 'numparams' */
void AppDeviceConfig_OnSetnumparams( AppDeviceConfig _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnSetnumparams()' */
void AppDeviceConfig__OnSetnumparams( void* _this, XUInt8 value );

/* The following define announces the presence of the method App::DeviceConfig.OnSetnumparams(). */
#define _AppDeviceConfig__OnSetnumparams_

/* Default onget method for the property 'combiname' */
XString AppDeviceConfig_OnGetcombiname( AppDeviceConfig _this );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnGetcombiname()' */
XString AppDeviceConfig__OnGetcombiname( void* _this );

/* The following define announces the presence of the method App::DeviceConfig.OnGetcombiname(). */
#define _AppDeviceConfig__OnGetcombiname_

/* Default onset method for the property 'combiname' */
void AppDeviceConfig_OnSetcombiname( AppDeviceConfig _this, XString value );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnSetcombiname()' */
void AppDeviceConfig__OnSetcombiname( void* _this, XString value );

/* The following define announces the presence of the method App::DeviceConfig.OnSetcombiname(). */
#define _AppDeviceConfig__OnSetcombiname_

/* Default onget method for the property 'numinputs' */
XUInt8 AppDeviceConfig_OnGetnuminputs( AppDeviceConfig _this );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnGetnuminputs()' */
XUInt8 AppDeviceConfig__OnGetnuminputs( void* _this );

/* The following define announces the presence of the method App::DeviceConfig.OnGetnuminputs(). */
#define _AppDeviceConfig__OnGetnuminputs_

/* Default onset method for the property 'numinputs' */
void AppDeviceConfig_OnSetnuminputs( AppDeviceConfig _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnSetnuminputs()' */
void AppDeviceConfig__OnSetnuminputs( void* _this, XUInt8 value );

/* The following define announces the presence of the method App::DeviceConfig.OnSetnuminputs(). */
#define _AppDeviceConfig__OnSetnuminputs_

/* Default onget method for the property 'numoutputs' */
XUInt8 AppDeviceConfig_OnGetnumoutputs( AppDeviceConfig _this );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnGetnumoutputs()' */
XUInt8 AppDeviceConfig__OnGetnumoutputs( void* _this );

/* The following define announces the presence of the method App::DeviceConfig.OnGetnumoutputs(). */
#define _AppDeviceConfig__OnGetnumoutputs_

/* Default onset method for the property 'numoutputs' */
void AppDeviceConfig_OnSetnumoutputs( AppDeviceConfig _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnSetnumoutputs()' */
void AppDeviceConfig__OnSetnumoutputs( void* _this, XUInt8 value );

/* The following define announces the presence of the method App::DeviceConfig.OnSetnumoutputs(). */
#define _AppDeviceConfig__OnSetnumoutputs_

/* Default onget method for the property 'flags' */
XSet AppDeviceConfig_OnGetflags( AppDeviceConfig _this );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnGetflags()' */
XSet AppDeviceConfig__OnGetflags( void* _this );

/* The following define announces the presence of the method App::DeviceConfig.OnGetflags(). */
#define _AppDeviceConfig__OnGetflags_

/* Default onset method for the property 'flags' */
void AppDeviceConfig_OnSetflags( AppDeviceConfig _this, XSet value );

/* Wrapper function for the non virtual method : 'App::DeviceConfig.OnSetflags()' */
void AppDeviceConfig__OnSetflags( void* _this, XSet value );

/* The following define announces the presence of the method App::DeviceConfig.OnSetflags(). */
#define _AppDeviceConfig__OnSetflags_

#ifdef __cplusplus
  }
#endif

#endif /* _AppDeviceConfig_H */

/* Embedded Wizard */
