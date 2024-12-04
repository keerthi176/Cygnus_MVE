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

#ifndef _AppRadioUnit_H
#define _AppRadioUnit_H

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

/* Forward declaration of the class App::RadioUnit */
#ifndef _AppRadioUnit_
  EW_DECLARE_CLASS( AppRadioUnit )
#define _AppRadioUnit_
#endif


/* Deklaration of class : 'App::RadioUnit' */
EW_DEFINE_FIELDS( AppRadioUnit, XObject )
  EW_PROPERTY( starttime,       XUInt32 )
  EW_PROPERTY( stage,           XEnum )
  EW_PROPERTY( sent,            XInt16 )
  EW_PROPERTY( received,        XInt16 )
  EW_PROPERTY( primary,         XInt16 )
  EW_PROPERTY( rssi,            XInt16 )
  EW_PROPERTY( unitnum,         XInt16 )
  EW_PROPERTY( zonenum,         XInt16 )
  EW_PROPERTY( valid,           XUInt8 )
  EW_PROPERTY( battery,         XInt8 )
  EW_PROPERTY( combination,     XUInt8 )
EW_END_OF_FIELDS( AppRadioUnit )

/* Virtual Method Table (VMT) for the class : 'App::RadioUnit' */
EW_DEFINE_METHODS( AppRadioUnit, XObject )
EW_END_OF_METHODS( AppRadioUnit )

