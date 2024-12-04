
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "jpeg_utils.h"
#include "decode_dma.h"
#include "image_320_240_jpg.h"


uint32_t JpegProcessing_End = 0;


JPEG_HandleTypeDef    JPEG_Handle;

static DMA2D_HandleTypeDef    DMA2D_Handle;
static JPEG_ConfTypeDef       JPEG_Info;

/* Private function prototypes -----------------------------------------------*/

static void DMA2D_CopyBuffer( uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize );


#define LCD_FRAME_BUFFER ( 0xC0000000 )
#define JPEG_OUTPUT_DATA_BUFFER ( 0xC0200000 )

void show_menu( void )
{
  /*##-1- JPEG Initialization ################################################*/   
  /* Init The JPEG Color Look Up Tables used for YCbCr to RGB conversion   */  
  JPEG_InitColorTables( ); 

   /* Init the HAL JPEG driver */
  JPEG_Handle.Instance = JPEG;
  HAL_JPEG_Init(&JPEG_Handle);  
  
  /*##-3- JPEG decoding with DMA (Not Blocking ) Method ################*/
  JPEG_Decode_DMA(&JPEG_Handle, (uint32_t)image_320_240_jpg, sizeof(image_320_240_jpg), JPEG_OUTPUT_DATA_BUFFER );
  
  /*##-4- Wait till end of JPEG decoding and perfom Output Processing in Background  #*/
  do
  {
      JpegProcessing_End = JPEG_OutputHandler(&JPEG_Handle);    
  }
  while(JpegProcessing_End == 0);
  
  /*##-5- Get JPEG Info  ###############################################*/
  HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);       
  
  {
	  
	  /*##-9- Copy RGB decoded Data to the display FrameBuffer  ############*/
	  int xPos = (800 - JPEG_Info.ImageWidth)/2;
	  int yPos = (480 - JPEG_Info.ImageHeight)/2;     
		 
	  DMA2D_CopyBuffer((uint32_t *)JPEG_OUTPUT_DATA_BUFFER, (uint32_t *)LCD_FRAME_BUFFER, xPos , yPos, JPEG_Info.ImageWidth, JPEG_Info.ImageHeight);
  }
}


static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t x, uint16_t y, uint16_t xsize, uint16_t ysize)
{   
  
  uint32_t destination = (uint32_t)pDst + (y * 800 + x) * 4;
  uint32_t source      = (uint32_t)pSrc;
  
  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/ 
  DMA2D_Handle.Init.Mode          = DMA2D_M2M_PFC;
  DMA2D_Handle.Init.ColorMode     = DMA2D_OUTPUT_RGB565;
  DMA2D_Handle.Init.OutputOffset  = 800 - xsize;
  DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/  
  DMA2D_Handle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */   
  
  /*##-2- DMA2D Callbacks Configuration ######################################*/
  DMA2D_Handle.XferCpltCallback  = NULL;
  
  /*##-3- Foreground Configuration ###########################################*/
  DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
  DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;
 
#if (JPEG_RGB_FORMAT == JPEG_ARGB8888)
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  
#elif (JPEG_RGB_FORMAT == JPEG_RGB888)  
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  
#elif (JPEG_RGB_FORMAT == JPEG_RGB565)  
  DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;

#endif /* JPEG_RGB_FORMAT * */
  

  DMA2D_Handle.LayerCfg[1].InputOffset = 0;
  DMA2D_Handle.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR; /* No ForeGround Red/Blue swap */
  DMA2D_Handle.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */
  
  DMA2D_Handle.Instance          = DMA2D; 
   
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&DMA2D_Handle) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&DMA2D_Handle, source, destination, xsize, ysize) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 100);
      }
    }
  }   
}
