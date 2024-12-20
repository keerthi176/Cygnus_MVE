/*******************************************************************************
*
* E M B E D D E D   W I Z A R D   P R O J E C T
*
*                                                Copyright (c) TARA Systems GmbH
*                                    written by Paul Banach and Manfred Schweyer
*
********************************************************************************
*
* This software is delivered "as is" and shows the usage of other software
* components. It is provided as an example software which is intended to be
* modified and extended according to particular requirements.
*
* TARA Systems hereby disclaims all warranties and conditions with regard to the
* software, including all implied warranties and conditions of merchantability
* and non-infringement of any third party IPR or other rights which may result
* from the use or the inability to use the software.
*
********************************************************************************
*
* DESCRIPTION:
*   This file is part of the interface (glue layer) between an Embedded Wizard
*   generated UI application and the board support package (BSP) of a dedicated
*   target.
*   This template is responsible to initialize the display hardware of the board
*   and to provide the necessary access to update the display content.
*
*******************************************************************************/
#ifdef STM32F767xx
#include "stm32f7xx_hal.h"

#else
#include "stm32f4xx_hal.h"
#include "stm32469i_eval.h"
#include "stm32469i_eval_lcd.h"
#include "stm32469i_eval_io.h"
#endif

#include <string.h>

#include "ewrte.h"
#include "ewgfx.h"
#include "ewextgfx.h"
#include "ewgfxdefs.h"

#include "ew_bsp_display.h"
#include "ew_bsp_clock.h"

#if EW_USE_FREE_RTOS == 1

  #include "cmsis_os.h"

  static osSemaphoreId        LcdUpdateSemaphoreId = 0;

#endif

#define LAYER_INDEX           0

//LTDC_HandleTypeDef            LtdcHandle;
//LTDC_LayerCfgTypeDef          LayerConfig;

#if EW_USE_DOUBLE_BUFFER == 1

  static volatile uint32_t    CurrentFramebuffer = 0;
  static volatile uint32_t    PendingFramebuffer = 0;

#else

  static volatile int         CurrentLine = -1;
  static volatile int         PendingLine = -1;

#endif

/*******************************************************************************
* FUNCTION:
*   LTDC_IRQHandler
*
* DESCRIPTION:
*   LTDC_IRQHandler Interrupt Handler.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
//void LTDC_IRQHandler( void )    - cube puts this in stm32f7xx_it.c
//{
//  HAL_LTDC_IRQHandler( &LtdcHandle );
//}


/*******************************************************************************
* FUNCTION:
*   EwBspConfigDisplay
*
* DESCRIPTION:
*   Configures the display hardware.
*
* ARGUMENTS:
*   aWidth   - Width of the framebuffer in pixel.
*   aHeight  - Height of the framebuffer in pixel.
*   aAddress - Startaddress of the framebuffer.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/

extern LTDC_HandleTypeDef hltdc;
extern LTDC_LayerCfgTypeDef pLayerCfg;

void EwBspConfigDisplay( int aWidth, int aHeight, void* aAddress )
{
	
  /* Initialization of ILI9341 component*/
 // LCD_IO_Init();

  /* Initialize given framebuffer */
  
   __DMA2D_CLK_ENABLE();

#if 0	
	
  #if EW_USE_FREE_RTOS == 1

    /* Create the LCD update semaphore */
    osSemaphoreDef( LcdUpdateSemaphore );
    LcdUpdateSemaphoreId = osSemaphoreCreate( osSemaphore( LcdUpdateSemaphore ), 1 );
  #endif
  
 
	

