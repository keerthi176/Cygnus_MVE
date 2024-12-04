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

#ifndef _WidgetSetHorizontalValueBarConfig_H
#define _WidgetSetHorizontalValueBarConfig_H

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

#include "_WidgetSetWidgetConfig.h"

/* Forward declaration of the class Resources::Bitmap */
#ifndef _ResourcesBitmap_
  EW_DECLARE_CLASS( ResourcesBitmap )
#define _ResourcesBitmap_
#endif

/* Forward declaration of the class WidgetSet::HorizontalValueBarConfig */
#ifndef _WidgetSetHorizontalValueBarConfig_
  EW_DECLARE_CLASS( WidgetSetHorizontalValueBarConfig )
#define _WidgetSetHorizontalValueBarConfig_
#endif


/* This class implements the functionality permitting you to simply customize the 
   look and feel of a 'horizontal value bar' widget (WidgetSet::HorizontalValueBar). 
   In the practice, you will create an instance of this class, configure its properties 
   with bitmaps, colors and other relevant attributes according to your design expectations 
   and assign such prepared configuration object to the property 'Appearance' of 
   every affected horizontal value bar widget. Thereupon the widgets will use the 
   configuration information provided in the object.
   Since with the configuration object you determine the appearance of the value 
   bar widget you should know from which views the widget is composed of:
   - 'Face' is a bitmap frame view (Views::Frame) filling vertically centered the 
   entire width in the background of the widget. In the configuration object you 
   can specify the desired bitmap (@Face), the frame number within the bitmap (@FaceFrame) 
   and opacity or color to tint the bitmap (@FaceTint). The value bar widget can 
   automatically play animated bitmaps if the specified frame number is -1.
   - 'TrackLeft' is a bitmap frame view (Views::Frame) filling vertically centered 
   the background of the widget between its left edge and the actual position of 
   the needle. In the configuration object you can specify the desired bitmap (@TrackLeft), 
   the frame number within the bitmap (@TrackLeftFrame) and opacity or color to 
   tint the bitmap (@TrackLeftTint). With the property @TrackLeftWithEdge you can 
   configure how the right edge of the track (the edge at the needle position) should 
   appear. The value bar can automatically play animated bitmaps if the specified 
   frame number is -1.
   - 'TrackRight' is a bitmap frame view (Views::Frame) filling vertically centered 
   the background of the widget between the actual position of the needle and the 
   right edge of the widget. In the configuration object you can specify the desired 
   bitmap (@TrackRight), the frame number within the bitmap (@TrackRightFrame) and 
   opacity or color to tint the bitmap (@TrackRightTint). With the property @TrackRightWithEdge 
   you can configure how the left edge of the track (the edge at the needle position) 
   should appear. The value bar can automatically play animated bitmaps if the specified 
   frame number is -1.
   - 'Needle' is an image view (Views::Image) displayed centered at the actual needle 
   position. In the configuration object you can specify the desired bitmap (@Needle), 
   the frame number within the bitmap (@NeedleFrame) and opacity or color to tint 
   the bitmap (@NeedleTint). If necessary, additional margins on the left and on 
   the right of the needle can be specified by using the properties @NeedleMarginLeft 
   and @NeedleMarginRight. The value bar can automatically play animated bitmaps 
   if the specified frame number is -1.
   - 'Cover' is a bitmap frame view (Views::Frame) filling vertically centered the 
   entire width of the widget and covering so eventually the needle and track. In 
   the configuration object you can specify the desired bitmap (@Cover), the frame 
   number within the bitmap (@CoverFrame) and opacity or color to tint the bitmap 
   (@CoverTint). The value bar widget can automatically play animated bitmaps if 
   the specified frame number is -1.
   The value bar widget can move the needle with a smooth animation. This can be 
   configured in the properties @SwingDuration and @SwingElastic.
   With the properties @WidgetMinSize and @WidgetMaxSize you can configure size 
   constraints for the widget itself. You can, for example, limit the value bar 
   widget to not shrink below a specified width or height. */
EW_DEFINE_FIELDS( WidgetSetHorizontalValueBarConfig, WidgetSetWidgetConfig )
  EW_PROPERTY( SwingDuration,   XInt32 )
  EW_PROPERTY( NeedleMarginRight, XInt32 )
  EW_PROPERTY( NeedleMarginLeft, XInt32 )
  EW_PROPERTY( CoverTint,       XColor )
  EW_PROPERTY( CoverFrame,      XInt32 )
  EW_PROPERTY( Cover,           ResourcesBitmap )
  EW_PROPERTY( NeedleTint,      XColor )
  EW_PROPERTY( NeedleFrame,     XInt32 )
  EW_PROPERTY( Needle,          ResourcesBitmap )
  EW_PROPERTY( TrackRightTint,  XColor )
  EW_PROPERTY( TrackRightFrame, XInt32 )
  EW_PROPERTY( TrackRight,      ResourcesBitmap )
  EW_PROPERTY( TrackLeftTint,   XColor )
  EW_PROPERTY( TrackLeftFrame,  XInt32 )
  EW_PROPERTY( TrackLeft,       ResourcesBitmap )
  EW_PROPERTY( FaceTint,        XColor )
  EW_PROPERTY( FaceFrame,       XInt32 )
  EW_PROPERTY( Face,            ResourcesBitmap )
  EW_PROPERTY( WidgetMaxSize,   XPoint )
  EW_PROPERTY( WidgetMinSize,   XPoint )
  EW_PROPERTY( TrackLeftWithEdge, XBool )
  EW_PROPERTY( TrackRightWithEdge, XBool )
  EW_PROPERTY( SwingElastic,    XBool )
EW_END_OF_FIELDS( WidgetSetHorizontalValueBarConfig )

/* Virtual Method Table (VMT) for the class : 'WidgetSet::HorizontalValueBarConfig' */
EW_DEFINE_METHODS( WidgetSetHorizontalValueBarConfig, WidgetSetWidgetConfig )
EW_END_OF_METHODS( WidgetSetHorizontalValueBarConfig )

/* 'C' function for method : 'WidgetSet::HorizontalValueBarConfig.OnSetTrackRightFrame()' */
void WidgetSetHorizontalValueBarConfig_OnSetTrackRightFrame( WidgetSetHorizontalValueBarConfig _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::HorizontalValueBarConfig.OnSetTrackRight()' */
void WidgetSetHorizontalValueBarConfig_OnSetTrackRight( WidgetSetHorizontalValueBarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::HorizontalValueBarConfig.OnSetTrackLeftFrame()' */
void WidgetSetHorizontalValueBarConfig_OnSetTrackLeftFrame( WidgetSetHorizontalValueBarConfig _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::HorizontalValueBarConfig.OnSetTrackLeft()' */
void WidgetSetHorizontalValueBarConfig_OnSetTrackLeft( WidgetSetHorizontalValueBarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::HorizontalValueBarConfig.OnSetWidgetMinSize()' */
void WidgetSetHorizontalValueBarConfig_OnSetWidgetMinSize( WidgetSetHorizontalValueBarConfig _this, 
  XPoint value );

#ifdef __cplusplus
  }
#endif

#endif /* _WidgetSetHorizontalValueBarConfig_H */

/* Embedded Wizard */
