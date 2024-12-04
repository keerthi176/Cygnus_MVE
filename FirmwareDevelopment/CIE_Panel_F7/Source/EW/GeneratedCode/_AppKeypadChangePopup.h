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

#ifndef _AppKeypadChangePopup_H
#define _AppKeypadChangePopup_H

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

#include "_AppKeypadKey.h"
#include "_AppPopup.h"
#include "_AppWritable.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreSystemEventHandler.h"
#include "_CoreTimer.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class App::KeypadChangePopup */
#ifndef _AppKeypadChangePopup_
  EW_DECLARE_CLASS( AppKeypadChangePopup )
#define _AppKeypadChangePopup_
#endif

/* Forward declaration of the class App::TextEditor */
#ifndef _AppTextEditor_
  EW_DECLARE_CLASS( AppTextEditor )
#define _AppTextEditor_
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


/* Deklaration of class : 'App::KeypadChangePopup' */
EW_DEFINE_FIELDS( AppKeypadChangePopup, AppPopup )
  EW_OBJECT  ( KeypadKey,       AppKeypadKey )
  EW_OBJECT  ( KeypadKey1,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey2,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey3,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey4,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey5,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey6,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey7,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey8,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey9,      AppKeypadKey )
  EW_OBJECT  ( KeypadKey10,     AppKeypadKey )
  EW_OBJECT  ( Writable,        AppWritable )
  EW_OBJECT  ( Text,            ViewsText )
  EW_OBJECT  ( AcceptTimer,     CoreTimer )
  EW_OBJECT  ( StarTimer,       CoreTimer )
  EW_PROPERTY( passkey1,        XString )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( deny2,           ViewsText )
  EW_OBJECT  ( accept2,         ViewsText )
  EW_OBJECT  ( Writable1,       AppWritable )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_PROPERTY( editor,          AppTextEditor )
  EW_OBJECT  ( deny3,           ViewsText )
  EW_PROPERTY( passkey2,        XString )
  EW_PROPERTY( passkey,         XRef )
  EW_OBJECT  ( DenyTimer,       CoreTimer )
  EW_OBJECT  ( deny4,           ViewsText )
  EW_PROPERTY( getting,         XBool )
EW_END_OF_FIELDS( AppKeypadChangePopup )

/* Virtual Method Table (VMT) for the class : 'App::KeypadChangePopup' */
EW_DEFINE_METHODS( AppKeypadChangePopup, AppPopup )
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
EW_END_OF_METHODS( AppKeypadChangePopup )

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppKeypadChangePopup_Init( AppKeypadChangePopup _this, XHandle aArg );

/* 'C' function for method : 'App::KeypadChangePopup.Press()' */
void AppKeypadChangePopup_Press( AppKeypadChangePopup _this, XObject sender );

/* 'C' function for method : 'App::KeypadChangePopup.Proceeed()' */
void AppKeypadChangePopup_Proceeed( AppKeypadChangePopup _this, XObject sender );

/* 'C' function for method : 'App::KeypadChangePopup.Cancel()' */
void AppKeypadChangePopup_Cancel( AppKeypadChangePopup _this, XObject sender );

/* 'C' function for method : 'App::KeypadChangePopup.AcceptSlot()' */
void AppKeypadChangePopup_AcceptSlot( AppKeypadChangePopup _this, XObject sender );

/* 'C' function for method : 'App::KeypadChangePopup.StarSlot()' */
void AppKeypadChangePopup_StarSlot( AppKeypadChangePopup _this, XObject sender );

/* 'C' function for method : 'App::KeypadChangePopup.Update()' */
void AppKeypadChangePopup_Update( AppKeypadChangePopup _this );

/* 'C' function for method : 'App::KeypadChangePopup.OnSeteditor()' */
void AppKeypadChangePopup_OnSeteditor( AppKeypadChangePopup _this, AppTextEditor 
  value );

/* 'C' function for method : 'App::KeypadChangePopup.DenySlot()' */
void AppKeypadChangePopup_DenySlot( AppKeypadChangePopup _this, XObject sender );

/* 'C' function for method : 'App::KeypadChangePopup.PressTop()' */
void AppKeypadChangePopup_PressTop( AppKeypadChangePopup _this, XObject sender );

/* 'C' function for method : 'App::KeypadChangePopup.PressBot()' */
void AppKeypadChangePopup_PressBot( AppKeypadChangePopup _this, XObject sender );

/* Default onget method for the property 'passkey1' */
XString AppKeypadChangePopup_OnGetpasskey1( AppKeypadChangePopup _this );

/* Default onset method for the property 'passkey1' */
void AppKeypadChangePopup_OnSetpasskey1( AppKeypadChangePopup _this, XString value );

/* Default onget method for the property 'passkey2' */
XString AppKeypadChangePopup_OnGetpasskey2( AppKeypadChangePopup _this );

/* Default onset method for the property 'passkey2' */
void AppKeypadChangePopup_OnSetpasskey2( AppKeypadChangePopup _this, XString value );

#ifdef __cplusplus
  }
#endif

#endif /* _AppKeypadChangePopup_H */

/* Embedded Wizard */
