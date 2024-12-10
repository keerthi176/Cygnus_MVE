/*************************************************************************************
*  CONFIDENTIAL between PA and Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2017 PA Consulting Group. All rights reserved.
*  Cambridge Technology Centre
*  Melbourn Royston
*  Herts SG8 6DP UK
*  Tel: <44> 1763 261222
*
**************************************************************************************
*  File         : DM_MCUPower.c
*
*  Description  :
*
*   This module provides functionality for:
*   -Entering STOP mode on the MCU
*
*************************************************************************************/



/* System Include files
*******************************************************************************/
#include <stdint.h>


/* User Include files
*******************************************************************************/
#include "stm32l4xx_hal.h"
#include "board.h"
#include "DM_SystemClock.h"
#include "DM_MCUPower.h"
#include "lptim.h"
#include "DM_Device.h"

#ifdef ENABLE_STOP_MODE

/* Global Variables
*******************************************************************************/
extern uint16_t gIRQlptim1Value;
extern LPTIM_HandleTypeDef hlptim1;
extern bool gIRQlptim1ValueFromWakeUp;
extern DM_BaseType_t gBaseType;

/*************************************************************************************/
/**
* function name   : gpio_set_stop_mode_config
* description      : Configure the GPIO in preparation for the stop mode
*
* @param - void
*
* @return - void
*/
__STATIC_INLINE void gpio_set_stop_mode_config(uint32_t (*configuration)[8])
{
   uint32_t port_mode;
   
   if ( configuration )
   {
      /* Save the current configuration */
      (*configuration)[0] = READ_REG(GPIOA->MODER); 
      (*configuration)[1] = READ_REG(GPIOB->MODER); 
      (*configuration)[2] = READ_REG(GPIOC->MODER); 
      (*configuration)[3] = READ_REG(GPIOD->MODER); 

      /********************* GPIO PORT A ***********************/
      port_mode = (((uint32_t)3u << ((SX_NSS%16u)*2))
                  |((uint32_t)3u << ((CN_PPU_TX%16u)*2))
                  |((uint32_t)3u << ((CN_PPU_RX%16u)*2))
                  |((uint32_t)3u << ((I2C_PULLUPS_ON%16u)*2)));

      if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_15) == GPIO_PIN_SET)
      {
         /* LED3 is High -> Do not change its configuration */
         port_mode |= ((uint32_t)3u << (CN_STATUS_LED_BLUE%16u)*2);
      }
      
      if ( BASE_RBU_E == gBaseType )
      {
         /* Preserve the SVI power-on signal */
         port_mode |= ((uint32_t)3u << (SVI_PWR_ON%16u)*2);
      }
      else if ( BASE_IOU_E == gBaseType )
      {
         /* IO Relay Enable is high.  Don't change it's config */
         if ( HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8) == GPIO_PIN_SET)
         {
            port_mode |= ((uint32_t)3u << (IOD_RLY_EN_IOD_R1R2%16u)*2);
         }
         /* IOD Set is high.  Don't change it's config */
         if ( HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_SET)
         {
            port_mode |= ((uint32_t)3u << (IOD_SET%16u)*2);
         }
      }
      else if ( BASE_CONSTRUCTION_E == gBaseType )
      {
         /* If the Sounder/Beacon is on, prserve the power-on signal */
         if ( HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11) == GPIO_PIN_SET)
         {
            port_mode |= ((uint32_t)3u << (SNDR_BEACON_ON%16u)*2);
         }
      }

      
      #ifdef _DEBUG
      /* Debug Pins */
      port_mode |= (((uint32_t)3u << ((CN_SWD_SWCLK%16u)*2))
                   |((uint32_t)3u << ((CN_SWD_SWDIO%16u)*2)));

      #endif

      /* Set all to analogue mode except the pins above */
      WRITE_REG(GPIOA->MODER, ((*configuration)[0] & port_mode) | ~port_mode); 
      
      /********************* GPIO PORT B ***********************/
      port_mode = (((uint32_t)3u << ((SX_DIO3%16u)*2))
                  |((uint32_t)3u << ((SX_DIO2%16u)*2))
                  |((uint32_t)3u << ((SX_DIO1%16u)*2))
                  |((uint32_t)3u << ((SX_DIO0%16u)*2))
                  |((uint32_t)3u << ((SX_RESET%16u)*2))
                  |((uint32_t)3u << ((ANT_RX_NTX%16u)*2))
                  |((uint32_t)3u << ((ANT_TX_NRX%16u)*2))
                  |((uint32_t)3u << ((PRI_BAT_EN%16u)*2))
                  |((uint32_t)3u << ((BKP_BAT_EN%16u)*2))
                  |((uint32_t)3u << ((PRI_BAT_IRQ%16u)*2)));
                  
      
      if ( (BASE_RBU_E == gBaseType) || (BASE_IOU_E == gBaseType) || (BASE_MCP_E == gBaseType) )
      {
         /* preserve the battery monitor line */
         port_mode |= ((uint32_t)3u << (BAT_MON_EN%16u)*2);
      }
      
      if ( (BASE_RBU_E == gBaseType) || (BASE_CONSTRUCTION_E == gBaseType) )
      {
         /* Head Power On is high.  Don't change it's config */
         if ( HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
         {
            port_mode |= ((uint32_t)3u << (CN_HEAD_PWR_ON%16u)*2);
         }
         
         if ( BASE_CONSTRUCTION_E == gBaseType )
         {
            //Preserve the battery monitor enable lines
            port_mode |= ((uint32_t)3u << (CONST_PRI_BAT_MON_EN%16u)*2);
            port_mode |= ((uint32_t)3u << (BKP_BAT_MON_EN%16u)*2);
         }
      }
      else if ( BASE_IOU_E == gBaseType )
      {
         /* Don't disturb the EN_DET input */
         port_mode |= ((uint32_t)3u << (EN_DET_1%16u)*2);
         /* Don't disturb the DET_Rx inputs */
         port_mode |= ((uint32_t)3u << (DET_R1%16u)*2);
         port_mode |= ((uint32_t)3u << (DET_R2%16u)*2);
      }
      
      /* Preserve the Head power signal and tamper enable for all base types except the NCU */
      if ( BASE_NCU_E != gBaseType )
      {
         port_mode |= ((uint32_t)3u << (CN_HEAD_PWR_ON%16u)*2);
         port_mode |= ((uint32_t)3u << (TAMPER_EN%16u)*2);
      }
      
      if ( BASE_CONSTRUCTION_E == gBaseType )
      {
         port_mode |= ((uint32_t)3u << (CONST_PRI_BAT_MON_EN%16u)*2);
         port_mode |= ((uint32_t)3u << (BKP_BAT_MON_EN%16u)*2);
      }

      
      /* Set all to analogue mode except the pins above */
      WRITE_REG(GPIOB->MODER, ((*configuration)[1] & port_mode) | ~port_mode); 
      
      /********************* GPIO PORT C ***********************/
      port_mode = (((uint32_t)3u << ((CN_DEBUG_TX%16u)*2))
                  |((uint32_t)3u << ((CN_DEBUG_RX%16u)*2))
                  |((uint32_t)3u << ((BKP_BAT_EN%16u)*2)));
      
      if ( BASE_RBU_E == gBaseType )
      {
         /* Don't disturb the Head comms */
         port_mode |= ((uint32_t)3u << ((CN_HEAD_RX%16u)*2));
         port_mode |= ((uint32_t)3u << ((CN_HEAD_TX%16u)*2));
         //Preserve the backup battery monitor enable
         port_mode |= ((uint32_t)3u << (BAT_VMON%16u)*2);
      }
      else if ( BASE_MCP_E == gBaseType )
      {
         /* Don't disable the first aid call point */
         port_mode |= ((uint32_t)3u << ((CN_FA_MCP_IN%16u)*2));
         //Preserve the backup battery monitor enable
         port_mode |= ((uint32_t)3u << (BAT_VMON%16u)*2);
      }
      else if ( BASE_IOU_E == gBaseType )
      {
         /* Don't disable the fault detect inputs */
         port_mode |= ((uint32_t)3u << ((CN_FAULT_DET_1_IN%16u)*2));
         port_mode |= ((uint32_t)3u << ((CN_FAULT_DET_2_IN%16u)*2));
         /* Don't disturb the EN_DET input */
         port_mode |= ((uint32_t)3u << (EN_DET_2%16u)*2);
         //Preserve the backup battery monitor enable
         port_mode |= ((uint32_t)3u << (BAT_VMON%16u)*2);
      }
      else if ( BASE_CONSTRUCTION_E == gBaseType )
      {
         /* Don't disturb the Head comms */
         port_mode |= ((uint32_t)3u << ((CN_HEAD_RX%16u)*2));
         port_mode |= ((uint32_t)3u << ((CN_HEAD_TX%16u)*2));
         /* Don't disable the PIR input */
         port_mode |= ((uint32_t)3u << ((CONSTR_PIR_IN%16u)*2));
         /* Don't disable the first aid call point */
         port_mode |= ((uint32_t)3u << ((CN_FA_MCP_IN%16u)*2));
      }
      
      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_SET)
      {
         /* LED1 is High -> Do not change its configuration */
         port_mode |= ((uint32_t)3u << (CN_STATUS_LED_RED%16u)*2);
      }

      if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_SET)
      {
         /* LED2 is High -> Do not change its configuration */
         port_mode |= ((uint32_t)3u << (CN_STATUS_LED_GREEN%16u)*2);
      }
      
      /* Set all to analogue mode except the pins above */
      WRITE_REG(GPIOC->MODER, ((*configuration)[2] & port_mode) | ~port_mode);    

      /********************* GPIO PORT D ***********************/
      port_mode = 0;
      if ( BASE_MCP_E == gBaseType ||
           BASE_CONSTRUCTION_E == gBaseType )
      {
         /* Don't disable the fire call point */
         port_mode |= ((uint32_t)3u << ((CN_FIRE_MCP_IN%16u)*2));
      }

      /* Set all to analogue mode except the pins above */
      WRITE_REG(GPIOD->MODER, ((*configuration)[3] & port_mode) | ~port_mode); 
   }
}

