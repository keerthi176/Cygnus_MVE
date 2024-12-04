
#include "bsp.h"
#include "bsp_ts.h"

#include "stmpe811/stmpe811.h"


static TS_DrvTypeDef     *TsDrv;


/* Default calibration values */

#define CUTOFF 10 
#define XMIN  0x62
#define YMIN  0x99
#define XMAX  0xFB9
#define YMAX  0xEAB 

static int ts_xmin = XMIN;
static int ts_ymin = YMIN;
static int ts_xmax = XMAX;
static int ts_ymax = YMAX;

#define XPIX(x) (((x - ts_xmin) * 800 ) / (ts_xmax - ts_xmin))
#define YPIX(y) (((y - ts_ymin) * 480 ) / (ts_ymax - ts_ymin))

#define XTOUCH(x) ((x * (ts_xmax - ts_xmin) / 800 ) + ts_xmin)
#define YTOUCH(y) ((y * (ts_ymax - ts_ymin) / 480 ) + ts_ymin)

#define XTRIM(x) (( x < 0 ? 0 : x ) < 800 ? ( x < 0 ? 0 : x ) : 799 )
#define YTRIM(y) (( y < 0 ? 0 : y ) < 480 ? ( y < 0 ? 0 : y ) : 479 )


void BSP_TS_Calibrate( int xmin, int ymin, int xmax, int ymax, int x_xmin, int x_ymin, int x_xmax, int x_ymax )
{
	int txmin = XTOUCH( xmin );
	int txmax = XTOUCH( xmax );
	int tymin = YTOUCH( ymin );
	int tymax = YTOUCH( ymax );
	
	ts_xmin = txmin - ( x_xmin * (txmax - txmin)) / (x_xmax - x_xmin);
	ts_ymin = tymin - ( x_ymin * (tymax - tymin)) / (x_ymax - x_ymin);
	
	ts_xmax = txmax + ( (800 - x_xmax) * (txmax - txmin)) / (x_xmax - x_xmin);
	ts_ymax = tymax + ( (480 - x_ymax) * (tymax - tymin)) / (x_ymax - x_ymin);
}	

/**
  * @brief  Initializes and configures the touch screen functionalities and 
  *         configures all necessary hardware resources (GPIOs, clocks..).
  * @param  XSize: The maximum X size of the TS area on LCD
  * @param  YSize: The maximum Y size of the TS area on LCD  
  * @retval TS_OK: if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_Init( )
{
  uint8_t ret = TS_ERROR;
	
  IOE_Init( );	

  /* Read ID and verify if the IO expander is ready */
  if(stmpe811_ts_drv.ReadID(TS_I2C_ADDRESS) == STMPE811_ID)
  {
    /* Initialize the TS driver structure */
    TsDrv = &stmpe811_ts_drv;

    ret = TS_OK;
  }

  if(ret == TS_OK)
  {
    /* Initialize the LL TS Driver */
    TsDrv->Init(TS_I2C_ADDRESS);
    TsDrv->Start(TS_I2C_ADDRESS);
  }

  return ret;
}

/**
  * @brief  Configures and enables the touch screen interrupts.
  * @retval TS_OK: if ITconfig is OK. Other value if error.
  */
uint8_t BSP_TS_ITConfig(void)
{
  /* Enable the TS ITs */
  TsDrv->EnableIT(TS_I2C_ADDRESS);

  return TS_OK;
}

/**
  * @brief  Gets the TS IT status.
  * @retval Interrupt status.
  */  
uint8_t BSP_TS_ITGetStatus(void)
{
  /* Return the TS IT status */
  return (TsDrv->GetITStatus(TS_I2C_ADDRESS));
}



/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TsState: Pointer to touch screen current state structure
  */
void BSP_TS_GetState(TS_StateTypeDef* TsState)
{
  uint16_t x, y;
  static int32_t _x = 0, _y = 0;

  int tx, ty, xDiff, yDiff;	
	
  if ( TsDrv == NULL )
  {
	  if ( BSP_TS_Init( ) == TS_ERROR )
		return;
  }
	
  TsState->TouchDetected = TsDrv->DetectTouch(TS_I2C_ADDRESS);
  
  if( TsState->TouchDetected )
  {
    TsDrv->GetXY( TS_I2C_ADDRESS, &x, &y );
	
    tx = XPIX( (int) x );
	ty = YPIX( (int) y ); 
	  
	tx = XTRIM(tx);  
	ty = YTRIM(ty);  
     
    xDiff = tx > _x ? (tx - _x): (_x - tx);
    yDiff = ty > _y ? (ty - _y): (_y - ty); 
    
    if ( xDiff + yDiff > 5 )
    {
      _x = tx;
      _y = ty; 
   }
    
    // Update the X position
    TsState->X = 799 - _x;
    
    // Update the Y position  
    TsState->Y = 480 - _y; 
	EwPrint("[%d,%d]\n", TsState->X, TsState->Y );
  }		
}

/**
  * @brief  Clears all touch screen interrupts.
  */  
void BSP_TS_ITClear(void)
{
  /* Clear TS IT pending bits */
  TsDrv->ClearIT(TS_I2C_ADDRESS); 
}

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
