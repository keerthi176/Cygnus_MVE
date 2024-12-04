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

#ifndef _ApplicationModifyPopup_H
#define _ApplicationModifyPopup_H

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
#include "_ApplicationMattsKeyboard.h"
#include "_ApplicationPopup.h"
#include "_ApplicationWritable.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class Application::DeviceConfig */
#ifndef _ApplicationDeviceConfig_
  EW_DECLARE_CLASS( ApplicationDeviceConfig )
#define _ApplicationDeviceConfig_
#endif

/* Forward declaration of the class Application::ModifyPopup */
#ifndef _ApplicationModifyPopup_
  EW_DECLARE_CLASS( ApplicationModifyPopup )
#define _ApplicationModifyPopup_
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


/* Deklaration of class : 'Application::ModifyPopup' */
EW_DEFINE_FIELDS( ApplicationModifyPopup, ApplicationPopup )
  EW_OBJECT  ( MattsKeyboard,   ApplicationMattsKeyboard )
  EW_OBJECT  ( Writable,        ApplicationWritable )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( InputChanButton, WidgetSetPushButton )
  EW_OBJECT  ( OutputChanButton, WidgetSetPushButton )
  EW_OBJECT  ( DropDownMenu,    ApplicationDropDownMenu )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_PROPERTY( device,          ApplicationDeviceConfig )
EW_END_OF_FIELDS( ApplicationModifyPopup )

/* Virtual Method Table (VMT) for the class : 'Application::ModifyPopup' */
EW_DEFINE_METHODS( ApplicationModifyPopup, ApplicationPopup )
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
EW_END_OF_METHODS( ApplicationModifyPopup )

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void ApplicationModifyPopup_Init( ApplicationModifyPopup _this, XHandle aArg );

/* 'C' function for method : 'Application::ModifyPopup.ProceedSlot()' */
void ApplicationModifyPopup_ProceedSlot( ApplicationModifyPopup _this, XObject sender );

/* 'C' function for method : 'Application::ModifyPopup.InputChannels()' */
void ApplicationModifyPopup_InputChannels( ApplicationModifyPopup _this, XObject 
  sender );

/* 'C' function for method : 'Application::ModifyPopup.OutputChannels()' */
void ApplicationModifyPopup_OutputChannels( ApplicationModifyPopup _this, XObject 
  sender );

/* 'C' function for method : 'Application::ModifyPopup.SetZoneSlot()' */
void ApplicationModifyPopup_SetZoneSlot( ApplicationModifyPopup _this, XObject sender );

/* 'C' function for method : 'Application::ModifyPopup.FillMenu()' */
void ApplicationModifyPopup_FillMenu( ApplicationModifyPopup _this, XObject sender );

/* 'C' function for method : 'Application::ModifyPopup.CancelSlot()' */
void ApplicationModifyPopup_CancelSlot( ApplicationModifyPopup _this, XObject sender );

/* 'C' function for method : 'Application::ModifyPopup.OnSetdevice()' */
void ApplicationModifyPopup_OnSetdevice( ApplicationModifyPopup _this, ApplicationDeviceConfig 
  value );

/* 'C' function for method : 'Application::ModifyPopup.SetFocus()' */
void ApplicationModifyPopup_SetFocus( ApplicationModifyPopup _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationModifyPopup_H */

/* Embedded Wizard */
