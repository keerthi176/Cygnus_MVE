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

#ifndef _AppAddDevicePopup_H
#define _AppAddDevicePopup_H

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
#include "_AppPopup.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreSystemEventHandler.h"
#include "_ViewsBevel.h"
#include "_ViewsImage.h"
#include "_ViewsLine.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetHorizontalValueBar.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class App::AddDevicePopup */
#ifndef _AppAddDevicePopup_
  EW_DECLARE_CLASS( AppAddDevicePopup )
#define _AppAddDevicePopup_
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


/* Deklaration of class : 'App::AddDevicePopup' */
EW_DEFINE_FIELDS( AppAddDevicePopup, AppPopup )
  EW_OBJECT  ( Image,           ViewsImage )
  EW_OBJECT  ( Text,            ViewsText )
  EW_OBJECT  ( Image1,          ViewsImage )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( Text2,           ViewsText )
  EW_OBJECT  ( HorizontalValueBar, WidgetSetHorizontalValueBar )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( SystemEventHandler10, CoreSystemEventHandler )
  EW_OBJECT  ( SystemEventHandler3, CoreSystemEventHandler )
  EW_OBJECT  ( Text5,           ViewsText )
  EW_OBJECT  ( PushButton2,     WidgetSetPushButton )
  EW_OBJECT  ( PushButton4,     WidgetSetPushButton )
  EW_OBJECT  ( Text3,           ViewsText )
  EW_OBJECT  ( Line,            ViewsLine )
  EW_OBJECT  ( DropDownMenu1,   AppDropDownMenu )
  EW_OBJECT  ( DropDownMenu,    AppDropDownMenu )
  EW_PROPERTY( NewUnitNum,      XInt32 )
  EW_OBJECT  ( PushButton3,     WidgetSetPushButton )
  EW_OBJECT  ( SystemEventHandler4, CoreSystemEventHandler )
  EW_OBJECT  ( Rectangle9,      ViewsRectangle )
  EW_OBJECT  ( Bevel1,          ViewsBevel )
  EW_OBJECT  ( Unit,            ViewsText )
  EW_OBJECT  ( SystemEventHandler5, CoreSystemEventHandler )
  EW_VARIABLE( typeCode,        XInt32 )
EW_END_OF_FIELDS( AppAddDevicePopup )

/* Virtual Method Table (VMT) for the class : 'App::AddDevicePopup' */
EW_DEFINE_METHODS( AppAddDevicePopup, AppPopup )
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
  EW_METHOD( UpdateLayout,      void )( AppPopup _this, XPoint aSize )
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
EW_END_OF_METHODS( AppAddDevicePopup )

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppAddDevicePopup_Init( AppAddDevicePopup _this, XHandle aArg );

/* 'C' function for method : 'App::AddDevicePopup.GetMenuItem()' */
void AppAddDevicePopup_GetMenuItem( AppAddDevicePopup _this, XObject sender );

/* 'C' function for method : 'App::AddDevicePopup.SetupMenus()' */
void AppAddDevicePopup_SetupMenus( AppAddDevicePopup _this );

/* 'C' function for method : 'App::AddDevicePopup.FillMenu()' */
void AppAddDevicePopup_FillMenu( AppAddDevicePopup _this, XObject sender );

/* 'C' function for method : 'App::AddDevicePopup.Setup()' */
void AppAddDevicePopup_Setup( AppAddDevicePopup _this, XObject sender );

/* 'C' function for method : 'App::AddDevicePopup.Cancel()' */
void AppAddDevicePopup_Cancel( AppAddDevicePopup _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler1' 
   receives an event. */
void AppAddDevicePopup_onEvent( AppAddDevicePopup _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler3' 
   receives an event. */
void AppAddDevicePopup_onEvent1( AppAddDevicePopup _this, XObject sender );

/* 'C' function for method : 'App::AddDevicePopup.SelectSlot()' */
void AppAddDevicePopup_SelectSlot( AppAddDevicePopup _this, XObject sender );

/* 'C' function for method : 'App::AddDevicePopup.Program()' */
void AppAddDevicePopup_Program( AppAddDevicePopup _this, XObject sender );

/* 'C' function for method : 'App::AddDevicePopup.CreateSlot()' */
void AppAddDevicePopup_CreateSlot( AppAddDevicePopup _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler4' 
   receives an event. */
void AppAddDevicePopup_onEvent2( AppAddDevicePopup _this, XObject sender );

/* 'C' function for method : 'App::AddDevicePopup.CancelRBUComs()' */
void AppAddDevicePopup_CancelRBUComs( AppAddDevicePopup _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler4' 
   receives an event. */
void AppAddDevicePopup_onEvent3( AppAddDevicePopup _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _AppAddDevicePopup_H */

/* Embedded Wizard */
