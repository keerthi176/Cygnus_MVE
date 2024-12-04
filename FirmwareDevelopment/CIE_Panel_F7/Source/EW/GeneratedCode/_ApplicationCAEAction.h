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

#ifndef _ApplicationCAEAction_H
#define _ApplicationCAEAction_H

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

#include "_ApplicationCAEModule.h"

/* Forward declaration of the class Application::CAEAction */
#ifndef _ApplicationCAEAction_
  EW_DECLARE_CLASS( ApplicationCAEAction )
#define _ApplicationCAEAction_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* Deklaration of class : 'Application::CAEAction' */
EW_DEFINE_FIELDS( ApplicationCAEAction, ApplicationCAEModule )
  EW_PROPERTY( Input,           ApplicationCAEModule )
EW_END_OF_FIELDS( ApplicationCAEAction )

/* Virtual Method Table (VMT) for the class : 'Application::CAEAction' */
EW_DEFINE_METHODS( ApplicationCAEAction, ApplicationCAEModule )
  EW_METHOD( Render,            XPoint )( ApplicationCAEAction _this, XPoint position, 
    XRect aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( ApplicationCAEAction _this )
EW_END_OF_METHODS( ApplicationCAEAction )

/* 'C' function for method : 'Application::CAEAction.Render()' */
XPoint ApplicationCAEAction_Render( ApplicationCAEAction _this, XPoint position, 
  XRect aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'Application::CAEAction.Size()' */
XPoint ApplicationCAEAction_Size( ApplicationCAEAction _this );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationCAEAction_H */

/* Embedded Wizard */
