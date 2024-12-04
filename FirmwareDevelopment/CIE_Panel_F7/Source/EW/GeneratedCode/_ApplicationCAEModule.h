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

#ifndef _ApplicationCAEModule_H
#define _ApplicationCAEModule_H

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


/* Deklaration of class : 'Application::CAEModule' */
EW_DEFINE_FIELDS( ApplicationCAEModule, XObject )
  EW_PROPERTY( gapSize,         XInt32 )
EW_END_OF_FIELDS( ApplicationCAEModule )

/* Virtual Method Table (VMT) for the class : 'Application::CAEModule' */
EW_DEFINE_METHODS( ApplicationCAEModule, XObject )
  EW_METHOD( Render,            XPoint )( ApplicationCAEModule _this, XPoint position, 
    XRect aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( ApplicationCAEModule _this )
EW_END_OF_METHODS( ApplicationCAEModule )

/* 'C' function for method : 'Application::CAEModule.Render()' */
XPoint ApplicationCAEModule_Render( ApplicationCAEModule _this, XPoint position, 
  XRect aClip, GraphicsCanvas aCanvas );

/* Wrapper function for the virtual method : 'Application::CAEModule.Render()' */
XPoint ApplicationCAEModule__Render( void* _this, XPoint position, XRect aClip, 
  GraphicsCanvas aCanvas );

/* 'C' function for method : 'Application::CAEModule.RenderHoz()' */
XPoint ApplicationCAEModule_RenderHoz( ApplicationCAEModule _this, XPoint start, 
  XInt32 length, XRect aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'Application::CAEModule.RenderVert()' */
XPoint ApplicationCAEModule_RenderVert( ApplicationCAEModule _this, XPoint start, 
  XInt32 length, XRect aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'Application::CAEModule.RenderConnect()' */
void ApplicationCAEModule_RenderConnect( ApplicationCAEModule _this, XPoint pos1, 
  XPoint pos2, XRect aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'Application::CAEModule.Size()' */
XPoint ApplicationCAEModule_Size( ApplicationCAEModule _this );

/* Wrapper function for the virtual method : 'Application::CAEModule.Size()' */
XPoint ApplicationCAEModule__Size( void* _this );

/* 'C' function for method : 'Application::CAEModule.Output()' */
XPoint ApplicationCAEModule_Output( ApplicationCAEModule _this, XPoint position );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationCAEModule_H */

/* Embedded Wizard */
