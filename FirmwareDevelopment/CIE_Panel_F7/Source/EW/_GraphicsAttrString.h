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

#ifndef _GraphicsAttrString_H
#define _GraphicsAttrString_H

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

/* Forward declaration of the class Graphics::AttrSet */
#ifndef _GraphicsAttrSet_
  EW_DECLARE_CLASS( GraphicsAttrSet )
#define _GraphicsAttrSet_
#endif

/* Forward declaration of the class Graphics::AttrString */
#ifndef _GraphicsAttrString_
  EW_DECLARE_CLASS( GraphicsAttrString )
#define _GraphicsAttrString_
#endif


/* Deklaration of class : 'Graphics::AttrString' */
EW_DEFINE_FIELDS( GraphicsAttrString, XObject )
  EW_VARIABLE( size,            XPoint )
  EW_VARIABLE( attrString,      XHandle )
EW_END_OF_FIELDS( GraphicsAttrString )

/* Virtual Method Table (VMT) for the class : 'Graphics::AttrString' */
EW_DEFINE_METHODS( GraphicsAttrString, XObject )
EW_END_OF_METHODS( GraphicsAttrString )

/* 'C' function for method : 'Graphics::AttrString.Done()' */
void GraphicsAttrString_Done( GraphicsAttrString _this );

/* 'C' function for method : 'Graphics::AttrString.Discard()' */
void GraphicsAttrString_Discard( GraphicsAttrString _this );

/* 'C' function for method : 'Graphics::AttrString.GetTextSize()' */
XPoint GraphicsAttrString_GetTextSize( GraphicsAttrString _this );

/* 'C' function for method : 'Graphics::AttrString.Parse()' */
void GraphicsAttrString_Parse( GraphicsAttrString _this, GraphicsAttrSet aAttrSet, 
  XString aString, XInt32 aWidth );

#ifdef __cplusplus
  }
#endif

#endif /* _GraphicsAttrString_H */

/* Embedded Wizard */
