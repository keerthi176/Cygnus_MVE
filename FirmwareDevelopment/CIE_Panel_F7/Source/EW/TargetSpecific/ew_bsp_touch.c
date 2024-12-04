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
*   This template is responsible to initialize the touch driver of the display
*   hardware and to receive the touch events for the UI application.
*
*******************************************************************************/

#include "stm32f4xx_hal.h"

#include "bsp_ts.h"

#include "ewrte.h"
#include "ewgfxdriver.h"
#include "ewextgfx.h"

#include "ew_bsp_clock.h"
#include "ew_bsp_touch.h"

static int                    TouchAreaWidth  = 0;
static int                    TouchAreaHeight = 0;

 

/*******************************************************************************
* FUNCTION:
*   EwBspConfigTouch
*
* DESCRIPTION:
*   Configure the touch driver.
*
* ARGUMENTS:
*   aWidth  - Width of the toucharea (framebuffer) in pixel.
*   aHeight - Height of the toucharea (framebuffer) in pixel.
*
* RETURN VALUE:
*   None
*
*******************************************************************************/
void EwBspConfigTouch( int aWidth, int aHeight )
{
  TouchAreaWidth  = aWidth;
  TouchAreaHeight = aHeight;

  if ( BSP_TS_Init( ) == TS_ERROR )
  {
	  EwPrint( "Failed to init touch panel" );
  }
}


/*******************************************************************************
* FUNCTION:
*   EwBspGetTouchPosition
*
* DESCRIPTION:
*   The function EwBspGetTouchPosition reads the current touch position from the
*   touch driver and returns the current position and status. The orientation
*   of the touch positions is adjusted to match GUI coordinates.
*
* ARGUMENTS:
*   aPos - Pointer to XPoint structure to return the current position.
*
* RETURN VALUE:
*   Returns 1 if a touch event is detected, otherwise 0.
*
*******************************************************************************/
//XPoint last_touch;
XPoint          lastPos;

int EwBspGetTouchPosition( XPoint* aPos )
{
  static TS_StateTypeDef touchPadState;
 
  static unsigned int    lastTime;

  /* access touch driver to receive current touch status and position */
  CPU_LOAD_SET_IDLE();
  BSP_TS_GetState( &touchPadState );
  CPU_LOAD_SET_ACTIVE();

  if ( touchPadState.TouchDetected )
  {
	//	 EwPrint( "Touch: %d %d\n", touchPadState.X, touchPadState.Y );
	
		
    /* check for valid coordinates */
    if (( touchPadState.X > 0 ) && ( touchPadState.X < TouchAreaWidth - 1 ) &&
        ( touchPadState.Y > 0 ) && ( touchPadState.Y < TouchAreaHeight - 1 ))
    {
      #if ( EW_SURFACE_ROTATION == 90 )

        aPos->X = touchPadState.Y;
        aPos->Y = TouchAreaWidth  - touchPadState.X;

      #elif ( EW_SURFACE_ROTATION == 270 )

        aPos->X = TouchAreaHeight - touchPadState.Y;
        aPos->Y = touchPadState.X;

      #elif ( EW_SURFACE_ROTATION == 180 )

        aPos->X = TouchAreaWidth  - touchPadState.X;
        aPos->Y = TouchAreaHeight - touchPadState.Y;

      #else

        aPos->X = touchPadState.X;
        aPos->Y = touchPadState.Y;

      #endif

      /* keep the position and time of last touch detection */
      lastPos.X = aPos->X;
      lastPos.Y = aPos->Y;
      lastTime  = EwGetTicks( );

      /* return valid touch event */
      return 1;
    }
  }

  /* in order to avoid that short undetected touch events will be interpreted
     as an 'up' event - we return for some milliseconds the last detected event */
  if ( lastTime + 20 > EwGetTicks( ) )
  {
    aPos->X = lastPos.X;
    aPos->Y = lastPos.Y;
    return 1;
  }

  /* return no touch event */
  return 0;
}


/* msy */
