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

#include "ewlocale.h"
#include "_GraphicsAttrSet.h"
#include "_GraphicsAttrSetBitmaps.h"
#include "_GraphicsAttrSetColors.h"
#include "_GraphicsAttrSetFonts.h"
#include "_GraphicsAttrString.h"
#include "_GraphicsCanvas.h"
#include "_GraphicsPath.h"
#include "_GraphicsWarpMatrix.h"
#include "_ResourcesBitmap.h"
#include "_ResourcesFont.h"
#include "Graphics.h"
#include "Resources.h"

/* Compressed strings for the language 'Default'. */
static const unsigned int _StringsDefault0[] =
{
  0x000003FC, /* ratio 46.67 % */
  0xB8007D00, 0x80000452, 0x00C20021, 0xC4000370, 0x006F0042, 0x400869D0, 0x0032800E,
  0x034800E6, 0x8A45CF40, 0x8000F118, 0x46CD8003, 0x4E4E6300, 0xA2879934, 0x151389C2,
  0x91F344A6, 0x1533C50C, 0x40048E13, 0x8D4A6393, 0xC432310E, 0x462191C9, 0x02546287,
  0xA493A368, 0x06358346, 0x04E00416, 0x0A9CCDE2, 0x9944F649, 0xA243E411, 0x48028F0F,
  0xD32271CA, 0xAA252EAC, 0x26B3C995, 0x1D1189C5, 0x940ADC4E, 0x768642AB, 0x283D3E74,
  0xB69520D5, 0x05DE5362, 0x5D615790, 0x9A15758E, 0xA15188B4, 0x8B0E96CD, 0xF8A8E52A,
  0x368C47A4, 0x4A51DE1D, 0x8742A1D3, 0xDDA22D38, 0xD0EBD3FA, 0x92294693, 0xA7767B55,
  0x0D049341, 0x9169D15E, 0xADA510A4, 0x9000A249, 0x92446282, 0xE5AB4518, 0x43A19118,
  0x7F55869A, 0xA97BEA14, 0x299889D2, 0x51F3ACDA, 0x34EB55AE, 0x4AEE32C5, 0xC944E4DD,
  0x6CE73D38, 0x78231CAC, 0xDA399B8A, 0xE2DCDE0C, 0xB33893F1, 0xD3AEC4E7, 0x001A0038,
  0x01B00060, 0x92450720, 0x9184891C, 0xE4182E49, 0x4C006C5C, 0x1F495761, 0x739656DD,
  0x9BD6ED68, 0x247DBF43, 0xD95677DC, 0x48575C87, 0x8510F72D, 0x1D083DCE, 0x08919900,
  0xC1627500, 0x146016F5, 0x7751A76D, 0xD6957DC3, 0xF58D9F76, 0x9D9A79D1, 0x567CDEB4,
  0x4716D7B9, 0xE2B859F1, 0xA7E1F686, 0x39FE7F1F, 0x02002004, 0x41A05832, 0xC468214C,
  0x04992649, 0x012C5585, 0x00715380, 0x701CD848, 0xE5C8C558, 0x8875C97B, 0x32207311,
  0x0E5BA225, 0x5A9D574D, 0x9F25A971, 0xC89E2E43, 0x89A78620, 0xA8796347, 0x8E9E8969,
  0xE3E88242, 0x47B24156, 0x190E60DF, 0xFF7F5FB9, 0x8124D49D, 0x101A4606, 0x00000000
};

/* Constant values used in this 'C' module only. */
static const XPoint _Const0000 = { 0, 0 };
static const XStringRes _Const0001 = { _StringsDefault0, 0x0003 };
static const XRect _Const0002 = {{ 0, 0 }, { 0, 0 }};
static const XStringRes _Const0003 = { _StringsDefault0, 0x0041 };
static const XStringRes _Const0004 = { _StringsDefault0, 0x0076 };
static const XStringRes _Const0005 = { _StringsDefault0, 0x00B9 };
static const XStringRes _Const0006 = { _StringsDefault0, 0x0125 };
static const XStringRes _Const0007 = { _StringsDefault0, 0x0192 };
static const XPoint _Const0008 = { 1, 1 };
static const XPoint _Const0009 = { 2, 2 };
static const XColor _Const000A = { 0x00, 0x00, 0x00, 0xFF };
static const XColor _Const000B = { 0x00, 0x00, 0x00, 0x00 };

#ifndef EW_DONT_CHECK_INDEX
  /* This function is used to check the indices when accessing an array.
     If you don't want this verification add the define EW_DONT_CHECK_INDEX
     to your Makefile or project settings. */
  static int EwCheckIndex( int aIndex, int aRange, const char* aFile, int aLine )
  {
    if (( aIndex < 0 ) || ( aIndex >= aRange ))
    {
      EwPrint( "[FATAL ERROR in %s:%d] Array index %d out of bounds %d",
                aFile, aLine, aIndex, aRange );
      EwPanic();
    }
    return aIndex;
  }

  #define EwCheckIndex( aIndex, aRange ) \
    EwCheckIndex( aIndex, aRange, __FILE__, __LINE__ )
#else
  #define EwCheckIndex( aIndex, aRange ) aIndex
#endif

/* Initializer for the class 'Graphics::Canvas' */
void GraphicsCanvas__Init( GraphicsCanvas _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  ResourcesBitmap__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsCanvas );

  /* Call the user defined constructor */
  GraphicsCanvas_Init( _this, aArg );
}

