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

#ifndef _ApplicationMeshInit_H
#define _ApplicationMeshInit_H

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
#include "_CoreSlideTouchHandler.h"
#include "_CoreSystemEventHandler.h"
#include "_CoreTimer.h"
#include "_CoreVerticalList.h"
#include "_WidgetSetHorizontalValueBar.h"
#include "_WidgetSetHorizontalValueBarConfig.h"
#include "_WidgetSetPushButton.h"
#include "_WidgetSetVerticalScrollbar.h"

/* Forward declaration of the class Application::MeshInit */
#ifndef _ApplicationMeshInit_
  EW_DECLARE_CLASS( ApplicationMeshInit )
#define _ApplicationMeshInit_
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


/* Deklaration of class : 'Application::MeshInit' */
EW_DEFINE_FIELDS( ApplicationMeshInit, CoreGroup )
  EW_OBJECT  ( Timer,           CoreTimer )
  EW_OBJECT  ( VerticalList,    CoreVerticalList )
  EW_OBJECT  ( SlideTouchHandler, CoreSlideTouchHandler )
  EW_OBJECT  ( SiteLoadedEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( MeshListChangeEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( MeshStatusChangeEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( Timer1,          CoreTimer )
  EW_OBJECT  ( Timer2,          CoreTimer )
  EW_OBJECT  ( VertScrollbar,   WidgetSetVerticalScrollbar )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( PushButton2,     WidgetSetPushButton )
  EW_OBJECT  ( HorizontalValueBar, WidgetSetHorizontalValueBar )
  EW_OBJECT  ( PushButton3,     WidgetSetPushButton )
  EW_OBJECT  ( HorizontalValueBar1, WidgetSetHorizontalValueBar )
  EW_OBJECT  ( PushButton4,     WidgetSetPushButton )
  EW_OBJECT  ( HorizontalValueBar2, WidgetSetHorizontalValueBar )
  EW_OBJECT  ( HorizontalValueBar_Small, WidgetSetHorizontalValueBarConfig )
EW_END_OF_FIELDS( ApplicationMeshInit )

/* Virtual Method Table (VMT) for the class : 'Application::MeshInit' */
EW_DEFINE_METHODS( ApplicationMeshInit, CoreGroup )
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
  EW_METHOD( UpdateLayout,      void )( ApplicationMeshInit _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( ApplicationMeshInit _this, XSet aState )
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
EW_END_OF_METHODS( ApplicationMeshInit )

/* The method UpdateLayout() is invoked automatically after the size of the component 
   has been changed. This method can be overridden and filled with logic to perform 
   a sophisticated arrangement calculation for one or more enclosed views. In this 
   case the parameter aSize can be used. It contains the current size of the component. 
   Usually, all enclosed views are arranged automatically accordingly to their @Layout 
   property. UpdateLayout() gives the derived components a chance to extend this 
   automatism by a user defined algorithm. */
void ApplicationMeshInit_UpdateLayout( ApplicationMeshInit _this, XPoint aSize );

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
void ApplicationMeshInit_UpdateViewState( ApplicationMeshInit _this, XSet aState );

/* 'C' function for method : 'Application::MeshInit.TickSlot()' */
void ApplicationMeshInit_TickSlot( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.ImportSlot()' */
void ApplicationMeshInit_ImportSlot( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.ExportSlot()' */
void ApplicationMeshInit_ExportSlot( ApplicationMeshInit _this, XObject sender );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void ApplicationMeshInit_OnLoadItem( ApplicationMeshInit _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void ApplicationMeshInit_onEvent1( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.StartSync()' */
void ApplicationMeshInit_StartSync( ApplicationMeshInit _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void ApplicationMeshInit_onEvent2( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.StartFormation()' */
void ApplicationMeshInit_StartFormation( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.StartActivation()' */
void ApplicationMeshInit_StartActivation( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.TickSlot1()' */
void ApplicationMeshInit_TickSlot1( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.TickSlot2()' */
void ApplicationMeshInit_TickSlot2( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.VScrollSlot()' */
void ApplicationMeshInit_VScrollSlot( ApplicationMeshInit _this, XObject sender );

/* 'C' function for method : 'Application::MeshInit.TScrollSlot()' */
void ApplicationMeshInit_TScrollSlot( ApplicationMeshInit _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationMeshInit_H */

/* Embedded Wizard */
