

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"
 
#include "assert.h"
#include "MM_Module.h"
#include "MM_Application.h"

typedef   uint32_t (* JPEG_YCbCrToRGB_Convert_Function)(uint8_t *pInBuffer, 
                                      uint8_t *pOutBuffer,
                                      uint32_t BlockIndex,
                                      uint32_t DataCount,
                                      uint32_t *ConvertedDataCount );

extern "C" void JPEG_InitColorTables(void);
extern "C" HAL_StatusTypeDef JPEG_GetDecodeColorConvertFunc(JPEG_ConfTypeDef *pJpegInfo, JPEG_YCbCrToRGB_Convert_Function *pFunction, uint32_t *ImageNbMCUs);
 


#define LCD_FRAME_BUFFER         0xC0000000
#define JPEG_OUTPUT_DATA_BUFFER  0xC0400000

/* Exported variables --------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint32_t JPEG_Decode_DMA(JPEG_HandleTypeDef *hjpeg, uint32_t DestAddress);
uint32_t JPEG_OutputHandler(JPEG_HandleTypeDef *hjpeg);
void JPEG_InputHandler(JPEG_HandleTypeDef *hjpeg);


/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup JPEG_MJPEG_VideoDecoding
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#if (JPEG_RGB_FORMAT == JPEG_ARGB8888)
  #define BYTES_PER_PIXEL 4  /* Number of bytes in a pixel */
#elif (JPEG_RGB_FORMAT == JPEG_RGB888)
  #define BYTES_PER_PIXEL 3  /* Number of bytes in a pixel */
#elif (JPEG_RGB_FORMAT == JPEG_RGB565)
  #define BYTES_PER_PIXEL 2  /* Number of bytes in a pixel */
#else
  #error "unknown JPEG_RGB_FORMAT "
#endif /* JPEG_RGB_FORMAT */

 

#define LEFT_AREA         1
#define RIGHT_AREA        2

#define PATTERN_SEARCH_BUFFERSIZE 512

 
#define JPEG_SOI_MARKER (0xFFD8) /* JPEG Start Of Image marker*/
#define JPEG_SOI_MARKER_BYTE0 (JPEG_SOI_MARKER & 0xFF)
#define JPEG_SOI_MARKER_BYTE1 ((JPEG_SOI_MARKER >> 8) & 0xFF)



/* Private variables ---------------------------------------------------------*/
uint32_t Previous_FrameSize;

uint8_t pColLeft[]    = {0x00, 0x00, 0x01, 0x8F}; /*   0 -> 399 */
uint8_t pColRight[]   = {0x01, 0x90, 0x03, 0x1F}; /* 400 -> 799 */
uint8_t pPage[]       = {0x00, 0x00, 0x01, 0xDF}; /*   0 -> 479 */
uint8_t pScanCol[]    = {0x02, 0x15};             /* Scan @ 533 */

 
//FIL JPEG_File;  /* File object */


uint32_t JpegProcessing_End = 0;

JPEG_HandleTypeDef    JPEG_Handle;

static JPEG_ConfTypeDef       JPEG_Info;
static __IO int32_t pending_buffer = -1;
static __IO int32_t active_area = 0;
static uint32_t FrameOffset = 0;
static uint8_t PatternSearchBuffer[PATTERN_SEARCH_BUFFERSIZE];
static uint32_t FrameRate;
static uint32_t FrameIndex;

  
/* Private function prototypes -----------------------------------------------*/
static void LCD_BriefDisplay(uint32_t Stage);
 
static void DMA2D_Init(uint32_t ImageWidth, uint32_t ImageHeight);
static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t ImageWidth, uint16_t ImageHeight);
static void DMA2D_TransferCompleteCallback(DMA2D_HandleTypeDef *hdma2d);
 
 
static uint32_t JPEG_FindFrameOffset(uint32_t offset );
 
static int qpos = 0;
static char* qbase = (char*) 0x90D20000;

