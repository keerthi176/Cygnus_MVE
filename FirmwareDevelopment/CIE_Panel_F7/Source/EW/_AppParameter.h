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

#ifndef _AppParameter_H
#define _AppParameter_H

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

/* Forward declaration of the class App::Parameter */
#ifndef _AppParameter_
  EW_DECLARE_CLASS( AppParameter )
#define _AppParameter_
#endif


/* Deklaration of class : 'App::Parameter' */
EW_DEFINE_FIELDS( AppParameter, XObject )
  EW_PROPERTY( name,            XString )
  EW_PROPERTY( min,             XInt16 )
  EW_PROPERTY( max,             XInt16 )
  EW_PROPERTY( code,            XEnum )
  EW_PROPERTY( value,           XInt16 )
  EW_PROPERTY( channel,         XUInt8 )
EW_END_OF_FIELDS( AppParameter )

/* Virtual Method Table (VMT) for the class : 'App::Parameter' */
EW_DEFINE_METHODS( AppParameter, XObject )
EW_END_OF_METHODS( AppParameter )

/* Default onget method for the property 'channel' */
XUInt8 AppParameter_OnGetchannel( AppParameter _this );

/* Wrapper function for the non virtual method : 'App::Parameter.OnGetchannel()' */
XUInt8 AppParameter__OnGetchannel( void* _this );

/* The following define announces the presence of the method App::Parameter.OnGetchannel(). */
#define _AppParameter__OnGetchannel_

/* Default onset method for the property 'channel' */
void AppParameter_OnSetchannel( AppParameter _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'App::Parameter.OnSetchannel()' */
void AppParameter__OnSetchannel( void* _this, XUInt8 value );

/* The following define announces the presence of the method App::Parameter.OnSetchannel(). */
#define _AppParameter__OnSetchannel_

/* Default onget method for the property 'name' */
XString AppParameter_OnGetname( AppParameter _this );

/* Wrapper function for the non virtual method : 'App::Parameter.OnGetname()' */
XString AppParameter__OnGetname( void* _this );

/* The following define announces the presence of the method App::Parameter.OnGetname(). */
#define _AppParameter__OnGetname_

/* Default onset method for the property 'name' */
void AppParameter_OnSetname( AppParameter _this, XString value );

/* Wrapper function for the non virtual method : 'App::Parameter.OnSetname()' */
void AppParameter__OnSetname( void* _this, XString value );

/* The following define announces the presence of the method App::Parameter.OnSetname(). */
#define _AppParameter__OnSetname_

/* Default onget method for the property 'value' */
XInt16 AppParameter_OnGetvalue( AppParameter _this );

/* Wrapper function for the non virtual method : 'App::Parameter.OnGetvalue()' */
XInt16 AppParameter__OnGetvalue( void* _this );

/* The following define announces the presence of the method App::Parameter.OnGetvalue(). */
#define _AppParameter__OnGetvalue_

/* Default onset method for the property 'value' */
void AppParameter_OnSetvalue( AppParameter _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::Parameter.OnSetvalue()' */
void AppParameter__OnSetvalue( void* _this, XInt16 value );

/* The following define announces the presence of the method App::Parameter.OnSetvalue(). */
#define _AppParameter__OnSetvalue_

/* Default onget method for the property 'code' */
XEnum AppParameter_OnGetcode( AppParameter _this );

/* Wrapper function for the non virtual method : 'App::Parameter.OnGetcode()' */
XEnum AppParameter__OnGetcode( void* _this );

/* The following define announces the presence of the method App::Parameter.OnGetcode(). */
#define _AppParameter__OnGetcode_

/* Default onset method for the property 'code' */
void AppParameter_OnSetcode( AppParameter _this, XEnum value );

/* Wrapper function for the non virtual method : 'App::Parameter.OnSetcode()' */
void AppParameter__OnSetcode( void* _this, XEnum value );

/* The following define announces the presence of the method App::Parameter.OnSetcode(). */
#define _AppParameter__OnSetcode_

/* Default onget method for the property 'max' */
XInt16 AppParameter_OnGetmax( AppParameter _this );

/* Wrapper function for the non virtual method : 'App::Parameter.OnGetmax()' */
XInt16 AppParameter__OnGetmax( void* _this );

/* The following define announces the presence of the method App::Parameter.OnGetmax(). */
#define _AppParameter__OnGetmax_

/* Default onset method for the property 'max' */
void AppParameter_OnSetmax( AppParameter _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::Parameter.OnSetmax()' */
void AppParameter__OnSetmax( void* _this, XInt16 value );

/* The following define announces the presence of the method App::Parameter.OnSetmax(). */
#define _AppParameter__OnSetmax_

/* Default onget method for the property 'min' */
XInt16 AppParameter_OnGetmin( AppParameter _this );

/* Wrapper function for the non virtual method : 'App::Parameter.OnGetmin()' */
XInt16 AppParameter__OnGetmin( void* _this );

/* The following define announces the presence of the method App::Parameter.OnGetmin(). */
#define _AppParameter__OnGetmin_

/* Default onset method for the property 'min' */
void AppParameter_OnSetmin( AppParameter _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::Parameter.OnSetmin()' */
void AppParameter__OnSetmin( void* _this, XInt16 value );

/* The following define announces the presence of the method App::Parameter.OnSetmin(). */
#define _AppParameter__OnSetmin_

#ifdef __cplusplus
  }
#endif

#endif /* _AppParameter_H */

/* Embedded Wizard */
