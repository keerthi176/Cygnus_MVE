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

#ifndef _AppAccess_H
#define _AppAccess_H

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

/* Forward declaration of the class App::Access */
#ifndef _AppAccess_
  EW_DECLARE_CLASS( AppAccess )
#define _AppAccess_
#endif


/* Deklaration of class : 'App::Access' */
EW_DEFINE_FIELDS( AppAccess, XObject )
  EW_PROPERTY( Level,           XInt32 )
EW_END_OF_FIELDS( AppAccess )

/* Virtual Method Table (VMT) for the class : 'App::Access' */
EW_DEFINE_METHODS( AppAccess, XObject )
EW_END_OF_METHODS( AppAccess )

#ifdef __cplusplus
  }
#endif

#endif /* _AppAccess_H */

/* Embedded Wizard */
