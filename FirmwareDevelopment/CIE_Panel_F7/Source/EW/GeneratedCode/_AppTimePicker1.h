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

#ifndef _AppTimePicker1_H
#define _AppTimePicker1_H

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
#include "_CoreTimer.h"
#include "_CoreVerticalList.h"
#include "_TemplatesPickerControl.h"
#include "_ViewsBorder.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class App::TimePicker1 */
#ifndef _AppTimePicker1_
  EW_DECLARE_CLASS( AppTimePicker1 )
#define _AppTimePicker1_
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
EW_DEFINE_FIELDS( AppTimePicker1, TemplatesPickerControl )
  EW_OBJECT  ( Rectangle,       ViewsRectangle )
  EW_OBJECT  ( TouchHandlerMinute, CoreSlideTouchHandler )
  EW_OBJECT  ( TouchHandlerHour, CoreSlideTouchHandler )
  EW_OBJECT  ( ListMinute,      CoreVerticalList )
  EW_OBJECT  ( ListHour,        CoreVerticalList )
  EW_OBJECT  ( Separator1,      ViewsText )
  EW_PROPERTY( OnChange,        XSlot )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( PushButton2,     WidgetSetPushButton )
  EW_OBJECT  ( PushButton3,     WidgetSetPushButton )
  EW_OBJECT  ( BorderCurrent,   ViewsBorder )
  EW_OBJECT  ( Rectangle1,      ViewsRectangle )
  EW_OBJECT  ( Rectangle2,      ViewsRectangle )
  EW_OBJECT  ( Timer,           CoreTimer )
  EW_PROPERTY( Pressed,         WidgetSetPushButton )
  EW_VARIABLE( enabled,         XBool )
  EW_VARIABLE( selected,        XBool )
  EW_VARIABLE( sliding,         XBool )
EW_END_OF_FIELDS( AppTimePicker1 )

/* Virtual Method Table (VMT) for the class : 'App::TimePicker1' */
EW_DEFINE_METHODS( AppTimePicker1, TemplatesPickerControl )
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
  EW_METHOD( OnSetVisible,      void )( CoreGroup _this, XBool value )
  EW_METHOD( DispatchEvent,     XObject )( CoreGroup _this, CoreEvent aEvent )
  EW_METHOD( BroadcastEvent,    XObject )( CoreGroup _this, CoreEvent aEvent, XSet 
    aFilter )
  EW_METHOD( UpdateLayout,      void )( CoreGroup _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( AppTimePicker1 _this, XSet aState )
  EW_METHOD( InvalidateArea,    void )( CoreGroup _this, XRect aArea )
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
EW_END_OF_METHODS( AppTimePicker1 )

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
void AppTimePicker1_UpdateViewState( AppTimePicker1 _this, XSet aState );

/* This method is called when one of the Slide Touch Handler (TouchHandlerHour, 
   TouchHandlerMinute or TouchHandlerSecond) has finished the slide animation. Thereupon 
   the owner of the time picker is notified, that the time has been changed. */
void AppTimePicker1_onEndSlide( AppTimePicker1 _this, XObject sender );

/* This method is called when one of the Slide Touch Handler (TouchHandlerHour, 
   TouchHandlerMinute or TouchHandlerSecond) has begun the slide animation. */
void AppTimePicker1_onStartSlide( AppTimePicker1 _this, XObject sender );

/* This method is called by 'ListMinute' vertical list every time the list loads 
   or updates an item. */
void AppTimePicker1_OnLoadMinuteItem( AppTimePicker1 _this, XObject sender );

/* This method is called by 'ListHour' vertical list every time the list loads or 
   updates an item. */
void AppTimePicker1_OnLoadHourItem( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.OnGetMinute()' */
XInt32 AppTimePicker1_OnGetMinute( AppTimePicker1 _this );

/* 'C' function for method : 'App::TimePicker1.OnSetMinute()' */
void AppTimePicker1_OnSetMinute( AppTimePicker1 _this, XInt32 value );

/* 'C' function for method : 'App::TimePicker1.OnGetHour()' */
XInt32 AppTimePicker1_OnGetHour( AppTimePicker1 _this );

/* 'C' function for method : 'App::TimePicker1.OnSetHour()' */
void AppTimePicker1_OnSetHour( AppTimePicker1 _this, XInt32 value );

/* 'C' function for method : 'App::TimePicker1.HourUp()' */
void AppTimePicker1_HourUp( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.HourDown()' */
void AppTimePicker1_HourDown( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.MinuteUp()' */
void AppTimePicker1_MinuteUp( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.MinuteDown()' */
void AppTimePicker1_MinuteDown( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.PressSlot()' */
void AppTimePicker1_PressSlot( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.ReleaseSlot()' */
void AppTimePicker1_ReleaseSlot( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.Tick()' */
void AppTimePicker1_Tick( AppTimePicker1 _this, XObject sender );

/* 'C' function for method : 'App::TimePicker1.SetHour()' */
void AppTimePicker1_SetHour( AppTimePicker1 _this, XInt32 hour );

#ifdef __cplusplus
  }
#endif

#endif /* _AppTimePicker1_H */

/* Embedded Wizard */