int qopen(   )
{
	qpos = 0;
	return FR_OK;
}

int qread( uint8_t* addr, int size,  UINT* read )
{
	*read = size;
	memcpy( addr, qbase + qpos, size );
	qpos += size;
	return FR_OK;
}

int qseek( int pos )
{
	qpos = pos;
	return FR_OK;
}

int qsize( )
{
	return 9347136;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

class Video
{
	public:
	Video();
};

Video::Video( )
{
	uint32_t isfirstFrame, startTime, endTime, currentFrameRate;
 
	/* STM32F7xx HAL library initialization:
		 - Configure the Flash ART accelerator on ITCM interface
		 - Systick timer is configured by default as source of time base, but user
			can eventually implement his proper time base source (a general purpose
			timer for example or other time source), keeping in mind that Time base
			duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
			handled in milliseconds basis.
		 - Set NVIC Group Priority to 4
		 - Low Level Initialization
	  */

	/*##-1- JPEG Initialization ################################################*/
	/* Init The JPEG Color Look Up Tables used for YCbCr to RGB conversion   */
	JPEG_InitColorTables();

	/* Init the HAL JPEG driver */
	JPEG_Handle.Instance = JPEG;
	HAL_JPEG_Init(&JPEG_Handle);

	/*##-2- LCD Configuration ##################################################*/
	/* Initialize the LCD   */


	pending_buffer = 0;
	active_area = LEFT_AREA;
  
	/*##-5- Open the MJPEG avi file with read access #############################*/
	if(qopen( ) == FR_OK)
	{
	 
		isfirstFrame = 1;
		FrameIndex = 0;
		FrameRate = 0;

		do
		{
			/*##-6- Find next JPEG Frame offset in the video file #############################*/
			FrameOffset =  JPEG_FindFrameOffset(FrameOffset + Previous_FrameSize );

			if(FrameOffset != 0)
			{
				startTime = HAL_GetTick();

				qseek( FrameOffset);

				/*##-7- Start decoding the current JPEG frame with DMA (Not Blocking ) Method ################*/
				JPEG_Decode_DMA(&JPEG_Handle, JPEG_OUTPUT_DATA_BUFFER);

				/*##-8- Wait till end of JPEG decoding, and perfom Input/Output Processing in BackGround  #*/
				do
				{
				   JPEG_InputHandler(&JPEG_Handle);
				   JpegProcessing_End = JPEG_OutputHandler(&JPEG_Handle);

				}while(JpegProcessing_End == 0);

				FrameIndex++;

				if(isfirstFrame == 1)
				{
				   isfirstFrame = 0;
				   /*##-9- Get JPEG Info  ###############################################*/
				   HAL_JPEG_GetInfo(&JPEG_Handle, &JPEG_Info);

				   /*##-10- Initialize the DMA2D ########################################*/
				   DMA2D_Init(JPEG_Info.ImageWidth, JPEG_Info.ImageHeight);
				}
				/*##-11- Copy the Decoded frame to the display frame buffer using the DMA2D #*/
				DMA2D_CopyBuffer((uint32_t *)JPEG_OUTPUT_DATA_BUFFER, (uint32_t *)LCD_FRAME_BUFFER, JPEG_Info.ImageWidth, JPEG_Info.ImageHeight);

				/*##-12- Calc the current decode frame rate #*/
				endTime = HAL_GetTick();
				currentFrameRate = 1000 / (endTime - startTime);

				app.DebOut( " %lu fps", currentFrameRate);
				FrameRate += currentFrameRate;
			}

		}
		while(FrameOffset != 0);

		/*Close the avi file*/
	

		if(FrameIndex > 0)
		{
			/*##-12- Calc the average decode frame rate #*/
			FrameRate /= FrameIndex;
			HAL_Delay(20);
			/* Display decoding info */
			LCD_BriefDisplay(1);
		}
	}
	else /* Can't Open avi file*/
	{

	}
}


DMA2D_HandleTypeDef DMA2D_Handle; 
  
static void DMA2D_Init(uint32_t ImageWidth, uint32_t ImageHeight)
{
  /* Init DMA2D */
  /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
  DMA2D_Handle.Init.Mode          = DMA2D_M2M_PFC;
  DMA2D_Handle.Init.ColorMode     = DMA2D_OUTPUT_RGB565;
  DMA2D_Handle.Init.OutputOffset  = 0;
  DMA2D_Handle.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/
  DMA2D_Handle.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */

  /*##-2- DMA2D Callbacks Configuration ######################################*/
  DMA2D_Handle.XferCpltCallback  = DMA2D_TransferCompleteCallback;

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

  /*##-4- DMA2D Initialization     ###########################################*/
  HAL_DMA2D_Init(&DMA2D_Handle);
  HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1);
}

