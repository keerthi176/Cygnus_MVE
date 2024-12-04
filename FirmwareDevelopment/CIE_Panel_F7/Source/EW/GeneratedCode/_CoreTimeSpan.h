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

#ifndef _CoreTimeSpan_H
#define _CoreTimeSpan_H

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

/* Forward declaration of the class Core::TimeSpan */
#ifndef _CoreTimeSpan_
  EW_DECLARE_CLASS( CoreTimeSpan )
#define _CoreTimeSpan_
#endif


/* The class Core::TimeSpan is used for date and time operations. Objects of this 
   class can store the time span, convert it and format strings with the time span, 
   etc. */
EW_DEFINE_FIELDS( CoreTimeSpan, XObject )
  EW_PROPERTY( Seconds,         XInt32 )
  EW_PROPERTY( Minutes,         XInt32 )
  EW_PROPERTY( Hours,           XInt32 )
  EW_PROPERTY( Days,            XInt32 )
EW_END_OF_FIELDS( CoreTimeSpan )

/* Virtual Method Table (VMT) for the class : 'Core::TimeSpan' */
EW_DEFINE_METHODS( CoreTimeSpan, XObject )
EW_END_OF_METHODS( CoreTimeSpan )

/* 'C' function for method : 'Core::TimeSpan.OnSetTime()' */
void CoreTimeSpan_OnSetTime( CoreTimeSpan _this, XInt32 value );

/* The method Initialize() initializes this Core::TimeSpan object with a time span 
   expressed in seconds and returns this object. */
CoreTimeSpan CoreTimeSpan_Initialize( CoreTimeSpan _this, XInt32 aTime );

#ifdef __cplusplus
  }
#endif

#endif /* _CoreTimeSpan_H */

/* Embedded Wizard */