/* Re-Initializer for the class 'Graphics::Canvas' */
void GraphicsCanvas__ReInit( GraphicsCanvas _this )
{
  /* At first re-initialize the super class ... */
  ResourcesBitmap__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::Canvas' */
void GraphicsCanvas__Done( GraphicsCanvas _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsCanvas );

  /* Call the user defined destructor of the class */
  GraphicsCanvas_Done( _this );

  /* Don't forget to deinitialize the super class ... */
  ResourcesBitmap__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::Canvas' */
void GraphicsCanvas__Mark( GraphicsCanvas _this )
{
  EwMarkSlot( _this->OnDraw );

  /* Give the super class a chance to mark its objects and references */
  ResourcesBitmap__Mark( &_this->_Super );
}

/* 'C' function for method : 'Graphics::Canvas.Done()' */
void GraphicsCanvas_Done( GraphicsCanvas _this )
{
  if ( _this->attached )
    GraphicsCanvas_DetachBitmap( _this );
}

/* 'C' function for method : 'Graphics::Canvas.Init()' */
void GraphicsCanvas_Init( GraphicsCanvas _this, XHandle aArg )
{
  /* Dummy expressions to avoid the 'C' warning 'unused argument'. */
  EW_UNUSED_ARG( aArg );

  _this->Super1.Mutable = 1;
}

/* 'C' function for method : 'Graphics::Canvas.OnSetFrameSize()' */
void GraphicsCanvas_OnSetFrameSize( GraphicsCanvas _this, XPoint value )
{
  XHandle handle;

  if (( value.X <= 0 ) || ( value.Y <= 0 ))
    value = _Const0000;

  if ( !EwCompPoint( value, _this->Super1.FrameSize ))
    return;

  if ( _this->attached )
  {
    EwThrow( EwLoadString( &_Const0001 ));
    return;
  }

  _this->Super1.FrameSize = value;
  _this->Super1.Animated = (XBool)(((( _this->Super1.FrameSize.X > 0 ) && ( _this->Super1.FrameSize.Y 
  > 0 )) && ( _this->Super1.FrameDelay > 0 )) && ( _this->Super1.NoOfFrames > 1 
  ));

  if ( _this->Super1.bitmap == 0 )
    return;

  handle = _this->Super1.bitmap;
  EwFreeBitmap((XBitmap*)handle );
  _this->Super1.bitmap = 0;
}

/* 'C' function for method : 'Graphics::Canvas.Update()' */
void GraphicsCanvas_Update( GraphicsCanvas _this )
{
  if ((( _this->Super1.bitmap == 0 ) && ( _this->Super1.FrameSize.X > 0 )) && ( 
      _this->Super1.FrameSize.Y > 0 ))
  {
    XPoint frameSize = _this->Super1.FrameSize;
    XInt32 noOfFrames = _this->Super1.NoOfFrames;
    XInt32 frameDelay = _this->Super1.FrameDelay;
    XHandle handle = 0;
    {
      handle = (XHandle)EwCreateBitmap( EW_PIXEL_FORMAT_NATIVE, frameSize, 
                                        frameDelay, noOfFrames );
    }
    _this->Super1.bitmap = handle;

    if ( _this->Super1.bitmap == 0 )
    {
      _this->Super1.FrameSize = _Const0000;
      _this->Super1.FrameDelay = 0;
      _this->Super1.NoOfFrames = 1;
    }

    _this->InvalidArea = EwNewRect2Point( _Const0000, _this->Super1.FrameSize );
  }

  if ( !EwIsRectEmpty( _this->InvalidArea ))
  {
    if (( _this->Super1.FrameSize.X > 0 ) && ( _this->Super1.FrameSize.Y > 0 ))
      EwSignal( _this->OnDraw, ((XObject)_this ));

    _this->InvalidArea = _Const0002;
  }
}

/* The method DetachBitmap() exists for the integration purpose with the underlying 
   target system. You will never need to invoke this method directly from your GUI 
   application. The method is invoked after the screen update is finished and the 
   canvas object should be detached from the framebuffer. */
GraphicsCanvas GraphicsCanvas_DetachBitmap( GraphicsCanvas _this )
{
  if ( !_this->attached )
  {
    EwThrow( EwLoadString( &_Const0003 ));
    return 0;
  }

  _this->Super1.bitmap = 0;
  _this->attached = 0;
  _this->Super1.FrameSize = _Const0000;
  _this->Super1.FrameDelay = 0;
  _this->Super1.NoOfFrames = 1;
  _this->Super1.Animated = 0;
  return _this;
}

/* Wrapper function for the non virtual method : 'Graphics::Canvas.DetachBitmap()' */
GraphicsCanvas GraphicsCanvas__DetachBitmap( void* _this )
{
  return GraphicsCanvas_DetachBitmap((GraphicsCanvas)_this );
}

/* The method AttachBitmap() exists for the integration purpose with the underlying 
   target system. You will never need to invoke this method directly from your GUI 
   application. The method is invoked at the beginning of the screen update.
   The method connects the canvas object with the framebuffer. */
GraphicsCanvas GraphicsCanvas_AttachBitmap( GraphicsCanvas _this, XHandle aBitmap )
{
  XInt32 noOfFrames;
  XPoint frameSize;
  XInt32 frameDelay;

  if ( _this->Super1.bitmap != 0 )
  {
    EwThrow( EwLoadString( &_Const0004 ));
    return 0;
  }

  if ( aBitmap == 0 )
    return _this;

  _this->Super1.bitmap = aBitmap;
  _this->attached = 1;
  noOfFrames = 1;
  frameSize = _Const0000;
  frameDelay = 0;
  {
    XBitmap* bmp = (XBitmap*)aBitmap;

    noOfFrames = bmp->NoOfFrames;
    frameSize  = bmp->FrameSize;
    frameDelay = bmp->FrameDelay;
  }
  _this->Super1.NoOfFrames = noOfFrames;
  _this->Super1.FrameSize = frameSize;
  _this->Super1.FrameDelay = frameDelay;
  _this->Super1.Animated = (XBool)(( _this->Super1.FrameDelay > 0 ) && ( _this->Super1.NoOfFrames 
  > 1 ));
  return _this;
}

/* Wrapper function for the non virtual method : 'Graphics::Canvas.AttachBitmap()' */
GraphicsCanvas GraphicsCanvas__AttachBitmap( void* _this, XHandle aBitmap )
{
  return GraphicsCanvas_AttachBitmap((GraphicsCanvas)_this, aBitmap );
}

/* 'C' function for method : 'Graphics::Canvas.DrawAttrText()' */
void GraphicsCanvas_DrawAttrText( GraphicsCanvas _this, XRect aClip, GraphicsAttrSet 
  aAttrSet, GraphicsAttrString aAttrString, XRect aDstRect, XPoint aSrcPos, XInt32 
  aOpacityTL, XInt32 aOpacityTR, XInt32 aOpacityBR, XInt32 aOpacityBL, XBool aBlend )
{
  XHandle attrSet;
  XHandle attrString;
  XHandle dstBitmap;
  XInt32 dstFrameNo;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  attrSet = 0;
  attrString = 0;

  if ( aAttrSet != 0 )
  {
    GraphicsAttrSet_update( aAttrSet );
    attrSet = aAttrSet->attrSet;
  }

  if ( aAttrString != 0 )
    attrString = aAttrString->attrString;

  if (( attrSet == 0 ) || ( attrString == 0 ))
    return;

  dstBitmap = _this->Super1.bitmap;
  dstFrameNo = _this->DstFrameNr;
  {
    EwDrawAttrText((XBitmap*)dstBitmap, (XAttrSet*)attrSet, (XAttrString*)attrString,
                    dstFrameNo, aClip, aDstRect, aSrcPos, aOpacityTL, aOpacityTR,
                    aOpacityBR, aOpacityBL, aBlend );
  }
}

/* The method DrawText() draws the text row passed in the parameter aString into 
   the canvas. The font to draw the text is passed in the parameter aFont. The parameter 
   aOffset determines within aString the sign to start the drawing operation. If 
   aOffset is zero, the operation starts with the first sign. The parameter aCount 
   determines the max. number of following sigs to draw. If aCount is -1, all signs 
   until the end of the string are drawn. 
   The area to draw the text is determined by the parameter aDstRect. The optional 
   aSrcPos parameter determines a displacement of the text within this aDstRect 
   area. The parameter aMinWidth determines the min. width in pixel of the drawn 
   text row. If necessary the space signs within the text will be stretched to fill 
   this area. The parameters aColorTL, aColorTR, aColorBL, aColorBR determine the 
   colors at the corresponding corners of the aDstRect area.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The last aBlend parameter controls the mode how drawn pixel 
   are combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_DrawText( GraphicsCanvas _this, XRect aClip, ResourcesFont aFont, 
  XString aString, XInt32 aOffset, XInt32 aCount, XRect aDstRect, XPoint aSrcPos, 
  XInt32 aMinWidth, XColor aColorTL, XColor aColorTR, XColor aColorBR, XColor aColorBL, 
  XBool aBlend )
{
  XInt32 dstFrameNo;
  XHandle dstBitmap;
  XHandle srcFont;
  XRect tempRect;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if ( aOffset < 0 )
    aOffset = 0;

  if ((( aFont == 0 ) || ( aFont->font == 0 )) || (( aOffset > 0 ) && ( aOffset 
      >= EwGetStringLength( aString ))))
    return;

  dstFrameNo = _this->DstFrameNr;
  dstBitmap = _this->Super1.bitmap;
  srcFont = aFont->font;
  tempRect = aClip;
  {
    EwDrawText((XBitmap*)dstBitmap, (XFont*)srcFont, aString + aOffset, aCount,
                dstFrameNo, tempRect, aDstRect, aSrcPos, aMinWidth, aColorTL, aColorTR,
                aColorBR, aColorBL, aBlend );
  }
}

/* The method DrawBitmapFrame() draws a free scalable frame by composing it of bitmap 
   segments. These segments are used to draw the frame's corners, to fill its edges 
   and to fill its interior area. The bitmap has thus to contain nine equal segments 
   arranged in three rows and three columns. The top-left segment e.g. is used to 
   draw the top-left corner of the frame. In contrast, the top-middle segment corresponds 
   to the frame's top edge. If the edge is wider than the segment, multiple copies 
   of the segment are used to fill the entire edge. In this manner the entire frame 
   is composed by simply copying bitmap segments.
   The bitmap is specified in the parameter aBitmap. In case of a multi-frame bitmap 
   the desired frame can be selected in the parameter aFrameNr. The resulting size 
   of the drawn frame is specified by aDstRect parameter. The parameter aEdges control 
   which edges are drawn and which are omitted. Optionally the copied pixel can 
   be modulated by a color gradient specified by the four parameters aColorTL .. 
   aColorBL.
   An additional clipping area aClip limits the operation. All pixel lying outside 
   this area will not be drawn. The last aBlend parameter controls the mode how 
   drawn pixel are combined with the pixel already existing in the destination bitmap. 
   If aBlend is 'true', the drawn pixel are alpha-blended with the background, otherwise 
   the drawn pixel will overwrite the old content. */
void GraphicsCanvas_DrawBitmapFrame( GraphicsCanvas _this, XRect aClip, ResourcesBitmap 
  aBitmap, XInt32 aFrameNr, XRect aDstRect, XSet aEdges, XColor aColorTL, XColor 
  aColorTR, XColor aColorBR, XColor aColorBL, XBool aBlend )
{
  XHandle dstBitmap;
  XHandle srcBitmap;
  XInt32 dstFrameNo;
  XRect srcRect;
  XBool left;
  XBool top;
  XBool right;
  XBool bottom;
  XBool interior;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if ((((( aBitmap == 0 ) || ( aBitmap->bitmap == 0 )) || !aEdges ) || ( aFrameNr 
      < 0 )) || ( aFrameNr >= aBitmap->NoOfFrames ))
    return;

  dstBitmap = _this->Super1.bitmap;
  srcBitmap = aBitmap->bitmap;
  dstFrameNo = _this->DstFrameNr;
  srcRect = EwNewRect2Point( _Const0000, aBitmap->FrameSize );
  left = (( aEdges & GraphicsEdgesLeft ) == GraphicsEdgesLeft );
  top = (( aEdges & GraphicsEdgesTop ) == GraphicsEdgesTop );
  right = (( aEdges & GraphicsEdgesRight ) == GraphicsEdgesRight );
  bottom = (( aEdges & GraphicsEdgesBottom ) == GraphicsEdgesBottom );
  interior = (( aEdges & GraphicsEdgesInterior ) == GraphicsEdgesInterior );
  {
    EwDrawBitmapFrame((XBitmap*)dstBitmap, (XBitmap*)srcBitmap, dstFrameNo, aFrameNr,
                       aClip, aDstRect, srcRect, left, top, right, bottom,
                       interior, aColorTL, aColorTR, aColorBR, aColorBL, aBlend );
  }
}

/* The method StrokePath() strokes within the rectangular area aDstRect of canvas 
   a path determined by data stored in the Graphics::Path object aPath. All path 
   coordinates are assumed as being relative to the top-left corner of the aDstRect 
   area, or if the parameter aFlipY is 'true', relative to the bottom-left corner. 
   With the parameter aFlipY equal 'true' the path coordinates are mirrored vertically 
   causing the positive Y-axis to point upwards. With the parameter aOffset the 
   origin of the path coordinate system can be moved within aDstRect. Accordingly 
   modifying this value scrolls the displayed path content.
   The thickness of the path is determined by the parameter aWidth and is expressed 
   in pixel. The parameters aStartCapStyle and aEndCapStyle determine how the start/end 
   position of the path are displayed. The possible values are specified in Graphics::PathCap. 
   Furthermore the parameter aJoinStyle controls how the line segments of the path 
   are connected together. Here the possible values are specified in Graphics::PathJoin. 
   Please note, if aJoinStyle is Graphics::PathJoin.Miter, the additional parameter 
   aMiterLimit determines the max. ratio between the length of the miter and the 
   half of the path thickness (aWidth / 2). If this limit is exceeded, the affected 
   corner is joined with an ordinary bevel (Graphics::PathJoin.Bevel) instead of 
   miter.
   The parameters aColorTL, aColorTR, aColorBL, aColorBR determine the colors at 
   the corresponding corners of the aDstRect area. If the parameter aAntialiased 
   is 'true', the method applies antialiasing while rasterizing the path pixel.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The aBlend parameter controls the mode how drawn pixel are 
   combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_StrokePath( GraphicsCanvas _this, XRect aClip, GraphicsPath 
  aPath, XRect aDstRect, XBool aFlipY, XPoint aOffset, XFloat aWidth, XEnum aStartCapStyle, 
  XEnum aEndCapStyle, XEnum aJoinStyle, XFloat aMiterLimit, XColor aColorTL, XColor 
  aColorTR, XColor aColorBR, XColor aColorBL, XBool aBlend, XBool aAntialiased )
{
  XUInt32 style;
  XHandle dstBitmap;
  XInt32 dstFrameNo;
  XHandle path;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if (( aPath == 0 ) || ( aPath->path == 0 ))
    return;

  style = 0;

  switch ( aStartCapStyle )
  {
    case GraphicsPathCapSquare :
      style = style | 1;
    break;

    case GraphicsPathCapTriangle :
      style = style | 2;
    break;

    case GraphicsPathCapRound :
      style = style | 3;
    break;

    default : 
      ;
  }

  switch ( aEndCapStyle )
  {
    case GraphicsPathCapSquare :
      style = style | 256;
    break;

    case GraphicsPathCapTriangle :
      style = style | 512;
    break;

    case GraphicsPathCapRound :
      style = style | 768;
    break;

    default : 
      ;
  }

  switch ( aJoinStyle )
  {
    case GraphicsPathJoinMiter :
      style = style | 65536;
    break;

    case GraphicsPathJoinRound :
      style = style | 131072;
    break;

    default : 
      ;
  }

  dstBitmap = _this->Super1.bitmap;
  dstFrameNo = _this->DstFrameNr;
  path = aPath->path;
  {
    EwStrokePath((XBitmap*)dstBitmap, (XPath*)path, dstFrameNo, aClip, aDstRect, aFlipY,
                  aOffset, aWidth, style, aMiterLimit, aColorTL, aColorTR, aColorBR, 
                  aColorBL, aBlend, aAntialiased );
  }
}

/* The method WarpBitmap() performs the projection of a rectangular source bitmap 
   area onto a four corner polygon within the destination canvas. The bitmap is 
   specified in the parameter aBitmap and the desired area to copy in aSrcRect. 
   In case of a multi-frame bitmap the desired frame can be selected in the parameter 
   aFrameNr.
   The destination polygon is determined by the coordinates aX1,aY1 .. aX4,aY4. 
   The coefficients aW1 .. aW4 are responsible for the perspective distortion. The 
   parameters aColor1, aColor2, aColor3, aColor4 determine the colors or opacities 
   at the corresponding corners of the polygon area. The parameter aClip limits 
   the drawing operation. Pixel lying outside this area remain unchanged. The last 
   aBlend parameter controls the mode how drawn pixel are combined with the pixel 
   already existing in the destination bitmap. If aBlend is 'true', the drawn pixel 
   are alpha-blended with the background, otherwise the drawn pixel will overwrite 
   the old content. */
void GraphicsCanvas_WarpBitmap( GraphicsCanvas _this, XRect aClip, ResourcesBitmap 
  aBitmap, XInt32 aFrameNr, XFloat aDstX1, XFloat aDstY1, XFloat aDstW1, XFloat 
  aDstX2, XFloat aDstY2, XFloat aDstW2, XFloat aDstX3, XFloat aDstY3, XFloat aDstW3, 
  XFloat aDstX4, XFloat aDstY4, XFloat aDstW4, XRect aSrcRect, XColor aColor1, XColor 
  aColor2, XColor aColor3, XColor aColor4, XBool aBlend, XBool aFilter )
{
  XFloat x1;
  XFloat x2;
  XFloat y1;
  XFloat y2;
  XHandle dstBitmap;
  XHandle srcBitmap;
  XInt32 dstFrameNr;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if (((( aBitmap == 0 ) || ( aBitmap->bitmap == 0 )) || ( aFrameNr < 0 )) || ( 
      aFrameNr >= aBitmap->NoOfFrames ))
    return;

  x1 = aDstX1;
  x2 = aDstX2;
  y1 = aDstY1;
  y2 = aDstY2;

  if ( aDstX2 < x1 )
    x1 = aDstX2;

  if ( aDstX3 < x1 )
    x1 = aDstX3;

  if ( aDstX4 < x1 )
    x1 = aDstX4;

  if ( aDstX2 > x2 )
    x2 = aDstX2;

  if ( aDstX3 > x2 )
    x2 = aDstX3;

  if ( aDstX4 > x2 )
    x2 = aDstX4;

  if ( aDstY2 < y1 )
    y1 = aDstY2;

  if ( aDstY3 < y1 )
    y1 = aDstY3;

  if ( aDstY4 < y1 )
    y1 = aDstY4;

  if ( aDstY2 > y2 )
    y2 = aDstY2;

  if ( aDstY3 > y2 )
    y2 = aDstY3;

  if ( aDstY4 > y2 )
    y2 = aDstY4;

  if ((((( x2 - x1 ) > 4096.000000f ) || (( x2 - x1 ) < -4096.000000f )) || (( y2 
      - y1 ) > 4096.000000f )) || (( y2 - y1 ) < -4096.000000f ))
  {
    EwTrace( "%s", EwLoadString( &_Const0005 ));
    return;
  }

  dstBitmap = _this->Super1.bitmap;
  srcBitmap = aBitmap->bitmap;
  dstFrameNr = _this->DstFrameNr;
  {
    EwWarpBitmap((XBitmap*)dstBitmap, (XBitmap*)srcBitmap, dstFrameNr, aFrameNr,
                  aClip, aDstX1, aDstY1, aDstW1, aDstX2, aDstY2, aDstW2, 
                  aDstX3, aDstY3, aDstW3, aDstX4, aDstY4, aDstW4, aSrcRect, aColor1,
                  aColor2, aColor3, aColor4, aBlend, aFilter );
  }
}

/* The method ScaleBitmap() copies and scales an area of a aBitmap into the canvas. 
   The bitmap is specified in the parameter aBitmap and the desired area to copy 
   in aSrcRect. In case of a multi-frame bitmap the desired frame can be selected 
   in the parameter aFrameNr.
   The destination area in canvas is determined by the parameter aDstRect. The parameters 
   aColorTL, aColorTR, aColorBL, aColorBR determine the colors or opacities at the 
   corresponding corners of the aDstRect area.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The last aBlend parameter controls the mode how drawn pixel 
   are combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_ScaleBitmap( GraphicsCanvas _this, XRect aClip, ResourcesBitmap 
  aBitmap, XInt32 aFrameNr, XRect aDstRect, XRect aSrcRect, XColor aColorTL, XColor 
  aColorTR, XColor aColorBR, XColor aColorBL, XBool aBlend, XBool aFilter )
{
  XFloat top;
  XFloat left;
  XFloat right;
  XFloat bottom;
  XHandle dstBitmap;
  XHandle srcBitmap;
  XInt32 dstFrameNo;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if (((( aBitmap == 0 ) || ( aBitmap->bitmap == 0 )) || ( aFrameNr < 0 )) || ( 
      aFrameNr >= aBitmap->NoOfFrames ))
    return;

  top = (XFloat)aDstRect.Point1.Y;
  left = (XFloat)aDstRect.Point1.X;
  right = (XFloat)aDstRect.Point2.X;
  bottom = (XFloat)aDstRect.Point2.Y;

  if ((((( right - left ) > 4096.000000f ) || (( right - left ) < -4096.000000f 
      )) || (( bottom - top ) > 4096.000000f )) || (( bottom - top ) < -4096.000000f 
      ))
  {
    EwTrace( "%s", EwLoadString( &_Const0006 ));
    return;
  }

  dstBitmap = _this->Super1.bitmap;
  srcBitmap = aBitmap->bitmap;
  dstFrameNo = _this->DstFrameNr;
  {
    EwWarpBitmap((XBitmap*)dstBitmap, (XBitmap*)srcBitmap, dstFrameNo, aFrameNr, aClip,
                  left,  top,    1.0f, right, top,    1.0f,
                  right, bottom, 1.0f, left,  bottom, 1.0f,
                  aSrcRect, aColorTL, aColorTR, aColorBR, aColorBL, aBlend, aFilter );
  }
}

/* The method TileBitmap() fills an area of the canvas with multiple copies of an 
   area of the bitmap. The bitmap is specified in the parameter aBitmap and the 
   desired area to copy in aSrcRect. In case of a multi-frame bitmap the desired 
   frame can be selected in the parameter aFrameNr.
   The area to fill in canvas is determined by the parameter aDstRect. The optional 
   aSrcPos parameter determines a displacement of the bitmap tiles within this aDstRect 
   area. The parameters aColorTL, aColorTR, aColorBL, aColorBR determine the colors 
   or opacities at the corresponding corners of the aDstRect area.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The last aBlend parameter controls the mode how drawn pixel 
   are combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_TileBitmap( GraphicsCanvas _this, XRect aClip, ResourcesBitmap 
  aBitmap, XInt32 aFrameNr, XRect aDstRect, XRect aSrcRect, XPoint aSrcPos, XColor 
  aColorTL, XColor aColorTR, XColor aColorBR, XColor aColorBL, XBool aBlend )
{
  XHandle dstBitmap;
  XHandle srcBitmap;
  XInt32 dstFrameNr;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if (((( aBitmap == 0 ) || ( aBitmap->bitmap == 0 )) || ( aFrameNr < 0 )) || ( 
      aFrameNr >= aBitmap->NoOfFrames ))
    return;

  dstBitmap = _this->Super1.bitmap;
  srcBitmap = aBitmap->bitmap;
  dstFrameNr = _this->DstFrameNr;
  {
    EwTileBitmap((XBitmap*)dstBitmap, (XBitmap*)srcBitmap, dstFrameNr, aFrameNr,
                  aClip, aDstRect, aSrcRect, aSrcPos, aColorTL, aColorTR, aColorBR,
                  aColorBL, aBlend );
  }
}

/* The method CopyBitmap() copies an area of a aBitmap into the canvas. The bitmap 
   is specified in the parameter aBitmap. In case of a multi-frame bitmap the desired 
   frame can be selected in the parameter aFrameNr.
   The area to copy the bitmap is determined by the parameter aDstRect. The optional 
   aSrcPos parameter determines a displacement of the bitmap within this aDstRect 
   area. The parameters aColorTL, aColorTR, aColorBL, aColorBR determine the colors 
   or opacities at the corresponding corners of the aDstRect area.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The last aBlend parameter controls the mode how drawn pixel 
   are combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_CopyBitmap( GraphicsCanvas _this, XRect aClip, ResourcesBitmap 
  aBitmap, XInt32 aFrameNr, XRect aDstRect, XPoint aSrcPos, XColor aColorTL, XColor 
  aColorTR, XColor aColorBR, XColor aColorBL, XBool aBlend )
{
  XHandle dstBitmap;
  XHandle srcBitmap;
  XInt32 dstFrameNr;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if (((( aBitmap == 0 ) || ( aBitmap->bitmap == 0 )) || ( aFrameNr < 0 )) || ( 
      aFrameNr >= aBitmap->NoOfFrames ))
    return;

  dstBitmap = _this->Super1.bitmap;
  srcBitmap = aBitmap->bitmap;
  dstFrameNr = _this->DstFrameNr;
  {
    EwCopyBitmap((XBitmap*)dstBitmap, (XBitmap*)srcBitmap, dstFrameNr, aFrameNr,
                  aClip, aDstRect, aSrcPos, aColorTL, aColorTR, aColorBR, aColorBL,
                  aBlend );
  }
}

/* The method FillPath() fills within the rectangular area aDstRect of canvas a 
   polygon determined by data stored in the Graphics::Path object aPath. All path 
   coordinates are assumed as being relative to the top-left corner of the aDstRect 
   area, or if the parameter aFlipY is 'true', relative to the bottom-left corner. 
   With the parameter aFlipY equal 'true' the path coordinates are additionally 
   mirrored vertically causing the positive Y-axis to point upwards. With the parameter 
   aOffset the origin of the path coordinate system can be moved within aDstRect. 
   Accordingly modifying this value scrolls the displayed path content.
   The parameters aColorTL, aColorTR, aColorBL, aColorBR determine the colors at 
   the corresponding corners of the aDstRect area. If the parameter aAntialiased 
   is 'true', the method applies antialiasing while rasterizing the path pixel. 
   The parameter aFillRule controls the fill algorithm how it treats nested polygon 
   areas. The possible values are specified in Graphics::FillRule.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The aBlend parameter controls the mode how drawn pixel are 
   combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_FillPath( GraphicsCanvas _this, XRect aClip, GraphicsPath aPath, 
  XRect aDstRect, XBool aFlipY, XPoint aOffset, XColor aColorTL, XColor aColorTR, 
  XColor aColorBR, XColor aColorBL, XBool aBlend, XBool aAntialiased, XEnum aFillRule )
{
  XBool nonZeroWinding;
  XHandle dstBitmap;
  XInt32 dstFrameNo;
  XHandle path;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  if (( aPath == 0 ) || ( aPath->path == 0 ))
    return;

  nonZeroWinding = (XBool)( aFillRule == GraphicsFillRuleNonZero );
  dstBitmap = _this->Super1.bitmap;
  dstFrameNo = _this->DstFrameNr;
  path = aPath->path;
  {
    EwFillPath((XBitmap*)dstBitmap, (XPath*)path, dstFrameNo, aClip, aDstRect, aFlipY,
                aOffset, aColorTL, aColorTR, aColorBR, aColorBL, aBlend, aAntialiased,
                nonZeroWinding );
  }
}

/* The method DrawBorder() draws a hollow rectangle in an area of canvas. The area 
   is determined by the parameter aDstRect. The parameter aEdgeWidth determine the 
   width of the border's edge. The parameters aColorTL, aColorTR, aColorBL, aColorBR 
   determine the colors at the corresponding corners of the aDstRect area.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The last aBlend parameter controls the mode how drawn pixel 
   are combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_DrawBorder( GraphicsCanvas _this, XRect aClip, XRect aDstRect, 
  XInt32 aEdgeWidth, XColor aColorTL, XColor aColorTR, XColor aColorBR, XColor aColorBL, 
  XBool aBlend )
{
  XHandle dstBitmap;
  XInt32 dstFrameNo;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  dstBitmap = _this->Super1.bitmap;
  dstFrameNo = _this->DstFrameNr;
  {
    EwDrawBorder((XBitmap*)dstBitmap, dstFrameNo, aClip, aDstRect, aEdgeWidth, 
                  aColorTL, aColorTR, aColorBR, aColorBL, aBlend );
  }
}

/* 'C' function for method : 'Graphics::Canvas.FillPolygon()' */
void GraphicsCanvas_FillPolygon( GraphicsCanvas _this, XRect aClip, XFloat aDstX1, 
  XFloat aDstY1, XFloat aDstW1, XFloat aDstX2, XFloat aDstY2, XFloat aDstW2, XFloat 
  aDstX3, XFloat aDstY3, XFloat aDstW3, XFloat aDstX4, XFloat aDstY4, XFloat aDstW4, 
  XColor aColor1, XColor aColor2, XColor aColor3, XColor aColor4, XBool aBlend )
{
  XFloat x1;
  XFloat x2;
  XFloat y1;
  XFloat y2;
  ResourcesBitmap opaqueBitmap;
  XRect srcRect;
  XHandle dstBitmap;
  XInt32 dstFrameNo;
  XHandle srcBitmap;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  x1 = aDstX1;
  x2 = aDstX2;
  y1 = aDstY1;
  y2 = aDstY2;

  if ( aDstX2 < x1 )
    x1 = aDstX2;

  if ( aDstX3 < x1 )
    x1 = aDstX3;

  if ( aDstX4 < x1 )
    x1 = aDstX4;

  if ( aDstX2 > x2 )
    x2 = aDstX2;

  if ( aDstX3 > x2 )
    x2 = aDstX3;

  if ( aDstX4 > x2 )
    x2 = aDstX4;

  if ( aDstY2 < y1 )
    y1 = aDstY2;

  if ( aDstY3 < y1 )
    y1 = aDstY3;

  if ( aDstY4 < y1 )
    y1 = aDstY4;

  if ( aDstY2 > y2 )
    y2 = aDstY2;

  if ( aDstY3 > y2 )
    y2 = aDstY3;

  if ( aDstY4 > y2 )
    y2 = aDstY4;

  if ((((( x2 - x1 ) > 4096.000000f ) || (( x2 - x1 ) < -4096.000000f )) || (( y2 
      - y1 ) > 4096.000000f )) || (( y2 - y1 ) < -4096.000000f ))
  {
    EwTrace( "%s", EwLoadString( &_Const0007 ));
    return;
  }

  opaqueBitmap = EwLoadResource( &ResourcesWhiteBitmap, ResourcesBitmap );
  srcRect = EwNewRect2Point( _Const0008, EwMovePointNeg( opaqueBitmap->FrameSize, 
  _Const0009 ));
  dstBitmap = _this->Super1.bitmap;
  dstFrameNo = _this->DstFrameNr;
  srcBitmap = opaqueBitmap->bitmap;
  {
    EwWarpBitmap((XBitmap*)dstBitmap, (XBitmap*)srcBitmap, dstFrameNo, 0, aClip,
                  aDstX1, aDstY1, aDstW1, aDstX2, aDstY2, aDstW2, 
                  aDstX3, aDstY3, aDstW3, aDstX4, aDstY4, aDstW4, srcRect,
                  aColor1, aColor2, aColor3, aColor4, aBlend, 0 );
  }
}

/* The method FillRectangle() fills an area of canvas. The area is determined by 
   the parameter aDstRect. The parameters aColorTL, aColorTR, aColorBL, aColorBR 
   determine the colors at the corresponding corners of the aDstRect area.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The last aBlend parameter controls the mode how drawn pixel 
   are combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_FillRectangle( GraphicsCanvas _this, XRect aClip, XRect aDstRect, 
  XColor aColorTL, XColor aColorTR, XColor aColorBR, XColor aColorBL, XBool aBlend )
{
  XHandle dstBitmap;
  XInt32 dstFrameNo;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  dstBitmap = _this->Super1.bitmap;
  dstFrameNo = _this->DstFrameNr;
  {
    EwFillRectangle((XBitmap*)dstBitmap, dstFrameNo, aClip, aDstRect, aColorTL, aColorTR,
                     aColorBR, aColorBL, aBlend );
  }
}

/* The method DrawLine() draws a line into the canvas. The line will be drawn from 
   the aDstPos1 point to the aDstPos2 point with the given colors aColor1 and aColor2.
   The parameter aClip limits the drawing operation. Pixel lying outside this area 
   remain unchanged. The last aBlend parameter controls the mode how drawn pixel 
   are combined with the pixel already existing in the destination bitmap. If aBlend 
   is 'true', the drawn pixel are alpha-blended with the background, otherwise the 
   drawn pixel will overwrite the old content. */
void GraphicsCanvas_DrawLine( GraphicsCanvas _this, XRect aClip, XPoint aDstPos1, 
  XPoint aDstPos2, XColor aColor1, XColor aColor2, XBool aBlend )
{
  XHandle dstBitmap;
  XInt32 dstFrameNo;

  if ( _this->Super1.bitmap == 0 )
    ResourcesBitmap__Update( _this );

  if ( _this->Super1.bitmap == 0 )
    return;

  dstBitmap = _this->Super1.bitmap;
  dstFrameNo = _this->DstFrameNr;
  {
    EwDrawLine((XBitmap*)dstBitmap, dstFrameNo, aClip, aDstPos1, aDstPos2, aColor1,
                aColor2, aBlend );
  }
}

/* Variants derived from the class : 'Graphics::Canvas' */
EW_DEFINE_CLASS_VARIANTS( GraphicsCanvas )
EW_END_OF_CLASS_VARIANTS( GraphicsCanvas )

/* Virtual Method Table (VMT) for the class : 'Graphics::Canvas' */
EW_DEFINE_CLASS( GraphicsCanvas, ResourcesBitmap, "Graphics::Canvas" )
  GraphicsCanvas_OnSetFrameSize,
  GraphicsCanvas_Update,
EW_END_OF_CLASS( GraphicsCanvas )

/* Initializer for the class 'Graphics::WarpMatrix' */
void GraphicsWarpMatrix__Init( GraphicsWarpMatrix _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsWarpMatrix );

  /* ... and initialize objects, variables, properties, etc. */
  _this->isIdentity = 1;
  _this->M11 = 1.000000f;
  _this->M22 = 1.000000f;
  _this->M33 = 1.000000f;
  _this->M44 = 1.000000f;
  _this->EyeDistance = 0.000000f;
}