/*************************************************************************************/
/**
* function name   : gpio_restore_config
* description      : Configure the GPIO on leaving stop mode
*
* @param - void
*
* @return - void
*/
__STATIC_INLINE void gpio_restore_config(uint32_t (*configuration)[8])
{
   if ( configuration )
   {
      WRITE_REG(GPIOA->MODER, (*configuration)[0]);
      WRITE_REG(GPIOB->MODER, (*configuration)[1]);
      WRITE_REG(GPIOC->MODER, (*configuration)[2]);
      WRITE_REG(GPIOD->MODER, (*configuration)[3]);
   }
}
/*************************************************************************************/
/**
* function name   : DM_MCUPower_Enter_STOPMode
* description      : Routine to enter STOP mode. Once stopped the MCU will remain stopped
*                   until a wake-up event occurs (LPTIM or GPIO input)
*
* @param - void
*
* @return - void
*/
void DM_MCUPower_Enter_STOPMode(void)
{
   uint32_t ModexArr[8];

   /* Put NVM in Power down mode whilst in STOP Mode */
   __HAL_FLASH_SLEEP_POWERDOWN_ENABLE();

   /* Change the GPIO config for the stop mode */
   gpio_set_stop_mode_config(&ModexArr);

   /* Prepare the system clocks for the stop mode */
   DM_SystemClockStopConfig();

   #ifdef _DEBUG
   /* Enable the Debug Module during Stop mode mode */
   HAL_DBGMCU_EnableDBGStopMode();
   #endif
   
   /* Enter STOP 2 mode */
   HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);

   /* First thing to do when leaving the stop mode is to capture the timestamp 
    * if a GPIO interrupt is pending (It might be raised by the SX1273 chip) 
    */
   if (HAL_NVIC_GetPendingIRQ(EXTI9_5_IRQn) != 0) /* SX1273 DIO0 */
   {
      /* Store the current value of the LPTIM1. This is used for synchronisation timing of RxDone
       * interrupt from LoRa modem.
       */
      gIRQlptim1Value = LPTIM_ReadCounter(&hlptim1);    

      /* Notification that we have a time value at the wake-up stage */
      gIRQlptim1ValueFromWakeUp = true;
   }
  
   /* Re-configure system clocks after wakeup */
   DM_SystemClockWakeUpConfig();

   /* Clear Wakeup event flag */
   __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

   /* Woken up from STOP mode; Restore the GPIO settings */
   gpio_restore_config(&ModexArr);
}
#endif