/**
  * @brief  Copy the Decoded image to the display Frame buffer.
  * @param  pSrc: Pointer to source buffer
  * @param  pDst: Pointer to destination buffer
  * @param  ImageWidth: image width
  * @param  ImageHeight: image Height
  * @retval None
  */
static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t ImageWidth, uint16_t ImageHeight)
{

  uint32_t xPos, yPos, destination;

  /*##-1- calculate the destination transfer address  ############*/
  xPos = (800 - JPEG_Info.ImageWidth)/2;
  yPos = (480 - JPEG_Info.ImageHeight)/2;

  destination = (uint32_t)pDst + ((yPos * 800) + xPos) * 2;

  /*##-2- Wait until the DSI ends the refresh of previous frame (Left and Right area)##*/
//  while(pending_buffer != -1)
  {
  }

  HAL_DMA2D_Start_IT(&DMA2D_Handle, (uint32_t)pSrc, destination, ImageWidth, ImageHeight);
}

/**
 * @brief  DMA2D Transfer completed callback
 * @param  hdma2d: DMA2D handle.
 * @retval None
 */
static void DMA2D_TransferCompleteCallback(DMA2D_HandleTypeDef *hdma2d)
{
  /* Frame Buffer updated , unmask the DSI TE pin to ask for a DSI refersh*/
  pending_buffer = 1;
  /* UnMask the TE */
 
}
 
extern "C" void DMA2D_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2D_IRQn 0 */

  /* USER CODE END DMA2D_IRQn 0 */
  HAL_DMA2D_IRQHandler(&DMA2D_Handle);
  /* USER CODE BEGIN DMA2D_IRQn 1 */

  /* USER CODE END DMA2D_IRQn 1 */
}

/**
  * @brief  Find Next JPEG frame offset in the video file.
  * @param  offset: Current offset in the video file.
  * @param  file: Pointer to the video file object.
  * @retval None
  */
static uint32_t JPEG_FindFrameOffset(uint32_t offset )
{
  uint32_t index = offset, i, readSize = 0;

  do
  {
    if(qsize( ) <=  (index + 1))
    {
      /* end of file reached*/
      return 0;
    }
    qseek(index);
    qread( PatternSearchBuffer, PATTERN_SEARCH_BUFFERSIZE, (UINT*)(&readSize));

    if(readSize != 0)
    {
      for(i = 0; i < (readSize - 1); i++)
      {
        if((PatternSearchBuffer[i] == JPEG_SOI_MARKER_BYTE1) && (PatternSearchBuffer[i + 1] == JPEG_SOI_MARKER_BYTE0))
        {
          return index + i;
        }
      }

      index +=  (readSize - 1);
    }
  }while(readSize != 0);

  return 0;
}


/**
  * @brief  Display Example description.
  * @param  Stage : can be 0 or 1
  *         0 : when the decoding has not started yet
  *         1 : When the decode has ended, in this case display the mjpeg file parameters
  * @retval None
  */
