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

#ifndef _ApplicationDeviceConfig_H
#define _ApplicationDeviceConfig_H

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


/* Deklaration of class : 'Application::DeviceConfig' */
EW_DEFINE_FIELDS( ApplicationDeviceConfig, XObject )
  EW_ARRAY   ( inputs,          ApplicationInputChannel, [32])
  EW_ARRAY   ( outputs,         ApplicationOutputChannel, [32])
  EW_ARRAY   ( params,          ApplicationParameter, [4])
  EW_PROPERTY( location,        XString )
  EW_PROPERTY( unitnum,         XInt16 )
  EW_PROPERTY( zonenum,         XInt16 )
  EW_PROPERTY( numparams,       XUInt8 )
  EW_PROPERTY( combination,     XUInt8 )
EW_END_OF_FIELDS( ApplicationDeviceConfig )

/* Virtual Method Table (VMT) for the class : 'Application::DeviceConfig' */
EW_DEFINE_METHODS( ApplicationDeviceConfig, XObject )
EW_END_OF_METHODS( ApplicationDeviceConfig )

/* 'C' function for method : 'Application::DeviceConfig.NumInputs()' */
XInt32 ApplicationDeviceConfig_NumInputs( ApplicationDeviceConfig _this );

/* 'C' function for method : 'Application::DeviceConfig.NumOutputs()' */
XInt32 ApplicationDeviceConfig_NumOutputs( ApplicationDeviceConfig _this );

/* Default onget method for the property 'combination' */
XUInt8 ApplicationDeviceConfig_OnGetcombination( ApplicationDeviceConfig _this );

/* Wrapper function for the non virtual method : 'Application::DeviceConfig.OnGetcombination()' */
XUInt8 ApplicationDeviceConfig__OnGetcombination( void* _this );

/* The following define announces the presence of the method Application::DeviceConfig.OnGetcombination(). */
#define _ApplicationDeviceConfig__OnGetcombination_

/* Default onset method for the property 'combination' */
void ApplicationDeviceConfig_OnSetcombination( ApplicationDeviceConfig _this, XUInt8 
  value );

/* Wrapper function for the non virtual method : 'Application::DeviceConfig.OnSetcombination()' */
void ApplicationDeviceConfig__OnSetcombination( void* _this, XUInt8 value );

/* The following define announces the presence of the method Application::DeviceConfig.OnSetcombination(). */
#define _ApplicationDeviceConfig__OnSetcombination_

/* Default onget method for the property 'numparams' */
XUInt8 ApplicationDeviceConfig_OnGetnumparams( ApplicationDeviceConfig _this );

/* Wrapper function for the non virtual method : 'Application::DeviceConfig.OnGetnumparams()' */
XUInt8 ApplicationDeviceConfig__OnGetnumparams( void* _this );

/* The following define announces the presence of the method Application::DeviceConfig.OnGetnumparams(). */
#define _ApplicationDeviceConfig__OnGetnumparams_

/* Default onset method for the property 'numparams' */
void ApplicationDeviceConfig_OnSetnumparams( ApplicationDeviceConfig _this, XUInt8 
  value );

/* Wrapper function for the non virtual method : 'Application::DeviceConfig.OnSetnumparams()' */
void ApplicationDeviceConfig__OnSetnumparams( void* _this, XUInt8 value );

/* The following define announces the presence of the method Application::DeviceConfig.OnSetnumparams(). */
#define _ApplicationDeviceConfig__OnSetnumparams_

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationDeviceConfig_H */

/* Embedded Wizard */
