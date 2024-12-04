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

#ifndef _AppFaultListChange_H
#define _AppFaultListChange_H

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

/* Forward declaration of the class App::FaultListChange */
#ifndef _AppFaultListChange_
  EW_DECLARE_CLASS( AppFaultListChange )
#define _AppFaultListChange_
#endif


/* Deklaration of class : 'App::FaultListChange' */
EW_DEFINE_FIELDS( AppFaultListChange, XObject )
  EW_PROPERTY( jump,            XBool )
EW_END_OF_FIELDS( AppFaultListChange )

/* Virtual Method Table (VMT) for the class : 'App::FaultListChange' */
EW_DEFINE_METHODS( AppFaultListChange, XObject )
EW_END_OF_METHODS( AppFaultListChange )

#ifdef __cplusplus
  }
#endif

#endif /* _AppFaultListChange_H */

/* Embedded Wizard */
