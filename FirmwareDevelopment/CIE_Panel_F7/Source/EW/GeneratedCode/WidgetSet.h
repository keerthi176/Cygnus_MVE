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

#ifndef WidgetSet_H
#define WidgetSet_H

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

#include "_WidgetSetHorizontalSlider.h"
#include "_WidgetSetHorizontalSliderConfig.h"
#include "_WidgetSetHorizontalValueBar.h"
#include "_WidgetSetHorizontalValueBarConfig.h"
#include "_WidgetSetPushButton.h"
#include "_WidgetSetPushButtonConfig.h"
#include "_WidgetSetRadioButton.h"
#include "_WidgetSetRadioButtonConfig.h"
#include "_WidgetSetToggleButton.h"
#include "_WidgetSetToggleButtonConfig.h"
#include "_WidgetSetVerticalScrollbar.h"
#include "_WidgetSetVerticalScrollbarConfig.h"
#include "_WidgetSetWidgetConfig.h"

/* Bitmap resource : 'WidgetSet::HorizontalValueBarSmall' */
EW_DECLARE_BITMAP_RES( WidgetSetHorizontalValueBarSmall )

/* Bitmap resource : 'WidgetSet::PushButtonSmall' */
EW_DECLARE_BITMAP_RES( WidgetSetPushButtonSmall )

/* Bitmap resource : 'WidgetSet::SwitchSmall' */
EW_DECLARE_BITMAP_RES( WidgetSetSwitchSmall )

/* Bitmap resource : 'WidgetSet::CheckBoxMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetCheckBoxMedium )

/* Bitmap resource : 'WidgetSet::HorizontalSliderTrackMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetHorizontalSliderTrackMedium )

/* Bitmap resource : 'WidgetSet::HorizontalValueBarMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetHorizontalValueBarMedium )

/* Bitmap resource : 'WidgetSet::PushButtonMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetPushButtonMedium )

/* Bitmap resource : 'WidgetSet::RadioButtonMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetRadioButtonMedium )

/* Bitmap resource : 'WidgetSet::SwitchMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetSwitchMedium )

/* Bitmap resource : 'WidgetSet::ThumbKnobMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetThumbKnobMedium )

/* Bitmap resource : 'WidgetSet::VerticalScrollbarThumbMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetVerticalScrollbarThumbMedium )

/* Bitmap resource : 'WidgetSet::VerticalScrollbarTrackMedium' */
EW_DECLARE_BITMAP_RES( WidgetSetVerticalScrollbarTrackMedium )

/* This autoobject provides the default customization for the 'horizontal value 
   bar' widget (WidgetSet::HorizontalValueBar) in its small size variant. */
EW_DECLARE_AUTOOBJECT( WidgetSetHorizontalValueBar_Small, WidgetSetHorizontalValueBarConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its small size variant. With this customization 
   the toggle button appears as a 'switch'. */
EW_DECLARE_AUTOOBJECT( WidgetSetSwitch_Small, WidgetSetToggleButtonConfig )

/* This autoobject provides the default customization for the 'push button' widget 
   (WidgetSet::PushButton) in its small size variant. */
EW_DECLARE_AUTOOBJECT( WidgetSetPushButton_Small, WidgetSetPushButtonConfig )

/* This autoobject provides the default customization for the 'horizontal value 
   bar' widget (WidgetSet::HorizontalValueBar) in its medium size variant. */
EW_DECLARE_AUTOOBJECT( WidgetSetHorizontalValueBar_Medium, WidgetSetHorizontalValueBarConfig )

/* This autoobject provides the default customization for the 'vertical scrollbar' 
   widget (WidgetSet::VerticalScrollbar) in its medium size variant. Scrollbars 
   using this configuration do react to user touch interactions. */
EW_DECLARE_AUTOOBJECT( WidgetSetVerticalScrollbar_Medium, WidgetSetVerticalScrollbarConfig )

/* This autoobject provides the default customization for the 'horizontal slider' 
   widget (WidgetSet::HorizontalSlider) in its medium size variant. */
EW_DECLARE_AUTOOBJECT( WidgetSetHorizontalSlider_Medium, WidgetSetHorizontalSliderConfig )

/* This autoobject provides one of the default customizations for the 'toggle' widget 
   (WidgetSet::ToggleButton) in its medium size variant. With this customization 
   the toggle button appears as a 'switch'. */
EW_DECLARE_AUTOOBJECT( WidgetSetSwitch_Medium, WidgetSetToggleButtonConfig )

#ifdef __cplusplus
  }
#endif

#endif /* WidgetSet_H */

/* Embedded Wizard */
