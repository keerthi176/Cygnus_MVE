********************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This software and related documentation ("Software") are intellectual
* property owned by TARA Systems and are copyright of TARA Systems.
* Any modification, copying, reproduction or redistribution of the Software in
* whole or in part by any means not in accordance with the End-User License
* Agreement for Embedded Wizard is expressly prohibited. The removal of this
* preamble is expressly prohibited.
*
********************************************************************************
*
* DESCRIPTION:
*
*   Embedded Wizard STM STM32 Platform Package
*   * Graphics Engine (GFX)
*   * Runtime Environment (RTE)
*
*   This directory contains the source codes and/or libraries of the components
*   Graphics Engine (GFX) and Runtime Environment (RTE) used to run a GUI
*   application generated by Embedded Wizard on the STM32 target.
*
*   This file contains a description of additional platform specific details:
*   - Platform Specific Release Notes (Version History of GFX and RTE)
*   - Supported Platforms and Color Formats
*   - Test and Verification Notes
*
*******************************************************************************/

Getting started with Embedded Wizard Studio:
--------------------------------------------
  In order to get familiar with Embedded Wizard Studio and the UI development
  work-flow, we highly recommend to study our online documentation:

  http://doc.embedded-wizard.de

  Furthermore, we have collected many 'Questions and Answers' covering
  typical Embedded Wizard programming aspects. Please visit our community:

  http://ask.embedded-wizard.de

  Please use this platform to drop your questions, answers and ideas.


Platform Specific Release Notes (Version History of GFX and RTE):
-----------------------------------------------------------------
  Please read first the Embedded Wizard Release Notes, to be informed about all
  general improvements or changes:

  http://doc.embedded-wizard.de/release-notes

  The following section contains additonal platform specific changes, related to
  Graphics Engine (GFX) or Runtime Environment (RTE):

* Version 9.00
  - Support of vector graphics. The new operations to fill and stroke paths
    can be disabled by compiling the Graphics Engine with the defined macro
    EW_DONT_USE_PATH_FUNCTIONS.

* Version 8.30.02
  - Entire set of GFX/RTE libraries are rebuild after adapting/checking for
    proper FPU settings.

* Version 8.30.01
  - GFX and RTE libraries for Cortex-M7 architecture are now built with corrected
    FPU settings for GCC toolchain (using -mfpu=fpv5-sp-d16 instead of
    -mfpu=fpv5-d16). The usage of double precision has caused a hardfault on
    F746/F756 targets when calling EwFormat() with float values.
  - Support of virtual framebuffer (width of the framebuffer can be larger than
    width of application). Used to support GFXMMU or framebuffer alignment.
  - Support for STM32L4R9 Discovery and Evalboard.

* Version 8.30
  - The limitation for max. 256 pixel font height has been increased to 1024 pixel.
    To display such large fonts the glyph cache has to be large enough.
  - Text alignment algorithms (changed with V8.20) are reworked (bug-fix).

* Version 8.20
  - Support of synchronized single buffer update by using partial display update.
    This approach makes it possible to draw directly into the visible framebuffer
    without flicker or tearing effects (as long as drawing operation is fast
    enough).
  - Renamed access to extern DMA2D graphics accelerator functionality.
  - Support of off-screen buffer is removed. Use new single buffer mode or
    double-buffering instead.
  - Usage of EW_FRAME_BUFFER_COLOR_FORMAT macros simplified.
  - Memory profiling feature is now always active - just add the function call
    EwPrintProfilierStatistics() within your main loop in case you want to get
    an overview of the current/maximum memory consumption.

* Version 8.10
  - Renaming "STM32Fx Platform Package" to "STM32 Platform Package".
  - Support of partial display updates in combination with a scratch-pad buffer,
    to reduce memory footprint dramatically. No framebuffer needed for displays
    with own graphics memory (GRAM).
    New function STM32BeginUpdateArea() for partial display update added.
  - Renamed access to realtime clock - extern EwBspGetTime() expected.
  - Renamed access to framebuffer update - extern EwBspSetFramebufferAddress()
    expected.
  - Renamed access to CLUT update - extern EwBspSetFramebufferClut() expected.

* Version 8.00.03
  - Support for STM32H743 Evalboard.
  - Support for STM32L496 Discovery board.
  - Bugfix in function RTC_GetTime() to provide realtime clock.

* Version 8.00.02
  - Fixed an issue within the software pixel driver, were the calculation of
    an alpha value was erroneous (underflow) under special conditions.
  - Bugfix in function RTC_GetTime() to provide realtime clock.
  - New function STM32FxIsDMA2DUsed() in ewextgfx.f to get the current state
    of the DMA2D usage (active/inactive).

