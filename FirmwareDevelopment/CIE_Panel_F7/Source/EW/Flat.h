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

#ifndef Flat_H
#define Flat_H

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

/* Font resources used for the different widgets. If you want to change the fonts 
   within your project, just create a variant of these font resources and define 
   the attributes of the font resources according your design ideas. */
EW_DECLARE_FONT_RES( FlatFontS )

/* Font resources used for the different widgets. If you want to change the fonts 
   within your project, just create a variant of these font resources and define 
   the attributes of the font resources according your design ideas. */
EW_DECLARE_FONT_RES( FlatFontM )

/* Font resources used for the different widgets. If you want to change the fonts 
   within your project, just create a variant of these font resources and define 
   the attributes of the font resources according your design ideas. */
EW_DECLARE_FONT_RES( FlatFontXL )

#ifdef __cplusplus
  }
#endif

#endif /* Flat_H */

/* Embedded Wizard */
