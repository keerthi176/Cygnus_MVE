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

#ifndef _AppPtrClass_H
#define _AppPtrClass_H

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

/* Forward declaration of the class App::PtrClass */
#ifndef _AppPtrClass_
  EW_DECLARE_CLASS( AppPtrClass )
#define _AppPtrClass_
#endif


/* Deklaration of class : 'App::PtrClass' */
EW_DEFINE_FIELDS( AppPtrClass, XObject )
  EW_PROPERTY( ptr,             XInt32 )
EW_END_OF_FIELDS( AppPtrClass )

/* Virtual Method Table (VMT) for the class : 'App::PtrClass' */
EW_DEFINE_METHODS( AppPtrClass, XObject )
EW_END_OF_METHODS( AppPtrClass )

#ifdef __cplusplus
  }
#endif

#endif /* _AppPtrClass_H */

/* Embedded Wizard */
