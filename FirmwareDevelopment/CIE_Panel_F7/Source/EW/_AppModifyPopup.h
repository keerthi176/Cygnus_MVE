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

#ifndef _AppModifyPopup_H
#define _AppModifyPopup_H

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

#include "_AppMattsKeyboard.h"
#include "_AppPopup.h"
#include "_AppWritable.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreSystemEventHandler.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class App::DeviceConfig */
#ifndef _AppDeviceConfig_
  EW_DECLARE_CLASS( AppDeviceConfig )
#define _AppDeviceConfig_
#endif

/* Forward declaration of the class App::Modify */
#ifndef _AppModify_
  EW_DECLARE_CLASS( AppModify )
#define _AppModify_
#endif

/* Forward declaration of the class App::ModifyPopup */
#ifndef _AppModifyPopup_
  EW_DECLARE_CLASS( AppModifyPopup )
#define _AppModifyPopup_
#endif

/* Forward declaration of the class App::UpdateDevicePopup */
#ifndef _AppUpdateDevicePopup_
  EW_DECLARE_CLASS( AppUpdateDevicePopup )
#define _AppUpdateDevicePopup_
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


/* Deklaration of class : 'App::ModifyPopup' */
EW_DEFINE_FIELDS( AppModifyPopup, AppPopup )
  EW_OBJECT  ( MattsKeyboard,   AppMattsKeyboard )
  EW_OBJECT  ( Writable,        AppWritable )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( CloseButton,     WidgetSetPushButton )
  EW_OBJECT  ( InputChanButton, WidgetSetPushButton )
  EW_OBJECT  ( OutputChanButton, WidgetSetPushButton )
  EW_OBJECT  ( WriteButton,     WidgetSetPushButton )
  EW_PROPERTY( device,          AppDeviceConfig )
  EW_PROPERTY( modify,          AppModify )
  EW_OBJECT  ( MeshChangeEventHandler, CoreSystemEventHandler )
  EW_PROPERTY( write,           AppUpdateDevicePopup )
EW_END_OF_FIELDS( AppModifyPopup )

/* Virtual Method Table (VMT) for the class : 'App::ModifyPopup' */
EW_DEFINE_METHODS( AppModifyPopup, AppPopup )
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
EW_END_OF_METHODS( AppModifyPopup )

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppModifyPopup_Init( AppModifyPopup _this, XHandle aArg );

/* 'C' function for method : 'App::ModifyPopup.ProceedSlot()' */
void AppModifyPopup_ProceedSlot( AppModifyPopup _this, XObject sender );

/* 'C' function for method : 'App::ModifyPopup.InputChannels()' */
void AppModifyPopup_InputChannels( AppModifyPopup _this, XObject sender );

/* 'C' function for method : 'App::ModifyPopup.OutputChannels()' */
void AppModifyPopup_OutputChannels( AppModifyPopup _this, XObject sender );

/* 'C' function for method : 'App::ModifyPopup.CancelSlot()' */
void AppModifyPopup_CancelSlot( AppModifyPopup _this, XObject sender );

/* 'C' function for method : 'App::ModifyPopup.OnSetdevice()' */
void AppModifyPopup_OnSetdevice( AppModifyPopup _this, AppDeviceConfig value );

/* 'C' function for method : 'App::ModifyPopup.SetFocus()' */
void AppModifyPopup_SetFocus( AppModifyPopup _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler1' 
   receives an event. */
void AppModifyPopup_onEvent( AppModifyPopup _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _AppModifyPopup_H */

/* Embedded Wizard */
