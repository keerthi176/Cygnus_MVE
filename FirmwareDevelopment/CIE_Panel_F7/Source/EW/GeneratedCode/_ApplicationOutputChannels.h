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

#ifndef _ApplicationOutputChannels_H
#define _ApplicationOutputChannels_H

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
#include "_ApplicationOutputAction.h"
#include "_ApplicationOutputDelays.h"
#include "_ApplicationOutputSignal.h"
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

/* Forward declaration of the class Application::OutputChannels */
#ifndef _ApplicationOutputChannels_
  EW_DECLARE_CLASS( ApplicationOutputChannels )
#define _ApplicationOutputChannels_
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


/* Deklaration of class : 'Application::OutputChannels' */
EW_DEFINE_FIELDS( ApplicationOutputChannels, ApplicationPopup )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( Rectangle9,      ViewsRectangle )
  EW_OBJECT  ( VerticalScrollbar, WidgetSetVerticalScrollbar )
  EW_OBJECT  ( SlideTouchHandler, CoreSlideTouchHandler )
  EW_OBJECT  ( Border,          ViewsBorder )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( Outline,         CoreOutline )
  EW_OBJECT  ( DropDownMenu,    ApplicationDropDownMenu )
  EW_PROPERTY( ChanIndex,       XInt32 )
  EW_OBJECT  ( OutputAction,    ApplicationOutputAction )
  EW_OBJECT  ( OutputSignal,    ApplicationOutputSignal )
  EW_OBJECT  ( OutputDelays,    ApplicationOutputDelays )
  EW_PROPERTY( device,          ApplicationDeviceConfig )
EW_END_OF_FIELDS( ApplicationOutputChannels )

/* Virtual Method Table (VMT) for the class : 'Application::OutputChannels' */
EW_DEFINE_METHODS( ApplicationOutputChannels, ApplicationPopup )
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
  EW_METHOD( UpdateLayout,      void )( ApplicationOutputChannels _this, XPoint 
    aSize )
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
EW_END_OF_METHODS( ApplicationOutputChannels )

/* The method UpdateLayout() is invoked automatically after the size of the component 
   has been changed. This method can be overridden and filled with logic to perform 
   a sophisticated arrangement calculation for one or more enclosed views. In this 
   case the parameter aSize can be used. It contains the current size of the component. 
   Usually, all enclosed views are arranged automatically accordingly to their @Layout 
   property. UpdateLayout() gives the derived components a chance to extend this 
   automatism by a user defined algorithm. */
void ApplicationOutputChannels_UpdateLayout( ApplicationOutputChannels _this, XPoint 
  aSize );

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void ApplicationOutputChannels_Init( ApplicationOutputChannels _this, XHandle aArg );

/* 'C' function for method : 'Application::OutputChannels.ProceedSlot()' */
void ApplicationOutputChannels_ProceedSlot( ApplicationOutputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::OutputChannels.Scroll()' */
void ApplicationOutputChannels_Scroll( ApplicationOutputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::OutputChannels.TouchScroll()' */
void ApplicationOutputChannels_TouchScroll( ApplicationOutputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::OutputChannels.Select()' */
void ApplicationOutputChannels_Select( ApplicationOutputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::OutputChannels.LoadMenu()' */
void ApplicationOutputChannels_LoadMenu( ApplicationOutputChannels _this, XObject 
  sender );

/* 'C' function for method : 'Application::OutputChannels.OnSetChanIndex()' */
void ApplicationOutputChannels_OnSetChanIndex( ApplicationOutputChannels _this, 
  XInt32 value );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationOutputChannels_H */

/* Embedded Wizard */
