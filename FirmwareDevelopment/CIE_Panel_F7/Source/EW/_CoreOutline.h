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

#ifndef _CoreOutline_H
#define _CoreOutline_H

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

#include "_CoreRectView.h"

/* Forward declaration of the class Core::Group */
#ifndef _CoreGroup_
  EW_DECLARE_CLASS( CoreGroup )
#define _CoreGroup_
#endif

/* Forward declaration of the class Core::LayoutContext */
#ifndef _CoreLayoutContext_
  EW_DECLARE_CLASS( CoreLayoutContext )
#define _CoreLayoutContext_
#endif

/* Forward declaration of the class Core::Outline */
#ifndef _CoreOutline_
  EW_DECLARE_CLASS( CoreOutline )
#define _CoreOutline_
#endif

/* Forward declaration of the class Core::SlideTouchHandler */
#ifndef _CoreSlideTouchHandler_
  EW_DECLARE_CLASS( CoreSlideTouchHandler )
#define _CoreSlideTouchHandler_
#endif

/* Forward declaration of the class Core::View */
#ifndef _CoreView_
  EW_DECLARE_CLASS( CoreView )
#define _CoreView_
#endif

/* Forward declaration of the class Effects::PointEffect */
#ifndef _EffectsPointEffect_
  EW_DECLARE_CLASS( EffectsPointEffect )
#define _EffectsPointEffect_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif


/* The class Core::Outline provides a special kind of a view able to unite and control 
   its sibling views. It acts like a lightweight container for the views without 
   being the owner of them:
   - Outline can control only views lying in front of it (views with higher Z-order). 
   Furthermore the affected views have to be signed explicitly with the state 'Embedded'. 
   Views lying behind the outline or views not signed as embedded are ignored by 
   the outline - they are controlled by their @Owner.
   - Outline limits the visibility of the embedded views to its own boundary area. 
   In this manner the views are clipped at the outlines edges.
   - Moving the outline causes the embedded views to be moved accordingly. Together 
   with the outline the views behave like a single union.
   - The embedded views can be scrolled within the outline boundary area. The property 
   @ScrollOffset determines the desired displacement.
   - A Core::SlideTouchHandler can be attached to the outline so the user can scroll 
   the outline content interactively by touching the screen. The property @SlideHandler 
   allows it. 
   - If the size of the outline changes, all embedded views are automatically arranged 
   within the outline boundary accordingly to their @Layout properties.
   - The property @Formation allows more sophisticated arrangement of embedded views 
   in columns and rows. It is useful to create scrollable menus or lists.
   - To find and enumerate the embedded views, the methods like @FindNextView(), 
   @FindPrevView(), @FindViewAtPosition, @FindViewInDirection() or @GetViewAtIndex() 
   are available. 
   The outline itself is invisible except the Embedded Wizard Composer, where all 
   outlines appear as semitransparent rectangles. This allows you to interact with 
   the outlines during the design time. */
EW_DEFINE_FIELDS( CoreOutline, CoreRectView )
  EW_VARIABLE( onDoneScroll,    XSlot )
  EW_VARIABLE( scrollEffect,    EffectsPointEffect )
  EW_PROPERTY( SlideHandler,    CoreSlideTouchHandler )
  EW_PROPERTY( ScrollOffset,    XPoint )
  EW_PROPERTY( Space,           XInt32 )
  EW_PROPERTY( Formation,       XEnum )
EW_END_OF_FIELDS( CoreOutline )

/* Virtual Method Table (VMT) for the class : 'Core::Outline' */
EW_DEFINE_METHODS( CoreOutline, CoreRectView )
  EW_METHOD( initLayoutContext, void )( CoreRectView _this, XRect aBounds, CoreOutline 
    aOutline )
  EW_METHOD( GetRoot,           CoreRoot )( CoreView _this )
  EW_METHOD( Draw,              void )( CoreOutline _this, GraphicsCanvas aCanvas, 
    XRect aClip, XPoint aOffset, XInt32 aOpacity, XBool aBlend )
  EW_METHOD( HandleEvent,       XObject )( CoreView _this, CoreEvent aEvent )
  EW_METHOD( CursorHitTest,     CoreCursorHit )( CoreView _this, XRect aArea, XInt32 
    aFinger, XInt32 aStrikeCount, CoreView aDedicatedView, XSet aRetargetReason )
  EW_METHOD( ArrangeView,       XPoint )( CoreRectView _this, XRect aBounds, XEnum 
    aFormation )
  EW_METHOD( MoveView,          void )( CoreRectView _this, XPoint aOffset, XBool 
    aFastMove )
  EW_METHOD( GetExtent,         XRect )( CoreRectView _this )
  EW_METHOD( ChangeViewState,   void )( CoreView _this, XSet aSetState, XSet aClearState )
  EW_METHOD( OnSetBounds,       void )( CoreOutline _this, XRect value )
EW_END_OF_METHODS( CoreOutline )