* Version 8.00.01
  - Blending a rectangular area with a color is now accelerated by using DMA2D
    for RGBA8888, RGB888, RGBA4444 and RGB565 color formats.
  - Usage of DMA2D_INPUT_xxx and DMA2D_OUTPUT_xxx defines instead of obsolete
    color mode constants.
  - Usage of DMA2D can now be switched on and off dynamically during runtime,
    in order to see the performance difference within demo applications.
    To activate/deactivate the DMA2D during runtime use the new function
    STM32FxUseDMA2D().
  - Access of DMA2D graphics accelerator is now seperated into the file DMA2D.c
    which is part of the board configuration. The entire access to DMA2D HAL
    is now centralized within one file independent from the color format of the
    Graphics Engine.
  - DMA2D operates now in interrupt mode by default (instead of polling mode).
  - Parallel processing is now support: The preparation of the next drawing
    instruction or a parallel software drawing operation can be execuded,
    while the DMA2D is doing a hardware accelerated drawing operation.
  - Update of framebuffer is now hardware accelerated also for Graphics Engine
    operating with Index8 color format.
  - EwGetTime() calls external function RTC_GetTime() to provide realtime clock.

* Version 8.00
  - Support for STM32F412 Discovery board.
  - All dependencies to LTDC are removed from ewextgfx.c. All board specific
    configurations and functions are located within the build environment
    (BoardConfig.c).
  - Colorformat of framebuffer is now configured by using an appropriate
    define (within makefile) instead of a parameter in STM32FxInitViewport().

* Version 7.10
  - Support of STM32F769 Discovery board.
  - Support of color format RGB565: The new RGB565 Graphics Engine is able
    to draw directly into a framebuffer by using the RGB565 color format (screen
    color format). In order to support alpha blending for buffered objects and
    for image resources the native color format is RGBA8888.
  - Support of additional color format RGB888: The new RGB888 Graphics Engine
    is able to draw directly into a framebuffer by using the 24 bit RGB888 color
    format (screen color format). the native color format is still RGBA8888 in
    order to support alpha blending.

* Version 7.00
  - Support of color formats RGBA8888, RGBA4444, Index8 and LumA44
  - Bitmap resources and strings can be accessed directly from flash memory,
    in order to reduce the SDRAM/SRAM memory footprint.
  - Support of differnt screen orientations: The generated user interface
    can have a different orientation than the display / framebuffer.
  - Build environments for the different STM32Fx targets are now available.
  - Embedded Wizard Studio Evaluation Edition and STM32Fx Platform Package
    for evaluation purposes is now available.

* Version 6.60
  - New macros EW_USE_OFFSCREEN_BUFFER and EW_USE_DOUBLE_BUFFER added. They
    can be used to define the frame-buffer configuration and screen update
    behavior of the Graphics Engine.
    Please have a look to ewextgfx.c for more details.

* Version 6.51
  - Support of STM32Fx targets with Chrom-ART graphics accelerator.


Supported Platforms and Color Formats:
--------------------------------------
This Platform Package supports the following chipsets and color formats:

STM32        : STM32 platforms
  + RGBA8888 : 32 bit color format with 8 bit red, 8 bit green, 8 bit blue
               and 8 bit alpha values.
  + RGB888   : 24 bit screen color format with 8 bit red, 8 bit green and 8 bit
               blue values.
  + RGBA4444 : 16 bit color format with 4 bit red, 4 bit green, 4 bit blue
               and 4 bit alpha values.
  + RGB565   : 16 bit screen color format with 5 bit red, 6 bit green and 5 bit
               blue values.
  + Index8   : 8 bit index color format (global CLUT with up to 256 colors).
  + LumA44   : 8 bit color format with 4 bit luminance and 4 bit alpha values.


Test and Verification Notes:
----------------------------
This Platform Package Release was tested on the following environments:

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32F429
Hardware         : STM32F429-Discovery
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F4 V1.18.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32F429
Hardware         : STM32F429-Evalboard
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F4 V1.18.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32F469
Hardware         : STM32F469-Discovery
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F4 V1.18.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32F469
Hardware         : STM32F469-Evalboard
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F4 V1.18.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32F746
Hardware         : STM32F746-Discovery
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F7 V1.8.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32F756
Hardware         : STM32F756-Evalboard
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F7 V1.8.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32F769
Hardware         : STM32F769-Discovery
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F7 V1.8.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGB565 / Index8 / LumA44
Chipset          : STM32F412
Hardware         : STM32F412-Discovery
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F4 V1.18.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGB565 / Index8 / LumA44
Chipset          : STM32F407
Hardware         : STM32F407-Discovery with STM32F4DIS-BB and STM32F4DIS-LCD
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_F4 V1.18.0
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGB565 / Index8 / LumA44
Chipset          : STM32L496
Hardware         : STM32L496-Discovery
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_L4
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

Version          : 8.30
Platform Package : STM32 - RGBA8888 / RGB888 / RGBA4444 / RGB565 / Index8 / LumA44
Chipset          : STM32H743
Hardware         : STM32H743-Evalboard
Operating System : None / FreeRTOS
Software Tree    : STM32Cube_FW_H7
Compiler/Toolset : GCC / IAR / Keil
Test Result      : o.k. - 21/11/17 - msy/mli