static void LCD_BriefDisplay(uint32_t Stage)
{
 
  if(Stage == 1)
  {
   
    /* JPEG resolution */
    app.DebOut( "Video Resolution   = %lu x %lu", JPEG_Info.ImageWidth, JPEG_Info.ImageHeight);
     
    /* JPEG Quality */
    app.DebOut(  "Approximative JPEG Quality  =   %ld",JPEG_Info.ImageQuality );
   

    /* JPEG Chroma Sampling */
    if(JPEG_Info.ChromaSubsampling  == JPEG_420_SUBSAMPLING)
    {
      app.DebOut(  "ChromaSubsampling           =  4:2:0");
    }
    else if(JPEG_Info.ChromaSubsampling  == JPEG_422_SUBSAMPLING)
    {
      app.DebOut(  "ChromaSubsampling           =  4:2:2");
    }
    else if(JPEG_Info.ChromaSubsampling  == JPEG_444_SUBSAMPLING)
    {
      app.DebOut( "ChromaSubsampling           =  4:4:4");
    }
 
    /* Decoding approximative decoding Frame rate */
    app.DebOut( "Average Decoding Frame Rate =   %lu fps", FrameRate);
    
    /* Number of decoded frames */
    app.DebOut( "Number of Frames            =   %lu", FrameIndex);
    
    pending_buffer = 1;
    
  }
}
  

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t State;
  uint8_t *DataBuffer;
  uint32_t DataBufferSize;

}JPEG_Data_BufferTypeDef;

/* Private define ------------------------------------------------------------*/

#define CHUNK_SIZE_IN  ((uint32_t)(4096))
#define CHUNK_SIZE_OUT ((uint32_t)(768 * 4))

#define JPEG_BUFFER_EMPTY 0
#define JPEG_BUFFER_FULL  1

#define NB_OUTPUT_DATA_BUFFERS      2
#define NB_INPUT_DATA_BUFFERS       2

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
JPEG_YCbCrToRGB_Convert_Function pConvert_Function;

 
uint8_t MCU_Data_OutBuffer0[CHUNK_SIZE_OUT];
uint8_t MCU_Data_OutBuffer1[CHUNK_SIZE_OUT];

uint8_t JPEG_Data_InBuffer0[CHUNK_SIZE_IN];
uint8_t JPEG_Data_InBuffer1[CHUNK_SIZE_IN];

JPEG_Data_BufferTypeDef Jpeg_OUT_BufferTab[NB_OUTPUT_DATA_BUFFERS] =
{
  {JPEG_BUFFER_EMPTY , MCU_Data_OutBuffer0 , 0},
  {JPEG_BUFFER_EMPTY , MCU_Data_OutBuffer1, 0}
};

JPEG_Data_BufferTypeDef Jpeg_IN_BufferTab[NB_INPUT_DATA_BUFFERS] =
{
  {JPEG_BUFFER_EMPTY , JPEG_Data_InBuffer0, 0},
  {JPEG_BUFFER_EMPTY , JPEG_Data_InBuffer1, 0}
};

uint32_t MCU_TotalNb = 0;
uint32_t MCU_BlockIndex = 0;
uint32_t Jpeg_HWDecodingEnd = 0;

uint32_t JPEG_OUT_Read_BufferIndex = 0;
uint32_t JPEG_OUT_Write_BufferIndex = 0;
__IO uint32_t Output_Is_Paused = 0;

uint32_t JPEG_IN_Read_BufferIndex = 0;
uint32_t JPEG_IN_Write_BufferIndex = 0;
__IO uint32_t Input_Is_Paused = 0;

uint32_t FrameBufferAddress;



/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Decode_DMA
  * @param hjpeg: JPEG handle pointer
  * @param  FileName    : jpg file path for decode.
  * @param  DestAddress : ARGB destination Frame Buffer Address.
  * @retval None
  */