/* Default onget method for the property 'zonenum' */
XInt16 AppRadioUnit_OnGetzonenum( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetzonenum()' */
XInt16 AppRadioUnit__OnGetzonenum( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetzonenum(). */
#define _AppRadioUnit__OnGetzonenum_

/* Default onset method for the property 'zonenum' */
void AppRadioUnit_OnSetzonenum( AppRadioUnit _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetzonenum()' */
void AppRadioUnit__OnSetzonenum( void* _this, XInt16 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetzonenum(). */
#define _AppRadioUnit__OnSetzonenum_

/* Default onget method for the property 'unitnum' */
XInt16 AppRadioUnit_OnGetunitnum( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetunitnum()' */
XInt16 AppRadioUnit__OnGetunitnum( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetunitnum(). */
#define _AppRadioUnit__OnGetunitnum_

/* Default onset method for the property 'unitnum' */
void AppRadioUnit_OnSetunitnum( AppRadioUnit _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetunitnum()' */
void AppRadioUnit__OnSetunitnum( void* _this, XInt16 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetunitnum(). */
#define _AppRadioUnit__OnSetunitnum_

/* Default onget method for the property 'combination' */
XUInt8 AppRadioUnit_OnGetcombination( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetcombination()' */
XUInt8 AppRadioUnit__OnGetcombination( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetcombination(). */
#define _AppRadioUnit__OnGetcombination_

/* Default onset method for the property 'combination' */
void AppRadioUnit_OnSetcombination( AppRadioUnit _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetcombination()' */
void AppRadioUnit__OnSetcombination( void* _this, XUInt8 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetcombination(). */
#define _AppRadioUnit__OnSetcombination_

/* Default onget method for the property 'starttime' */
XUInt32 AppRadioUnit_OnGetstarttime( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetstarttime()' */
XUInt32 AppRadioUnit__OnGetstarttime( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetstarttime(). */
#define _AppRadioUnit__OnGetstarttime_

/* Default onset method for the property 'starttime' */
void AppRadioUnit_OnSetstarttime( AppRadioUnit _this, XUInt32 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetstarttime()' */
void AppRadioUnit__OnSetstarttime( void* _this, XUInt32 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetstarttime(). */
#define _AppRadioUnit__OnSetstarttime_

/* Default onget method for the property 'rssi' */
XInt16 AppRadioUnit_OnGetrssi( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetrssi()' */
XInt16 AppRadioUnit__OnGetrssi( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetrssi(). */
#define _AppRadioUnit__OnGetrssi_

/* Default onset method for the property 'rssi' */
void AppRadioUnit_OnSetrssi( AppRadioUnit _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetrssi()' */
void AppRadioUnit__OnSetrssi( void* _this, XInt16 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetrssi(). */
#define _AppRadioUnit__OnSetrssi_

/* Default onget method for the property 'primary' */
XInt16 AppRadioUnit_OnGetprimary( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetprimary()' */
XInt16 AppRadioUnit__OnGetprimary( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetprimary(). */
#define _AppRadioUnit__OnGetprimary_

/* Default onset method for the property 'primary' */
void AppRadioUnit_OnSetprimary( AppRadioUnit _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetprimary()' */
void AppRadioUnit__OnSetprimary( void* _this, XInt16 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetprimary(). */
#define _AppRadioUnit__OnSetprimary_

/* Default onget method for the property 'received' */
XInt16 AppRadioUnit_OnGetreceived( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetreceived()' */
XInt16 AppRadioUnit__OnGetreceived( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetreceived(). */
#define _AppRadioUnit__OnGetreceived_

/* Default onset method for the property 'received' */
void AppRadioUnit_OnSetreceived( AppRadioUnit _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetreceived()' */
void AppRadioUnit__OnSetreceived( void* _this, XInt16 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetreceived(). */
#define _AppRadioUnit__OnSetreceived_

/* Default onget method for the property 'sent' */
XInt16 AppRadioUnit_OnGetsent( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetsent()' */
XInt16 AppRadioUnit__OnGetsent( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetsent(). */
#define _AppRadioUnit__OnGetsent_

/* Default onset method for the property 'sent' */
void AppRadioUnit_OnSetsent( AppRadioUnit _this, XInt16 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetsent()' */
void AppRadioUnit__OnSetsent( void* _this, XInt16 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetsent(). */
#define _AppRadioUnit__OnSetsent_

/* Default onget method for the property 'stage' */
XEnum AppRadioUnit_OnGetstage( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetstage()' */
XEnum AppRadioUnit__OnGetstage( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetstage(). */
#define _AppRadioUnit__OnGetstage_

/* Default onset method for the property 'stage' */
void AppRadioUnit_OnSetstage( AppRadioUnit _this, XEnum value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetstage()' */
void AppRadioUnit__OnSetstage( void* _this, XEnum value );

/* The following define announces the presence of the method App::RadioUnit.OnSetstage(). */
#define _AppRadioUnit__OnSetstage_

/* Default onget method for the property 'battery' */
XInt8 AppRadioUnit_OnGetbattery( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetbattery()' */
XInt8 AppRadioUnit__OnGetbattery( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetbattery(). */
#define _AppRadioUnit__OnGetbattery_

/* Default onset method for the property 'battery' */
void AppRadioUnit_OnSetbattery( AppRadioUnit _this, XInt8 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetbattery()' */
void AppRadioUnit__OnSetbattery( void* _this, XInt8 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetbattery(). */
#define _AppRadioUnit__OnSetbattery_

/* Default onget method for the property 'valid' */
XUInt8 AppRadioUnit_OnGetvalid( AppRadioUnit _this );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnGetvalid()' */
XUInt8 AppRadioUnit__OnGetvalid( void* _this );

/* The following define announces the presence of the method App::RadioUnit.OnGetvalid(). */
#define _AppRadioUnit__OnGetvalid_

/* Default onset method for the property 'valid' */
void AppRadioUnit_OnSetvalid( AppRadioUnit _this, XUInt8 value );

/* Wrapper function for the non virtual method : 'App::RadioUnit.OnSetvalid()' */
void AppRadioUnit__OnSetvalid( void* _this, XUInt8 value );

/* The following define announces the presence of the method App::RadioUnit.OnSetvalid(). */
#define _AppRadioUnit__OnSetvalid_

#ifdef __cplusplus
  }
#endif

#endif /* _AppRadioUnit_H */

/* Embedded Wizard */
