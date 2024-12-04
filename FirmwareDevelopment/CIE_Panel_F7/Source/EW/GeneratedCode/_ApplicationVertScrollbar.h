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

#ifndef _ApplicationVertScrollbar_H
#define _ApplicationVertScrollbar_H

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

#include "_CoreSimpleTouchHandler.h"
#include "_TemplatesVerticalScrollbar.h"
#include "_ViewsBevel.h"
#include "_ViewsBorder.h"
#include "_ViewsImage.h"
#include "_ViewsRectangle.h"

/* Forward declaration of the class Application::VertScrollbar */
#ifndef _ApplicationVertScrollbar_
  EW_DECLARE_CLASS( ApplicationVertScrollbar )
#define _ApplicationVertScrollbar_
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


/* This class implements a 'scrollbar' widget. The widget is intended to show the 
   current position within a large document (e.g. a long list with thousand of items). 
   It is not intended to react to user interactions.
   When using the scrollbar you need to set its property @ContentArea to the size 
   of the entire document and the property @ViewArea to the size of the visible 
   portion of the document. The scrollbar will calculate from these both values 
   the size of the scrollbar thumb. The current scroll position within the document 
   is determined by the property @Position.
   Per default the scrollbar is invisible. It appears when the properties @ContentSize, 
   @ViewSize or @Position are changed. After a short delay the scrollbar disappears 
   again. */
EW_DEFINE_FIELDS( ApplicationVertScrollbar, TemplatesVerticalScrollbar )
  EW_OBJECT  ( Bevel,           ViewsBevel )
  EW_PROPERTY( Position,        XInt32 )
  EW_PROPERTY( ViewArea,        XInt32 )
  EW_PROPERTY( ContentArea,     XInt32 )
  EW_OBJECT  ( Rectangle2,      ViewsRectangle )
  EW_OBJECT  ( Thumb,           ViewsRectangle )
  EW_OBJECT  ( Rectangle,       ViewsRectangle )
  EW_OBJECT  ( SimpleTouchHandler, CoreSimpleTouchHandler )
  EW_OBJECT  ( Rectangle1,      ViewsRectangle )
  EW_OBJECT  ( SimpleTouchHandler1, CoreSimpleTouchHandler )
  EW_OBJECT  ( Rectangle3,      ViewsRectangle )
  EW_OBJECT  ( SimpleTouchHandler2, CoreSimpleTouchHandler )
  EW_OBJECT  ( Rectangle4,      ViewsRectangle )
  EW_OBJECT  ( SimpleTouchHandler3, CoreSimpleTouchHandler )
  EW_OBJECT  ( Border,          ViewsBorder )
  EW_OBJECT  ( Border1,         ViewsBorder )
  EW_OBJECT  ( Border2,         ViewsBorder )
  EW_OBJECT  ( Border3,         ViewsBorder )
  EW_OBJECT  ( Image,           ViewsImage )
  EW_OBJECT  ( Image1,          ViewsImage )
  EW_OBJECT  ( Image2,          ViewsImage )
  EW_OBJECT  ( Image3,          ViewsImage )
  EW_PROPERTY( ThumbSize,       XInt32 )
  EW_PROPERTY( Outlet,          XRef )
  EW_PROPERTY( OnChange,        XSlot )
  EW_OBJECT  ( SimpleTouchHandler4, CoreSimpleTouchHandler )
EW_END_OF_FIELDS( ApplicationVertScrollbar )

/* Virtual Method Table (VMT) for the class : 'Application::VertScrollbar' */
EW_DEFINE_METHODS( ApplicationVertScrollbar, TemplatesVerticalScrollbar )
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
  EW_METHOD( UpdateViewState,   void )( ApplicationVertScrollbar _this, XSet aState )
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
EW_END_OF_METHODS( ApplicationVertScrollbar )

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
void ApplicationVertScrollbar_UpdateViewState( ApplicationVertScrollbar _this, XSet 
  aState );

/* 'C' function for method : 'Application::VertScrollbar.OnGetPosition()' */
XInt32 ApplicationVertScrollbar_OnGetPosition( ApplicationVertScrollbar _this );

/* 'C' function for method : 'Application::VertScrollbar.OnSetPosition()' */
void ApplicationVertScrollbar_OnSetPosition( ApplicationVertScrollbar _this, XInt32 
  value );

/* 'C' function for method : 'Application::VertScrollbar.OnSetViewArea()' */
void ApplicationVertScrollbar_OnSetViewArea( ApplicationVertScrollbar _this, XInt32 
  value );

/* 'C' function for method : 'Application::VertScrollbar.OnSetContentArea()' */
void ApplicationVertScrollbar_OnSetContentArea( ApplicationVertScrollbar _this, 
  XInt32 value );

/* 'C' function for method : 'Application::VertScrollbar.Bottom()' */
void ApplicationVertScrollbar_Bottom( ApplicationVertScrollbar _this, XObject sender );

/* 'C' function for method : 'Application::VertScrollbar.Top()' */
void ApplicationVertScrollbar_Top( ApplicationVertScrollbar _this, XObject sender );

/* 'C' function for method : 'Application::VertScrollbar.Down()' */
void ApplicationVertScrollbar_Down( ApplicationVertScrollbar _this, XObject sender );

/* 'C' function for method : 'Application::VertScrollbar.Up()' */
void ApplicationVertScrollbar_Up( ApplicationVertScrollbar _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationVertScrollbar_H */

/* Embedded Wizard */
