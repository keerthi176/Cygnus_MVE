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

#ifndef _AppCAEAction_H
#define _AppCAEAction_H

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

#include "_AppCAEModule.h"

/* Forward declaration of the class App::CAEAction */
#ifndef _AppCAEAction_
  EW_DECLARE_CLASS( AppCAEAction )
#define _AppCAEAction_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* Deklaration of class : 'App::CAEAction' */
EW_DEFINE_FIELDS( AppCAEAction, AppCAEModule )
  EW_PROPERTY( Input,           AppCAEModule )
EW_END_OF_FIELDS( AppCAEAction )

/* Virtual Method Table (VMT) for the class : 'App::CAEAction' */
EW_DEFINE_METHODS( AppCAEAction, AppCAEModule )
  EW_METHOD( Render,            XPoint )( AppCAEAction _this, XPoint position, XRect 
    aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( AppCAEAction _this )
EW_END_OF_METHODS( AppCAEAction )

/* 'C' function for method : 'App::CAEAction.Render()' */
XPoint AppCAEAction_Render( AppCAEAction _this, XPoint position, XRect aClip, GraphicsCanvas 
  aCanvas );

/* 'C' function for method : 'App::CAEAction.Size()' */
XPoint AppCAEAction_Size( AppCAEAction _this );

#ifdef __cplusplus
  }
#endif

#endif /* _AppCAEAction_H */

/* Embedded Wizard */
