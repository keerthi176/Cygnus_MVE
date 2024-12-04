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

#ifndef _AppMeshLogPopup2_H
#define _AppMeshLogPopup2_H

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
#include "_AppPopup.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreSystemEventHandler.h"
#include "_ViewsRectangle.h"
#include "_WidgetSetPushButton.h"
#include "_WidgetSetToggleButton.h"

/* Forward declaration of the class App::MeshLogPopup2 */
#ifndef _AppMeshLogPopup2_
  EW_DECLARE_CLASS( AppMeshLogPopup2 )
#define _AppMeshLogPopup2_
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


/* Deklaration of class : 'App::MeshLogPopup2' */
EW_DEFINE_FIELDS( AppMeshLogPopup2, AppPopup )
  EW_OBJECT  ( Rectangle9,      ViewsRectangle )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( LogEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( ListViewer,      AppListViewer )
  EW_OBJECT  ( Sync,            WidgetSetToggleButton )
  EW_OBJECT  ( Faults,          WidgetSetToggleButton )
  EW_OBJECT  ( Broadcast,       WidgetSetToggleButton )
  EW_OBJECT  ( Fire,            WidgetSetToggleButton )
  EW_OBJECT  ( Settings,        WidgetSetToggleButton )
  EW_OBJECT  ( Mesh,            WidgetSetToggleButton )
  EW_OBJECT  ( Status,          WidgetSetToggleButton )
  EW_OBJECT  ( Misc,            WidgetSetToggleButton )
EW_END_OF_FIELDS( AppMeshLogPopup2 )

/* Virtual Method Table (VMT) for the class : 'App::MeshLogPopup2' */
EW_DEFINE_METHODS( AppMeshLogPopup2, AppPopup )
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
EW_END_OF_METHODS( AppMeshLogPopup2 )

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppMeshLogPopup2_Init( AppMeshLogPopup2 _this, XHandle aArg );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void AppMeshLogPopup2_OnLoadItem( AppMeshLogPopup2 _this, XObject sender );

/* 'C' function for method : 'App::MeshLogPopup2.Close()' */
void AppMeshLogPopup2_Close( AppMeshLogPopup2 _this, XObject sender );

/* 'C' function for method : 'App::MeshLogPopup2.GetFilter()' */
XInt32 AppMeshLogPopup2_GetFilter( AppMeshLogPopup2 _this );

/* 'C' function for method : 'App::MeshLogPopup2.ScrollRefresh()' */
void AppMeshLogPopup2_ScrollRefresh( AppMeshLogPopup2 _this, XObject sender );

/* 'C' function for method : 'App::MeshLogPopup2.ChckOnOff()' */
void AppMeshLogPopup2_ChckOnOff( AppMeshLogPopup2 _this, XObject sender );

/* 'C' function for method : 'App::MeshLogPopup2.RefreshAll()' */
void AppMeshLogPopup2_RefreshAll( AppMeshLogPopup2 _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _AppMeshLogPopup2_H */

/* Embedded Wizard */
