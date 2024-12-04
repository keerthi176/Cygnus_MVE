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

#ifndef _ApplicationApplication_H
#define _ApplicationApplication_H

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

#include "_ApplicationAlarms.h"
#include "_ApplicationButton.h"
#include "_ApplicationCauseAndEffects.h"
#include "_ApplicationDFUPopup.h"
#include "_ApplicationDisable.h"
#include "_ApplicationDisablements.h"
#include "_ApplicationEvents.h"
#include "_ApplicationFaults.h"
#include "_ApplicationLevelAccess.h"
#include "_ApplicationMeshTree.h"
#include "_ApplicationModify.h"
#include "_ApplicationOnTest.h"
#include "_ApplicationProgressPopup.h"
#include "_ApplicationSettings.h"
#include "_ApplicationSystemPage.h"
#include "_ApplicationTabButton.h"
#include "_ApplicationTabButtonPull.h"
#include "_ApplicationTest.h"
#include "_CoreRoot.h"
#include "_CoreSystemEventHandler.h"
#include "_CoreTimer.h"
#include "_GraphicsAttrSet.h"
#include "_ViewsImage.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"

/* Forward declaration of the class Application::Application */
#ifndef _ApplicationApplication_
  EW_DECLARE_CLASS( ApplicationApplication )
#define _ApplicationApplication_
#endif

/* Forward declaration of the class Application::Popup */
#ifndef _ApplicationPopup_
  EW_DECLARE_CLASS( ApplicationPopup )
