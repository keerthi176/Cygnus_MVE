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

#ifndef _GraphicsAttrSet_H
#define _GraphicsAttrSet_H

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

/* Forward declaration of the class Graphics::AttrSetBitmaps */
#ifndef _GraphicsAttrSetBitmaps_
  EW_DECLARE_CLASS( GraphicsAttrSetBitmaps )
#define _GraphicsAttrSetBitmaps_
#endif

/* Forward declaration of the class Graphics::AttrSetColors */
#ifndef _GraphicsAttrSetColors_
  EW_DECLARE_CLASS( GraphicsAttrSetColors )
#define _GraphicsAttrSetColors_
#endif

/* Forward declaration of the class Graphics::AttrSetFonts */
#ifndef _GraphicsAttrSetFonts_
  EW_DECLARE_CLASS( GraphicsAttrSetFonts )
#define _GraphicsAttrSetFonts_
#endif

/* Forward declaration of the class Resources::Bitmap */
#ifndef _ResourcesBitmap_
  EW_DECLARE_CLASS( ResourcesBitmap )
#define _ResourcesBitmap_
#endif

/* Forward declaration of the class Resources::Font */
#ifndef _ResourcesFont_
  EW_DECLARE_CLASS( ResourcesFont )
#define _ResourcesFont_
#endif


/* The class Graphics::AttrSet provides a container for attributed text operands. 
   It is intended to store the colors, bitmap and font objects needed when the attributed 
   text is drawn. Its function is similar to the of an array of colors, bitmaps 
   and fonts. The class provides properties and methods to access and manage these 
   arrays.
   Primarily, the properties @Color0 .. @Color9, @Bitmap0 .. @Bitmap9, @Font0 .. 
   @Font3 are available for the direct access to the corresponding array entries. 
   Thus the set can be initialized by simply assigning values to these properties. 
   In the case, the attributed text expects more colors, fonts or bitmaps, the desired 
   maximum capacity has to be determined explicitly by the method @SetMaxNoOfColors(), 
   @SetMaxNoOfFonts() and @SetMaxNoOfBitmaps(). These methods resize the set accordingly. 
   The additional entries can be modified only via methods @SetColor(), @SetFont 
   and @SetBitmap(). To query the entries the methods @GetColor(), @GetFont and 
   @GetBitmap() are available. */
EW_DEFINE_FIELDS( GraphicsAttrSet, XObject )
  EW_VARIABLE( attrSet,         XHandle )
  EW_VARIABLE( attrSetFonts,    GraphicsAttrSetFonts )
  EW_VARIABLE( attrSetBitmaps,  GraphicsAttrSetBitmaps )
  EW_VARIABLE( attrSetColors,   GraphicsAttrSetColors )
  EW_ARRAY   ( fonts,           ResourcesFont, [4])
  EW_ARRAY   ( bitmaps,         ResourcesBitmap, [10])
  EW_ARRAY   ( colors,          XColor, [10])
  EW_PROPERTY( Font0,           ResourcesFont )
  EW_PROPERTY( Bitmap0,         ResourcesBitmap )
EW_END_OF_FIELDS( GraphicsAttrSet )

/* Virtual Method Table (VMT) for the class : 'Graphics::AttrSet' */
EW_DEFINE_METHODS( GraphicsAttrSet, XObject )
EW_END_OF_METHODS( GraphicsAttrSet )

/* 'C' function for method : 'Graphics::AttrSet.Done()' */
void GraphicsAttrSet_Done( GraphicsAttrSet _this );

/* 'C' function for method : 'Graphics::AttrSet.discard()' */
void GraphicsAttrSet_discard( GraphicsAttrSet _this );

/* 'C' function for method : 'Graphics::AttrSet.update()' */
void GraphicsAttrSet_update( GraphicsAttrSet _this );