/*************************************************************************************/
/**
* function name   : DM_MCUPower_Enter_SLEEPMode
* description      : Configure the GPIO in preparation for sleep mode
*
* @param - void
*
* @return - void
*/
void DM_MCUPower_Enter_SLEEPMode(void)
{
   uint32_t port_mode;
   uint32_t configuration[8];
   
   /* Save the current configuration */
   configuration[0] = READ_REG(GPIOA->MODER); 
   configuration[1] = READ_REG(GPIOB->MODER); 
   configuration[2] = READ_REG(GPIOC->MODER); 
   configuration[3] = READ_REG(GPIOD->MODER); 

   /********************* GPIO PORT A ***********************/
   port_mode = (((uint32_t)3u << ((SX_NSS%16u)*2)));

   
   #ifdef _DEBUG
   /* Debug Pins */
   port_mode |= (((uint32_t)3u << ((CN_SWD_SWCLK%16u)*2))
                |((uint32_t)3u << ((CN_SWD_SWDIO%16u)*2)));

   #endif

   /* Set all to analogue mode except the pins above */
   WRITE_REG(GPIOA->MODER, (configuration[0] & port_mode) | ~port_mode); 
   
   /********************* GPIO PORT B ***********************/
   port_mode = (((uint32_t)3u << ((SX_DIO3%16u)*2))
               |((uint32_t)3u << ((SX_DIO2%16u)*2))
               |((uint32_t)3u << ((SX_DIO1%16u)*2))
               |((uint32_t)3u << ((SX_DIO0%16u)*2))
               |((uint32_t)3u << ((SX_RESET%16u)*2))
               |((uint32_t)3u << ((ANT_RX_NTX%16u)*2))
               |((uint32_t)3u << ((ANT_TX_NRX%16u)*2)));
               
   
   if ( (BASE_RBU_E == gBaseType) || (BASE_IOU_E == gBaseType) || (BASE_MCP_E == gBaseType) )
   {
      /* preserve the battery monitor lines */
      port_mode |= ((uint32_t)3u << (PRI_BAT_EN%16u)*2);
      port_mode |= ((uint32_t)3u << (BAT_MON_EN%16u)*2);
      port_mode |= ((uint32_t)3u << (PRI_BAT_IRQ%16u)*2);
   }
   
   
   /* Set all to analogue mode except the pins above */
   WRITE_REG(GPIOB->MODER, (configuration[1] & port_mode) | ~port_mode); 
   
   /********************* GPIO PORT C ***********************/
   port_mode = (((uint32_t)3u << ((CN_DEBUG_TX%16u)*2))
               |((uint32_t)3u << ((CN_DEBUG_RX%16u)*2)));
   
   if ( BASE_RBU_E == gBaseType )
   {
      /* Don't disturb the Head comms */
      port_mode |= ((uint32_t)3u << ((CN_HEAD_RX%16u)*2));
      port_mode |= ((uint32_t)3u << ((CN_HEAD_TX%16u)*2));
      /* Preserve the back-up battery ON signal */
      port_mode |= ((uint32_t)3u << ((BKP_BAT_EN%16u)*2));
   }
   else if ( BASE_MCP_E == gBaseType )
   {
      /* Preserve the back-up battery ON signal */
      port_mode |= ((uint32_t)3u << ((BKP_BAT_EN%16u)*2));
   }
   else if ( BASE_IOU_E == gBaseType )
   {
      /* Don't disturb the EN_DET input */
      port_mode |= ((uint32_t)3u << (EN_DET_2%16u)*2);
      /* Preserve the back-up battery ON signal */
      port_mode |= ((uint32_t)3u << ((BKP_BAT_EN%16u)*2));
   }
   
   /* Set all to analogue mode except the pins above */
   WRITE_REG(GPIOC->MODER, (configuration[2] & port_mode) | ~port_mode);    

   /********************* GPIO PORT D ***********************/
   port_mode = 0;

   /* Set all to analogue mode except the pins above */
   WRITE_REG(GPIOD->MODER, (configuration[3] & port_mode) | ~port_mode); 

   
}
