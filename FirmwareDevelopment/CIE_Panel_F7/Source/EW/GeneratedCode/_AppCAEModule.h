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

#ifndef _AppCAEModule_H
#define _AppCAEModule_H

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


/* Deklaration of class : 'App::CAEModule' */
EW_DEFINE_FIELDS( AppCAEModule, XObject )
  EW_PROPERTY( gapSize,         XInt32 )
EW_END_OF_FIELDS( AppCAEModule )

/* Virtual Method Table (VMT) for the class : 'App::CAEModule' */
EW_DEFINE_METHODS( AppCAEModule, XObject )
  EW_METHOD( Render,            XPoint )( AppCAEModule _this, XPoint position, XRect 
    aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( AppCAEModule _this )
EW_END_OF_METHODS( AppCAEModule )

/* 'C' function for method : 'App::CAEModule.Render()' */
XPoint AppCAEModule_Render( AppCAEModule _this, XPoint position, XRect aClip, GraphicsCanvas 
  aCanvas );

/* Wrapper function for the virtual method : 'App::CAEModule.Render()' */
XPoint AppCAEModule__Render( void* _this, XPoint position, XRect aClip, GraphicsCanvas 
  aCanvas );

/* 'C' function for method : 'App::CAEModule.RenderHoz()' */
XPoint AppCAEModule_RenderHoz( AppCAEModule _this, XPoint start, XInt32 length, 
  XRect aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'App::CAEModule.RenderVert()' */
XPoint AppCAEModule_RenderVert( AppCAEModule _this, XPoint start, XInt32 length, 
  XRect aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'App::CAEModule.RenderConnect()' */
void AppCAEModule_RenderConnect( AppCAEModule _this, XPoint pos1, XPoint pos2, XRect 
  aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'App::CAEModule.Size()' */
XPoint AppCAEModule_Size( AppCAEModule _this );

/* Wrapper function for the virtual method : 'App::CAEModule.Size()' */
XPoint AppCAEModule__Size( void* _this );

/* 'C' function for method : 'App::CAEModule.Output()' */
XPoint AppCAEModule_Output( AppCAEModule _this, XPoint position );

#ifdef __cplusplus
  }
#endif

#endif /* _AppCAEModule_H */

/* Embedded Wizard */
