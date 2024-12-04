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

#ifndef _AppRadioSetup_H
#define _AppRadioSetup_H

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
#include "_CoreVerticalList.h"
#include "_ViewsBevel.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"
#include "_WidgetSetToggleButton.h"

/* Forward declaration of the class App::RadioSetup */
#ifndef _AppRadioSetup_
  EW_DECLARE_CLASS( AppRadioSetup )
#define _AppRadioSetup_
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


/* Deklaration of class : 'App::RadioSetup' */
EW_DEFINE_FIELDS( AppRadioSetup, AppPopup )
  EW_OBJECT  ( Rectangle10,     ViewsRectangle )
  EW_OBJECT  ( Rectangle11,     ViewsRectangle )
  EW_OBJECT  ( Bevel,           ViewsBevel )
  EW_OBJECT  ( Bevel1,          ViewsBevel )
  EW_OBJECT  ( Text,            ViewsText )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( ToggleButton,    WidgetSetToggleButton )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( Text2,           ViewsText )
  EW_OBJECT  ( Text3,           ViewsText )
  EW_OBJECT  ( PushButton2,     WidgetSetPushButton )
  EW_OBJECT  ( Rectangle9,      ViewsRectangle )
  EW_OBJECT  ( VerticalList,    CoreVerticalList )
  EW_OBJECT  ( PushButton3,     WidgetSetPushButton )
  EW_OBJECT  ( AcumeshEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( Text4,           ViewsText )
  EW_OBJECT  ( DropDownMenu,    AppDropDownMenu )
EW_END_OF_FIELDS( AppRadioSetup )

/* Virtual Method Table (VMT) for the class : 'App::RadioSetup' */
EW_DEFINE_METHODS( AppRadioSetup, AppPopup )
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
  EW_METHOD( UpdateViewState,   void )( AppRadioSetup _this, XSet aState )
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
EW_END_OF_METHODS( AppRadioSetup )

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
void AppRadioSetup_UpdateViewState( AppRadioSetup _this, XSet aState );

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppRadioSetup_Init( AppRadioSetup _this, XHandle aArg );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void AppRadioSetup_OnLoadItem( AppRadioSetup _this, XObject sender );

/* 'C' function for method : 'App::RadioSetup.Up()' */
void AppRadioSetup_Up( AppRadioSetup _this, XObject sender );

/* 'C' function for method : 'App::RadioSetup.Down()' */
void AppRadioSetup_Down( AppRadioSetup _this, XObject sender );

/* 'C' function for method : 'App::RadioSetup.Encrpt()' */
void AppRadioSetup_Encrpt( AppRadioSetup _this, XObject sender );

/* 'C' function for method : 'App::RadioSetup.Configure()' */
void AppRadioSetup_Configure( AppRadioSetup _this, XObject sender );

/* 'C' function for method : 'App::RadioSetup.AddLine()' */
void AppRadioSetup_AddLine( AppRadioSetup _this, XString s );

/* 'C' function for method : 'App::RadioSetup.Close()' */
void AppRadioSetup_Close( AppRadioSetup _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler1' 
   receives an event. */
void AppRadioSetup_onEvent( AppRadioSetup _this, XObject sender );

/* 'C' function for method : 'App::RadioSetup.GetItems()' */
void AppRadioSetup_GetItems( AppRadioSetup _this, XObject sender );

/* 'C' function for method : 'App::RadioSetup.Select()' */
void AppRadioSetup_Select( AppRadioSetup _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _AppRadioSetup_H */

/* Embedded Wizard */
