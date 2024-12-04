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

#ifndef _AppCAEZoneFireAlarm_H
#define _AppCAEZoneFireAlarm_H

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

#include "_AppCAEZone.h"

/* Forward declaration of the class App::CAEZoneFireAlarm */
#ifndef _AppCAEZoneFireAlarm_
  EW_DECLARE_CLASS( AppCAEZoneFireAlarm )
#define _AppCAEZoneFireAlarm_
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


/* Deklaration of class : 'App::CAEZoneFireAlarm' */
EW_DEFINE_FIELDS( AppCAEZoneFireAlarm, AppCAEZone )
EW_END_OF_FIELDS( AppCAEZoneFireAlarm )

/* Virtual Method Table (VMT) for the class : 'App::CAEZoneFireAlarm' */
EW_DEFINE_METHODS( AppCAEZoneFireAlarm, AppCAEZone )
  EW_METHOD( Render,            XPoint )( AppCAEZoneFireAlarm _this, XPoint position, 
    XRect aClip, GraphicsCanvas aCanvas )
  EW_METHOD( Size,              XPoint )( AppCAEZoneFireAlarm _this )
EW_END_OF_METHODS( AppCAEZoneFireAlarm )

/* 'C' function for method : 'App::CAEZoneFireAlarm.Render()' */
XPoint AppCAEZoneFireAlarm_Render( AppCAEZoneFireAlarm _this, XPoint position, XRect 
  aClip, GraphicsCanvas aCanvas );

/* 'C' function for method : 'App::CAEZoneFireAlarm.Size()' */
XPoint AppCAEZoneFireAlarm_Size( AppCAEZoneFireAlarm _this );

#ifdef __cplusplus
  }
#endif

#endif /* _AppCAEZoneFireAlarm_H */

/* Embedded Wizard */
