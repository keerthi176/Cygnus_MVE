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

#ifndef _WidgetSetVerticalScrollbar_H
#define _WidgetSetVerticalScrollbar_H

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

#include "_CoreGroup.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreTimer.h"

/* Forward declaration of the class Core::DialogContext */
#ifndef _CoreDialogContext_
  EW_DECLARE_CLASS( CoreDialogContext )
#define _CoreDialogContext_
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

/* Forward declaration of the class Effects::Int32Effect */
#ifndef _EffectsInt32Effect_
  EW_DECLARE_CLASS( EffectsInt32Effect )
#define _EffectsInt32Effect_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif

/* Forward declaration of the class Views::Frame */
#ifndef _ViewsFrame_
  EW_DECLARE_CLASS( ViewsFrame )
#define _ViewsFrame_
#endif

/* Forward declaration of the class WidgetSet::VerticalScrollbar */
#ifndef _WidgetSetVerticalScrollbar_
  EW_DECLARE_CLASS( WidgetSetVerticalScrollbar )
#define _WidgetSetVerticalScrollbar_
#endif

/* Forward declaration of the class WidgetSet::VerticalScrollbarConfig */
#ifndef _WidgetSetVerticalScrollbarConfig_
  EW_DECLARE_CLASS( WidgetSetVerticalScrollbarConfig )
#define _WidgetSetVerticalScrollbarConfig_
#endif


/* This class implements a 'vertical scrollbar' widget. The widget is intended to 
   show the vertical scroll position and the size of the actually visible area (view) 
   within the body of a large document (content). The size of the entire scrollable 
   content and the size of the actually visible area are specified in the properties 
   @ContentArea and @ViewArea. The current scroll position is determined by the 
   property @Position. The scrollbar calculates from these values the size and the 
   position of the scrollbar thumb.
   If the scrollbar is configured to handle user inputs (see below) and the user 
   touches the thumb and drags it vertically, the scrollbar current scroll position 
   (@Position) is adjusted and signal is sent to the slot method stored in the property 
   @OnScroll. By connecting further slot methods to the properties @OnStart and 
   @OnEnd you can react to other events triggered while the user starts and ends 
   the interaction with the scrollbar. With the properties @SnapFirst, @SnapNext 
   and @SnapLast the scrollbar can be configured to automatically snap at predetermined 
   positions when the user finishes the scroll interaction.
   During its lifetime the scrollbar remains always in one of the three states: 
   'disabled', 'default' and 'active'. The state 'disabled' is true for every not 
   available scrollbar (the property @Enabled of the scrollbar is 'false'). Such 
   scrollbars will ignore all user inputs. The state 'default' determines a scrollbar, 
   which is ready to be touched by the user. Finally, the state 'active' is true, 
   if the user actually interacts with the scrollbar (the scrollbar's thumb is pressed).
   The exact look and feel of the scrollbar is determined by the 'Vertical Scrollbar 
   Configuration' object assigned to the property @Appearance. The configuration 
   object provides bitmaps, colors and other configuration parameters needed to 
   construct and display the scrollbar. Usually, you will manage in your project 
   your own configuration objects and customize the scrollbars according to your 
   design expectations. Depending on the information provided in the associated 
   configuration object, the scrollbar will be composed of following views:
   - 'Track' is a bitmap frame view (Views::Frame) filling horizontally centered 
   the entire height of the scrollbar. In the configuration object you can individually 
   specify for every scrollbar state the desired bitmap, its opacity, frame number 
   (if the bitmap is multi-frame) and tint color (if the bitmap contains Alpha8 
   information only). The scrollbar can automatically play animated bitmaps.
   - 'Thumb' is a bitmap frame view (Views::Frame) centered horizontally at the 
   current scroll position. The height of the thumb corresponds to the ratio between 
   the @ViewArea and @ContentArea. In the configuration object you can individually 
   specify for every scrollbar state the desired bitmap, its opacity, frame number 
   (if the bitmap is multi-frame) and tint color (if the bitmap contains Alpha8 
   information only). The scrollbar can automatically play animated bitmaps. If 
   necessary, additional margins below and above the thumb can be specified.
   In the configuration object the scrollbar can be configured to disappear automatically 
   when it is not needed (e.g. when the @ViewArea is greater than or equal to the 
   @ContentArea) or it has not been used for a while. In the last case, the scrollbar 
   appears again automatically when one of the properties @Position, @ContentArea 
   or @ViewArea has changed or the user has touched within the scrollbar area and 
   the scrollbar is enabled for user interactions. Furthermore, the show and hide 
   operation can be configured to be performed with an opacity fade-in/out effect.
   In the configuration object you can determine, whether the scrollbar should be 
   able to react to user touch interactions or not. If this mode is enabled, the 
   user can touch and drag the thumb as described above. If this mode is not enabled, 
   the scrollbar serves as a pure passive widget.
   For more details regarding the customization of the scrollbar see the description 
   of WidgetSet::VerticalScrollbarConfig class. */
