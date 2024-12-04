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

#ifndef _ApplicationDatePicker_H
#define _ApplicationDatePicker_H

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

#include "_CoreSlideTouchHandler.h"
#include "_CoreVerticalList.h"
#include "_TemplatesPickerControl.h"
#include "_ViewsBevel.h"
#include "_ViewsBorder.h"
#include "_ViewsRectangle.h"

/* Forward declaration of the class Application::DatePicker */
#ifndef _ApplicationDatePicker_
  EW_DECLARE_CLASS( ApplicationDatePicker )
#define _ApplicationDatePicker_
#endif

/* Forward declaration of the class Core::DialogContext */
#ifndef _CoreDialogContext_
  EW_DECLARE_CLASS( CoreDialogContext )
#define _CoreDialogContext_
#endif

/* Forward declaration of the class Core::Group */
#ifndef _CoreGroup_
  EW_DECLARE_CLASS( CoreGroup )
#define _CoreGroup_
#endif

/* Forward declaration of the class Core::KeyPressHandler */
#ifndef _CoreKeyPressHandler_
  EW_DECLARE_CLASS( CoreKeyPressHandler )
#define _CoreKeyPressHandler_
#endif

/* Forward declaration of the class Core::LayoutContext */
#ifndef _CoreLayoutContext_
  EW_DECLARE_CLASS( CoreLayoutContext )
#define _CoreLayoutContext_
#endif

/* Forward declaration of the class Core::TaskQueue */
#ifndef _CoreTaskQueue_
  EW_DECLARE_CLASS( CoreTaskQueue )
#define _CoreTaskQueue_
#endif

/* Forward declaration of the class Core::View */
#ifndef _CoreView_
  EW_DECLARE_CLASS( CoreView )
#define _CoreView_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* This is a GUI component. */
EW_DEFINE_FIELDS( ApplicationDatePicker, TemplatesPickerControl )
  EW_OBJECT  ( Background,      ViewsRectangle )
  EW_OBJECT  ( BorderCurrent,   ViewsBorder )
  EW_OBJECT  ( TouchHandlerYear, CoreSlideTouchHandler )
  EW_OBJECT  ( TouchHandlerMonth, CoreSlideTouchHandler )
  EW_OBJECT  ( TouchHandlerDay, CoreSlideTouchHandler )
  EW_OBJECT  ( ListYear,        CoreVerticalList )
  EW_OBJECT  ( ListMonth,       CoreVerticalList )
  EW_OBJECT  ( ListDay,         CoreVerticalList )
  EW_OBJECT  ( Bevel,           ViewsBevel )
  EW_OBJECT  ( Bevel1,          ViewsBevel )
  EW_OBJECT  ( Bevel2,          ViewsBevel )
  EW_OBJECT  ( Bevel3,          ViewsBevel )
  EW_OBJECT  ( ShineAtBottom,   ViewsRectangle )
  EW_OBJECT  ( ShineAtTop,      ViewsRectangle )
  EW_PROPERTY( OnChange,        XSlot )
  EW_OBJECT  ( Border,          ViewsBorder )
  EW_VARIABLE( enabled,         XBool )
  EW_VARIABLE( selected,        XBool )
  EW_VARIABLE( sliding,         XBool )
EW_END_OF_FIELDS( ApplicationDatePicker )

