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

#ifndef _ApplicationKeyboard_H
#define _ApplicationKeyboard_H

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

#include "_ApplicationKeyBmp.h"
#include "_ApplicationKeyLetter.h"
#include "_ApplicationKeySwitch.h"
#include "_CoreGroup.h"
#include "_CoreSimpleTouchHandler.h"

/* Forward declaration of the class Application::Keyboard */
#ifndef _ApplicationKeyboard_
  EW_DECLARE_CLASS( ApplicationKeyboard )
#define _ApplicationKeyboard_
#endif

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

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* Deklaration of class : 'Application::Keyboard' */
EW_DEFINE_FIELDS( ApplicationKeyboard, CoreGroup )
  EW_OBJECT  ( Key,             ApplicationKeyLetter )
  EW_OBJECT  ( Key1,            ApplicationKeyLetter )
  EW_OBJECT  ( Key2,            ApplicationKeyLetter )
  EW_OBJECT  ( Key3,            ApplicationKeyLetter )
  EW_OBJECT  ( Key4,            ApplicationKeyLetter )
  EW_OBJECT  ( Key5,            ApplicationKeyLetter )
  EW_OBJECT  ( Key6,            ApplicationKeyLetter )
  EW_OBJECT  ( Key7,            ApplicationKeyLetter )
  EW_OBJECT  ( Key8,            ApplicationKeyLetter )
  EW_OBJECT  ( Key9,            ApplicationKeyLetter )
  EW_OBJECT  ( Key10,           ApplicationKeyLetter )
  EW_OBJECT  ( Key11,           ApplicationKeyLetter )
  EW_OBJECT  ( Key12,           ApplicationKeyLetter )
  EW_OBJECT  ( Key13,           ApplicationKeyLetter )
  EW_OBJECT  ( Key14,           ApplicationKeyLetter )
  EW_OBJECT  ( Key15,           ApplicationKeyLetter )
  EW_OBJECT  ( Key16,           ApplicationKeyLetter )
  EW_OBJECT  ( Key17,           ApplicationKeyLetter )
  EW_OBJECT  ( Key18,           ApplicationKeyLetter )
  EW_OBJECT  ( Key20,           ApplicationKeyLetter )
  EW_OBJECT  ( Key21,           ApplicationKeyLetter )
  EW_OBJECT  ( Key22,           ApplicationKeyLetter )
  EW_OBJECT  ( Key23,           ApplicationKeyLetter )
  EW_OBJECT  ( Key24,           ApplicationKeyLetter )
  EW_OBJECT  ( Key25,           ApplicationKeyLetter )
  EW_OBJECT  ( Key26,           ApplicationKeyLetter )
  EW_OBJECT  ( Key28,           ApplicationKeyLetter )
  EW_OBJECT  ( KeybBmp1,        ApplicationKeyBmp )
  EW_OBJECT  ( KeybBmp2,        ApplicationKeyBmp )
  EW_OBJECT  ( KeySwitch,       ApplicationKeySwitch )
  EW_OBJECT  ( Touch,           CoreSimpleTouchHandler )
EW_END_OF_FIELDS( ApplicationKeyboard )

/* Virtual Method Table (VMT) for the class : 'Application::Keyboard' */
EW_DEFINE_METHODS( ApplicationKeyboard, CoreGroup )
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
  EW_METHOD( UpdateLayout,      void )( ApplicationKeyboard _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( ApplicationKeyboard _this, XSet aState )
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
EW_END_OF_METHODS( ApplicationKeyboard )

/* The method UpdateLayout() is invoked automatically after the size of the component 
   has been changed. This method can be overridden and filled with logic to perform 
   a sophisticated arrangement calculation for one or more enclosed views. In this 
   case the parameter aSize can be used. It contains the current size of the component. 
   Usually, all enclosed views are arranged automatically accordingly to their @Layout 
   property. UpdateLayout() gives the derived components a chance to extend this 
   automatism by a user defined algorithm. */
void ApplicationKeyboard_UpdateLayout( ApplicationKeyboard _this, XPoint aSize );

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
void ApplicationKeyboard_UpdateViewState( ApplicationKeyboard _this, XSet aState );

/* 'C' function for method : 'Application::Keyboard.PressSlot()' */
void ApplicationKeyboard_PressSlot( ApplicationKeyboard _this, XObject sender );

/* 'C' function for method : 'Application::Keyboard.ReleaseSlot()' */
void ApplicationKeyboard_ReleaseSlot( ApplicationKeyboard _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationKeyboard_H */

/* Embedded Wizard */
