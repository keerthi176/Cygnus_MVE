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

#ifndef _AppCAEDayNight_H
#define _AppCAEDayNight_H

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

/* Forward declaration of the class App::CAEDayNight */
#ifndef _AppCAEDayNight_
  EW_DECLARE_CLASS( AppCAEDayNight )
#define _AppCAEDayNight_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* Deklaration of class : 'App::CAEDayNight' */
EW_DEFINE_FIELDS( AppCAEDayNight, AppCAEModule )
  EW_PROPERTY( Input,           AppCAEModule )
  EW_PROPERTY( NightTime,       XBool )
EW_END_OF_FIELDS( AppCAEDayNight )

/* Virtual Method Table (VMT) for the class : 'App::CAEDayNight' */
EW_DEFINE_METHODS( AppCAEDayNight, AppCAEModule )
  EW_METHOD( Render,            XPoint )( AppCAEDayNight _this, XPoint position, 
    XRect aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( AppCAEDayNight _this )
EW_END_OF_METHODS( AppCAEDayNight )

/* 'C' function for method : 'App::CAEDayNight.Render()' */
XPoint AppCAEDayNight_Render( AppCAEDayNight _this, XPoint position, XRect aClip, 
  GraphicsCanvas aCanvas );

/* 'C' function for method : 'App::CAEDayNight.Size()' */
XPoint AppCAEDayNight_Size( AppCAEDayNight _this );

#ifdef __cplusplus
  }
#endif

#endif /* _AppCAEDayNight_H */

/* Embedded Wizard */
