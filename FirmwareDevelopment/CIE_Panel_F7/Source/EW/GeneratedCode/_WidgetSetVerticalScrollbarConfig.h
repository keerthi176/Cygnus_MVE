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

#ifndef _WidgetSetVerticalScrollbarConfig_H
#define _WidgetSetVerticalScrollbarConfig_H

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

/* Forward declaration of the class WidgetSet::VerticalScrollbarConfig */
#ifndef _WidgetSetVerticalScrollbarConfig_
  EW_DECLARE_CLASS( WidgetSetVerticalScrollbarConfig )
#define _WidgetSetVerticalScrollbarConfig_
#endif


/* This class implements the functionality permitting you to simply customize the 
   look and feel of a 'vertical scrollbar' widget (WidgetSet::VerticalScrollbar). 
   In the practice, you will create an instance of this class, configure its properties 
   with bitmaps, colors and other relevant attributes according to your design expectations 
   and assign such prepared configuration object to the property 'Appearance' of 
   every affected vertical scrollbar widget. Thereupon the widgets will use the 
   configuration information provided in the object.
   During its lifetime the scrollbar remains always in one of the three states: 
   'disabled', 'default' and 'active'. The state 'disabled' is true for every not 
   available scrollbar (the property 'Enabled' of the scrollbar is 'false'). Such 
   scrollbars will ignore all user inputs. The state 'default' determines a scrollbar, 
   which is ready to be touched by the user. Finally, the state 'active' is true, 
   if the user actually interacts with the scrollbar (the scrollbar's thumb is pressed). 
   With the configuration object you can specify the appearance of the scrollbar 
   for every state individually. For this purpose you should know from which views 
   the scrollbar is composed of:
   - 'Track' is a bitmap frame view (Views::Frame) filling horizontally centered 
   the entire height of the scrollbar. In the configuration object you can individually 
   specify for every scrollbar state the desired bitmap (@TrackActive, @TrackDefault, 
   @TrackDisabled), the frame number within the bitmap (@TrackFrameActive, @TrackFrameDefault, 
   @TrackFrameDisabled) and opacity or color to tint the bitmap (@TrackTintActive, 
   @TrackTintDefault, @TrackTintDisabled). The scrollbar can automatically play 
   animated bitmaps if the specified frame number is -1.
   - 'Thumb' is a bitmap frame view (Views::Frame) centered horizontally at the 
   current scroll position. The height of the thumb corresponds to the ratio between 
   'view area' and 'content area' as specified in the scrollbar widget. In the configuration 
   object you can individually specify for every scrollbar state the desired bitmap 
   (@ThumbActive, @ThumbDefault, @ThumbDisabled), the frame number within the bitmap 
   (@ThumbFrameActive, @ThumbFrameDefault, @ThumbFrameDisabled) and opacity or color 
   to tint the bitmap (@ThumbTintActive, @ThumbTintDefault, @ThumbTintDisabled). 
   If necessary, additional margins below and above the thumb can be specified by 
   using the properties @ThumbMarginBelow and @ThumbMarginAbove. With the property 
   @ThumbSizeFixed you can suppress the thumb from being adjustable in its height. 
   The scrollbar can automatically play animated bitmaps if the specified frame 
   number is -1.
   With the properties @AutoHideDelay and @HideIfNotNeeded you can configure the 
   behavior of the scrollbar, whether and when it should disappear automatically. 
   If the scrollbar is configured to disappear and appear automatically, you can 
   configure an opacity fade-in/out effect to be used for this operation by using 
   the properties @FadeInDuration and @FadeOutDuration.
   Whether the scrollbar should be able to react to user touch interactions or not 
   can be configured in the property @Touchable. If this property is 'true', the 
   user can touch and drag the scrollbar thumb. If this property is 'false', the 
   scrollbar serves as a pure passive widget.
   With the properties @WidgetMinSize and @WidgetMaxSize you can configure size 
   constraints for the widget itself. You can, for example, limit the scrollbar 
   to not shrink below a specified width or height. */