/* The method Draw() is invoked automatically if parts of the view should be redrawn 
   on the screen. This can occur when e.g. the view has been moved or the appearance 
   of the view has changed before.
   Draw() is invoked automatically by the framework, you never will need to invoke 
   this method directly. However you can request an invocation of this method by 
   calling the method InvalidateArea() of the views @Owner. Usually this is also 
   unnecessary unless you are developing your own view.
   The passed parameters determine the drawing destination aCanvas and the area 
   to redraw aClip in the coordinate space of the canvas. The parameter aOffset 
   contains the displacement between the origin of the views owner and the origin 
   of the canvas. You will need it to convert views coordinates into these of the 
   canvas.
   The parameter aOpacity contains the opacity descended from this view's @Owner. 
   It lies in range 0 .. 255. If the view implements its own 'Opacity', 'Color', 
   etc. properties, the Draw() method should calculate the resulting real opacity 
   by mixing the values of these properties with the one passed in aOpacity parameter.
   The parameter aBlend contains the blending mode descended from this view's @Owner. 
   It determines, whether the view should be drawn with alpha-blending active or 
   not. If aBlend is false, the outputs of the view should overwrite the corresponding 
   pixel in the drawing destination aCanvas. If aBlend is true, the outputs should 
   be mixed with the pixel already stored in aCanvas. For this purpose all Graphics 
   Engine functions provide a parameter to specify the mode for the respective drawing 
   operation. If the view implements its own 'Blend' property, the Draw() method 
   should calculate the resulting real blend mode by using logical AND operation 
   of the value of the property and the one passed in aBlend parameter. */
void CoreOutline_Draw( CoreOutline _this, GraphicsCanvas aCanvas, XRect aClip, XPoint 
  aOffset, XInt32 aOpacity, XBool aBlend );

/* 'C' function for method : 'Core::Outline.OnSetBounds()' */
void CoreOutline_OnSetBounds( CoreOutline _this, XRect value );

/* 'C' function for method : 'Core::Outline.onFinishScrollSlot()' */
void CoreOutline_onFinishScrollSlot( CoreOutline _this, XObject sender );

/* 'C' function for method : 'Core::Outline.onSlideSlot()' */
void CoreOutline_onSlideSlot( CoreOutline _this, XObject sender );

/* 'C' function for method : 'Core::Outline.onStartSlideSlot()' */
void CoreOutline_onStartSlideSlot( CoreOutline _this, XObject sender );

/* 'C' function for method : 'Core::Outline.OnSetSlideHandler()' */
void CoreOutline_OnSetSlideHandler( CoreOutline _this, CoreSlideTouchHandler value );

/* 'C' function for method : 'Core::Outline.OnSetScrollOffset()' */
void CoreOutline_OnSetScrollOffset( CoreOutline _this, XPoint value );

/* 'C' function for method : 'Core::Outline.OnSetSpace()' */
void CoreOutline_OnSetSpace( CoreOutline _this, XInt32 value );

/* 'C' function for method : 'Core::Outline.OnSetFormation()' */
void CoreOutline_OnSetFormation( CoreOutline _this, XEnum value );

/* The method EnsureVisible() scrolls the content of the outline until the given 
   view aView is partially or fully within the outline boundary area @Bounds. The 
   respective mode is determined by the parameter aFullyVisible.
   This scroll operation can optionally be animated by an effect passed in the parameter 
   aAnimationEffect. If aAnimationEffect == null, no animation is used and the scrolling 
   is executed immediately. After the operation is done, a signal is sent to the 
   optional slot method specified in the parameter aOnDoneScroll.
   Please note, calling the method EnsureVisible() while an animation is running 
   will terminate it abruptly without the slot method aOnDoneScroll being notified. 
   More flexible approach to stop an activate animation is to use the method @StopScrollEffect(). 
   Whether an animation is currently running can be queried by using the method 
   @IsScrollEffectActive(). */
void CoreOutline_EnsureVisible( CoreOutline _this, CoreView aView, XBool aFullyVisible, 
  EffectsPointEffect aAnimationEffect, XSlot aOnDoneScroll );

/* The method CountViews() returns the total number of views controlled by this 
   outline. In case of an empty outline without any views, 0 is returned.
   Please note, this method is limited to the views embedded within the outline. 
   Other sibling views not belonging to the outline are simply ignored. */
XInt32 CoreOutline_CountViews( CoreOutline _this );

/* The method GetContentArea() determines a rectangular area occupied by the views 
   embedded within the outline. The additional parameter aFilter can be used to 
   limit the operation to special views only, e.g. to visible and touchable views.
   If there are no views complying the filter condition, the method returns an empty 
   area.
   Please note, this method is limited to the views embedded within the outline. 
   Other sibling views not belonging to the outline are simply ignored. */
XRect CoreOutline_GetContentArea( CoreOutline _this, XSet aFilter );

/* Default onget method for the property 'ScrollOffset' */
XPoint CoreOutline_OnGetScrollOffset( CoreOutline _this );

#ifdef __cplusplus
  }
#endif

#endif /* _CoreOutline_H */

/* Embedded Wizard */