#if 1	
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  LtdcHandle.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  LtdcHandle.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  LtdcHandle.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  LtdcHandle.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

  /* Timing configuration */
  /* Horizontal synchronization width = Hsync - 1 */
  LtdcHandle.Init.HorizontalSync = 71;
  /* Vertical synchronization height = Vsync - 1 */
  LtdcHandle.Init.VerticalSync = 1;
  /* Accumulated horizontal back porch = Hsync + HBP - 1 */
  LtdcHandle.Init.AccumulatedHBP = 71+128;
  /* Accumulated vertical back porch = Vsync + VBP - 1 */
  LtdcHandle.Init.AccumulatedVBP = 1+22;
  /* Accumulated active width = Hsync + HBP + Active Width - 1 */
  LtdcHandle.Init.AccumulatedActiveW = 71+128+800;
  /* Accumulated active height = Vsync + VBP + Active Heigh - 1 */
  LtdcHandle.Init.AccumulatedActiveH = 1+22+480;
  /* Total width = Hsync + HBP + Active Width + HFP - 1 */
  LtdcHandle.Init.TotalWidth = 71+128+800+40;
  /* Total height = Vsync + VBP + Active Heigh + VFP - 1 */
  LtdcHandle.Init.TotalHeigh = 1+22+480+1;

#endif

  /* Configure R,G,B component values for LCD background color */
  LtdcHandle.Init.Backcolor.Blue  = 0;
  LtdcHandle.Init.Backcolor.Green = 0;
  LtdcHandle.Init.Backcolor.Red   = 0;

  LtdcHandle.Instance = LTDC;



  /* Layer1 Configuration */
  LayerConfig.WindowX0 = 0;
  LayerConfig.WindowX1 = aWidth;
  LayerConfig.WindowY0 = 0;
  LayerConfig.WindowY1 = aHeight;

  /* Pixel Format configuration: translate framebuffer color format into LTDC mode */
  #if ( EW_FRAME_BUFFER_COLOR_FORMAT == EW_FRAME_BUFFER_COLOR_FORMAT_RGBA8888 )
    LayerConfig.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
  #elif ( EW_FRAME_BUFFER_COLOR_FORMAT == EW_FRAME_BUFFER_COLOR_FORMAT_RGB888 )
    LayerConfig.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
  #elif ( EW_FRAME_BUFFER_COLOR_FORMAT == EW_FRAME_BUFFER_COLOR_FORMAT_RGBA4444 )
    LayerConfig.PixelFormat = LTDC_PIXEL_FORMAT_ARGB4444;
  #elif ( EW_FRAME_BUFFER_COLOR_FORMAT == EW_FRAME_BUFFER_COLOR_FORMAT_RGB565 )
    LayerConfig.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  #elif ( EW_FRAME_BUFFER_COLOR_FORMAT == EW_FRAME_BUFFER_COLOR_FORMAT_Index8 )
    LayerConfig.PixelFormat = LTDC_PIXEL_FORMAT_L8;
  #elif ( EW_FRAME_BUFFER_COLOR_FORMAT == EW_FRAME_BUFFER_COLOR_FORMAT_LumA44 )
    LayerConfig.PixelFormat = LTDC_PIXEL_FORMAT_AL44;
  #endif

  /* Start Address will be set by Graphics Engine */
  LayerConfig.FBStartAdress = (uint32_t)(aAddress);

  /* Alpha constant (255 totally opaque) */
  LayerConfig.Alpha = 255;

  /* Default Color configuration (configure A,R,G,B component values) */
  LayerConfig.Alpha0          = 255;
  LayerConfig.Backcolor.Blue  = 100;
  LayerConfig.Backcolor.Green = 100;
  LayerConfig.Backcolor.Red   = 100;

  /* Configure blending factors */
  LayerConfig.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  LayerConfig.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

  /* Configure the number of lines and number of pixels per line */
  LayerConfig.ImageWidth  = aWidth;
  LayerConfig.ImageHeight = aHeight;

  /* Configure the LTDC */
  if ( HAL_LTDC_Init( &LtdcHandle ) != HAL_OK )
    EwPrint( "EwBspConfigDisplay: Could not configure LTDC!\n" );

  
  /* Configure the Layer */
  if ( HAL_LTDC_ConfigLayer( &LtdcHandle, &LayerConfig, LAYER_INDEX ) != HAL_OK )
    EwPrint( "EwBspConfigDisplay: Could not configure layer!\n" );

  #if EW_USE_FREE_RTOS == 1

    /* initially take the LcdUpdate token for the first LCD update */
    osSemaphoreWait( LcdUpdateSemaphoreId , 1000 );

  #endif