EW_DEFINE_FIELDS( WidgetSetVerticalScrollbarConfig, WidgetSetWidgetConfig )
  EW_PROPERTY( FadeOutDuration, XInt32 )
  EW_PROPERTY( FadeInDuration,  XInt32 )
  EW_PROPERTY( AutoHideDelay,   XInt32 )
  EW_PROPERTY( SnapDuration,    XInt32 )
  EW_PROPERTY( ThumbMarginBelow, XInt32 )
  EW_PROPERTY( ThumbMarginAbove, XInt32 )
  EW_PROPERTY( ThumbTintActive, XColor )
  EW_PROPERTY( ThumbTintDisabled, XColor )
  EW_PROPERTY( ThumbTintDefault, XColor )
  EW_PROPERTY( ThumbFrameActive, XInt32 )
  EW_PROPERTY( ThumbFrameDisabled, XInt32 )
  EW_PROPERTY( ThumbFrameDefault, XInt32 )
  EW_PROPERTY( ThumbActive,     ResourcesBitmap )
  EW_PROPERTY( ThumbDisabled,   ResourcesBitmap )
  EW_PROPERTY( ThumbDefault,    ResourcesBitmap )
  EW_PROPERTY( TrackTintActive, XColor )
  EW_PROPERTY( TrackTintDisabled, XColor )
  EW_PROPERTY( TrackTintDefault, XColor )
  EW_PROPERTY( TrackFrameActive, XInt32 )
  EW_PROPERTY( TrackFrameDisabled, XInt32 )
  EW_PROPERTY( TrackFrameDefault, XInt32 )
  EW_PROPERTY( TrackActive,     ResourcesBitmap )
  EW_PROPERTY( TrackDisabled,   ResourcesBitmap )
  EW_PROPERTY( TrackDefault,    ResourcesBitmap )
  EW_PROPERTY( WidgetMaxSize,   XPoint )
  EW_PROPERTY( WidgetMinSize,   XPoint )
  EW_PROPERTY( ThumbSizeFixed,  XBool )
  EW_PROPERTY( HideIfNotNeeded, XBool )
  EW_PROPERTY( Touchable,       XBool )
EW_END_OF_FIELDS( WidgetSetVerticalScrollbarConfig )

/* Virtual Method Table (VMT) for the class : 'WidgetSet::VerticalScrollbarConfig' */
EW_DEFINE_METHODS( WidgetSetVerticalScrollbarConfig, WidgetSetWidgetConfig )
EW_END_OF_METHODS( WidgetSetVerticalScrollbarConfig )

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetSnapDuration()' */
void WidgetSetVerticalScrollbarConfig_OnSetSnapDuration( WidgetSetVerticalScrollbarConfig _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetThumbFrameActive()' */
void WidgetSetVerticalScrollbarConfig_OnSetThumbFrameActive( WidgetSetVerticalScrollbarConfig _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetThumbFrameDisabled()' */
void WidgetSetVerticalScrollbarConfig_OnSetThumbFrameDisabled( WidgetSetVerticalScrollbarConfig _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetThumbFrameDefault()' */
void WidgetSetVerticalScrollbarConfig_OnSetThumbFrameDefault( WidgetSetVerticalScrollbarConfig _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetThumbActive()' */
void WidgetSetVerticalScrollbarConfig_OnSetThumbActive( WidgetSetVerticalScrollbarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetThumbDisabled()' */
void WidgetSetVerticalScrollbarConfig_OnSetThumbDisabled( WidgetSetVerticalScrollbarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetThumbDefault()' */
void WidgetSetVerticalScrollbarConfig_OnSetThumbDefault( WidgetSetVerticalScrollbarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetTrackActive()' */
void WidgetSetVerticalScrollbarConfig_OnSetTrackActive( WidgetSetVerticalScrollbarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetTrackDisabled()' */
void WidgetSetVerticalScrollbarConfig_OnSetTrackDisabled( WidgetSetVerticalScrollbarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetTrackDefault()' */
void WidgetSetVerticalScrollbarConfig_OnSetTrackDefault( WidgetSetVerticalScrollbarConfig _this, 
  ResourcesBitmap value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbarConfig.OnSetWidgetMinSize()' */
void WidgetSetVerticalScrollbarConfig_OnSetWidgetMinSize( WidgetSetVerticalScrollbarConfig _this, 
  XPoint value );

#ifdef __cplusplus
  }
#endif

#endif /* _WidgetSetVerticalScrollbarConfig_H */

/* Embedded Wizard */
