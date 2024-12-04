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

#ifndef _AppOutputChannels_H
#define _AppOutputChannels_H

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

#include "_AppDropDownMenu.h"
#include "_AppOutputAction.h"
#include "_AppOutputDelays.h"
#include "_AppOutputSignal.h"
#include "_AppPopup.h"
#include "_CoreOutline.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreSlideTouchHandler.h"
#include "_CoreSystemEventHandler.h"
#include "_ViewsBorder.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"
#include "_WidgetSetVerticalScrollbar.h"

/* Forward declaration of the class App::DeviceConfig */
#ifndef _AppDeviceConfig_
  EW_DECLARE_CLASS( AppDeviceConfig )
#define _AppDeviceConfig_
#endif

/* Forward declaration of the class App::OutputChannels */
#ifndef _AppOutputChannels_
  EW_DECLARE_CLASS( AppOutputChannels )
#define _AppOutputChannels_
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


/* Deklaration of class : 'App::OutputChannels' */
EW_DEFINE_FIELDS( AppOutputChannels, AppPopup )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( Rectangle9,      ViewsRectangle )
  EW_OBJECT  ( VerticalScrollbar, WidgetSetVerticalScrollbar )
  EW_OBJECT  ( SlideTouchHandler, CoreSlideTouchHandler )
  EW_OBJECT  ( Border,          ViewsBorder )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( Outline,         CoreOutline )
  EW_OBJECT  ( DropDownMenu,    AppDropDownMenu )
  EW_PROPERTY( ChanIndex,       XInt32 )
  EW_OBJECT  ( OutputAction,    AppOutputAction )
  EW_OBJECT  ( OutputSignal,    AppOutputSignal )
  EW_OBJECT  ( OutputDelays,    AppOutputDelays )
  EW_PROPERTY( device,          AppDeviceConfig )
EW_END_OF_FIELDS( AppOutputChannels )

/* Virtual Method Table (VMT) for the class : 'App::OutputChannels' */
EW_DEFINE_METHODS( AppOutputChannels, AppPopup )
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
  EW_METHOD( UpdateLayout,      void )( AppOutputChannels _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( AppPopup _this, XSet aState )
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
EW_END_OF_METHODS( AppOutputChannels )

/* The method UpdateLayout() is invoked automatically after the size of the component 
   has been changed. This method can be overridden and filled with logic to perform 
   a sophisticated arrangement calculation for one or more enclosed views. In this 
   case the parameter aSize can be used. It contains the current size of the component. 
   Usually, all enclosed views are arranged automatically accordingly to their @Layout 
   property. UpdateLayout() gives the derived components a chance to extend this 
   automatism by a user defined algorithm. */
void AppOutputChannels_UpdateLayout( AppOutputChannels _this, XPoint aSize );

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppOutputChannels_Init( AppOutputChannels _this, XHandle aArg );

/* 'C' function for method : 'App::OutputChannels.Scroll()' */
void AppOutputChannels_Scroll( AppOutputChannels _this, XObject sender );

/* 'C' function for method : 'App::OutputChannels.TouchScroll()' */
void AppOutputChannels_TouchScroll( AppOutputChannels _this, XObject sender );

/* 'C' function for method : 'App::OutputChannels.Select()' */
void AppOutputChannels_Select( AppOutputChannels _this, XObject sender );

/* 'C' function for method : 'App::OutputChannels.LoadMenu()' */
void AppOutputChannels_LoadMenu( AppOutputChannels _this, XObject sender );

/* 'C' function for method : 'App::OutputChannels.OnSetChanIndex()' */
void AppOutputChannels_OnSetChanIndex( AppOutputChannels _this, XInt32 value );

/* 'C' function for method : 'App::OutputChannels.CancelSlot()' */
void AppOutputChannels_CancelSlot( AppOutputChannels _this, XObject sender );

/* 'C' function for method : 'App::OutputChannels.SaveData()' */
void AppOutputChannels_SaveData( AppOutputChannels _this );

#ifdef __cplusplus
  }
#endif

#endif /* _AppOutputChannels_H */

/* Embedded Wizard */
