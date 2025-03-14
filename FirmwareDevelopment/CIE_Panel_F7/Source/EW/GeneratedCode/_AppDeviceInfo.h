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

#ifndef _AppDeviceInfo_H
#define _AppDeviceInfo_H

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

#include "_AppRoundBox.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreTimer.h"
#include "_GraphicsPath.h"
#include "_ViewsFillPath.h"
#include "_ViewsStrokePath.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class App::DeviceInfo */
#ifndef _AppDeviceInfo_
  EW_DECLARE_CLASS( AppDeviceInfo )
#define _AppDeviceInfo_
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


/* Deklaration of class : 'App::DeviceInfo' */
EW_DEFINE_FIELDS( AppDeviceInfo, AppRoundBox )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( Text3,           ViewsText )
  EW_OBJECT  ( Text4,           ViewsText )
  EW_OBJECT  ( Text5,           ViewsText )
  EW_OBJECT  ( rssi,            ViewsText )
  EW_OBJECT  ( secondary,       ViewsText )
  EW_OBJECT  ( status,          ViewsText )
  EW_OBJECT  ( battery,         ViewsText )
  EW_OBJECT  ( Text11,          ViewsText )
  EW_OBJECT  ( live,            ViewsText )
  EW_PROPERTY( device,          XInt32 )
  EW_OBJECT  ( stats_4,         ViewsText )
  EW_OBJECT  ( stats_sent,      ViewsText )
  EW_OBJECT  ( Timer,           CoreTimer )
  EW_OBJECT  ( stats_1,         ViewsText )
  EW_OBJECT  ( stats_3,         ViewsText )
  EW_OBJECT  ( stats_lost,      ViewsText )
  EW_OBJECT  ( stats_success,   ViewsText )
  EW_OBJECT  ( Text6,           ViewsText )
  EW_OBJECT  ( battery1,        ViewsText )
  EW_OBJECT  ( SimpleTouchHandler, CoreSimpleTouchHandler )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( Text2,           ViewsText )
  EW_OBJECT  ( Text7,           ViewsText )
  EW_OBJECT  ( ptype,           ViewsText )
EW_END_OF_FIELDS( AppDeviceInfo )

/* Virtual Method Table (VMT) for the class : 'App::DeviceInfo' */
EW_DEFINE_METHODS( AppDeviceInfo, AppRoundBox )
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
  EW_METHOD( UpdateLayout,      void )( AppRoundBox _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( AppDeviceInfo _this, XSet aState )
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
EW_END_OF_METHODS( AppDeviceInfo )

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
void AppDeviceInfo_UpdateViewState( AppDeviceInfo _this, XSet aState );

/* 'C' function for method : 'App::DeviceInfo.OnSetdevice()' */
void AppDeviceInfo_OnSetdevice( AppDeviceInfo _this, XInt32 value );

/* 'C' function for method : 'App::DeviceInfo.Slot()' */
void AppDeviceInfo_Slot( AppDeviceInfo _this, XObject sender );

/* 'C' function for method : 'App::DeviceInfo.Reset()' */
void AppDeviceInfo_Reset( AppDeviceInfo _this, XObject sender );

/* 'C' function for method : 'App::DeviceInfo.Close()' */
void AppDeviceInfo_Close( AppDeviceInfo _this, XObject sender );

/* 'C' function for method : 'App::DeviceInfo.RemoveDevice()' */
void AppDeviceInfo_RemoveDevice( AppDeviceInfo _this, XObject sender );

/* 'C' function for method : 'App::DeviceInfo.RemoveDeviceGo()' */
void AppDeviceInfo_RemoveDeviceGo( AppDeviceInfo _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _AppDeviceInfo_H */

/* Embedded Wizard */
