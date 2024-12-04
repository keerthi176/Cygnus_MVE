
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_TS_H
#define __BSP_TS_H

#ifdef __cplusplus
 extern "C" {
#endif   
   
/* Includes ------------------------------------------------------------------*/

	 
/* Include TouchScreen component driver */
#include "stmpe811/stmpe811.h"   
   

typedef struct
{
  uint16_t TouchDetected;
  uint16_t X;
  uint16_t Y;
  uint16_t Z;
}TS_StateTypeDef;

#define TS_SWAP_NONE                    0x00
#define TS_SWAP_X                       0x01
#define TS_SWAP_Y                       0x02
#define TS_SWAP_XY                      0x04

typedef enum 
{
  TS_OK       = 0x00,
  TS_ERROR    = 0x01,
  TS_TIMEOUT  = 0x02
} TS_StatusTypeDef;

uint8_t BSP_TS_Init( void );
void    BSP_TS_GetState(TS_StateTypeDef *TsState);
uint8_t BSP_TS_ITConfig(void);
uint8_t BSP_TS_ITGetStatus(void);
void BSP_TS_ITClear(void);
void BSP_TS_Calibrate( int xmin, int ymin, int xmax, int ymax, int x_xmin, int x_ymin, int x_xmax, int x_ymax );


#endif /* __BSP_TS_H */

