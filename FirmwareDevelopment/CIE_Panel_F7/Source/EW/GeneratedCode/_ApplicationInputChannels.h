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

#ifndef _ApplicationInputChannels_H
#define _ApplicationInputChannels_H

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

#include "_ApplicationDropDownMenu.h"
#include "_ApplicationInputAction.h"
#include "_ApplicationInputDelay.h"
#include "_ApplicationInputSignal.h"
#include "_ApplicationPopup.h"
#include "_CoreOutline.h"
#include "_CoreSlideTouchHandler.h"
#include "_ViewsBorder.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"
#include "_WidgetSetVerticalScrollbar.h"

/* Forward declaration of the class Application::DeviceConfig */
#ifndef _ApplicationDeviceConfig_
  EW_DECLARE_CLASS( ApplicationDeviceConfig )
#define _ApplicationDeviceConfig_
#endif

/* Forward declaration of the class Application::InputChannels */
#ifndef _ApplicationInputChannels_
  EW_DECLARE_CLASS( ApplicationInputChannels )
#define _ApplicationInputChannels_
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

/* Forward declaration of the class Core::Root */
#ifndef _CoreRoot_
  EW_DECLARE_CLASS( CoreRoot )
#define _CoreRoot_
#endif

/* Forward declaration of the class Core::SimpleTouchHandler */
#ifndef _CoreSimpleTouchHandler_
  EW_DECLARE_CLASS( CoreSimpleTouchHandler )
#define _CoreSimpleTouchHandler_
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


/* Deklaration of class : 'Application::InputChannels' */
EW_DEFINE_FIELDS( ApplicationInputChannels, ApplicationPopup )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( Rectangle9,      ViewsRectangle )
  EW_OBJECT  ( VerticalScrollbar, WidgetSetVerticalScrollbar )
  EW_OBJECT  ( SlideTouchHandler, CoreSlideTouchHandler )
  EW_OBJECT  ( Border,          ViewsBorder )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( Outline,         CoreOutline )
  EW_OBJECT  ( Cancel,          WidgetSetPushButton )
  EW_OBJECT  ( InputAction,     ApplicationInputAction )
  EW_OBJECT  ( DropDownMenu,    ApplicationDropDownMenu )
  EW_OBJECT  ( InputSignal,     ApplicationInputSignal )
  EW_OBJECT  ( InputDelay,      ApplicationInputDelay )
  EW_PROPERTY( ChanIndex,       XInt32 )
  EW_PROPERTY( device,          ApplicationDeviceConfig )
EW_END_OF_FIELDS( ApplicationInputChannels )

/* Virtual Method Table (VMT) for the class : 'Application::InputChannels' */
EW_DEFINE_METHODS( ApplicationInputChannels, ApplicationPopup )
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
  EW_METHOD( UpdateLayout,      void )( ApplicationPopup _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( ApplicationPopup _this, XSet aState )
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
EW_END_OF_METHODS( ApplicationInputChannels )

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void ApplicationInputChannels_Init( ApplicationInputChannels _this, XHandle aArg );

/* 'C' function for method : 'Application::InputChannels.ProceedSlot()' */
void ApplicationInputChannels_ProceedSlot( ApplicationInputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::InputChannels.Scroll()' */
void ApplicationInputChannels_Scroll( ApplicationInputChannels _this, XObject sender );

/* 'C' function for method : 'Application::InputChannels.TouchScroll()' */
void ApplicationInputChannels_TouchScroll( ApplicationInputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::InputChannels.Select()' */
void ApplicationInputChannels_Select( ApplicationInputChannels _this, XObject sender );

/* 'C' function for method : 'Application::InputChannels.LoadMenu()' */
void ApplicationInputChannels_LoadMenu( ApplicationInputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::InputChannels.OnSetChanIndex()' */
void ApplicationInputChannels_OnSetChanIndex( ApplicationInputChannels _this, XInt32 
  value );

/* 'C' function for method : 'Application::InputChannels.CancelSlot()' */
void ApplicationInputChannels_CancelSlot( ApplicationInputChannels _this, XObject 
  sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationInputChannels_H */

/* Embedded Wizard */
