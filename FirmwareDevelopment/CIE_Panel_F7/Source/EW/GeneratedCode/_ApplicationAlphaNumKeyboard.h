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
* Version  : 8.30
* Profile  : STM32F746
* Platform : STM.STM32.RGB565
*
*******************************************************************************/

#ifndef _ApplicationAlphaNumKeyboard_H
#define _ApplicationAlphaNumKeyboard_H

#ifdef __cplusplus
  extern "C"
  {
#endif

#include "ewrte.h"
#if EW_RTE_VERSION != 0x0008001E
  #error Wrong version of Embedded Wizard Runtime Environment.
#endif

#include "ewgfx.h"
#if EW_GFX_VERSION != 0x0008001E
  #error Wrong version of Embedded Wizard Graphics Engine.
#endif

#include "_CoreSimpleTouchHandler.h"
#include "_CoreTimer.h"
#include "_TemplatesAlphaNumKeyboard.h"
#include "_ViewsBorder.h"
#include "_ViewsImage.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"

/* Forward declaration of the class Application::AlphaNumKeyboard */
#ifndef _ApplicationAlphaNumKeyboard_
  EW_DECLARE_CLASS( ApplicationAlphaNumKeyboard )
#define _ApplicationAlphaNumKeyboard_
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


/* This component implements a virtual keyboard. */
EW_DEFINE_FIELDS( ApplicationAlphaNumKeyboard, TemplatesAlphaNumKeyboard )
  EW_VARIABLE( layoutShiftOn,   XString )
  EW_VARIABLE( layoutShiftOff,  XString )
  EW_VARIABLE( keyView,         CoreView )
  EW_VARIABLE( recentPosition,  XPoint )
  EW_OBJECT  ( FlashTimer,      CoreTimer )
  EW_OBJECT  ( Background,      ViewsRectangle )
  EW_OBJECT  ( Border,          ViewsBorder )
  EW_OBJECT  ( TouchHandler,    CoreSimpleTouchHandler )
  EW_OBJECT  ( AreaKeySpace,    ViewsBorder )
  EW_OBJECT  ( ActiveKeyShiftRight, ViewsBorder )
  EW_OBJECT  ( ActiveKeyShiftLeft, ViewsBorder )
  EW_OBJECT  ( ActiveKeyCapsLock, ViewsBorder )
  EW_OBJECT  ( Highlight,       ViewsRectangle )
  EW_OBJECT  ( TextKey01,       ViewsText )
  EW_OBJECT  ( TextKey02,       ViewsText )
  EW_OBJECT  ( TextKey03,       ViewsText )
  EW_OBJECT  ( TextKey04,       ViewsText )
  EW_OBJECT  ( TextKey05,       ViewsText )
  EW_OBJECT  ( TextKey06,       ViewsText )
  EW_OBJECT  ( TextKey07,       ViewsText )
  EW_OBJECT  ( TextKey08,       ViewsText )
  EW_OBJECT  ( TextKey09,       ViewsText )
  EW_OBJECT  ( TextKey10,       ViewsText )
  EW_OBJECT  ( TextKey11,       ViewsText )
  EW_OBJECT  ( TextKey12,       ViewsText )
  EW_OBJECT  ( TextKey13,       ViewsText )
  EW_OBJECT  ( TextKey14,       ViewsText )
  EW_OBJECT  ( TextKey15,       ViewsText )
  EW_OBJECT  ( TextKey16,       ViewsText )
  EW_OBJECT  ( TextKey17,       ViewsText )
  EW_OBJECT  ( TextKey18,       ViewsText )
  EW_OBJECT  ( TextKey19,       ViewsText )
  EW_OBJECT  ( TextKey20,       ViewsText )
  EW_OBJECT  ( TextKey21,       ViewsText )
  EW_OBJECT  ( TextKey22,       ViewsText )
  EW_OBJECT  ( TextKey23,       ViewsText )
  EW_OBJECT  ( TextKey24,       ViewsText )
  EW_OBJECT  ( TextKey25,       ViewsText )
  EW_OBJECT  ( TextKey26,       ViewsText )
  EW_OBJECT  ( TextKey27,       ViewsText )
  EW_OBJECT  ( TextKey28,       ViewsText )
  EW_OBJECT  ( TextKey29,       ViewsText )
  EW_OBJECT  ( TextKey30,       ViewsText )
  EW_OBJECT  ( TextKey31,       ViewsText )
  EW_OBJECT  ( TextKey32,       ViewsText )
  EW_OBJECT  ( TextKey33,       ViewsText )
  EW_OBJECT  ( TextKey34,       ViewsText )
  EW_OBJECT  ( TextKey35,       ViewsText )
  EW_OBJECT  ( TextKey36,       ViewsText )
  EW_OBJECT  ( TextKey37,       ViewsText )
  EW_OBJECT  ( TextKey38,       ViewsText )
  EW_OBJECT  ( TextKey39,       ViewsText )
  EW_OBJECT  ( TextKey40,       ViewsText )
  EW_OBJECT  ( TextKey41,       ViewsText )
  EW_OBJECT  ( TextKey42,       ViewsText )
  EW_OBJECT  ( TextKey43,       ViewsText )
  EW_OBJECT  ( TextKey44,       ViewsText )
  EW_OBJECT  ( TextKey45,       ViewsText )
  EW_OBJECT  ( TextKey46,       ViewsText )
  EW_OBJECT  ( TextKey47,       ViewsText )
  EW_OBJECT  ( ImageKeyTab,     ViewsImage )
  EW_OBJECT  ( ImageKeyCapsLock, ViewsImage )
  EW_OBJECT  ( ImageKeyShiftLeft, ViewsImage )
  EW_OBJECT  ( ImageKeyClear,   ViewsImage )
  EW_OBJECT  ( ImageKeyEnter,   ViewsImage )
  EW_OBJECT  ( ImageKeyShiftRight, ViewsImage )
EW_END_OF_FIELDS( ApplicationAlphaNumKeyboard )

/* Virtual Method Table (VMT) for the class : 'Application::AlphaNumKeyboard' */
EW_DEFINE_METHODS( ApplicationAlphaNumKeyboard, TemplatesAlphaNumKeyboard )
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
  EW_METHOD( UpdateLayout,      void )( CoreGroup _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( ApplicationAlphaNumKeyboard _this, XSet 
    aState )
  EW_METHOD( InvalidateArea,    void )( CoreGroup _this, XRect aArea )
  EW_METHOD( FindViewAtPosition, CoreView )( CoreGroup _this, CoreView aView, XPoint 
    aPosition, XSet aFilter )
  EW_METHOD( FindNextView,      CoreView )( CoreGroup _this, CoreView aView, XSet 
    aFilter )
  EW_METHOD( FindSiblingView,   CoreView )( CoreGroup _this, CoreView aView, XSet 
    aFilter )
  EW_METHOD( Restack,           void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )
  EW_METHOD( Remove,            void )( CoreGroup _this, CoreView aView )
  EW_METHOD( Add,               void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )
EW_END_OF_METHODS( ApplicationAlphaNumKeyboard )

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
void ApplicationAlphaNumKeyboard_UpdateViewState( ApplicationAlphaNumKeyboard _this, 
  XSet aState );

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void ApplicationAlphaNumKeyboard_Init( ApplicationAlphaNumKeyboard _this, XHandle 
  aArg );

/* This method loads new keyboard layout. The layout is determined by the passed 
   string parameter aLayoutString. The characters within the string correspond to 
   the keys accordingly their order.
   The order in which the characters are listed in the string corresponds to the 
   z-order of the 'text' views 'TextKey01' .. 'TextKey47'. */
void ApplicationAlphaNumKeyboard_loadLayoutFromString( ApplicationAlphaNumKeyboard _this, 
  XString aLayoutString );

/* This internal slot method is called when the recently pressed key should be activated. 
   This is when the user presses on a key and then releases the finger again. */
void ApplicationAlphaNumKeyboard_activateKey( ApplicationAlphaNumKeyboard _this, 
  XObject sender );

/* This internal slot method is called when the '@FlashTimer' is expired. It ends 
   the short flash feedback effect. */
void ApplicationAlphaNumKeyboard_onFlashTimer( ApplicationAlphaNumKeyboard _this, 
  XObject sender );

/* This internal slot method is called when the user drags the finger while pressing 
   the keyboard. This updates the keyboard to highlight the key at the new touch 
   position. */
void ApplicationAlphaNumKeyboard_onDragTouch( ApplicationAlphaNumKeyboard _this, 
  XObject sender );

/* This internal slot method is called when the user releases the touch screen after 
   touching the keyboard area. This activates the key the user has touched. */
void ApplicationAlphaNumKeyboard_onReleaseTouch( ApplicationAlphaNumKeyboard _this, 
  XObject sender );

/* This internal slot method is called when the user touches the keyboard area. */
void ApplicationAlphaNumKeyboard_onPressTouch( ApplicationAlphaNumKeyboard _this, 
  XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationAlphaNumKeyboard_H */

/* Embedded Wizard */
