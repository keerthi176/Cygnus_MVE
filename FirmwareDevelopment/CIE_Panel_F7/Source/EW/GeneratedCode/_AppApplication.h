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

#ifndef _AppApplication_H
#define _AppApplication_H

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

#include "_AppButton.h"
#include "_AppDFUPopup.h"
#include "_AppDisable.h"
#include "_AppDisablements.h"
#include "_AppDisablementsTabButton.h"
#include "_AppEvents.h"
#include "_AppFaults.h"
#include "_AppFire.h"
#include "_AppLevelAccess.h"
#include "_AppMeshTree.h"
#include "_AppModify.h"
#include "_AppOnTest.h"
#include "_AppProgressPopup.h"
#include "_AppSettings.h"
#include "_AppSystemPage.h"
#include "_AppTabButton.h"
#include "_AppTabButtonPull.h"
#include "_AppTest.h"
#include "_AppUserButton.h"
#include "_CoreRoot.h"
#include "_CoreSystemEventHandler.h"
#include "_CoreTimer.h"
#include "_GraphicsAttrSet.h"
#include "_ViewsImage.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_ViewsWallpaper.h"

/* Forward declaration of the class App::Application */
#ifndef _AppApplication_
  EW_DECLARE_CLASS( AppApplication )
#define _AppApplication_
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

/* Forward declaration of the class Core::ModalContext */
#ifndef _CoreModalContext_
  EW_DECLARE_CLASS( CoreModalContext )
#define _CoreModalContext_
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


