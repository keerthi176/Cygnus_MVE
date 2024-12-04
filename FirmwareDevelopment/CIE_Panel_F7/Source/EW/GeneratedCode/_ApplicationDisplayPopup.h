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

#ifndef _ApplicationDisplayPopup_H
#define _ApplicationDisplayPopup_H

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

#include "_ApplicationPopup.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class Application::DisplayPopup */
#ifndef _ApplicationDisplayPopup_
  EW_DECLARE_CLASS( ApplicationDisplayPopup )
#define _ApplicationDisplayPopup_
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


/* Deklaration of class : 'Application::DisplayPopup' */
EW_DEFINE_FIELDS( ApplicationDisplayPopup, ApplicationPopup )
  EW_OBJECT  ( Rectangle9,      ViewsRectangle )
  EW_OBJECT  ( Rectangle10,     ViewsRectangle )
  EW_OBJECT  ( Rectangle11,     ViewsRectangle )
  EW_OBJECT  ( Rectangle12,     ViewsRectangle )
  EW_OBJECT  ( Rectangle13,     ViewsRectangle )
  EW_OBJECT  ( Rectangle14,     ViewsRectangle )
  EW_OBJECT  ( Rectangle15,     ViewsRectangle )
  EW_OBJECT  ( Text1,           ViewsText )
  EW_OBJECT  ( Text2,           ViewsText )
  EW_OBJECT  ( Text3,           ViewsText )
  EW_OBJECT  ( Text4,           ViewsText )
  EW_OBJECT  ( Text5,           ViewsText )
  EW_OBJECT  ( Text6,           ViewsText )
  EW_OBJECT  ( Text7,           ViewsText )
  EW_OBJECT  ( Rectangle16,     ViewsRectangle )
  EW_OBJECT  ( Rectangle23,     ViewsRectangle )
  EW_OBJECT  ( Rectangle24,     ViewsRectangle )
  EW_OBJECT  ( Rectangle25,     ViewsRectangle )
  EW_OBJECT  ( Rectangle26,     ViewsRectangle )
  EW_OBJECT  ( Rectangle27,     ViewsRectangle )
  EW_OBJECT  ( Rectangle28,     ViewsRectangle )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
EW_END_OF_FIELDS( ApplicationDisplayPopup )

/* Virtual Method Table (VMT) for the class : 'Application::DisplayPopup' */
EW_DEFINE_METHODS( ApplicationDisplayPopup, ApplicationPopup )
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
EW_END_OF_METHODS( ApplicationDisplayPopup )

/* 'C' function for method : 'Application::DisplayPopup.Proceed()' */
void ApplicationDisplayPopup_Proceed( ApplicationDisplayPopup _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationDisplayPopup_H */

/* Embedded Wizard */
