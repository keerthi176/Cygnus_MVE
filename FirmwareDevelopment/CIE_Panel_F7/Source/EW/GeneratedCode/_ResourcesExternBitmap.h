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

#ifndef _ResourcesExternBitmap_H
#define _ResourcesExternBitmap_H

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

#include "_ResourcesBitmap.h"

/* Forward declaration of the class Resources::ExternBitmap */
#ifndef _ResourcesExternBitmap_
  EW_DECLARE_CLASS( ResourcesExternBitmap )
#define _ResourcesExternBitmap_
#endif


/* The class Resources::ExternBitmap provides the functionality to handle a bitmap 
   loaded at the runtime from an extern image file. Bitmaps loaded in this manner 
   can be used as regular bitmap resources -> they can be shown in all adequate 
   views like Views::Image, Views::Wallpaper, Views::Frame, Views::WarpImage and 
   Views::AttrText.
   This class depends on the bitmap loader implementation which of course is specific 
   for each target and pixel format. It is thus up to you to provide this loader 
   implementation for your particular target system. You will need to implement 
   following 'C' function and to link it with the generated GUI application code:

   XBitmap* EwLoadExternBitmap( XString aName );

   When called the function has the job to open the image file identified by the 
   parameter aName, to create a new GraphicsEngine bitmap object and to load the 
   bitmap object with the content of the image file. The parameter aName corresponds 
   to the property @Name.
   In the case the content of the image file has changed at the runtime, the extern 
   bitmap object can be triggered to reload the image file by using its method @Reload(). */
EW_DEFINE_FIELDS( ResourcesExternBitmap, ResourcesBitmap )
  EW_PROPERTY( Name,            XString )
EW_END_OF_FIELDS( ResourcesExternBitmap )

/* Virtual Method Table (VMT) for the class : 'Resources::ExternBitmap' */
EW_DEFINE_METHODS( ResourcesExternBitmap, ResourcesBitmap )
  EW_METHOD( OnSetFrameSize,    void )( ResourcesBitmap _this, XPoint value )
  EW_METHOD( Update,            void )( ResourcesBitmap _this )
EW_END_OF_METHODS( ResourcesExternBitmap )

/* 'C' function for method : 'Resources::ExternBitmap.Init()' */
void ResourcesExternBitmap_Init( ResourcesExternBitmap _this, XHandle aArg );

/* 'C' function for method : 'Resources::ExternBitmap.OnSetName()' */
void ResourcesExternBitmap_OnSetName( ResourcesExternBitmap _this, XString value );

/* The method Reload() triggers the extern bitmap object to reload the image file 
   identified by the property @Name. This method is useful if the image file content 
   has changed at the runtime. */
void ResourcesExternBitmap_Reload( ResourcesExternBitmap _this );

/* 'C' function for method : 'Resources::ExternBitmap.load()' */
void ResourcesExternBitmap_load( ResourcesExternBitmap _this, XObject sender );

#ifdef __cplusplus
  }
#endif

#endif /* _ResourcesExternBitmap_H */

/* Embedded Wizard */
