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
* Version  : 8.30
* Profile  : STM32F746
* Platform : STM.STM32.RGB565
*
*******************************************************************************/

#ifndef _FlatRotaryKnob_H
#define _FlatRotaryKnob_H

#ifdef __cplusplus
  extern "C"
  {
#endif

#include "ewrte.h"
#if EW_RTE_VERSION != 0x0008001E
  #error Wrong version of Embedded Wizard Runtime Environment.
#endif

#include "ewgfx.h"
#if EW_GFX_VERSION != 0x0008001E
  #error Wrong version of Embedded Wizard Graphics Engine.
#endif

#include "_CoreGroup.h"
#include "_CoreRotateTouchHandler.h"
#include "_ViewsImage.h"
#include "_ViewsText.h"
#include "_ViewsWarpImage.h"

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

/* Forward declaration of the class Core::View */
#ifndef _CoreView_
  EW_DECLARE_CLASS( CoreView )
#define _CoreView_
#endif

/* Forward declaration of the class Flat::MinMaxText */
#ifndef _FlatMinMaxText_
  EW_DECLARE_CLASS( FlatMinMaxText )
#define _FlatMinMaxText_
#endif

/* Forward declaration of the class Flat::RotaryKnob */
#ifndef _FlatRotaryKnob_
  EW_DECLARE_CLASS( FlatRotaryKnob )
#define _FlatRotaryKnob_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* Rotary know widget with a flat design. The widget allows the user to change a 
   value by rotating a thumb arround the widgets center. */
EW_DEFINE_FIELDS( FlatRotaryKnob, CoreGroup )
  EW_PROPERTY( MinValue,        XInt32 )
  EW_PROPERTY( MaxValue,        XInt32 )
  EW_PROPERTY( CurrentValue,    XInt32 )
  EW_PROPERTY( Outlet,          XRef )
  EW_PROPERTY( OnApply,         XSlot )
  EW_PROPERTY( OnChange,        XSlot )
  EW_VARIABLE( startValue,      XInt32 )
  EW_OBJECT  ( RotateTouchHandler, CoreRotateTouchHandler )
  EW_OBJECT  ( Image,           ViewsImage )
  EW_OBJECT  ( Thumb,           ViewsWarpImage )
  EW_OBJECT  ( ThumbText,       ViewsText )
  EW_OBJECT  ( MoveText,        ViewsText )
  EW_PROPERTY( DotPosition,     XInt32 )
  EW_PROPERTY( Divisor,         XInt32 )
  EW_PROPERTY( MinText,         FlatMinMaxText )
  EW_PROPERTY( MaxText,         FlatMinMaxText )
  EW_PROPERTY( BackColor,       XColor )
  EW_PROPERTY( ItemColor,       XColor )
  EW_PROPERTY( TextColor,       XColor )
  EW_PROPERTY( ItemColorActive, XColor )
  EW_PROPERTY( ShowCurrent,     XBool )
EW_END_OF_FIELDS( FlatRotaryKnob )

/* Virtual Method Table (VMT) for the class : 'Flat::RotaryKnob' */
EW_DEFINE_METHODS( FlatRotaryKnob, CoreGroup )
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
  EW_METHOD( UpdateViewState,   void )( FlatRotaryKnob _this, XSet aState )
  EW_METHOD( InvalidateArea,    void )( CoreGroup _this, XRect aArea )
  EW_METHOD( FindViewAtPosition, CoreView )( CoreGroup _this, CoreView aView, XPoint 
    aPosition, XSet aFilter )
  EW_METHOD( FindNextView,      CoreView )( CoreGroup _this, CoreView aView, XSet 
    aFilter )
  EW_METHOD( FindSiblingView,   CoreView )( CoreGroup _this, CoreView aView, XSet 
    aFilter )
  EW_METHOD( Restack,           void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )
  EW_METHOD( Remove,            void )( CoreGroup _this, CoreView aView )
  EW_METHOD( Add,               void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )
EW_END_OF_METHODS( FlatRotaryKnob )

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
void FlatRotaryKnob_UpdateViewState( FlatRotaryKnob _this, XSet aState );

/* 'C' function for method : 'Flat::RotaryKnob.OnSetMinValue()' */
void FlatRotaryKnob_OnSetMinValue( FlatRotaryKnob _this, XInt32 value );

/* 'C' function for method : 'Flat::RotaryKnob.OnSetMaxValue()' */
void FlatRotaryKnob_OnSetMaxValue( FlatRotaryKnob _this, XInt32 value );

/* 'C' function for method : 'Flat::RotaryKnob.OnSetCurrentValue()' */
void FlatRotaryKnob_OnSetCurrentValue( FlatRotaryKnob _this, XInt32 value );

/* This internal slot method is used to receive the corresponding signals form the 
   touch handler. */
void FlatRotaryKnob_onStartSlot( FlatRotaryKnob _this, XObject sender );

/* This internal slot method is used to receive the corresponding signals form the 
   touch handler. */
void FlatRotaryKnob_onRotateSlot( FlatRotaryKnob _this, XObject sender );

/* This internal slot method is used to receive the corresponding signals form the 
   touch handler. */
void FlatRotaryKnob_onEndSlot( FlatRotaryKnob _this, XObject sender );

/* Method to format a string based on the given aValue and the current @DotPosition 
   and @Divisor. */
XString FlatRotaryKnob_formatValueString( FlatRotaryKnob _this, XInt32 aValue );

/* 'C' function for method : 'Flat::RotaryKnob.OnSetBackColor()' */
void FlatRotaryKnob_OnSetBackColor( FlatRotaryKnob _this, XColor value );

#ifdef __cplusplus
  }
#endif

#endif /* _FlatRotaryKnob_H */

/* Embedded Wizard */