/* This is the root component of the entire GUI application. */
EW_DEFINE_FIELDS( AppApplication, CoreRoot )
  EW_OBJECT  ( Rectangle,       ViewsRectangle )
  EW_OBJECT  ( Seperator,       ViewsRectangle )
  EW_OBJECT  ( TimeDateText,    ViewsText )
  EW_OBJECT  ( AlarmsTabButton, AppTabButton )
  EW_OBJECT  ( EvacuateButton,  AppButton )
  EW_OBJECT  ( MuteButton,      AppButton )
  EW_OBJECT  ( Image,           ViewsImage )
  EW_OBJECT  ( FaultsTabButton, AppTabButton )
  EW_OBJECT  ( OnTestTabButton, AppTabButton )
  EW_OBJECT  ( EventsTabButton, AppTabButton )
  EW_PROPERTY( SelectedTab,     XString )
  EW_OBJECT  ( ResetButton,     AppButton )
  EW_OBJECT  ( SilenceButton,   AppButton )
  EW_OBJECT  ( LogOnButton,     AppButton )
  EW_OBJECT  ( OtherEvents,     AppEvents )
  EW_OBJECT  ( Test,            AppTest )
  EW_OBJECT  ( Settings,        AppSettings )
  EW_OBJECT  ( SystemPage,      AppSystemPage )
  EW_OBJECT  ( Modify,          AppModify )
  EW_OBJECT  ( Faults,          AppFaults )
  EW_OBJECT  ( LevelAccess,     AppLevelAccess )
  EW_OBJECT  ( AttrSet,         GraphicsAttrSet )
  EW_OBJECT  ( Timer,           CoreTimer )
  EW_OBJECT  ( FireListChangeEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( StartProgressEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( Alarms,          AppFire )
  EW_OBJECT  ( Disablements,    AppDisablements )
  EW_OBJECT  ( OnTest,          AppOnTest )
  EW_OBJECT  ( EnableDisable,   AppDisable )
  EW_OBJECT  ( MeshTree,        AppMeshTree )
  EW_OBJECT  ( ProgressPopup,   AppProgressPopup )
  EW_OBJECT  ( TabButtonPull,   AppTabButtonPull )
  EW_OBJECT  ( WriteProtectedEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( DFUPopup,        AppDFUPopup )
  EW_OBJECT  ( DFUEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( GotoScreenEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( SwitchConfirmFireEvent, CoreSystemEventHandler )
  EW_OBJECT  ( ConfirmButtonTimer, CoreTimer )
  EW_OBJECT  ( SwitchAcknowledgeEvent, CoreSystemEventHandler )
  EW_OBJECT  ( DisablementsTabButton, AppDisablementsTabButton )
  EW_OBJECT  ( HazardWallpaper, ViewsWallpaper )
  EW_OBJECT  ( SafeStateEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( SiteChangedEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( UserButton,      AppUserButton )
  EW_OBJECT  ( UserButton1,     AppUserButton )
  EW_OBJECT  ( GotoSysPageEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( RestEventHandler, CoreSystemEventHandler )
EW_END_OF_FIELDS( AppApplication )

/* Virtual Method Table (VMT) for the class : 'App::Application' */
EW_DEFINE_METHODS( AppApplication, CoreRoot )
  EW_METHOD( initLayoutContext, void )( CoreRectView _this, XRect aBounds, CoreOutline 
    aOutline )
  EW_METHOD( GetRoot,           CoreRoot )( CoreRoot _this )
  EW_METHOD( Draw,              void )( CoreRoot _this, GraphicsCanvas aCanvas, 
    XRect aClip, XPoint aOffset, XInt32 aOpacity, XBool aBlend )
  EW_METHOD( HandleEvent,       XObject )( CoreView _this, CoreEvent aEvent )
  EW_METHOD( CursorHitTest,     CoreCursorHit )( CoreGroup _this, XRect aArea, XInt32 
    aFinger, XInt32 aStrikeCount, CoreView aDedicatedView, XSet aRetargetReason )
  EW_METHOD( ArrangeView,       XPoint )( CoreRectView _this, XRect aBounds, XEnum 
    aFormation )
  EW_METHOD( MoveView,          void )( CoreRectView _this, XPoint aOffset, XBool 
    aFastMove )
  EW_METHOD( GetExtent,         XRect )( CoreRectView _this )
  EW_METHOD( ChangeViewState,   void )( CoreRoot _this, XSet aSetState, XSet aClearState )
  EW_METHOD( OnSetBounds,       void )( CoreGroup _this, XRect value )
  EW_METHOD( OnSetFocus,        void )( CoreRoot _this, CoreView value )
  EW_METHOD( OnSetBuffered,     void )( CoreRoot _this, XBool value )
  EW_METHOD( OnSetOpacity,      void )( CoreRoot _this, XInt32 value )
  EW_METHOD( OnSetVisible,      void )( CoreGroup _this, XBool value )
  EW_METHOD( DispatchEvent,     XObject )( CoreRoot _this, CoreEvent aEvent )
  EW_METHOD( BroadcastEvent,    XObject )( CoreRoot _this, CoreEvent aEvent, XSet 
    aFilter )
  EW_METHOD( UpdateLayout,      void )( CoreGroup _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( CoreGroup _this, XSet aState )
  EW_METHOD( InvalidateArea,    void )( CoreRoot _this, XRect aArea )
  EW_METHOD( FindSiblingView,   CoreView )( CoreGroup _this, CoreView aView, XSet 
    aFilter )
  EW_METHOD( RestackBehind,     void )( CoreRoot _this, CoreView aView, CoreView 
    aSibling )
  EW_METHOD( RestackTop,        void )( CoreRoot _this, CoreView aView )
  EW_METHOD( Restack,           void )( CoreRoot _this, CoreView aView, XInt32 aOrder )
  EW_METHOD( Remove,            void )( CoreGroup _this, CoreView aView )
  EW_METHOD( Add,               void )( CoreRoot _this, CoreView aView, XInt32 aOrder )
EW_END_OF_METHODS( AppApplication )

/* 'C' function for method : 'App::Application.AlarmsEnter()' */
void AppApplication_AlarmsEnter( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.DisablementsEnter()' */
void AppApplication_DisablementsEnter( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.DisablementsExit()' */
void AppApplication_DisablementsExit( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.EventsEnter()' */
void AppApplication_EventsEnter( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.EventsExit()' */
void AppApplication_EventsExit( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.AlarmsExit()' */
void AppApplication_AlarmsExit( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.PageSelection()' */
void AppApplication_PageSelection( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.PageDeselection()' */
void AppApplication_PageDeselection( AppApplication _this );

/* 'C' function for method : 'App::Application.FaultsEnter()' */
void AppApplication_FaultsEnter( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.FaultsExit()' */
void AppApplication_FaultsExit( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.OnTestEnter()' */
void AppApplication_OnTestEnter( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.OnTestExit()' */
void AppApplication_OnTestExit( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Mute()' */
void AppApplication_Mute( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Tick()' */
void AppApplication_Tick( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Evacuate()' */
void AppApplication_Evacuate( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.LogOn()' */
void AppApplication_LogOn( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Reset()' */
void AppApplication_Reset( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.LowLevel()' */
void AppApplication_LowLevel( AppApplication _this, XInt8 level );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onFireEvent( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Reset1()' */
void AppApplication_Reset1( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Evacuate1()' */
void AppApplication_Evacuate1( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Silence()' */
void AppApplication_Silence( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onEvent( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onEvent1( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onEvent2( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onConfirmFireSwitch( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.TimerEvent()' */
void AppApplication_TimerEvent( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onAcknowldegeSwitch( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onSafeState( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Function1()' */
void AppApplication_Function1( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Function2()' */
void AppApplication_Function2( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Function1off()' */
void AppApplication_Function1off( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.Function2off()' */
void AppApplication_Function2off( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onEvent3( AppApplication _this, XObject sender );

/* 'C' function for method : 'App::Application.ButtonFunc()' */
void AppApplication_ButtonFunc( AppApplication _this, XBool pressed, XObject sender, 
  XInt32 chan );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onEvent4( AppApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppApplication_onGotoScreenEvent( AppApplication _this, XObject sender );

/* Default onget method for the property 'SelectedTab' */
XString AppApplication_OnGetSelectedTab( AppApplication _this );

/* Default onset method for the property 'SelectedTab' */
void AppApplication_OnSetSelectedTab( AppApplication _this, XString value );

#ifdef __cplusplus
  }
#endif

#endif /* _AppApplication_H */

/* Embedded Wizard */