uint32_t JPEG_Decode_DMA(JPEG_HandleTypeDef *hjpeg, uint32_t DestAddress)
{
  uint32_t i;

 
  FrameBufferAddress = DestAddress;

  Previous_FrameSize = 0;

  MCU_TotalNb = 0;
  MCU_BlockIndex = 0;
  Jpeg_HWDecodingEnd = 0;

  JPEG_OUT_Read_BufferIndex = 0;
  JPEG_OUT_Write_BufferIndex = 0;
  Output_Is_Paused = 0;

  JPEG_IN_Read_BufferIndex = 0;
  JPEG_IN_Write_BufferIndex = 0;
  Input_Is_Paused = 0;

  Jpeg_OUT_BufferTab[0].State = JPEG_BUFFER_EMPTY;
  Jpeg_OUT_BufferTab[0].DataBufferSize = 0;

  Jpeg_OUT_BufferTab[1].State = JPEG_BUFFER_EMPTY;
  Jpeg_OUT_BufferTab[1].DataBufferSize = 0;

  /* Read from JPG file and fill input buffers */
  for(i = 0; i < NB_INPUT_DATA_BUFFERS; i++)
  {
    if(qread ( Jpeg_IN_BufferTab[i].DataBuffer , CHUNK_SIZE_IN, (UINT*)(&Jpeg_IN_BufferTab[i].DataBufferSize)) == FR_OK)
    {
      Jpeg_IN_BufferTab[i].State = JPEG_BUFFER_FULL;
    }
    else
    {
      assert(0);
    }
  }
  /* Start JPEG decoding with DMA method */
  HAL_JPEG_Decode_DMA(hjpeg ,Jpeg_IN_BufferTab[0].DataBuffer ,Jpeg_IN_BufferTab[0].DataBufferSize ,Jpeg_OUT_BufferTab[0].DataBuffer ,CHUNK_SIZE_OUT);

  return 0;
}

/**
  * @brief  JPEG Ouput Data BackGround Postprocessing .
  * @param hjpeg: JPEG handle pointer
  * @retval 1 : if JPEG processing has finiched, 0 : if JPEG processing still ongoing
  */
uint32_t JPEG_OutputHandler(JPEG_HandleTypeDef *hjpeg)
{
  uint32_t ConvertedDataCount;

  if(Jpeg_OUT_BufferTab[JPEG_OUT_Read_BufferIndex].State == JPEG_BUFFER_FULL)
  {
    MCU_BlockIndex += pConvert_Function(Jpeg_OUT_BufferTab[JPEG_OUT_Read_BufferIndex].DataBuffer, (uint8_t *)FrameBufferAddress, MCU_BlockIndex, Jpeg_OUT_BufferTab[JPEG_OUT_Read_BufferIndex].DataBufferSize, &ConvertedDataCount);

    Jpeg_OUT_BufferTab[JPEG_OUT_Read_BufferIndex].State = JPEG_BUFFER_EMPTY;
    Jpeg_OUT_BufferTab[JPEG_OUT_Read_BufferIndex].DataBufferSize = 0;

    JPEG_OUT_Read_BufferIndex++;
    if(JPEG_OUT_Read_BufferIndex >= NB_OUTPUT_DATA_BUFFERS)
    {
      JPEG_OUT_Read_BufferIndex = 0;
    }

    if(MCU_BlockIndex >= MCU_TotalNb)
    {
      return 1;
    }
  }
  else if((Output_Is_Paused == 1) && \
          (JPEG_OUT_Write_BufferIndex == JPEG_OUT_Read_BufferIndex) && \
          (Jpeg_OUT_BufferTab[JPEG_OUT_Read_BufferIndex].State == JPEG_BUFFER_EMPTY))
  {
    Output_Is_Paused = 0;
    HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
  }

  return 0;
}