#endif  
  
  
  #if EW_USE_DOUBLE_BUFFER == 1

    /* Configure the vsync */
    if ( HAL_LTDC_ProgramLineEvent( &hltdc, 0 ) != HAL_OK )
      EwPrint( "EwBspConfigDisplay: Could not configure interrupt for vsync!\n" );

  #endif
	
}



/*******************************************************************************
* FUNCTION:
*   EwBspWaitForDisplayCompletion
*
* DESCRIPTION:
*   The function EwBspWaitForDisplayCompletion returns as soon as the LCD update
*   has been completed.
*
* ARGUMENTS:
*   None
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
#if EW_USE_DOUBLE_BUFFER == 1

extern int display_on;

static void EwBspWaitForDisplayCompletion( void )
{
	if ( !display_on ) return;
	
  register uint32_t pendingBuffer = PendingFramebuffer;
  if ( CurrentFramebuffer == pendingBuffer )
    return;

  CPU_LOAD_SET_IDLE();

  #if EW_USE_FREE_RTOS == 1

    osSemaphoreWait( LcdUpdateSemaphoreId , 1000 );
    CurrentFramebuffer = pendingBuffer;

  #else

  while( CurrentFramebuffer != pendingBuffer )
    ;

  #endif

  CPU_LOAD_SET_ACTIVE();
}
#endif


/*******************************************************************************
* FUNCTION:
*   EwBspSyncOnDisplayLine
*
* DESCRIPTION:
*   The function EwBspSyncOnDisplayLine returns as soon as the LTDC is updating
*   the requested line number.
*
* ARGUMENTS:
*   aLine - Number of the display line to be reached by LTDC
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
#if EW_USE_DOUBLE_BUFFER == 0

void EwBspSyncOnDisplayLine( int aLine )
{
  /* either take line 0 or add the vertical back porch */
  if ( aLine <= 0 )
    aLine = 0;
  else
    aLine = aLine + hltdc.Init.AccumulatedVBP;

  CurrentLine = -1;
  PendingLine = aLine;

  CPU_LOAD_SET_IDLE();

  /* program line number of next field */
  HAL_LTDC_ProgramLineEvent( &hltdc, aLine );

  #if EW_USE_FREE_RTOS == 1

    osSemaphoreWait( LcdUpdateSemaphoreId , 1000 );

  #else

    while( aLine != CurrentLine )
      ;

  #endif

  CPU_LOAD_SET_ACTIVE();
}
#endif


/*******************************************************************************
* FUNCTION:
*   HAL_LTDC_LineEventCallback
*
* DESCRIPTION:
*   Interrupt service routine for V-sync.
*
* ARGUMENTS:
*   aLtdc - Specifies the pins connected EXTI line
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void HAL_LTDC_LineEventCallback( LTDC_HandleTypeDef* aLtdc )
{
#if EW_USE_DOUBLE_BUFFER == 1

  register uint32_t pendingBuffer = PendingFramebuffer;
  if ( CurrentFramebuffer != pendingBuffer )
  {
    /* change address within layer configuration structure */
    pLayerCfg.FBStartAdress = pendingBuffer;

    /* change layer configuration */
    if ( HAL_LTDC_ConfigLayer( aLtdc, &pLayerCfg, LAYER_INDEX ) != HAL_OK )
      EwPrint( "HAL_LTDC_LineEventCallback: Could not change layer configuration!\n" );

    #if EW_USE_FREE_RTOS == 1

      osSemaphoreRelease( LcdUpdateSemaphoreId );

    #else

      /* save new address */
      CurrentFramebuffer = pendingBuffer;

    #endif
  }

  /* program next V-sync */
  HAL_LTDC_ProgramLineEvent( aLtdc, 0 );