/* Virtual Method Table (VMT) for the class : 'Application::DatePicker' */
EW_DEFINE_METHODS( ApplicationDatePicker, TemplatesPickerControl )
  EW_METHOD( initLayoutContext, void )( CoreRectView _this, XRect aBounds, CoreOutline 
    aOutline )
  EW_METHOD( GetRoot,           CoreRoot )( CoreView _this )
  EW_METHOD( Draw,              void )( CoreGroup _this, GraphicsCanvas aCanvas, 
    XRect aClip, XPoint aOffset, XInt32 aOpacity, XBool aBlend )
  EW_METHOD( HandleEvent,       XObject )( CoreView _this, CoreEvent aEvent )
  EW_METHOD( CursorHitTest,     CoreCursorHit )( CoreGroup _this, XRect aArea, XInt32 
    aFinger, XInt32 aStrikeCount, CoreView aDedicatedView, XSet aRetargetReason )
  EW_METHOD( ArrangeView,       XPoint )( CoreRectView _this, XRect aBounds, XEnum 
    aFormation )
  EW_METHOD( MoveView,          void )( CoreRectView _this, XPoint aOffset, XBool 
    aFastMove )
  EW_METHOD( GetExtent,         XRect )( CoreRectView _this )
  EW_METHOD( ChangeViewState,   void )( CoreGroup _this, XSet aSetState, XSet aClearState )
  EW_METHOD( OnSetBounds,       void )( CoreGroup _this, XRect value )
  EW_METHOD( OnSetFocus,        void )( CoreGroup _this, CoreView value )
  EW_METHOD( OnSetBuffered,     void )( CoreGroup _this, XBool value )
  EW_METHOD( OnSetOpacity,      void )( CoreGroup _this, XInt32 value )
  EW_METHOD( DispatchEvent,     XObject )( CoreGroup _this, CoreEvent aEvent )
  EW_METHOD( BroadcastEvent,    XObject )( CoreGroup _this, CoreEvent aEvent, XSet 
    aFilter )
  EW_METHOD( UpdateLayout,      void )( CoreGroup _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( ApplicationDatePicker _this, XSet aState )
  EW_METHOD( InvalidateArea,    void )( CoreGroup _this, XRect aArea )
  EW_METHOD( GetIndexOfView,    XInt32 )( CoreGroup _this, CoreView aView )
  EW_METHOD( FindSiblingView,   CoreView )( CoreGroup _this, CoreView aView, XSet 
    aFilter )
  EW_METHOD( RestackBehind,     void )( CoreGroup _this, CoreView aView, CoreView 
    aSibling )
  EW_METHOD( RestackTop,        void )( CoreGroup _this, CoreView aView )
  EW_METHOD( Restack,           void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )
  EW_METHOD( Remove,            void )( CoreGroup _this, CoreView aView )
  EW_METHOD( Add,               void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )
EW_END_OF_METHODS( ApplicationDatePicker )

/* The method UpdateViewState() is invoked automatically after the state of the 
   component has been changed. This method can be overridden and filled with logic 
   to ensure the visual aspect of the component does reflect its current state. 
   For example, the 'enabled' state of the component can affect its colors (disabled 
   components may appear pale). In this case the logic of the method should modify 
   the respective color properties accordingly to the current 'enabled' state. 
   The current state of the component is passed as a set in the parameter aState. 
   It reflects the very basic component state like its visibility or the ability 
   to react to user inputs. Beside this common state, the method can also involve 
   any other variables used in the component as long as they reflect its current 
   state. For example, the toggle switch component can take in account its toggle 
   state 'on' or 'off' and change accordingly the location of the slider, etc.
   Usually, this method will be invoked automatically by the framework. Optionally 
   you can request its invocation by using the method @InvalidateViewState(). */
void ApplicationDatePicker_UpdateViewState( ApplicationDatePicker _this, XSet aState );

/* This method is called when one of the Slide Touch Handler (TouchHandlerDay, TouchHandlerMonth 
   or TouchHandlerYear) has finished the slide animation. Thereupon the owner of 
   the date picker is notified, that the date has been changed. */
void ApplicationDatePicker_onEndSlide( ApplicationDatePicker _this, XObject sender );

/* This method is called when one of the Slide Touch Handler (TouchHandlerDay, TouchHandlerMonth 
   or TouchHandlerYear) has begun the slide animation. */
void ApplicationDatePicker_onStartSlide( ApplicationDatePicker _this, XObject sender );

/* This method is called by 'ListYear' vertical list every time the list loads or 
   updates an item. */
void ApplicationDatePicker_OnLoadYearItem( ApplicationDatePicker _this, XObject 
  sender );

/* This method is called by 'ListMonth' vertical list every time the list loads 
   or updates an item. */
void ApplicationDatePicker_OnLoadMonthItem( ApplicationDatePicker _this, XObject 
  sender );

/* This method is called by 'ListDay' vertical list every time the list loads or 
   updates an item. */
void ApplicationDatePicker_OnLoadDayItem( ApplicationDatePicker _this, XObject sender );

/* 'C' function for method : 'Application::DatePicker.OnGetYear()' */
XInt32 ApplicationDatePicker_OnGetYear( ApplicationDatePicker _this );

/* 'C' function for method : 'Application::DatePicker.OnSetYear()' */
void ApplicationDatePicker_OnSetYear( ApplicationDatePicker _this, XInt32 value );

/* 'C' function for method : 'Application::DatePicker.OnGetMonth()' */
XInt32 ApplicationDatePicker_OnGetMonth( ApplicationDatePicker _this );

/* 'C' function for method : 'Application::DatePicker.OnSetMonth()' */
void ApplicationDatePicker_OnSetMonth( ApplicationDatePicker _this, XInt32 value );

/* 'C' function for method : 'Application::DatePicker.OnGetDay()' */
XInt32 ApplicationDatePicker_OnGetDay( ApplicationDatePicker _this );

/* 'C' function for method : 'Application::DatePicker.OnSetDay()' */
void ApplicationDatePicker_OnSetDay( ApplicationDatePicker _this, XInt32 value );

/* This function returns the number of days in the month and year specified in the 
   method parameters aMonth and aYear. The function takes care of the February and 
   its dependency of the leap year. */
XInt32 ApplicationDatePicker_getDaysInMonth( ApplicationDatePicker _this, XInt32 
  aMonth, XInt32 aYear );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationDatePicker_H */

/* Embedded Wizard */