/* 'C' function for method : 'Graphics::AttrSet.observerSlot()' */
void GraphicsAttrSet_observerSlot( GraphicsAttrSet _this, XObject sender );

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont3()' */
void GraphicsAttrSet_OnSetFont3( GraphicsAttrSet _this, ResourcesFont value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont2()' */
void GraphicsAttrSet_OnSetFont2( GraphicsAttrSet _this, ResourcesFont value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont1()' */
void GraphicsAttrSet_OnSetFont1( GraphicsAttrSet _this, ResourcesFont value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont0()' */
void GraphicsAttrSet_OnSetFont0( GraphicsAttrSet _this, ResourcesFont value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap4()' */
void GraphicsAttrSet_OnSetBitmap4( GraphicsAttrSet _this, ResourcesBitmap value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap3()' */
void GraphicsAttrSet_OnSetBitmap3( GraphicsAttrSet _this, ResourcesBitmap value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap2()' */
void GraphicsAttrSet_OnSetBitmap2( GraphicsAttrSet _this, ResourcesBitmap value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap1()' */
void GraphicsAttrSet_OnSetBitmap1( GraphicsAttrSet _this, ResourcesBitmap value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap0()' */
void GraphicsAttrSet_OnSetBitmap0( GraphicsAttrSet _this, ResourcesBitmap value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetColor2()' */
void GraphicsAttrSet_OnSetColor2( GraphicsAttrSet _this, XColor value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetColor1()' */
void GraphicsAttrSet_OnSetColor1( GraphicsAttrSet _this, XColor value );

/* 'C' function for method : 'Graphics::AttrSet.OnSetColor0()' */
void GraphicsAttrSet_OnSetColor0( GraphicsAttrSet _this, XColor value );

/* The method SetFont() assigns the given font object aFont to the entry aFontNo 
   within this set. The first entry has the number 0, the second 1, and so far. 
   Passing 'null' in aFont parameter causes the set to forget the font currently 
   stored in this entry. If there is no font entry with the number aFontNo in the 
   set, the operation is ignored. The number of available entries can be determined 
   by method @SetMaxNoOfFonts(). This modification can cause all associated Views::AttrText 
   views to update themselves. */
void GraphicsAttrSet_SetFont( GraphicsAttrSet _this, XInt32 aFontNo, ResourcesFont 
  aFont );

/* The method GetMaxNoOfFonts() returns the maximum font array capacity of this 
   set. To change the capacity, the method @SetMaxNoOfFonts() should be used. */
XInt32 GraphicsAttrSet_GetMaxNoOfFonts( GraphicsAttrSet _this );

/* The method SetBitmap() assigns the given bitmap object aBitmap to the entry aBitmapNo 
   within this set. The first entry has the number 0, the second 1, and so far. 
   Passing 'null' in aBitmap parameter causes the set to forget the bitmap currently 
   stored in this entry. If there is no bitmap entry with the number aBitmapNo in 
   the set, the operation is ignored. The number of available entries can be determined 
   by method @SetMaxNoOfBitmaps(). This modification can cause all associated Views::AttrText 
   views to update themselves. */
void GraphicsAttrSet_SetBitmap( GraphicsAttrSet _this, XInt32 aBitmapNo, ResourcesBitmap 
  aBitmap );

/* The method GetMaxNoOfBitmaps() returns the maximum bitmap array capacity of this 
   set. To change the capacity, the method @SetMaxNoOfBitmaps() should be used. */
XInt32 GraphicsAttrSet_GetMaxNoOfBitmaps( GraphicsAttrSet _this );

/* The method SetColor() assigns the given color aColor to the entry aColorNo within 
   this set. The first entry has the number 0, the second 1, and so far. If there 
   is no color entry with the number aColorNo in the set, the operation is ignored. 
   The number of available entries can be determined by the method @SetMaxNoOfColors(). 
   This modification can cause all associated Views::AttrText views to update themselves. */
void GraphicsAttrSet_SetColor( GraphicsAttrSet _this, XInt32 aColorNo, XColor aColor );

/* The method GetMaxNoOfColors() returns the maximum color array capacity of this 
   set. To change the capacity, the method @SetMaxNoOfColors() should be used. */
XInt32 GraphicsAttrSet_GetMaxNoOfColors( GraphicsAttrSet _this );

#ifdef __cplusplus
  }
#endif

#endif /* _GraphicsAttrSet_H */

/* Embedded Wizard */
