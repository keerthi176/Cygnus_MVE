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

#ifndef _ViewsAttrText_H
#define _ViewsAttrText_H

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

/* Forward declaration of the class Core::View */
#ifndef _CoreView_
  EW_DECLARE_CLASS( CoreView )
#define _CoreView_
#endif

/* Forward declaration of the class Graphics::AttrSet */
#ifndef _GraphicsAttrSet_
  EW_DECLARE_CLASS( GraphicsAttrSet )
#define _GraphicsAttrSet_
#endif

/* Forward declaration of the class Graphics::AttrString */
#ifndef _GraphicsAttrString_
  EW_DECLARE_CLASS( GraphicsAttrString )
#define _GraphicsAttrString_
#endif

/* Forward declaration of the class Graphics::Canvas */
#ifndef _GraphicsCanvas_
  EW_DECLARE_CLASS( GraphicsCanvas )
#define _GraphicsCanvas_
#endif

/* Forward declaration of the class Views::AttrText */
#ifndef _ViewsAttrText_
  EW_DECLARE_CLASS( ViewsAttrText )
#define _ViewsAttrText_
#endif


/* The class Views::AttrText provides a kind of view specialized to print text enriched 
   with attributes to control the text output. Similarly to HTML these attributes 
   determine the alignment and the colors and fonts to print the text. Additionally 
   the text can be combined with bitmaps and hyper-links can be defined. Thus the 
   attributed text is very suitable for displaying help pages and other formatted 
   information. The attributed text uses a special optimized syntax to code the 
   attributes. All attributes are enclosed in curly braces. They begin with an operation-code 
   consisting of 3 or 4 signs followed by none or more parameters. No blanks nor 
   white-spaces are allowed in the attribute. For example:

   "This is a {clr2}highlighted{clr0} word."

   The attribute {clr2} forces the view to draw the word 'highlighted' with the 
   color number 2. The value of the desired color is stored in so-called attribute 
   set together with necessary fonts and bitmaps. The following list describes the 
   available attributes. Please note the square brackets in the syntax below. They 
   don't belong to the attributes and are used to enclose parts of the attributes 
   which can be omitted.
   - {clr#} - Select color for the following text portion. # sign stands for the 
   number of the desired color. Each paragraph starts per default with the color 
   number 0.
   - {fnt#} - Select font for the following text portion. # sign stands for the 
   number of the desired font. Each paragraph starts per default with the font number 
   0.
   - {imgt#[.#]} - Show a bitmap in the text. The bitmap is top aligned vertically 
   within the row. The first # sign stands for the number of the desired bitmap. 
   The second # sign can optionally specify the bitmap frame to select in case of 
   a multi-frame bitmap. If it is omitted, the frame with the number 0 is shown.
   - {imgb#[.#]} - The same as above but the bitmap is bottom aligned.
   - {imgm#[.#]} - The same as above but the bitmap is vertically centered.
   - {ul+} - Switch on the underlined mode for the following text portion.
   - {ul-} - Switch off the underlined mode for the following text portion.
   - {parl[#l,#r,#t,#b][:#c]} - Start new paragraph. The text is left aligned in 
   the area of the paragraph. The optional parameters #l,#r,#t,#b specify margins 
   around the paragraph. They are expressed in pixel except #l and #r, which can 
   also be expressed in percent of the paragraph width. For the percent notation 
   the parameter is terminated with the percent sign, e.g. {parl10%,10%,5,5}. The 
   last parameter #c specifies the color number to fill the background of the paragraph.
   - {parr[#l,#r,#t,#b][:#c]} - The same as above but the text is right aligned.
   - {parc[#l,#r,#t,#b][:#c]} - The same as above but the text is center aligned.
   - {parj[#l,#r,#t,#b][:#c]} - The same as above but the text is left aligned and 
   justified.
   - {lay#1,#2[,#3]} - Start multi-column layout with to 2 or 3 columns. The width 
   of the columns is specified in the parameter #1, #2 and #3. They are expressed 
   in pixel or as percent of the available width. For the percent notation the parameter 
   is terminated with the percent sign. Columns signed with the '*' sign will automatically 
   take the remaining space. For example {lay10%,*,25} divides the available width 
   in three columns. The first will take 10% of the width, the third will have a 
   fixed width of 25 pixel. The middle column will take the remaining space. The 
   content of the columns is specified by the {col} attribute. At the end of the 
   layout an {end} attribute is expected otherwise no layout nesting is possible.
   - {colt[#l,#r,#t,#b][:#c]} - Start a column within a multi-column layout. The 
   content of the column is top aligned. The optional parameters #l,#r,#t,#b specify 
   margins around the column. They are expressed in pixel except #l and #r, which 
   can also be expressed in percent of the column width. For the percent notation 
   the parameter is terminated with the percent sign. The last parameter #c specifies 
   the color number to fill the background of the column.
   - {colb[#l,#r,#t,#b][:#c]} - The same as above but the column's content is bottom 
   aligned.
   - {colm[#l,#r,#t,#b][:#c]} - The same as above but the column's content is vertically 
   centered.
   - {end} - Terminate the layout {lay}.
   - {lnk:#} - Start a hyper-link attribute. The # sign stands for the name of the 
   link. This name can consist of any characters except the '}' sign and the zero 
   terminator '\x0000'. For example, {lnk:MainPage} defines a link with the name 
   'MainPage'. The name is used to identify the operation associated with the link. 
   It's up to you to perform the operation.
   - {lnk} - End hyper-link attribute.
   The text can consist of several rows separated by the new-line sign '\\n'. Additionally 
   an automatic text wrap will be performed. The automatic text wrap takes place 
   primarily between words. More sophisticated text wrap can be controlled by following 
   special signs used within the text:
   - The tilde sign '~' and the soft-hyphen sign '\x00AD' identify a potential text 
   wrap position. They are usually not displayed until the text wrap took place 
   at their position. Then the text row is terminated with a hyphen '-' sign.
   - The circumflex accent '^' sign identifies a potential text wrap position without 
   ever being visible.  
   In order to be able to output the special characters '^', '~' and '\x00AD' as 
   regular signs, the character '%' can be applied in front of the affected sign 
   to convert it to a regular sign. To show the '%' sign itself, use '%%' sequence.
   The text is determined by the property @String. The property @AttrSet refers 
   to an object providing all the colors, fonts and bitmaps used in the text. When 
   using the attributed text object you will also need to provide the Graphics::AttrSet 
   object.
   The position and the size of the area where the text is drawn is determined by 
   the property @Bounds. If the size of this area differs from the size of the text, 
   the text can be aligned. This is controlled by the property @Alignment. The properties 
   @Opacity, @OpacityTL, @OpacityTR, @OpacityBL and @OpacityBR can be used to modulate 
   the transparency of the shown text. For each view's corner different opacity 
   can be set, so the the text can be shown with fancy opacity gradients.
   The property @SlideHandler permits the text view to be connected together with 
   an interactive Core::SlideTouchHandler. In this manner the user can scroll the 
   displayed text by simply touching the slide handler on the screen. Alternatively, 
   the scroll position can be controlled by the value of the property @ScrollOffset.
   The visibility of the text is controlled by the properties @Visible and @Embedded. 
   In particular the property @Embedded can determine whether the corresponding 
   view is limited (== embedded) to the boundary of a sibling Core::Outline view 
   or not. The embedding of views within a Core::Outline allows a kind of sub-groups 
   within the GUI component itself. Very useful for any kind of scrollable lists, 
   menus, etc.
   The attributed text provides an additional set of methods useful to access the 
   hyper-links stored within the text, e.g. @GetLinkName(), @GetLinkRect(), etc. 
   These methods can be invoked from a slot method assigned to the property @OnUpdate. 
   In this manner additional decorations can be drawn around the links, etc. */
