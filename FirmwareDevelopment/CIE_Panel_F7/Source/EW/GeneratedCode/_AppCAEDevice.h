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

#ifndef _AppCAEDevice_H
#define _AppCAEDevice_H

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

/* Forward declaration of the class App::CAEDevice */
#ifndef _AppCAEDevice_
  EW_DECLARE_CLASS( AppCAEDevice )
#define _AppCAEDevice_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif

/* Forward declaration of the class Resources::Bitmap */
#ifndef _ResourcesBitmap_
  EW_DECLARE_CLASS( ResourcesBitmap )
#define _ResourcesBitmap_
#endif


/* Deklaration of class : 'App::CAEDevice' */
EW_DEFINE_FIELDS( AppCAEDevice, AppCAEModule )
  EW_PROPERTY( type,            ResourcesBitmap )
  EW_PROPERTY( UnitNum,         XInt32 )
  EW_PROPERTY( ZoneNum,         XInt32 )
EW_END_OF_FIELDS( AppCAEDevice )

/* Virtual Method Table (VMT) for the class : 'App::CAEDevice' */
EW_DEFINE_METHODS( AppCAEDevice, AppCAEModule )
  EW_METHOD( Render,            XPoint )( AppCAEDevice _this, XPoint position, XRect 
    aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( AppCAEDevice _this )
EW_END_OF_METHODS( AppCAEDevice )

/* 'C' function for method : 'App::CAEDevice.Render()' */
XPoint AppCAEDevice_Render( AppCAEDevice _this, XPoint position, XRect aClip, GraphicsCanvas 
  aCanvas );

/* 'C' function for method : 'App::CAEDevice.Size()' */
XPoint AppCAEDevice_Size( AppCAEDevice _this );

#ifdef __cplusplus
  }
#endif

#endif /* _AppCAEDevice_H */

/* Embedded Wizard */