EW_DEFINE_FIELDS( WidgetSetVerticalScrollbar, CoreGroup )
  EW_VARIABLE( snapEffect,      EffectsInt32Effect )
  EW_VARIABLE( fadeInOutEffect, EffectsInt32Effect )
  EW_VARIABLE( touchStartPosition, XInt32 )
  EW_VARIABLE( frameView2,      ViewsFrame )
  EW_VARIABLE( frameView1,      ViewsFrame )
  EW_OBJECT  ( AutoHideTimer,   CoreTimer )
  EW_OBJECT  ( TouchHandlerTrack, CoreSimpleTouchHandler )
  EW_OBJECT  ( TouchHandlerThumb, CoreSimpleTouchHandler )
  EW_PROPERTY( OnScroll,        XSlot )
  EW_PROPERTY( OnEnd,           XSlot )
  EW_PROPERTY( OnStart,         XSlot )
  EW_PROPERTY( SnapLast,        XInt32 )
  EW_PROPERTY( SnapNext,        XInt32 )
  EW_PROPERTY( SnapFirst,       XInt32 )
  EW_PROPERTY( Position,        XInt32 )
  EW_PROPERTY( ViewArea,        XInt32 )
  EW_PROPERTY( ContentArea,     XInt32 )
  EW_PROPERTY( Appearance,      WidgetSetVerticalScrollbarConfig )
  EW_VARIABLE( hideIfNotNeeded, XBool )
  EW_VARIABLE( autoHideTimerExpired, XBool )
EW_END_OF_FIELDS( WidgetSetVerticalScrollbar )

/* Virtual Method Table (VMT) for the class : 'WidgetSet::VerticalScrollbar' */
EW_DEFINE_METHODS( WidgetSetVerticalScrollbar, CoreGroup )
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
  EW_METHOD( OnSetBounds,       void )( WidgetSetVerticalScrollbar _this, XRect 
    value )
  EW_METHOD( OnSetFocus,        void )( CoreGroup _this, CoreView value )
  EW_METHOD( OnSetBuffered,     void )( CoreGroup _this, XBool value )
  EW_METHOD( OnSetOpacity,      void )( CoreGroup _this, XInt32 value )
  EW_METHOD( OnSetVisible,      void )( CoreGroup _this, XBool value )
  EW_METHOD( DispatchEvent,     XObject )( CoreGroup _this, CoreEvent aEvent )
  EW_METHOD( BroadcastEvent,    XObject )( CoreGroup _this, CoreEvent aEvent, XSet 
    aFilter )
  EW_METHOD( UpdateLayout,      void )( CoreGroup _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( WidgetSetVerticalScrollbar _this, XSet aState )
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
EW_END_OF_METHODS( WidgetSetVerticalScrollbar )

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.OnSetBounds()' */
void WidgetSetVerticalScrollbar_OnSetBounds( WidgetSetVerticalScrollbar _this, XRect 
  value );

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
void WidgetSetVerticalScrollbar_UpdateViewState( WidgetSetVerticalScrollbar _this, 
  XSet aState );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onFinishedEffect()' */
void WidgetSetVerticalScrollbar_onFinishedEffect( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.updateVisibility()' */
void WidgetSetVerticalScrollbar_updateVisibility( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onAutoHideTimer()' */
void WidgetSetVerticalScrollbar_onAutoHideTimer( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onFinishedSnapEffect()' */
void WidgetSetVerticalScrollbar_onFinishedSnapEffect( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onSnapEffect()' */
void WidgetSetVerticalScrollbar_onSnapEffect( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onConfigChanged()' */
void WidgetSetVerticalScrollbar_onConfigChanged( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onReleaseTrack()' */
void WidgetSetVerticalScrollbar_onReleaseTrack( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onPressTrack()' */
void WidgetSetVerticalScrollbar_onPressTrack( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onDragThumb()' */
void WidgetSetVerticalScrollbar_onDragThumb( WidgetSetVerticalScrollbar _this, XObject 
  sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onReleaseThumb()' */
void WidgetSetVerticalScrollbar_onReleaseThumb( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.onPressThumb()' */
void WidgetSetVerticalScrollbar_onPressThumb( WidgetSetVerticalScrollbar _this, 
  XObject sender );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.OnGetPosition()' */
XInt32 WidgetSetVerticalScrollbar_OnGetPosition( WidgetSetVerticalScrollbar _this );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.OnSetPosition()' */
void WidgetSetVerticalScrollbar_OnSetPosition( WidgetSetVerticalScrollbar _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.OnSetViewArea()' */
void WidgetSetVerticalScrollbar_OnSetViewArea( WidgetSetVerticalScrollbar _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.OnSetContentArea()' */
void WidgetSetVerticalScrollbar_OnSetContentArea( WidgetSetVerticalScrollbar _this, 
  XInt32 value );

/* 'C' function for method : 'WidgetSet::VerticalScrollbar.OnSetAppearance()' */
void WidgetSetVerticalScrollbar_OnSetAppearance( WidgetSetVerticalScrollbar _this, 
  WidgetSetVerticalScrollbarConfig value );

#ifdef __cplusplus
  }
#endif

#endif /* _WidgetSetVerticalScrollbar_H */

/* Embedded Wizard */