#else

  /* update current line number */
  CurrentLine = PendingLine;

  #if EW_USE_FREE_RTOS == 1

    osSemaphoreRelease( LcdUpdateSemaphoreId );

  #endif

#endif
}

extern LTDC_HandleTypeDef hltdc;

#define LAYER1              ((int*)(LTDC + 0x84U  ))
	
volatile int flipping_display;

void LTDC_IRQHandler( )
{
	uint32_t isrflags  = READ_REG(hltdc.Instance->ISR);
   uint32_t itsources = READ_REG(hltdc.Instance->IER);

  /* Line Interrupt management ************************************************/
  if (((isrflags & LTDC_ISR_LIF) != 0U) && ((itsources & LTDC_IER_LIE) != 0U))
  {
    /* Disable the Line interrupt */
    LTDC->IER &= LTDC_IT_LI;

    /* Clear the Line interrupt flag */
    LTDC->ICR = LTDC_FLAG_LI;

    /* Change LTDC state */
    hltdc.State = HAL_LTDC_STATE_READY;

    /* Process unlocked */
    __HAL_UNLOCK(&hltdc);

	register uint32_t pendingBuffer = PendingFramebuffer;
	  
	if ( CurrentFramebuffer != pendingBuffer )
	{
 
	  /* Configure the LTDC Layer */
		((LTDC_Layer_TypeDef*)LTDC_Layer1_BASE)->CFBAR = pendingBuffer;

		/* Set the Immediate Reload type */
		LTDC->SRCR = LTDC_SRCR_IMR;
	 
      /* save new address */
      CurrentFramebuffer = pendingBuffer; 
	 
	  }
	 
  }
 
}



/*******************************************************************************
* FUNCTION:
*   EwBspSetFramebufferAddress
*
* DESCRIPTION:
*   The function EwBspSetFramebufferAddress is called from the Graphics Engine
*   in order to change the currently active framebuffer address. If the display
*   is running in a double-buffering mode, the function is called after each
*   screen update.
*   Changing the framebuffer address should be synchronized with V-sync.
*   In case of double-buffering, the function has to wait and return after
*   the V-sync was detected.
*
* ARGUMENTS:
*   aAddress - New address of the framebuffer to be shown on the display.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwBspSetFramebufferAddress( unsigned long aAddress )
{
#if EW_USE_DOUBLE_BUFFER == 1

  /* set pending framebuffer address to be used on next V-sync */
  PendingFramebuffer = (uint32_t)aAddress;

  /* now, the function has to wait until the pending back-buffer is visible -
     otherwise the currently visible frontbuffer may be overwritten with next
     scene. */
  EwBspWaitForDisplayCompletion();

#else

  /* change address within layer configuration structure */
  LayerConfig.FBStartAdress = (uint32_t)aAddress;

  /* change layer configuration */
  if ( HAL_LTDC_ConfigLayer( &hltdc, &pLayerCfg, LAYER_INDEX ) != HAL_OK )
    EwPrint( "EwBspSetFramebufferAddress: Could not change layer configuration!\n" );

#endif
}


/*******************************************************************************
* FUNCTION:
*   EwBspSetFramebufferClut
*
* DESCRIPTION:
*   The function EwBspSetFramebufferClut is called from the Graphics Engine
*   in order to update the hardware CLUT of the current framebuffer.
*   The function is only called when the color format of the framebuffer is
*   Index8 or LumA44.
*
* ARGUMENTS:
*   aClut - Pointer to a color lookup table with 256 enries.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwBspSetFramebufferClut( unsigned long* aClut )
{
  if ( HAL_LTDC_ConfigCLUT( &hltdc, (uint32_t*)aClut, 256, LAYER_INDEX ) != HAL_OK )
    EwPrint( "EwBspSetFramebufferClut: Could not configure layer CLUT!\n" );
  if ( HAL_LTDC_EnableCLUT( &hltdc, LAYER_INDEX ) != HAL_OK )
    EwPrint( "EwBspSetFramebufferClut: Could not enable layer CLUT!\n" );
}


/* msy */