#define _ApplicationPopup_
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
EW_DEFINE_FIELDS( ApplicationApplication, CoreRoot )
  EW_OBJECT  ( Rectangle,       ViewsRectangle )
  EW_OBJECT  ( Seperator,       ViewsRectangle )
  EW_OBJECT  ( TimeDateText,    ViewsText )
  EW_OBJECT  ( AlarmsTabButton, ApplicationTabButton )
  EW_OBJECT  ( EvacuateButton,  ApplicationButton )
  EW_OBJECT  ( MuteButton,      ApplicationButton )
  EW_OBJECT  ( Image,           ViewsImage )
  EW_OBJECT  ( FaultsTabButton, ApplicationTabButton )
  EW_OBJECT  ( DisablementsTabButton, ApplicationTabButton )
  EW_OBJECT  ( OnTestTabButton, ApplicationTabButton )
  EW_OBJECT  ( EventsTabButton, ApplicationTabButton )
  EW_PROPERTY( SelectedTab,     XString )
  EW_OBJECT  ( ResetButton,     ApplicationButton )
  EW_OBJECT  ( ViewLogButton,   ApplicationButton )
  EW_OBJECT  ( LogOnButton,     ApplicationButton )
  EW_OBJECT  ( DeviceList,      ApplicationEvents )
  EW_OBJECT  ( Alarms,          ApplicationAlarms )
  EW_OBJECT  ( CauseAndEffects, ApplicationCauseAndEffects )
  EW_OBJECT  ( Test,            ApplicationTest )
  EW_OBJECT  ( Settings,        ApplicationSettings )
  EW_OBJECT  ( SystemPage,      ApplicationSystemPage )
  EW_OBJECT  ( Modify,          ApplicationModify )
  EW_OBJECT  ( Faults,          ApplicationFaults )
  EW_OBJECT  ( LevelAccess,     ApplicationLevelAccess )
  EW_OBJECT  ( AttrSet,         GraphicsAttrSet )
  EW_OBJECT  ( Timer,           CoreTimer )
  EW_PROPERTY( TestPop,         ApplicationPopup )
  EW_PROPERTY( DisablePop,      ApplicationPopup )
  EW_OBJECT  ( FireListChangeEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( StartProgressEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( Disablements,    ApplicationDisablements )
  EW_OBJECT  ( OnTest,          ApplicationOnTest )
  EW_OBJECT  ( EnableDisable,   ApplicationDisable )
  EW_OBJECT  ( MeshTree,        ApplicationMeshTree )
  EW_OBJECT  ( ProgressPopup,   ApplicationProgressPopup )
  EW_OBJECT  ( TabButtonPull,   ApplicationTabButtonPull )
  EW_OBJECT  ( WriteProtectedEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( DFUPopup,        ApplicationDFUPopup )
  EW_OBJECT  ( DFUEventHandler, CoreSystemEventHandler )
EW_END_OF_FIELDS( ApplicationApplication )

/* Virtual Method Table (VMT) for the class : 'Application::Application' */
EW_DEFINE_METHODS( ApplicationApplication, CoreRoot )
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
  EW_METHOD( DispatchEvent,     XObject )( CoreRoot _this, CoreEvent aEvent )
  EW_METHOD( BroadcastEvent,    XObject )( CoreRoot _this, CoreEvent aEvent, XSet 
    aFilter )
  EW_METHOD( UpdateLayout,      void )( CoreGroup _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( CoreGroup _this, XSet aState )
  EW_METHOD( InvalidateArea,    void )( CoreRoot _this, XRect aArea )
  EW_METHOD( GetIndexOfView,    XInt32 )( CoreGroup _this, CoreView aView )
  EW_METHOD( FindSiblingView,   CoreView )( CoreGroup _this, CoreView aView, XSet 
    aFilter )
  EW_METHOD( RestackBehind,     void )( CoreRoot _this, CoreView aView, CoreView 
    aSibling )
  EW_METHOD( RestackTop,        void )( CoreRoot _this, CoreView aView )
  EW_METHOD( Restack,           void )( CoreRoot _this, CoreView aView, XInt32 aOrder )
  EW_METHOD( Remove,            void )( CoreGroup _this, CoreView aView )
  EW_METHOD( Add,               void )( CoreRoot _this, CoreView aView, XInt32 aOrder )
EW_END_OF_METHODS( ApplicationApplication )

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void ApplicationApplication_Init( ApplicationApplication _this, XHandle aArg );

/* 'C' function for method : 'Application::Application.AlarmsEnter()' */
void ApplicationApplication_AlarmsEnter( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.DisablementsEnter()' */
void ApplicationApplication_DisablementsEnter( ApplicationApplication _this, XObject 
  sender );

/* 'C' function for method : 'Application::Application.DisablementsExit()' */
void ApplicationApplication_DisablementsExit( ApplicationApplication _this, XObject 
  sender );

/* 'C' function for method : 'Application::Application.EventsEnter()' */
void ApplicationApplication_EventsEnter( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.EventsExit()' */
void ApplicationApplication_EventsExit( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.AlarmsExit()' */
void ApplicationApplication_AlarmsExit( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.PageSelection()' */
void ApplicationApplication_PageSelection( ApplicationApplication _this, XObject 
  sender );

/* 'C' function for method : 'Application::Application.PageDeselection()' */
void ApplicationApplication_PageDeselection( ApplicationApplication _this );

/* 'C' function for method : 'Application::Application.FaultsEnter()' */
void ApplicationApplication_FaultsEnter( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.FaultsExit()' */
void ApplicationApplication_FaultsExit( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.OnTestEnter()' */
void ApplicationApplication_OnTestEnter( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.OnTestExit()' */
void ApplicationApplication_OnTestExit( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.Mute()' */
void ApplicationApplication_Mute( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.Tick()' */
void ApplicationApplication_Tick( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.Evacuate()' */
void ApplicationApplication_Evacuate( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.LogOn()' */
void ApplicationApplication_LogOn( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.Reset()' */
void ApplicationApplication_Reset( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.LowLevel()' */
void ApplicationApplication_LowLevel( ApplicationApplication _this );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void ApplicationApplication_onFireEvent( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.Reset1()' */
void ApplicationApplication_Reset1( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.Evacuate1()' */
void ApplicationApplication_Evacuate1( ApplicationApplication _this, XObject sender );

/* 'C' function for method : 'Application::Application.Silence()' */
void ApplicationApplication_Silence( ApplicationApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void ApplicationApplication_onEvent( ApplicationApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void ApplicationApplication_onEvent1( ApplicationApplication _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void ApplicationApplication_onEvent2( ApplicationApplication _this, XObject sender );

/* Default onget method for the property 'SelectedTab' */
XString ApplicationApplication_OnGetSelectedTab( ApplicationApplication _this );

/* Default onset method for the property 'SelectedTab' */
void ApplicationApplication_OnSetSelectedTab( ApplicationApplication _this, XString 
  value );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationApplication_H */

/* Embedded Wizard */