/* Re-Initializer for the class 'Graphics::WarpMatrix' */
void GraphicsWarpMatrix__ReInit( GraphicsWarpMatrix _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::WarpMatrix' */
void GraphicsWarpMatrix__Done( GraphicsWarpMatrix _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsWarpMatrix );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::WarpMatrix' */
void GraphicsWarpMatrix__Mark( GraphicsWarpMatrix _this )
{
  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* 'C' function for method : 'Graphics::WarpMatrix.CalculateZ()' */
XBool GraphicsWarpMatrix_CalculateZ( GraphicsWarpMatrix _this, XFloat aX, XFloat 
  aY )
{
  XFloat z = (( aX * _this->M31 ) + ( aY * _this->M32 )) + _this->M34;
  XFloat w = (( aX * _this->M41 ) + ( aY * _this->M42 )) + _this->M44;

  if (( w != 0.000000f ) && ( w != 1.000000f ))
    z = z / w;

  _this->Z = z;
  return 1;
}

/* 'C' function for method : 'Graphics::WarpMatrix.Project()' */
XBool GraphicsWarpMatrix_Project( GraphicsWarpMatrix _this, XFloat aX, XFloat aY )
{
  XFloat eyeDistance = _this->EyeDistance;
  XFloat x;
  XFloat y;
  XFloat z;
  XFloat w;
  XFloat q;

  if ( eyeDistance < 0.000000f )
    return 0;

  x = (( aX * _this->M11 ) + ( aY * _this->M12 )) + _this->M14;
  y = (( aX * _this->M21 ) + ( aY * _this->M22 )) + _this->M24;
  z = (( aX * _this->M31 ) + ( aY * _this->M32 )) + _this->M34;
  w = (( aX * _this->M41 ) + ( aY * _this->M42 )) + _this->M44;
  q = 1.000000f;

  if (( w != 0.000000f ) && ( w != 1.000000f ))
  {
    x = x / w;
    y = y / w;
    z = z / w;
  }

  if ( eyeDistance != -z )
    q = q / ( z + eyeDistance );

  if ( eyeDistance != 0.000000f )
    q = q * eyeDistance;

  _this->X = x * q;
  _this->Y = y * q;
  _this->Z = z + eyeDistance;
  return 1;
}

/* 'C' function for method : 'Graphics::WarpMatrix.DeriveFromQuad()' */
GraphicsWarpMatrix GraphicsWarpMatrix_DeriveFromQuad( GraphicsWarpMatrix _this, 
  XFloat aX1, XFloat aY1, XFloat aX2, XFloat aY2, XFloat aX3, XFloat aY3, XFloat 
  aX4, XFloat aY4 )
{
  XFloat deltaX1 = aX2 - aX3;
  XFloat deltaY1 = aY2 - aY3;
  XFloat deltaX2 = aX4 - aX3;
  XFloat deltaY2 = aY4 - aY3;
  XFloat sumX = (( aX1 - aX2 ) + aX3 ) - aX4;
  XFloat sumY = (( aY1 - aY2 ) + aY3 ) - aY4;
  XFloat det = ( deltaX1 * deltaY2 ) - ( deltaY1 * deltaX2 );

  if ( det == 0.000000f )
    return 0;

  _this->M31 = (( sumX * deltaY2 ) - ( sumY * deltaX2 )) / det;
  _this->M32 = (( deltaX1 * sumY ) - ( deltaY1 * sumX )) / det;
  _this->M33 = 0.000000f;
  _this->M34 = 1.000000f;
  _this->M11 = ( aX2 - aX1 ) + ( _this->M31 * aX2 );
  _this->M12 = ( aX4 - aX1 ) + ( _this->M32 * aX4 );
  _this->M13 = 0.000000f;
  _this->M14 = aX1;
  _this->M21 = ( aY2 - aY1 ) + ( _this->M31 * aY2 );
  _this->M22 = ( aY4 - aY1 ) + ( _this->M32 * aY4 );
  _this->M23 = 0.000000f;
  _this->M24 = aY1;
  _this->M41 = 0.000000f;
  _this->M42 = 0.000000f;
  _this->M43 = 0.000000f;
  _this->M44 = 1.000000f;
  _this->isIdentity = 0;
  return _this;
}

/* Variants derived from the class : 'Graphics::WarpMatrix' */
EW_DEFINE_CLASS_VARIANTS( GraphicsWarpMatrix )
EW_END_OF_CLASS_VARIANTS( GraphicsWarpMatrix )

/* Virtual Method Table (VMT) for the class : 'Graphics::WarpMatrix' */
EW_DEFINE_CLASS( GraphicsWarpMatrix, XObject, "Graphics::WarpMatrix" )
EW_END_OF_CLASS( GraphicsWarpMatrix )

/* Initializer for the class 'Graphics::AttrString' */
void GraphicsAttrString__Init( GraphicsAttrString _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsAttrString );
}

/* Re-Initializer for the class 'Graphics::AttrString' */
void GraphicsAttrString__ReInit( GraphicsAttrString _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::AttrString' */
void GraphicsAttrString__Done( GraphicsAttrString _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsAttrString );

  /* Call the user defined destructor of the class */
  GraphicsAttrString_Done( _this );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::AttrString' */
void GraphicsAttrString__Mark( GraphicsAttrString _this )
{
  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* 'C' function for method : 'Graphics::AttrString.Done()' */
void GraphicsAttrString_Done( GraphicsAttrString _this )
{
  GraphicsAttrString_Discard( _this );
}

/* 'C' function for method : 'Graphics::AttrString.Discard()' */
void GraphicsAttrString_Discard( GraphicsAttrString _this )
{
  XHandle handle = _this->attrString;

  if ( handle == 0 )
    return;

  EwFreeAttrString((XAttrString*)handle );
  _this->attrString = 0;
  _this->size = _Const0000;
}

/* 'C' function for method : 'Graphics::AttrString.GetTextSize()' */
XPoint GraphicsAttrString_GetTextSize( GraphicsAttrString _this )
{
  return _this->size;
}

/* 'C' function for method : 'Graphics::AttrString.Parse()' */
void GraphicsAttrString_Parse( GraphicsAttrString _this, GraphicsAttrSet aAttrSet, 
  XString aString, XInt32 aWidth )
{
  XHandle attrSet;
  XHandle handle;
  XPoint s;

  GraphicsAttrString_Discard( _this );
  attrSet = 0;
  handle = 0;
  s = _Const0000;

  if ( aAttrSet != 0 )
  {
    GraphicsAttrSet_update( aAttrSet );
    attrSet = aAttrSet->attrSet;
  }

  if ( !EwCompString( aString, 0 ) || ( attrSet == 0 ))
    return;

  handle = (XHandle)EwParseAttrString((XAttrSet*)attrSet, aString, aWidth );

  if ( handle == 0 )
    return;

  s = EwGetAttrTextSize((XAttrString*)handle );
  _this->attrString = handle;
  _this->size = s;
}

/* Variants derived from the class : 'Graphics::AttrString' */
EW_DEFINE_CLASS_VARIANTS( GraphicsAttrString )
EW_END_OF_CLASS_VARIANTS( GraphicsAttrString )

/* Virtual Method Table (VMT) for the class : 'Graphics::AttrString' */
EW_DEFINE_CLASS( GraphicsAttrString, XObject, "Graphics::AttrString" )
EW_END_OF_CLASS( GraphicsAttrString )

/* Initializer for the class 'Graphics::AttrSet' */
void GraphicsAttrSet__Init( GraphicsAttrSet _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsAttrSet );

  /* ... and initialize objects, variables, properties, etc. */
  _this->colors[ 0 ] = _Const000A;
  _this->fonts[ 0 ] = EwLoadResource( &ResourcesDefaultFont, ResourcesFont );
  _this->bitmaps[ 0 ] = EwLoadResource( &ResourcesDefaultBitmap, ResourcesBitmap 
  );
  _this->Font0 = EwLoadResource( &ResourcesDefaultFont, ResourcesFont );
  _this->Bitmap0 = EwLoadResource( &ResourcesDefaultBitmap, ResourcesBitmap );
}

/* Re-Initializer for the class 'Graphics::AttrSet' */
void GraphicsAttrSet__ReInit( GraphicsAttrSet _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::AttrSet' */
void GraphicsAttrSet__Done( GraphicsAttrSet _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsAttrSet );

  /* Call the user defined destructor of the class */
  GraphicsAttrSet_Done( _this );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::AttrSet' */
void GraphicsAttrSet__Mark( GraphicsAttrSet _this )
{
  EwMarkObject( _this->attrSetFonts );
  EwMarkObject( _this->attrSetBitmaps );
  EwMarkObject( _this->attrSetColors );
  EwMarkObjects( _this->fonts );
  EwMarkObjects( _this->bitmaps );
  EwMarkObject( _this->Font0 );
  EwMarkObject( _this->Bitmap0 );

  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* 'C' function for method : 'Graphics::AttrSet.Done()' */
void GraphicsAttrSet_Done( GraphicsAttrSet _this )
{
  GraphicsAttrSet_discard( _this );
}

/* 'C' function for method : 'Graphics::AttrSet.discard()' */
void GraphicsAttrSet_discard( GraphicsAttrSet _this )
{
  XHandle handle = _this->attrSet;

  if ( handle == 0 )
    return;

  EwFreeAttrSet((XAttrSet*)handle );
  _this->attrSet = 0;
}

/* 'C' function for method : 'Graphics::AttrSet.update()' */
void GraphicsAttrSet_update( GraphicsAttrSet _this )
{
  XHandle handle;
  XInt32 noOfColors;
  XInt32 noOfBitmaps;
  XInt32 noOfFonts;
  GraphicsAttrSetColors listC;
  GraphicsAttrSetBitmaps listB;
  GraphicsAttrSetFonts listF;
  XInt32 inx;
  XInt32 lnx;

  if ( _this->attrSet != 0 )
    return;

  handle = 0;
  noOfColors = GraphicsAttrSet_GetMaxNoOfColors( _this );
  noOfBitmaps = GraphicsAttrSet_GetMaxNoOfBitmaps( _this );
  noOfFonts = GraphicsAttrSet_GetMaxNoOfFonts( _this );
  handle = (XHandle)EwCreateAttrSet( noOfFonts, noOfBitmaps, noOfColors );
  _this->attrSet = handle;

  if ( handle == 0 )
    return;

  listC = _this->attrSetColors;
  listB = _this->attrSetBitmaps;
  listF = _this->attrSetFonts;

  for ( inx = 0; inx < 10; inx = inx + 1 )
  {
    XColor entry = _this->colors[ EwCheckIndex( inx, 10 )];

    if ( EwCompColor( entry, _Const000B ))
    {
      EwSetAttrColor((XAttrSet*)handle, inx, entry );
    }
  }

  while ( listC != 0 )
  {
    for ( lnx = 0; lnx < 20; lnx = lnx + 1 )
    {
      XColor entry = listC->entries[ EwCheckIndex( lnx, 20 )];

      if ( EwCompColor( entry, _Const000B ))
      {
        EwSetAttrColor((XAttrSet*)handle, inx + lnx, entry );
      }
    }

    inx = inx + 20;
    listC = listC->next;
  }

  for ( inx = 0; inx < 10; inx = inx + 1 )
  {
    ResourcesBitmap entry = _this->bitmaps[ EwCheckIndex( inx, 10 )];

    if ( entry != 0 )
    {
      XHandle bitmap;
      ResourcesBitmap__Update( entry );
      bitmap = entry->bitmap;
      EwSetAttrBitmap((XAttrSet*)handle, inx, (XBitmap*)bitmap );
    }
  }

  while ( listB != 0 )
  {
    for ( lnx = 0; lnx < 20; lnx = lnx + 1 )
    {
      ResourcesBitmap entry = listB->entries[ EwCheckIndex( lnx, 20 )];

      if ( entry != 0 )
      {
        XHandle bitmap;
        ResourcesBitmap__Update( entry );
        bitmap = entry->bitmap;
        EwSetAttrBitmap((XAttrSet*)handle, inx + lnx, (XBitmap*)bitmap );
      }
    }

    inx = inx + 20;
    listB = listB->next;
  }

  for ( inx = 0; inx < 4; inx = inx + 1 )
  {
    ResourcesFont entry = _this->fonts[ EwCheckIndex( inx, 4 )];

    if ( entry != 0 )
    {
      XHandle font = entry->font;
      EwSetAttrFont((XAttrSet*)handle, inx, (XFont*)font );
    }
  }

  while ( listF != 0 )
  {
    for ( lnx = 0; lnx < 20; lnx = lnx + 1 )
    {
      ResourcesFont entry = listF->entries[ EwCheckIndex( lnx, 20 )];

      if ( entry != 0 )
      {
        XHandle font = entry->font;
        EwSetAttrFont((XAttrSet*)handle, inx + lnx, (XFont*)font );
      }
    }

    inx = inx + 20;
    listF = listF->next;
  }
}

/* 'C' function for method : 'Graphics::AttrSet.observerSlot()' */
void GraphicsAttrSet_observerSlot( GraphicsAttrSet _this, XObject sender )
{
  /* Dummy expressions to avoid the 'C' warning 'unused argument'. */
  EW_UNUSED_ARG( sender );

  EwNotifyObjObservers((XObject)_this, 1 );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont3()' */
void GraphicsAttrSet_OnSetFont3( GraphicsAttrSet _this, ResourcesFont value )
{
  GraphicsAttrSet_SetFont( _this, 3, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont2()' */
void GraphicsAttrSet_OnSetFont2( GraphicsAttrSet _this, ResourcesFont value )
{
  GraphicsAttrSet_SetFont( _this, 2, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont1()' */
void GraphicsAttrSet_OnSetFont1( GraphicsAttrSet _this, ResourcesFont value )
{
  GraphicsAttrSet_SetFont( _this, 1, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetFont0()' */
void GraphicsAttrSet_OnSetFont0( GraphicsAttrSet _this, ResourcesFont value )
{
  GraphicsAttrSet_SetFont( _this, 0, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap4()' */
void GraphicsAttrSet_OnSetBitmap4( GraphicsAttrSet _this, ResourcesBitmap value )
{
  GraphicsAttrSet_SetBitmap( _this, 4, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap3()' */
void GraphicsAttrSet_OnSetBitmap3( GraphicsAttrSet _this, ResourcesBitmap value )
{
  GraphicsAttrSet_SetBitmap( _this, 3, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap2()' */
void GraphicsAttrSet_OnSetBitmap2( GraphicsAttrSet _this, ResourcesBitmap value )
{
  GraphicsAttrSet_SetBitmap( _this, 2, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap1()' */
void GraphicsAttrSet_OnSetBitmap1( GraphicsAttrSet _this, ResourcesBitmap value )
{
  GraphicsAttrSet_SetBitmap( _this, 1, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetBitmap0()' */
void GraphicsAttrSet_OnSetBitmap0( GraphicsAttrSet _this, ResourcesBitmap value )
{
  GraphicsAttrSet_SetBitmap( _this, 0, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetColor2()' */
void GraphicsAttrSet_OnSetColor2( GraphicsAttrSet _this, XColor value )
{
  GraphicsAttrSet_SetColor( _this, 2, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetColor1()' */
void GraphicsAttrSet_OnSetColor1( GraphicsAttrSet _this, XColor value )
{
  GraphicsAttrSet_SetColor( _this, 1, value );
}

/* 'C' function for method : 'Graphics::AttrSet.OnSetColor0()' */
void GraphicsAttrSet_OnSetColor0( GraphicsAttrSet _this, XColor value )
{
  GraphicsAttrSet_SetColor( _this, 0, value );
}

/* The method SetFont() assigns the given font object aFont to the entry aFontNo 
   within this set. The first entry has the number 0, the second 1, and so far. 
   Passing 'null' in aFont parameter causes the set to forget the font currently 
   stored in this entry. If there is no font entry with the number aFontNo in the 
   set, the operation is ignored. The number of available entries can be determined 
   by method @SetMaxNoOfFonts(). This modification can cause all associated Views::AttrText 
   views to update themselves. */
void GraphicsAttrSet_SetFont( GraphicsAttrSet _this, XInt32 aFontNo, ResourcesFont 
  aFont )
{
  XInt32 fontNo = aFontNo;
  XBool modified = 0;

  if ( fontNo < 0 )
    return;

  if ( fontNo < 4 )
  {
    modified = (XBool)( _this->fonts[ EwCheckIndex( fontNo, 4 )] != aFont );
    _this->fonts[ EwCheckIndex( fontNo, 4 )] = aFont;
  }
  else
  {
    GraphicsAttrSetFonts list = _this->attrSetFonts;
    fontNo = fontNo - 4;

    while (( list != 0 ) && ( fontNo >= 20 ))
    {
      list = list->next;
      fontNo = fontNo - 20;
    }

    if ( list != 0 )
    {
      modified = (XBool)( list->entries[ EwCheckIndex( fontNo, 20 )] != aFont );
      list->entries[ EwCheckIndex( fontNo, 20 )] = aFont;
    }
  }

  if ( modified && ( _this->attrSet != 0 ))
  {
    XHandle handle = _this->attrSet;
    XHandle font = 0;

    if ( aFont != 0 )
      font = aFont->font;

    EwSetAttrFont((XAttrSet*)handle, aFontNo, (XFont*)font );
  }

  if ( modified )
    EwNotifyObjObservers((XObject)_this, 1 );
}

/* The method GetMaxNoOfFonts() returns the maximum font array capacity of this 
   set. To change the capacity, the method @SetMaxNoOfFonts() should be used. */
XInt32 GraphicsAttrSet_GetMaxNoOfFonts( GraphicsAttrSet _this )
{
  XInt32 count = 4;
  GraphicsAttrSetFonts list = _this->attrSetFonts;

  while ( list != 0 )
  {
    count = count + 20;
    list = list->next;
  }

  return count;
}

/* The method SetBitmap() assigns the given bitmap object aBitmap to the entry aBitmapNo 
   within this set. The first entry has the number 0, the second 1, and so far. 
   Passing 'null' in aBitmap parameter causes the set to forget the bitmap currently 
   stored in this entry. If there is no bitmap entry with the number aBitmapNo in 
   the set, the operation is ignored. The number of available entries can be determined 
   by method @SetMaxNoOfBitmaps(). This modification can cause all associated Views::AttrText 
   views to update themselves. */
void GraphicsAttrSet_SetBitmap( GraphicsAttrSet _this, XInt32 aBitmapNo, ResourcesBitmap 
  aBitmap )
{
  XInt32 bitmapNo = aBitmapNo;
  XBool modified = 0;

  if ( bitmapNo < 0 )
    return;

  if ( bitmapNo < 10 )
  {
    modified = (XBool)( _this->bitmaps[ EwCheckIndex( bitmapNo, 10 )] != aBitmap 
    );

    if (( _this->bitmaps[ EwCheckIndex( bitmapNo, 10 )] != 0 ) && _this->bitmaps[ 
        EwCheckIndex( bitmapNo, 10 )]->Mutable )
      EwDetachObjObserver( EwNewSlot( _this, GraphicsAttrSet_observerSlot ), (XObject)_this->bitmaps[ 
        EwCheckIndex( bitmapNo, 10 )], 0 );

    _this->bitmaps[ EwCheckIndex( bitmapNo, 10 )] = aBitmap;

    if (( aBitmap != 0 ) && aBitmap->Mutable )
      EwAttachObjObserver( EwNewSlot( _this, GraphicsAttrSet_observerSlot ), (XObject)aBitmap, 
        0 );
  }
  else
  {
    GraphicsAttrSetBitmaps list = _this->attrSetBitmaps;
    bitmapNo = bitmapNo - 10;

    while (( list != 0 ) && ( bitmapNo >= 20 ))
    {
      list = list->next;
      bitmapNo = bitmapNo - 20;
    }

    if ( list != 0 )
    {
      modified = (XBool)( list->entries[ EwCheckIndex( bitmapNo, 20 )] != aBitmap 
      );

      if (( list->entries[ EwCheckIndex( bitmapNo, 20 )] != 0 ) && list->entries[ 
          EwCheckIndex( bitmapNo, 20 )]->Mutable )
        EwDetachObjObserver( EwNewSlot( _this, GraphicsAttrSet_observerSlot ), (XObject)list->entries[ 
          EwCheckIndex( bitmapNo, 20 )], 0 );

      list->entries[ EwCheckIndex( bitmapNo, 20 )] = aBitmap;

      if (( aBitmap != 0 ) && aBitmap->Mutable )
        EwAttachObjObserver( EwNewSlot( _this, GraphicsAttrSet_observerSlot ), (XObject)aBitmap, 
          0 );
    }
  }

  if ( modified && ( _this->attrSet != 0 ))
  {
    XHandle handle = _this->attrSet;
    XHandle bitmap = 0;

    if ( aBitmap != 0 )
    {
      ResourcesBitmap__Update( aBitmap );
      bitmap = aBitmap->bitmap;
    }

    EwSetAttrBitmap((XAttrSet*)handle, aBitmapNo, (XBitmap*)bitmap );
  }

  if ( modified )
    EwNotifyObjObservers((XObject)_this, 1 );
}

/* The method GetMaxNoOfBitmaps() returns the maximum bitmap array capacity of this 
   set. To change the capacity, the method @SetMaxNoOfBitmaps() should be used. */
XInt32 GraphicsAttrSet_GetMaxNoOfBitmaps( GraphicsAttrSet _this )
{
  XInt32 count = 10;
  GraphicsAttrSetBitmaps list = _this->attrSetBitmaps;

  while ( list != 0 )
  {
    count = count + 20;
    list = list->next;
  }

  return count;
}

/* The method SetColor() assigns the given color aColor to the entry aColorNo within 
   this set. The first entry has the number 0, the second 1, and so far. If there 
   is no color entry with the number aColorNo in the set, the operation is ignored. 
   The number of available entries can be determined by the method @SetMaxNoOfColors(). 
   This modification can cause all associated Views::AttrText views to update themselves. */
void GraphicsAttrSet_SetColor( GraphicsAttrSet _this, XInt32 aColorNo, XColor aColor )
{
  XInt32 colorNo = aColorNo;
  XBool modified = 0;

  if ( colorNo < 0 )
    return;

  if ( colorNo < 10 )
  {
    modified = (XBool)EwCompColor( _this->colors[ EwCheckIndex( colorNo, 10 )], 
    aColor );
    _this->colors[ EwCheckIndex( colorNo, 10 )] = aColor;
  }
  else
  {
    GraphicsAttrSetColors list = _this->attrSetColors;
    colorNo = colorNo - 10;

    while (( list != 0 ) && ( colorNo >= 20 ))
    {
      list = list->next;
      colorNo = colorNo - 20;
    }

    if ( list != 0 )
    {
      modified = (XBool)EwCompColor( list->entries[ EwCheckIndex( colorNo, 20 )], 
      aColor );
      list->entries[ EwCheckIndex( colorNo, 20 )] = aColor;
    }
  }

  if ( modified && ( _this->attrSet != 0 ))
  {
    XHandle handle = _this->attrSet;
    EwSetAttrColor((XAttrSet*)handle, aColorNo, aColor );
  }

  if ( modified )
    EwNotifyObjObservers((XObject)_this, 0 );
}

/* The method GetMaxNoOfColors() returns the maximum color array capacity of this 
   set. To change the capacity, the method @SetMaxNoOfColors() should be used. */
XInt32 GraphicsAttrSet_GetMaxNoOfColors( GraphicsAttrSet _this )
{
  XInt32 count = 10;
  GraphicsAttrSetColors list = _this->attrSetColors;

  while ( list != 0 )
  {
    count = count + 20;
    list = list->next;
  }

  return count;
}

/* Variants derived from the class : 'Graphics::AttrSet' */
EW_DEFINE_CLASS_VARIANTS( GraphicsAttrSet )
EW_END_OF_CLASS_VARIANTS( GraphicsAttrSet )

/* Virtual Method Table (VMT) for the class : 'Graphics::AttrSet' */
EW_DEFINE_CLASS( GraphicsAttrSet, XObject, "Graphics::AttrSet" )
EW_END_OF_CLASS( GraphicsAttrSet )

/* Initializer for the class 'Graphics::AttrSetFonts' */
void GraphicsAttrSetFonts__Init( GraphicsAttrSetFonts _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsAttrSetFonts );
}

/* Re-Initializer for the class 'Graphics::AttrSetFonts' */
void GraphicsAttrSetFonts__ReInit( GraphicsAttrSetFonts _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::AttrSetFonts' */
void GraphicsAttrSetFonts__Done( GraphicsAttrSetFonts _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsAttrSetFonts );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::AttrSetFonts' */
void GraphicsAttrSetFonts__Mark( GraphicsAttrSetFonts _this )
{
  EwMarkObject( _this->next );
  EwMarkObjects( _this->entries );

  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* Variants derived from the class : 'Graphics::AttrSetFonts' */
EW_DEFINE_CLASS_VARIANTS( GraphicsAttrSetFonts )
EW_END_OF_CLASS_VARIANTS( GraphicsAttrSetFonts )

/* Virtual Method Table (VMT) for the class : 'Graphics::AttrSetFonts' */
EW_DEFINE_CLASS( GraphicsAttrSetFonts, XObject, "Graphics::AttrSetFonts" )
EW_END_OF_CLASS( GraphicsAttrSetFonts )

/* Initializer for the class 'Graphics::AttrSetBitmaps' */
void GraphicsAttrSetBitmaps__Init( GraphicsAttrSetBitmaps _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsAttrSetBitmaps );
}

/* Re-Initializer for the class 'Graphics::AttrSetBitmaps' */
void GraphicsAttrSetBitmaps__ReInit( GraphicsAttrSetBitmaps _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::AttrSetBitmaps' */
void GraphicsAttrSetBitmaps__Done( GraphicsAttrSetBitmaps _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsAttrSetBitmaps );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::AttrSetBitmaps' */
void GraphicsAttrSetBitmaps__Mark( GraphicsAttrSetBitmaps _this )
{
  EwMarkObject( _this->next );
  EwMarkObjects( _this->entries );

  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* Variants derived from the class : 'Graphics::AttrSetBitmaps' */
EW_DEFINE_CLASS_VARIANTS( GraphicsAttrSetBitmaps )
EW_END_OF_CLASS_VARIANTS( GraphicsAttrSetBitmaps )

/* Virtual Method Table (VMT) for the class : 'Graphics::AttrSetBitmaps' */
EW_DEFINE_CLASS( GraphicsAttrSetBitmaps, XObject, "Graphics::AttrSetBitmaps" )
EW_END_OF_CLASS( GraphicsAttrSetBitmaps )

/* Initializer for the class 'Graphics::AttrSetColors' */
void GraphicsAttrSetColors__Init( GraphicsAttrSetColors _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsAttrSetColors );
}

/* Re-Initializer for the class 'Graphics::AttrSetColors' */
void GraphicsAttrSetColors__ReInit( GraphicsAttrSetColors _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::AttrSetColors' */
void GraphicsAttrSetColors__Done( GraphicsAttrSetColors _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsAttrSetColors );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::AttrSetColors' */
void GraphicsAttrSetColors__Mark( GraphicsAttrSetColors _this )
{
  EwMarkObject( _this->next );

  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* Variants derived from the class : 'Graphics::AttrSetColors' */
EW_DEFINE_CLASS_VARIANTS( GraphicsAttrSetColors )
EW_END_OF_CLASS_VARIANTS( GraphicsAttrSetColors )

/* Virtual Method Table (VMT) for the class : 'Graphics::AttrSetColors' */
EW_DEFINE_CLASS( GraphicsAttrSetColors, XObject, "Graphics::AttrSetColors" )
EW_END_OF_CLASS( GraphicsAttrSetColors )

/* Initializer for the class 'Graphics::Path' */
void GraphicsPath__Init( GraphicsPath _this, XObject aLink, XHandle aArg )
{
  /* At first initialize the super class ... */
  XObject__Init( &_this->_Super, aLink, aArg );

  /* Setup the VMT pointer */
  _this->_VMT = EW_CLASS( GraphicsPath );
}

/* Re-Initializer for the class 'Graphics::Path' */
void GraphicsPath__ReInit( GraphicsPath _this )
{
  /* At first re-initialize the super class ... */
  XObject__ReInit( &_this->_Super );
}

/* Finalizer method for the class 'Graphics::Path' */
void GraphicsPath__Done( GraphicsPath _this )
{
  /* Finalize this class */
  _this->_VMT = EW_CLASS( GraphicsPath );

  /* Call the user defined destructor of the class */
  GraphicsPath_Done( _this );

  /* Don't forget to deinitialize the super class ... */
  XObject__Done( &_this->_Super );
}

/* Garbage Collector method for the class 'Graphics::Path' */
void GraphicsPath__Mark( GraphicsPath _this )
{
  /* Give the super class a chance to mark its objects and references */
  XObject__Mark( &_this->_Super );
}

/* 'C' function for method : 'Graphics::Path.onUpdate()' */
void GraphicsPath_onUpdate( GraphicsPath _this, XObject sender )
{
  /* Dummy expressions to avoid the 'C' warning 'unused argument'. */
  EW_UNUSED_ARG( sender );

  EwNotifyObjObservers((XObject)_this, 0 );
}

/* 'C' function for method : 'Graphics::Path.Done()' */
void GraphicsPath_Done( GraphicsPath _this )
{
  XHandle handle;

  if ( _this->path == 0 )
    return;

  handle = _this->path;
  EwFreePath((XPath*)handle );
  _this->path = 0;
}

/* The method AddArc() adds to the sub-path a new elliptical arc composed of exact 
   aNoOfEdges straight line segments. The more edges the more smooth the resulting 
   curve. The center of the ellipse is determined in the parameters aCenterX, aCenterY. 
   The parameters aRadiusX and aRadiusY determine the horizontal and vertical radius 
   of the ellipse. The start and the end angle of the arc is determined in the parameters 
   aStartAngle and aEndAngle, both expressed in degree and measured clockwise relative 
   to the positive X-axis of the path coordinate system.
   If the absolute difference between aStartAngle and aEndAngle is equal to or greater 
   than 360.0, the method calculates with a closed ellipse instead of an opened 
   arc. If the parameters aRadiusX and aRadiusY are equal, the method calculates 
   with a circle instead of an ellipse.
   If the affected sub-path contains already curve information, the end of the last 
   existing segment and the start of the new arc (resulting from the parameter aStartAngle) 
   are connected with an additional straight line segment. Similarly, if start position 
   for the sub-path has been specified by using the @Begin() method, an additional 
   line segment is added to connect the arc with the given sub-path start position. 
   After the elliptical arc curve is added, the sub-path actual position is adjusted 
   to refer to the end of the last line segment of the curve (resulting from the 
   parameter aEndAngle).
   The specified position aX, aY is relative to the origin of the path coordinate 
   system. If the path own matrix is not an identity matrix, the corresponding transformations 
   are applied on the resulting arc segments before storing them in the sub-path. 
   See methods @Translate(), @Scale() and @Rotate().
   The method returns the index of the node within the sub-path where the start 
   position of the first arc line segment has been stored (parameter aStartAngle). 
   Knowing this index and the number of segments the arc curve is composed of (aNoOfEdges), 
   the position of every arc line segment can be modified later by using the method 
   @SetNode().
   If the sub-path has not been initialized previously by using the method @InitSubPath(), 
   the sub-path has been closed by @Close() method or the memory reserved for the 
   sub-path (see parameter aMaxNoOfEdges in the method @InitSubPath()) is occupied 
   by other edges so no sufficient space is available to accommodate aNoOfEdges 
   new segments, the method fails and returns 0.
   The affected sub-path is determined in the parameter aSubPathNo. Within the path 
   all existing sub-paths are numbered starting with the value 0 for the first sub-path, 
   1 for the second and so far. The maximum number of sub-paths the path can manage 
   is determined by the method @SetMaxNoOfSubPaths(). */
XInt32 GraphicsPath_AddArc( GraphicsPath _this, XInt32 aSubPathNo, XFloat aCenterX, 
  XFloat aCenterY, XFloat aRadiusX, XFloat aRadiusY, XFloat aStartAngle, XFloat 
  aEndAngle, XInt32 aNoOfEdges )
{
  XHandle handle;
  XInt32 result;

  EwPostSignal( EwNewSlot( _this, GraphicsPath_onUpdate ), ((XObject)_this ));
  handle = _this->path;
  result = 0;
  {
    result = EwAddSubPathArc((XPath*)handle, aSubPathNo, aCenterX, aCenterY,
                              aRadiusX, aRadiusY, aStartAngle, aEndAngle,
                              aNoOfEdges );
  }
  return result;
}

/* The method AddLine() appends between the actual position of the sub-path and 
   the position aX, aY a new straight line segment consisting of one edge. Then 
   the sub-path actual position is adjusted to refer to the end of the just added 
   line segment (aX, aY).
   Within a sub-path the actual position is either the end position of the preceding 
   sub-path segment or it is the value specified in the invocation of the @Begin() 
   method. If the sub-path is empty and no start position has been specified in 
   the preceding @Begin() invocation, the method assumes the origin of the path 
   coordinate system (X=0, Y=0) as the start position for the new line segment.
   The specified position aX, aY is relative to the origin of the path coordinate 
   system. If the path own matrix is not an identity matrix, the corresponding transformations 
   are applied on the position before storing it in the sub-path. See methods @Translate(), 
   @Scale() and @Rotate().
   The method returns the index of the node within the sub-path where the end position 
   of the new line segment has been stored. Knowing this index, the position can 
   be modified later by using the method @SetNode().
   If the sub-path has not been initialized previously by using the method @InitSubPath(), 
   the sub-path has been closed by @Close() method or the entire memory reserved 
   for the sub-path (see parameter aMaxNoOfEdges in the method @InitSubPath()) is 
   already occupied by other edges, the method fails and returns 0.
   The affected sub-path is determined in the parameter aSubPathNo. Within the path 
   all existing sub-paths are numbered starting with the value 0 for the first sub-path, 
   1 for the second and so far. The maximum number of sub-paths the path can manage 
   is determined by the method @SetMaxNoOfSubPaths(). */
XInt32 GraphicsPath_AddLine( GraphicsPath _this, XInt32 aSubPathNo, XFloat aX, XFloat 
  aY )
{
  XHandle handle;
  XInt32 result;

  EwPostSignal( EwNewSlot( _this, GraphicsPath_onUpdate ), ((XObject)_this ));
  handle = _this->path;
  result = 0;
  result = EwAddSubPathLine((XPath*)handle, aSubPathNo, aX, aY );
  return result;
}

/* The method Close() marks the affected sub-path as closed. The method verifies 
   whether the first and the last position of the sub-path are equal and if this 
   is not the case, adds an additional straight line segment to the sub-path in 
   order to connect them together.
   Once the method is invoked, no additional path information can be added to the 
   affected sub-path unless it is initialized or cleared again by using the method 
   @InitSubPath() or @Begin(). If the affected sub-path is empty, the method returns 
   without any effect.
   Within the path all existing sub-paths are numbered starting with the value 0 
   for the first sub-path, 1 for the second and so far. The maximum number of sub-paths 
   the path can manage is determined by the method @SetMaxNoOfSubPaths(). */
void GraphicsPath_Close( GraphicsPath _this, XInt32 aSubPathNo )
{
  XHandle handle;

  EwPostSignal( EwNewSlot( _this, GraphicsPath_onUpdate ), ((XObject)_this ));
  handle = _this->path;
  EwCloseSubPath((XPath*)handle, aSubPathNo );
}

/* The method Begin() sets the parameters aX, aY as the start position for the sub-path 
   with the specified number aSubPathNo. The affected sub-path has to be initialized 
   previously by invoking the method @InitSubPath(). Beginning with the specified 
   position, the path can be filled with curve data by using methods like @AddLine(), 
   @AddBezier2(), @AddArc(), etc.
   The specified position aX, aY is relative to the origin of the path coordinate 
   system. If the path own matrix is not an identity matrix, the corresponding transformations 
   are applied on the position before storing it in the sub-path. See methods @Translate(), 
   @Scale() and @Rotate().
   Every sub-path contains exact one start position. Calling this method for a sub-path 
   being already filled with edge coordinates will clear the actual sub-path as 
   if the @InitSubPath() method has been called before.
   Within the path all existing sub-paths are numbered starting with the value 0 
   for the first sub-path, 1 for the second and so far. The maximum number of sub-paths 
   the path can manage is determined by the method @SetMaxNoOfSubPaths(). */
void GraphicsPath_Begin( GraphicsPath _this, XInt32 aSubPathNo, XFloat aX, XFloat 
  aY )
{
  XHandle handle;

  EwPostSignal( EwNewSlot( _this, GraphicsPath_onUpdate ), ((XObject)_this ));
  handle = _this->path;
  EwBeginSubPath((XPath*)handle, aSubPathNo, aX, aY );
}

/* The method InitSubPath() prepares the sub-path with the number aSubPathNo to 
   be able to store up to aMaxNoOfEdges path edges (straight line segments). With 
   this operation memory for the sub-path data is reserved. Initially the just initialized 
   sub-path is considered as being empty. To fill the sub-path with data use the 
   methods like @AddLine(), @AddBezier2(), @AddArc(), etc.
   If the affected sub-path has been already initialized in the past, the old information 
   is discarded before initializing the new sub-path. Passing 0 (zero) in the parameter 
   aMaxNoOfEdges results in the old sub-path data being simply released without 
   allocating the memory for the new sub-path.
   If successful, the method returns 'true'. If there is no memory available for 
   the specified number of edges, the method fails and 'false' is returned leaving 
   the sub-path not initialized. Similarly, trying to initialize a sub-path not 
   existing in the path causes the method to return 'false'.
   Within the path all existing sub-paths are numbered starting with the value 0 
   for the first sub-path, 1 for the second and so far. The maximum number of sub-paths 
   the path can manage is determined by the method @SetMaxNoOfSubPaths(). */
XBool GraphicsPath_InitSubPath( GraphicsPath _this, XInt32 aSubPathNo, XInt32 aMaxNoOfEdges )
{
  XHandle handle;
  XBool result;

  if (( _this->path == 0 ) && ( aSubPathNo == 0 ))
    GraphicsPath_SetMaxNoOfSubPaths( _this, 1 );

  EwPostSignal( EwNewSlot( _this, GraphicsPath_onUpdate ), ((XObject)_this ));
  handle = _this->path;
  result = 0;
  result = EwInitSubPath((XPath*)handle, aSubPathNo, aMaxNoOfEdges );
  return result;
}

/* The method Translate() applies the given displacement aDeltaX, aDeltaY to the 
   path own matrix. This corresponds to the translation of the origin of the path 
   coordinate system in the X- and Y-direction. The modification of the path matrix 
   affects all coordinates specified in subsequent invocations of methods like @Begin(), 
   @AddLine(), @AddBezier2(), etc. */
void GraphicsPath_Translate( GraphicsPath _this, XFloat aDeltaX, XFloat aDeltaY )
{
  XHandle handle;

  if ( _this->path == 0 )
    GraphicsPath_SetMaxNoOfSubPaths( _this, 1 );

  handle = _this->path;
  EwTranslatePathMatrix((XPath*)handle, aDeltaX, aDeltaY );
}

/* The method GetMaxNoOfSubPaths() returns how many sub-paths the affected path 
   can maximally store. The returned value corresponds to the parameter passed in 
   the invocation of the @SetMaxNoOfSubPaths() method. */
XInt32 GraphicsPath_GetMaxNoOfSubPaths( GraphicsPath _this )
{
  XHandle handle;
  XInt32 result;

  if ( _this->path == 0 )
    return 1;

  handle = _this->path;
  result = 0;
  result = EwGetMaxNoOfSubPaths((XPath*)handle );
  return result;
}

/* The method SetMaxNoOfSubPaths() changes the number of sub-paths the affected 
   path can maximally manage to the specified value aMaxNoOfSubPaths. After modifying 
   its size the affected path is empty. This means all contents of previously existing 
   sub-paths are discarded. Before new path data can be stored in a sub-path the 
   method @InitSubPath() should be called. Please note, if the size of the path 
   does already correspond to the parameter aMaxNoOfSubPaths, the path remains unaffected 
   and retains all existing contents.
   The method returns 'true' if the path could be initialized with the new size. 
   In case, there is no sufficient memory to store the desired number of sub-paths, 
   the method fails and returns 'false' leaving the path empty. */
XBool GraphicsPath_SetMaxNoOfSubPaths( GraphicsPath _this, XInt32 aMaxNoOfSubPaths )
{
  XHandle handle;

  if ( aMaxNoOfSubPaths < 1 )
    aMaxNoOfSubPaths = 1;

  if (( _this->path != 0 ) && ( GraphicsPath_GetMaxNoOfSubPaths( _this ) == aMaxNoOfSubPaths 
      ))
    return 1;

  handle = _this->path;
  {
    EwFreePath((XPath*)handle );
    handle = (XHandle)EwCreatePath( aMaxNoOfSubPaths );
  }
  _this->path = handle;
  EwPostSignal( EwNewSlot( _this, GraphicsPath_onUpdate ), ((XObject)_this ));
  return (XBool)( _this->path != 0 );
}

/* Variants derived from the class : 'Graphics::Path' */
EW_DEFINE_CLASS_VARIANTS( GraphicsPath )
EW_END_OF_CLASS_VARIANTS( GraphicsPath )

/* Virtual Method Table (VMT) for the class : 'Graphics::Path' */
EW_DEFINE_CLASS( GraphicsPath, XObject, "Graphics::Path" )
EW_END_OF_CLASS( GraphicsPath )

/* Embedded Wizard */