/**
  * @brief  JPEG Input Data BackGround processing .
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void JPEG_InputHandler(JPEG_HandleTypeDef *hjpeg)
{
  if(Jpeg_IN_BufferTab[JPEG_IN_Write_BufferIndex].State == JPEG_BUFFER_EMPTY)
  {
    if(qread ( Jpeg_IN_BufferTab[JPEG_IN_Write_BufferIndex].DataBuffer , CHUNK_SIZE_IN, (UINT*)(&Jpeg_IN_BufferTab[JPEG_IN_Write_BufferIndex].DataBufferSize)) == FR_OK)
    {
      Jpeg_IN_BufferTab[JPEG_IN_Write_BufferIndex].State = JPEG_BUFFER_FULL;
    }
    else
    {
      assert(0);
    }

    if((Input_Is_Paused == 1) && (JPEG_IN_Write_BufferIndex == JPEG_IN_Read_BufferIndex))
    {
      Input_Is_Paused = 0;
      HAL_JPEG_ConfigInputBuffer(hjpeg,Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBuffer, Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBufferSize);

      HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_INPUT);
    }

    JPEG_IN_Write_BufferIndex++;
    if(JPEG_IN_Write_BufferIndex >= NB_INPUT_DATA_BUFFERS)
    {
      JPEG_IN_Write_BufferIndex = 0;
    }
  }
}

/**
  * @brief  JPEG Info ready callback
  * @param hjpeg: JPEG handle pointer
  * @param pInfo: JPEG Info Struct pointer
  * @retval None
  */
void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hjpeg, JPEG_ConfTypeDef *pInfo)
{
  assert(JPEG_GetDecodeColorConvertFunc(pInfo, &pConvert_Function, &MCU_TotalNb) == HAL_OK);
 
}

/**
  * @brief  JPEG Get Data callback
  * @param hjpeg: JPEG handle pointer
  * @param NbDecodedData: Number of decoded (consummed) bytes from input buffer
  * @retval None
  */
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
  if(NbDecodedData == Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBufferSize)
  {
    Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].State = JPEG_BUFFER_EMPTY;
    Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBufferSize = 0;

    JPEG_IN_Read_BufferIndex++;
    if(JPEG_IN_Read_BufferIndex >= NB_INPUT_DATA_BUFFERS)
    {
      JPEG_IN_Read_BufferIndex = 0;
    }

    if(Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].State == JPEG_BUFFER_EMPTY)
    {
      HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_INPUT);
      Input_Is_Paused = 1;
    }
    else
    {
      HAL_JPEG_ConfigInputBuffer(hjpeg,Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBuffer, Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBufferSize);
    }
  }
  else
  {
    HAL_JPEG_ConfigInputBuffer(hjpeg,Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBuffer + NbDecodedData, Jpeg_IN_BufferTab[JPEG_IN_Read_BufferIndex].DataBufferSize - NbDecodedData);
  }

  Previous_FrameSize += NbDecodedData;
}

/**
  * @brief  JPEG Data Ready callback
  * @param hjpeg: JPEG handle pointer
  * @param pDataOut: pointer to the output data buffer
  * @param OutDataLength: length of output buffer in bytes
  * @retval None
  */
void HAL_JPEG_DataReadyCallback (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
  Jpeg_OUT_BufferTab[JPEG_OUT_Write_BufferIndex].State = JPEG_BUFFER_FULL;
  Jpeg_OUT_BufferTab[JPEG_OUT_Write_BufferIndex].DataBufferSize = OutDataLength;

  JPEG_OUT_Write_BufferIndex++;
  if(JPEG_OUT_Write_BufferIndex >= NB_OUTPUT_DATA_BUFFERS)
  {
    JPEG_OUT_Write_BufferIndex = 0;
  }

  if(Jpeg_OUT_BufferTab[JPEG_OUT_Write_BufferIndex].State != JPEG_BUFFER_EMPTY)
  {
    HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
    Output_Is_Paused = 1;
  }
  HAL_JPEG_ConfigOutputBuffer(hjpeg, Jpeg_OUT_BufferTab[JPEG_OUT_Write_BufferIndex].DataBuffer, CHUNK_SIZE_OUT);
}

/**
  * @brief  JPEG Error callback
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg)
{
  assert(0);
}

/**
  * @brief  JPEG Decode complete callback
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{
  Jpeg_HWDecodingEnd = 1;
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
