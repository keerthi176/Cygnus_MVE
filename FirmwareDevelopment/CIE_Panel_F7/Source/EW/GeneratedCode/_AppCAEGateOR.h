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

#ifndef _AppCAEGateOR_H
#define _AppCAEGateOR_H

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

#include "_AppCAEGate.h"

/* Forward declaration of the class App::CAEGateOR */
#ifndef _AppCAEGateOR_
  EW_DECLARE_CLASS( AppCAEGateOR )
#define _AppCAEGateOR_
#endif

/* Forward declaration of the class App::CAEModule */
#ifndef _AppCAEModule_
  EW_DECLARE_CLASS( AppCAEModule )
#define _AppCAEModule_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* Deklaration of class : 'App::CAEGateOR' */
EW_DEFINE_FIELDS( AppCAEGateOR, AppCAEGate )
EW_END_OF_FIELDS( AppCAEGateOR )

/* Virtual Method Table (VMT) for the class : 'App::CAEGateOR' */
EW_DEFINE_METHODS( AppCAEGateOR, AppCAEGate )
  EW_METHOD( Render,            XPoint )( AppCAEGateOR _this, XPoint position, XRect 
    aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( AppCAEGate _this )
EW_END_OF_METHODS( AppCAEGateOR )

/* 'C' function for method : 'App::CAEGateOR.Render()' */
XPoint AppCAEGateOR_Render( AppCAEGateOR _this, XPoint position, XRect aClip, GraphicsCanvas 
  aCanvas );

#ifdef __cplusplus
  }
#endif

#endif /* _AppCAEGateOR_H */

/* Embedded Wizard */
