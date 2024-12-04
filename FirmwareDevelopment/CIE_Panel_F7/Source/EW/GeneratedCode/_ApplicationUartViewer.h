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
* Profile  : STM32F429
* Platform : STM.STM32.RGB565
*
*******************************************************************************/

#ifndef _ApplicationUartViewer_H
#define _ApplicationUartViewer_H

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

#include "_CoreGroup.h"
#include "_ViewsBevel.h"
#include "_ViewsRectangle.h"
#include "_ViewsText.h"

/* Forward declaration of the class Application::UartViewer */
#ifndef _ApplicationUartViewer_
  EW_DECLARE_CLASS( ApplicationUartViewer )
#define _ApplicationUartViewer_
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


/* Deklaration of class : 'Application::UartViewer' */
EW_DEFINE_FIELDS( ApplicationUartViewer, CoreGroup )
  EW_OBJECT  ( Rectangle,       ViewsRectangle )
  EW_OBJECT  ( Bevel,           ViewsBevel )
  EW_OBJECT  ( Text,            ViewsText )
EW_END_OF_FIELDS( ApplicationUartViewer )

/* Virtual Method Table (VMT) for the class : 'Application::UartViewer' */
EW_DEFINE_METHODS( ApplicationUartViewer, CoreGroup )
  EW_METHOD( initLayoutContext, void )( CoreRectView _this, XRect aBounds, CoreOutline 
    aOutline )

  /* The method GetRoot() delivers the application object, this view belongs to. 
     The application object represents the entire screen of the GUI application. 
     Thus in the views hierarchy, the application object serves as the root view.
     This method can fail and return null if the view still doesn't belong to any 
     owner group. */
  EW_METHOD( GetRoot,           CoreRoot )( CoreView _this )

  /* The method Draw() is invoked automatically if parts of the view should be redrawn 
     on the screen. This can occur when e.g. the view has been moved or the appearance 
     of the view has changed before.
     Draw() is invoked automatically by the framework, you never will need to invoke 
     this method directly. However you can request an invocation of this method 
     by calling the method InvalidateArea() of the views @Owner. Usually this is 
     also unnecessary unless you are developing your own view.
     The passed parameters determine the drawing destination aCanvas and the area 
     to redraw aClip in the coordinate space of the canvas. The parameter aOffset 
     contains the displacement between the origin of the views owner and the origin 
     of the canvas. You will need it to convert views coordinates into these of 
     the canvas.
     The parameter aOpacity contains the opacity descended from this view's @Owner. 
     It lies in range 0 .. 255. If the view implements its own 'Opacity', 'Color', 
     etc. properties, the Draw() method should calculate the resulting real opacity 
     by mixing the values of these properties with the one passed in aOpacity parameter.
     The parameter aBlend contains the blending mode descended from this view's 
     @Owner. It determines, whether the view should be drawn with alpha-blending 
     active or not. If aBlend is false, the outputs of the view should overwrite 
     the corresponding pixel in the drawing destination aCanvas. If aBlend is true, 
     the outputs should be mixed with the pixel already stored in aCanvas. For this 
     purpose all Graphics Engine functions provide a parameter to specify the mode 
     for the respective drawing operation. If the view implements its own 'Blend' 
     property, the Draw() method should calculate the resulting real blend mode 
     by using logical AND operation of the value of the property and the one passed 
     in aBlend parameter. */
  EW_METHOD( Draw,              void )( CoreGroup _this, GraphicsCanvas aCanvas, 
    XRect aClip, XPoint aOffset, XInt32 aOpacity, XBool aBlend )

  /* The method HandleEvent() is invoked automatically if the view has received 
     an event. For example, touching the view on the touch screen can cause the 
     view to receive a Core::CursorEvent event. Within this method the view can 
     evaluate the event and react to it.
     Whether the event has been handled by the view or not is determined by the 
     return value. To sign an event as handled HandleEvent() should return a valid 
     object (e.g. 'this'). If the event has not been handled, 'null' should be returned.
     Depending on the kind of the event, the framework can continue dispatching 
     of still unhandled events. For example, keyboard events (Core::KeyEvent class) 
     are automatically delivered to the superior @Owner of the receiver view if 
     this view has ignored the event.
     HandleEvent() is invoked automatically by the framework, so you never should 
     need to invoke it directly. However you can prepare and post new events by 
     using the methods DispatchEvent() and BroadcastEvent() of the application class 
     Core::Root. */
  EW_METHOD( HandleEvent,       XObject )( CoreView _this, CoreEvent aEvent )

  /* The method CursorHitTest() is invoked automatically in order to determine whether 
     the view is interested in the receipt of cursor events or not. This method 
     will be invoked immediately after the user has tapped the visible area of the 
     view. If the view is not interested in the cursor event, the framework repeats 
     this procedure for the next behind view until a willing view has been found 
     or all views are evaluated.
     In the implementation of the method the view can evaluate the passed aArea 
     parameter. It determines the place where the user has tapped the view with 
     his fingertip expressed in the coordinates of the views @Owner. The method 
     can test e.g. whether the tapped area does intersect any touchable areas within 
     the view, etc. The affected finger is identified in the parameter aFinger. 
     The first finger (or the first mouse device button) has the number 0.
     The parameter aStrikeCount determines how many times the same area has been 
     tapped in series. This is useful to detect series of multiple touches, e.g. 
     in case of the double click, aStrikeCount == 2.
     The parameter aDedicatedView, if it is not 'null', restricts the event to be 
     handled by this view only. If aDedicatedView == null, no special restriction 
     exists.
     This method is also invoked if during an existing grab cycle the current target 
     view has decided to resign and deflect the cursor events to another view. This 
     is usually the case after the user has performed a gesture the current target 
     view is not interested to process. Thereupon, the system looks for another 
     view willing to take over the cursor event processing after the performed gesture. 
     Which gesture has caused the operation, is specified in the parameter aRetargetReason.
     If the view is willing to process the event, the method should create, initialize 
     and return a new Core::CursorHit object. This object identify the willing view. 
     Otherwise the method should return 'null'.
     CursorHitTest() is invoked automatically by the framework, so you never should 
     need to invoke it directly. This method is predetermined for the hit-test only. 
     The proper processing of events should take place in the @HandleEvent() method 
     by reacting to Core::CursorEvent and Core::DragEvent events. */
  EW_METHOD( CursorHitTest,     CoreCursorHit )( CoreGroup _this, XRect aArea, XInt32 
    aFinger, XInt32 aStrikeCount, CoreView aDedicatedView, XSet aRetargetReason )

  /* The method ArrangeView() is invoked automatically if the superior @Owner group 
     needs to re-arrange its views. For example, the changing of the owners size 
     can cause the enclosed views to adapt their position and size, so all views 
     still fit within the new owners area. This method provides the core functionality 
     for the automatic GUI layout mechanism.
     The arrangement is controlled primarily by the @Layout property of the view. 
     It specifies a set of alignment constraints and determines whether the view 
     can change its size. The bounds area where the view should be arranged, is 
     passed in the parameter aBounds. This is usually the current area of the views 
     owner.
     The parameter aFormation defines the desired arrangement mode. Depending on 
     the value of this parameter, the views can be arranged in rows or columns. 
     If aFormation == Core::Formation.None, no automatic row/column arrangement 
     is performed and the view is moved and scaled only to fit inside the aBounds 
     area.
     ArrangeView() is invoked automatically by the framework, so you never should 
     need to invoke it directly.
     The method returns the size of the view after it has been arranged. */
  EW_METHOD( ArrangeView,       XPoint )( CoreRectView _this, XRect aBounds, XEnum 
    aFormation )

  /* The method MoveView() changes the position of the view by adding the value 
     aOffset to all corners of the view. For example, in case of a line view the 
     value is added to the both line end points.
     The parameter aFastMove serves for the optimization purpose. If it is 'true', 
     the corners are modified without performing any updates of the view and without 
     redrawing the screen. This is useful, when you wish to move or arrange a lot 
     of views at once. In this case it's up to you to request the finally screen 
     update. To do this you can use the method InvalidateArea() of the views @Owner.
     In the case aFastMove == false, the operation automatically requests the screen 
     redraw of the view areas before and after the movement. You don't need to take 
     care about it. */
  EW_METHOD( MoveView,          void )( CoreRectView _this, XPoint aOffset, XBool 
    aFastMove )

  /* The method GetExtent() returns the position and the size of the view relative 
     to the origin of its @Owner. In case of views with a non rectangular shape 
     the method returns the rectangular boundary area enclosing the entire shape. */
  EW_METHOD( GetExtent,         XRect )( CoreRectView _this )

  /* The method ChangeViewState() modifies the current state of the view. The state 
     is a set of switches determining whether a view is visible, whether it can 
     react to user inputs or whether it is just performing some update operations, 
     etc.
     The modification is controlled by the the both parameters. The first aSetState 
     contains the switches to activate within the view state. The second aClearState 
     determines all switches to disable.
     Depending on the state alteration the method will perform different operations, 
     e.g. in response to the clearing of the visible state, the method will request 
     a screen redraw to make disappear the view from the screen.
     ChangeViewState() is invoked automatically by the framework, so you never should 
     need to invoke it directly. All relevant states are available as properties 
     e.g. the property Visible in derived classes reflects the visible state of 
     the view. */
  EW_METHOD( ChangeViewState,   void )( CoreGroup _this, XSet aSetState, XSet aClearState )
  EW_METHOD( OnSetBounds,       void )( CoreGroup _this, XRect value )
  EW_METHOD( OnSetFocus,        void )( CoreGroup _this, CoreView value )
  EW_METHOD( OnSetBuffered,     void )( CoreGroup _this, XBool value )

  /* The method DispatchEvent() feeds the component with the event passed in the 
     parameter aEvent and propagates it along the so-called focus path. This focus 
     path leads to the currently selected keyboard event receiver view. If the event 
     is rejected by the view, the same operation is repeated for the next superior 
     view. This permits the hierarchical event dispatching until a willing view 
     has handled the event or all views in the focus path have been evaluated. If 
     the event remains still unhandled, it will be passed to the component itself.
     The focus path is established by the property @Focus. 
     DispatchEvent() returns the value returned by the @HandleEvent() method of 
     the view which has handled the event. In the case, the event was not handled, 
     the method returns 'null'. */
  EW_METHOD( DispatchEvent,     XObject )( CoreGroup _this, CoreEvent aEvent )

  /* The method BroadcastEvent() feeds the component with the event passed in the 
     parameter aEvent and propagates it to all views enclosed within the component 
     until the event has been handled or all views are evaluated. If the event remains 
     still unhandled, it will be passed to the component itself.
     The additional parameter aFilter can be used to limit the operation to special 
     views only, e.g. to visible and touchable views. To broadcast the event to 
     all views pass in the parameter aFilter the value 'Core::ViewState[]'.
     BroadcastEvent() is very useful to provide all views with one and the same 
     event in order e.g. to inform all views about an important global state alteration. 
     To send events to views enclosing a given position use the method @BroadcastEventAtPosition().
     BroadcastEvent() returns the value returned by the @HandleEvent() method of 
     the view which has handled the event. In the case, the event was not handled, 
     the method returns 'null'. */
  EW_METHOD( BroadcastEvent,    XObject )( CoreGroup _this, CoreEvent aEvent, XSet 
    aFilter )

  /* The method UpdateLayout() is invoked automatically after the size of the component 
     has been changed. This method can be overridden and filled with logic to perform 
     a sophisticated arrangement calculation for one or more enclosed views. In 
     this case the parameter aSize can be used. It contains the current size of 
     the component. 
     Usually, all enclosed views are arranged automatically accordingly to their 
     @Layout property. UpdateLayout() gives the derived components a chance to extend 
     this automatism by a user defined algorithm. */
  EW_METHOD( UpdateLayout,      void )( ApplicationUartViewer _this, XPoint aSize )

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
  EW_METHOD( UpdateViewState,   void )( ApplicationUartViewer _this, XSet aState )

  /* The method InvalidateArea() declares the given area of the component as invalid, 
     this means this area should be redrawn at the next screen update. */
  EW_METHOD( InvalidateArea,    void )( CoreGroup _this, XRect aArea )

  /* The method Restack() changes the Z-order of views in the component. Depending 
     on the parameter aOrder the method will elevate or lower the given view aView. 
     If aOrder is negative, the view will be lowered to the background. If aOrder 
     is positive, the view will be elevated to the foreground. If aOrder == 0, no 
     modification will take place.
     The absolute value of aOrder determines the number of sibling views the view 
     has to skip over in order to reach its new Z-order.
     Please note, changing the Z-order of views within a component containing a 
     Core::Outline view can cause this outline to update its automatic row or column 
     formation. */
  EW_METHOD( Restack,           void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )

  /* The method Add() inserts the given view aView into this component and places 
     it at a Z-order position resulting from the parameter aOrder. The parameter 
     determines the number of sibling views the view has to skip over starting with 
     the top most view. If aOrder == 0, the newly added view will obtain the top 
     most position. If the value is negative, the view will be lowered to the background 
     accordingly to the absolute value of aOrder. After this operation the view 
     belongs to the component - the view can appear on the screen and it can receive 
     events.
     Please note, adding of views to a component containing a Core::Outline view 
     can cause this outline to update its automatic row or column formation. */
  EW_METHOD( Add,               void )( CoreGroup _this, CoreView aView, XInt32 
    aOrder )
EW_END_OF_METHODS( ApplicationUartViewer )

/* The method UpdateLayout() is invoked automatically after the size of the component 
   has been changed. This method can be overridden and filled with logic to perform 
   a sophisticated arrangement calculation for one or more enclosed views. In this 
   case the parameter aSize can be used. It contains the current size of the component. 
   Usually, all enclosed views are arranged automatically accordingly to their @Layout 
   property. UpdateLayout() gives the derived components a chance to extend this 
   automatism by a user defined algorithm. */
void ApplicationUartViewer_UpdateLayout( ApplicationUartViewer _this, XPoint aSize );

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
void ApplicationUartViewer_UpdateViewState( ApplicationUartViewer _this, XSet aState );

#ifdef __cplusplus
  }
#endif

#endif /* _ApplicationUartViewer_H */

/* Embedded Wizard */
