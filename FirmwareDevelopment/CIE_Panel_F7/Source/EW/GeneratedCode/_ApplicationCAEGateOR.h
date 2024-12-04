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

#ifndef _ApplicationCAEGateOR_H
#define _ApplicationCAEGateOR_H

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

#include "_ApplicationCAEGate.h"

/* Forward declaration of the class Application::CAEGateOR */
#ifndef _ApplicationCAEGateOR_
  EW_DECLARE_CLASS( ApplicationCAEGateOR )
#define _ApplicationCAEGateOR_
#endif

/* Forward declaration of the class Application::CAEModule */
#ifndef _ApplicationCAEModule_
  EW_DECLARE_CLASS( ApplicationCAEModule )
#define _ApplicationCAEModule_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* Deklaration of class : 'Application::CAEGateOR' */
EW_DEFINE_FIELDS( ApplicationCAEGateOR, ApplicationCAEGate )
EW_END_OF_FIELDS( ApplicationCAEGateOR )

/* Virtual Method Table (VMT) for the class : 'Application::CAEGateOR' */
EW_DEFINE_METHODS( ApplicationCAEGateOR, ApplicationCAEGate )
  EW_METHOD( Render,            XPoint )( ApplicationCAEGateOR _this, XPoint position, 
    XRect aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( ApplicationCAEGate _this )
EW_END_OF_METHODS( ApplicationCAEGateOR )

/* 'C' function for method : 'Application::CAEGateOR.Render()' */
XPoint ApplicationCAEGateOR_Render( ApplicationCAEGateOR _this, XPoint position, 
  XRect aClip, GraphicsCanvas aCanvas );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationCAEGateOR_H */

/* Embedded Wizard */
