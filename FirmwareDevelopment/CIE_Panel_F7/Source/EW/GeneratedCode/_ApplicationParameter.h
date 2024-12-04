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

#ifndef _ApplicationParameter_H
#define _ApplicationParameter_H

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

/* Forward declaration of the class Application::Parameter */
#ifndef _ApplicationParameter_
  EW_DECLARE_CLASS( ApplicationParameter )
#define _ApplicationParameter_
#endif


/* Deklaration of class : 'Application::Parameter' */
EW_DEFINE_FIELDS( ApplicationParameter, XObject )
  EW_PROPERTY( name,            XString )
  EW_PROPERTY( min,             XInt16 )
  EW_PROPERTY( max,             XInt16 )
  EW_PROPERTY( code,            XEnum )
  EW_PROPERTY( value,           XInt16 )
  EW_PROPERTY( channel,         XUInt8 )
EW_END_OF_FIELDS( ApplicationParameter )

/* Virtual Method Table (VMT) for the class : 'Application::Parameter' */
EW_DEFINE_METHODS( ApplicationParameter, XObject )
EW_END_OF_METHODS( ApplicationParameter )

/* Default onget method for the property 'channel' */
XUInt8 ApplicationParameter_OnGetchannel( ApplicationParameter _this );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnGetchannel()' */
XUInt8 ApplicationParameter__OnGetchannel( void* _this );

/* The following define announces the presence of the method Application::Parameter.OnGetchannel(). */
#define _ApplicationParameter__OnGetchannel_

/* Default onset method for the property 'channel' */
void ApplicationParameter_OnSetchannel( ApplicationParameter _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnSetchannel()' */
void ApplicationParameter__OnSetchannel( void* _this, XUInt8 value );

/* The following define announces the presence of the method Application::Parameter.OnSetchannel(). */
#define _ApplicationParameter__OnSetchannel_

/* Default onget method for the property 'name' */
XString ApplicationParameter_OnGetname( ApplicationParameter _this );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnGetname()' */
XString ApplicationParameter__OnGetname( void* _this );

/* The following define announces the presence of the method Application::Parameter.OnGetname(). */
#define _ApplicationParameter__OnGetname_

/* Default onset method for the property 'name' */
void ApplicationParameter_OnSetname( ApplicationParameter _this, XString value );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnSetname()' */
void ApplicationParameter__OnSetname( void* _this, XString value );

/* The following define announces the presence of the method Application::Parameter.OnSetname(). */
#define _ApplicationParameter__OnSetname_

/* Default onget method for the property 'value' */
XInt16 ApplicationParameter_OnGetvalue( ApplicationParameter _this );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnGetvalue()' */
XInt16 ApplicationParameter__OnGetvalue( void* _this );

/* The following define announces the presence of the method Application::Parameter.OnGetvalue(). */
#define _ApplicationParameter__OnGetvalue_

/* Default onset method for the property 'value' */
void ApplicationParameter_OnSetvalue( ApplicationParameter _this, XInt16 value );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnSetvalue()' */
void ApplicationParameter__OnSetvalue( void* _this, XInt16 value );

/* The following define announces the presence of the method Application::Parameter.OnSetvalue(). */
#define _ApplicationParameter__OnSetvalue_

/* Default onget method for the property 'code' */
XEnum ApplicationParameter_OnGetcode( ApplicationParameter _this );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnGetcode()' */
XEnum ApplicationParameter__OnGetcode( void* _this );

/* The following define announces the presence of the method Application::Parameter.OnGetcode(). */
#define _ApplicationParameter__OnGetcode_

/* Default onset method for the property 'code' */
void ApplicationParameter_OnSetcode( ApplicationParameter _this, XEnum value );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnSetcode()' */
void ApplicationParameter__OnSetcode( void* _this, XEnum value );

/* The following define announces the presence of the method Application::Parameter.OnSetcode(). */
#define _ApplicationParameter__OnSetcode_

/* Default onget method for the property 'max' */
XInt16 ApplicationParameter_OnGetmax( ApplicationParameter _this );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnGetmax()' */
XInt16 ApplicationParameter__OnGetmax( void* _this );

/* The following define announces the presence of the method Application::Parameter.OnGetmax(). */
#define _ApplicationParameter__OnGetmax_

/* Default onset method for the property 'max' */
void ApplicationParameter_OnSetmax( ApplicationParameter _this, XInt16 value );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnSetmax()' */
void ApplicationParameter__OnSetmax( void* _this, XInt16 value );

/* The following define announces the presence of the method Application::Parameter.OnSetmax(). */
#define _ApplicationParameter__OnSetmax_

/* Default onget method for the property 'min' */
XInt16 ApplicationParameter_OnGetmin( ApplicationParameter _this );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnGetmin()' */
XInt16 ApplicationParameter__OnGetmin( void* _this );

/* The following define announces the presence of the method Application::Parameter.OnGetmin(). */
#define _ApplicationParameter__OnGetmin_

/* Default onset method for the property 'min' */
void ApplicationParameter_OnSetmin( ApplicationParameter _this, XInt16 value );

/* Wrapper function for the non virtual method : 'Application::Parameter.OnSetmin()' */
void ApplicationParameter__OnSetmin( void* _this, XInt16 value );

/* The following define announces the presence of the method Application::Parameter.OnSetmin(). */
#define _ApplicationParameter__OnSetmin_

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationParameter_H */

/* Embedded Wizard */
