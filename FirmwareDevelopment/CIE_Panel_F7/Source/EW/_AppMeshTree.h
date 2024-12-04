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

#ifndef _AppMeshTree_H
#define _AppMeshTree_H

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

#include "_AppCountView.h"
#include "_AppDeviceInfo.h"
#include "_AppListViewer.h"
#include "_CoreGroup.h"
#include "_CoreSimpleTouchHandler.h"
#include "_CoreSystemEventHandler.h"
#include "_GraphicsPath.h"
#include "_ViewsStrokePath.h"
#include "_WidgetSetPushButton.h"

/* Forward declaration of the class App::DeviceConfig */
#ifndef _AppDeviceConfig_
  EW_DECLARE_CLASS( AppDeviceConfig )
#define _AppDeviceConfig_
#endif

/* Forward declaration of the class App::MeshTree */
#ifndef _AppMeshTree_
  EW_DECLARE_CLASS( AppMeshTree )
#define _AppMeshTree_
#endif

/* Forward declaration of the class App::MeshUnitItem */
#ifndef _AppMeshUnitItem_
  EW_DECLARE_CLASS( AppMeshUnitItem )
#define _AppMeshUnitItem_
#endif

/* Forward declaration of the class App::RadioUnit */
#ifndef _AppRadioUnit_
  EW_DECLARE_CLASS( AppRadioUnit )
#define _AppRadioUnit_
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


/* Deklaration of class : 'App::MeshTree' */
EW_DEFINE_FIELDS( AppMeshTree, CoreGroup )
  EW_OBJECT  ( Path,            GraphicsPath )
  EW_PROPERTY( Origin,          XPoint )
  EW_PROPERTY( Spacing,         XPoint )
  EW_PROPERTY( SubIndex,        XInt32 )
  EW_PROPERTY( Row,             XInt32 )
  EW_OBJECT  ( BranchPath,      ViewsStrokePath )
  EW_OBJECT  ( MeshListChangeEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( MeshStatusChangeEventHandler, CoreSystemEventHandler )
  EW_OBJECT  ( ListViewer,      AppListViewer )
  EW_OBJECT  ( SimpleTouchHandler, CoreSimpleTouchHandler )
  EW_PROPERTY( device,          AppMeshUnitItem )
  EW_OBJECT  ( SiteChangeEventHandler1, CoreSystemEventHandler )
  EW_OBJECT  ( PushButton3,     WidgetSetPushButton )
  EW_OBJECT  ( PushButton,      WidgetSetPushButton )
  EW_OBJECT  ( DetectedCountView, AppCountView )
  EW_OBJECT  ( ActiveCountView, AppCountView )
  EW_OBJECT  ( DeviceInfo,      AppDeviceInfo )
  EW_PROPERTY( current_item,    XInt32 )
  EW_OBJECT  ( PushButton1,     WidgetSetPushButton )
  EW_OBJECT  ( AccessModeChangeEventHandler1, CoreSystemEventHandler )
  EW_OBJECT  ( PushButton4,     WidgetSetPushButton )
  EW_OBJECT  ( MeshUnitChangeEventHandler, CoreSystemEventHandler )
  EW_PROPERTY( extra,           XInt32 )
  EW_PROPERTY( totalItems,      XInt16 )
EW_END_OF_FIELDS( AppMeshTree )

/* Virtual Method Table (VMT) for the class : 'App::MeshTree' */
EW_DEFINE_METHODS( AppMeshTree, CoreGroup )
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
  EW_METHOD( OnSetVisible,      void )( AppMeshTree _this, XBool value )
  EW_METHOD( DispatchEvent,     XObject )( CoreGroup _this, CoreEvent aEvent )
  EW_METHOD( BroadcastEvent,    XObject )( CoreGroup _this, CoreEvent aEvent, XSet 
    aFilter )
  EW_METHOD( UpdateLayout,      void )( AppMeshTree _this, XPoint aSize )
  EW_METHOD( UpdateViewState,   void )( AppMeshTree _this, XSet aState )
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
EW_END_OF_METHODS( AppMeshTree )

/* 'C' function for method : 'App::MeshTree.OnSetVisible()' */
void AppMeshTree_OnSetVisible( AppMeshTree _this, XBool value );

/* The method UpdateLayout() is invoked automatically after the size of the component 
   has been changed. This method can be overridden and filled with logic to perform 
   a sophisticated arrangement calculation for one or more enclosed views. In this 
   case the parameter aSize can be used. It contains the current size of the component. 
   Usually, all enclosed views are arranged automatically accordingly to their @Layout 
   property. UpdateLayout() gives the derived components a chance to extend this 
   automatism by a user defined algorithm. */
void AppMeshTree_UpdateLayout( AppMeshTree _this, XPoint aSize );

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
void AppMeshTree_UpdateViewState( AppMeshTree _this, XSet aState );

/* The method Init() is invoked automatically after the component has been created. 
   This method can be overridden and filled with logic containing additional initialization 
   statements. */
void AppMeshTree_Init( AppMeshTree _this, XHandle aArg );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void AppMeshTree_OnLoadItem( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.DrawBranch()' */
void AppMeshTree_DrawBranch( AppMeshTree _this, XInt32 indent, XInt32 parent );

/* 'C' function for method : 'App::MeshTree.FindNextChild()' */
XInt32 AppMeshTree_FindNextChild( AppMeshTree _this, XInt32 start, XInt32 parent );

/* 'C' function for method : 'App::MeshTree.GetChildren()' */
XBool AppMeshTree_GetChildren( AppMeshTree _this, AppMeshUnitItem device, XInt32 
  parent, XInt32 itemNo, XInt32 indent );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppMeshTree_onEvent1( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.StartSync()' */
void AppMeshTree_StartSync( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.ScrollSlot()' */
void AppMeshTree_ScrollSlot( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.InfoTap()' */
void AppMeshTree_InfoTap( AppMeshTree _this, XObject sender );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void AppMeshTree_RefreshAll( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.GetAllChildren()' */
XBool AppMeshTree_GetAllChildren( AppMeshTree _this, XInt32 parent, XInt32 itemNo, 
  XInt32 indent );

/* 'C' function for method : 'App::MeshTree.DrawLine()' */
void AppMeshTree_DrawLine( AppMeshTree _this, XInt32 x, XInt32 y, XInt32 y2 );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppMeshTree_onEvent3( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.StartSyncGo()' */
void AppMeshTree_StartSyncGo( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.AdditionalSetSlot()' */
void AppMeshTree_AdditionalSetSlot( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.OptimizeMesh()' */
void AppMeshTree_OptimizeMesh( AppMeshTree _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppMeshTree_onEvent2( AppMeshTree _this, XObject sender );

/* This method is called by 'VerticalList' every time the list loads or updates 
   an item. */
void AppMeshTree_CheckFW( AppMeshTree _this, XObject sender );

/* This slot method is executed when the associated system event handler 'SystemEventHandler' 
   receives an event. */
void AppMeshTree_onMeshUnitChangeEvent( AppMeshTree _this, XObject sender );

/* 'C' function for method : 'App::MeshTree.SetupUnit()' */
void AppMeshTree_SetupUnit( AppMeshTree _this, AppMeshUnitItem device, AppRadioUnit 
  ru, AppDeviceConfig dev, XInt16 selected );

#ifdef __cplusplus
  }
#endif

#endif /* _AppMeshTree_H */

/* Embedded Wizard */