EW_DEFINE_FIELDS( ViewsAttrText, CoreRectView )
  EW_VARIABLE( attrString,      GraphicsAttrString )
  EW_PROPERTY( OpacityBL,       XInt32 )
  EW_PROPERTY( OpacityBR,       XInt32 )
  EW_PROPERTY( OpacityTR,       XInt32 )
  EW_PROPERTY( OpacityTL,       XInt32 )
  EW_PROPERTY( OnUpdate,        XSlot )
  EW_PROPERTY( WrapWidth,       XInt32 )
  EW_PROPERTY( ScrollOffset,    XPoint )
  EW_PROPERTY( Alignment,       XSet )
  EW_PROPERTY( String,          XString )
  EW_PROPERTY( AttrSet,         GraphicsAttrSet )
  EW_PROPERTY( Padding,         XInt32 )
  EW_PROPERTY( AutoSize,        XBool )
EW_END_OF_FIELDS( ViewsAttrText )

/* Virtual Method Table (VMT) for the class : 'Views::AttrText' */
EW_DEFINE_METHODS( ViewsAttrText, CoreRectView )
  EW_METHOD( initLayoutContext, void )( CoreRectView _this, XRect aBounds, CoreOutline 
    aOutline )
  EW_METHOD( GetRoot,           CoreRoot )( CoreView _this )
  EW_METHOD( Draw,              void )( ViewsAttrText _this, GraphicsCanvas aCanvas, 
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
  EW_METHOD( OnSetBounds,       void )( ViewsAttrText _this, XRect value )
EW_END_OF_METHODS( ViewsAttrText )

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
void ViewsAttrText_Draw( ViewsAttrText _this, GraphicsCanvas aCanvas, XRect aClip, 
  XPoint aOffset, XInt32 aOpacity, XBool aBlend );

/* 'C' function for method : 'Views::AttrText.OnSetBounds()' */
void ViewsAttrText_OnSetBounds( ViewsAttrText _this, XRect value );

/* 'C' function for method : 'Views::AttrText.preOnUpdateSlot()' */
void ViewsAttrText_preOnUpdateSlot( ViewsAttrText _this, XObject sender );

/* 'C' function for method : 'Views::AttrText.reparseSlot()' */
void ViewsAttrText_reparseSlot( ViewsAttrText _this, XObject sender );

/* 'C' function for method : 'Views::AttrText.preReparseSlot()' */
void ViewsAttrText_preReparseSlot( ViewsAttrText _this, XObject sender );

/* 'C' function for method : 'Views::AttrText.redrawSlot()' */
void ViewsAttrText_redrawSlot( ViewsAttrText _this, XObject sender );

/* 'C' function for method : 'Views::AttrText.OnSetWrapWidth()' */
void ViewsAttrText_OnSetWrapWidth( ViewsAttrText _this, XInt32 value );

/* 'C' function for method : 'Views::AttrText.OnSetAlignment()' */
void ViewsAttrText_OnSetAlignment( ViewsAttrText _this, XSet value );

/* 'C' function for method : 'Views::AttrText.OnSetString()' */
void ViewsAttrText_OnSetString( ViewsAttrText _this, XString value );

/* 'C' function for method : 'Views::AttrText.OnSetAttrSet()' */
void ViewsAttrText_OnSetAttrSet( ViewsAttrText _this, GraphicsAttrSet value );

/* 'C' function for method : 'Views::AttrText.OnSetEmbedded()' */
void ViewsAttrText_OnSetEmbedded( ViewsAttrText _this, XBool value );

/* The method GetContentArea() returns the position and the size of an area where 
   the view will show the text. This area is expressed in coordinates relative to 
   the top-left corner of the view's @Owner. The method takes in account all properties 
   which do affect the position and the alignment of the text, e.g. @Alignment, 
   @ScrollOffset, etc.
   Please note, the width of the returned area corresponds always to the width the 
   text has been wrapped for. Usually this width results from the actual value of 
   the property @Bounds unless another width has been specified in the property 
   @WrapWidth. */
XRect ViewsAttrText_GetContentArea( ViewsAttrText _this );

#ifdef __cplusplus
  }
#endif

#endif /* _ViewsAttrText_H */

/* Embedded Wizard */
