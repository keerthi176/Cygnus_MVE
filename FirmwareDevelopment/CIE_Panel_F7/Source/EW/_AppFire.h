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

#ifndef _AppFire_H
#define _AppFire_H

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

#include "_AppListViewer.h"
#include "_CoreGroup.h"
#include "_CoreSystemEventHandler.h"
#include "_ViewsImage.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class App::Fire */
#ifndef _AppFire_
  EW_DECLARE_CLASS( AppFire )
#define _AppFire_
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


/* Deklaration of class : 'App::Fire' */
EW_DEFINE_FIELDS( AppFire, CoreGroup )
  EW_OBJECT  ( ResoundAlarmsButton, WidgetSetPushButton )
  EW_OBJECT  ( ConfirmFireButton, WidgetSetPushButton )
  EW_OBJECT  ( AcknowledgeFireButton, WidgetSetPushButton )
  EW_OBJECT  ( LogoImage,       ViewsImage )
  EW_OBJECT  ( ListViewer,      AppListViewer )
  EW_OBJECT  ( NotOperationalBitmap, ViewsImage )
  EW_OBJECT  ( NotOperationalText, ViewsText )
  EW_OBJECT  ( NewBitmapLogoEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( ResoundStateEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( ResoundAlarmsButton1, WidgetSetPushButton )
  EW_VARIABLE( numZonesInFire,  XInt32 )
EW_END_OF_FIELDS( AppFire )

/* Virtual Method Table (VMT) for the class : 'App::Fire' */
EW_DEFINE_METHODS( AppFire, CoreGroup )
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
  EW_METHOD( UpdateLayout,      void )( AppFire _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( AppFire _this, XSet aState )
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
EW_END_OF_METHODS( AppFire )

/* The method UpdateLayout() is invoked automatically after the size of the component 
   has been changed. This method can be overridden and filled with logic to perform 
   a sophisticated arrangement calculation for one or more enclosed views. In this 
   case the parameter aSize can be used. It contains the current size of the component. 
   Usually, all enclosed views are arranged automatically accordingly to their @Layout 
   property. UpdateLayout() gives the derived components a chance to extend this 
   automatism by a user defined algorithm. */
void AppFire_UpdateLayout( AppFire _this, XPoint aSize );

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
void AppFire_UpdateViewState( AppFire _this, XSet aState );

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppFire_Init( AppFire _this, XHandle aArg );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void AppFire_OnLoadItem( AppFire _this, XObject sender );

/* 'C' function for method : 'App::Fire.ResoundSlot()' */
void AppFire_ResoundSlot( AppFire _this, XObject sender );

/* 'C' function for method : 'App::Fire.SetListSize()' */
XInt32 AppFire_SetListSize( AppFire _this );

/* 'C' function for method : 'App::Fire.ConfirmSlot()' */
void AppFire_ConfirmSlot( AppFire _this, XObject sender );

/* 'C' function for method : 'App::Fire.AckSlot()' */
void AppFire_AckSlot( AppFire _this, XObject sender );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void AppFire_OnRefreshAll( AppFire _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppFire_onEvent( AppFire _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppFire_onResoundStateEvent( AppFire _this, XObject sender );

/* 'C' function for method : 'App::Fire.ResetFire()' */
void AppFire_ResetFire( AppFire _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _AppFire_H */

/* Embedded Wizard */
