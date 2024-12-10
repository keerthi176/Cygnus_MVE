/*
 / _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
 \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 _____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
    (C)2013 Semtech

Description: Target board general functions implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/
#include "board.h"
#include "MM_Main.h"

/*!
 * Battery level ratio (battery dependent)
 */
#define BATTERY_STEP_LEVEL                          0.23

/*!
 * Unique Devices IDs register set ( STM32L1xxx )
 */
#define         ID1                                 ( 0x1FF80050 )
#define         ID2                                 ( 0x1FF80054 )
#define         ID3                                 ( 0x1FF80064 )

/*!
 * IO Extander pins objects
 */
Gpio_t IrqMpl3115;
Gpio_t IrqMag3110;
Gpio_t GpsPowerEn;
Gpio_t NcIoe3;
Gpio_t NcIoe4;
Gpio_t NcIoe5;
Gpio_t NcIoe6;
Gpio_t NcIoe7;
Gpio_t NIrqSx9500;
Gpio_t Irq1Mma8451;
Gpio_t Irq2Mma8451;
Gpio_t TxEnSx9500;
Gpio_t Led1;
Gpio_t Led2;
Gpio_t Led3;

#if defined( USE_DEBUG_PINS )
Gpio_t DbgPin1;
Gpio_t DbgPin2;
Gpio_t DbgPin3;
Gpio_t DbgPin4;
Gpio_t DbgPin5;
#endif

/*
 * MCU objects
 */
Gpio_t GpsPps;
Gpio_t GpsRx;
Gpio_t GpsTx;

//Adc_t Adc;
//I2c_t I2c;
Uart_t Uart1;
#if defined( USE_USB_CDC )
Uart_t UartUsb;
#endif


#ifdef USE_PIN_11
Gpio_t Pin11;
#endif
#ifdef USE_PIN_12
Gpio_t Pin12;
#endif

/*
 * MCU GPIO pin objects
 */
Gpio_t PA2;
Gpio_t PA3;
Gpio_t Battery1;
Gpio_t Battery2;
Gpio_t Battery3;
Gpio_t Battery4;
Gpio_t InstallationTamper;
Gpio_t Osc32In;
Gpio_t Osc32Out;
Gpio_t Ph0OscIn;
Gpio_t Ph1OscOut;
Gpio_t HeadRx;
Gpio_t HeadTx;
Gpio_t HeadPowerOn;
Gpio_t PC2;
Gpio_t DismantleTamper;
Gpio_t AdcIn1;
Gpio_t PC4;
Gpio_t PC5;
Gpio_t AntRxNtx;
Gpio_t AntTxNrx;
Gpio_t StatusLedGreen;
Gpio_t StatusLedRed;
Gpio_t PrimaryBatIrq;
Gpio_t BatMonEn;
Gpio_t PriBatteryEn;
Gpio_t TamperEnable;
Gpio_t PB1;
Gpio_t PB13;
Gpio_t VbatMonEn;
Gpio_t VBkpBatMonEn;
Gpio_t SVIPowerOn;
Gpio_t SVIclk;
Gpio_t SVIdata;
Gpio_t Ant2Sel;
Gpio_t Ant1Sel;
Gpio_t FirstAidMCP;
Gpio_t FireMCP;
Gpio_t PpuTx;
Gpio_t PpuRx;
Gpio_t StatusLedBlue;
Gpio_t DebugTx;
Gpio_t DebugRx;
Gpio_t PB6;
Gpio_t SxReset;
Gpio_t PB9;
Gpio_t SwdDIO;
Gpio_t SwdCLK;
Gpio_t IP1Det;
Gpio_t IP2Det;
Gpio_t BkpBatEn;
Gpio_t SndrBeaconOn;
Gpio_t IODSet;
Gpio_t IODReset;
Gpio_t IODRlyEn;
Gpio_t DetR1;
Gpio_t DetR2;

Gpio_t EnDet1;

Gpio_t PA0;
Gpio_t PC0;
Gpio_t PC1;
Gpio_t PC8;
Gpio_t I2CPullupsOn;
Gpio_t BatVMon;//IOU,MCP
Gpio_t AnVbatMon;//construction
Gpio_t IODRlyEn;
Gpio_t DetR1;
Gpio_t DetR2;

Gpio_t FaultDet1;
Gpio_t FaultDet2;
Gpio_t Pir;


/*************************************************************************************/
/**
* BoardConfigureGpio
* Routine to set the Pin configuration for each target base type (NCU, RBU, Construction etc)
*
* @param - baseType     - The role of the radio board (NCU, RBU, MCP, IOU, CONSTRUCTION)
* @param - noHeadFitted - True if no plugin in configured (for blanking plate gpio)
*
* @return - None.
*/
void BoardConfigureGpio( const DM_BaseType_t baseType, const bool noHeadFitted  )
{
   /* initialise pins that are common to all unit types */
   GpioInit( &StatusLedGreen, CN_STATUS_LED_GREEN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
   GpioInit( &StatusLedRed, CN_STATUS_LED_RED, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
   GpioInit( &StatusLedBlue, CN_STATUS_LED_BLUE, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
   GpioInit( &PA0, HEAD_TMP_FLT, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );

   SX1272AntSwInit( );
   
   /* Initialise the pins that change with base type */
   switch (baseType)
   {
      case BASE_RBU_E:
         /* Port A */
         GpioInit( &InstallationTamper, BASE_TMP_FLT, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery1, BAT_I1_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery2, BAT_I2_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &SVIPowerOn, SVI_PWR_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &I2CPullupsOn, I2C_PULLUPS_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port B */
         GpioInit( &PrimaryBatIrq, PRI_BAT_IRQ, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BatMonEn, BAT_MON_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &PriBatteryEn, PRI_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &TamperEnable, TAMPER_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &SVIclk, I2C2_SCL, PIN_ALTERNATE_FCT_E, PIN_OPEN_DRAIN_E, PIN_PULL_UP_E, GPIO_AF4_I2C1 );
         GpioInit( &SVIdata, I2C2_SDA, PIN_ALTERNATE_FCT_E, PIN_OPEN_DRAIN_E, PIN_PULL_UP_E, GPIO_AF4_I2C1 );
         GpioInit( &PB1, CN_HEAD_PWR_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port C */
         GpioInit( &BatVMon, BAT_VMON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery3, BAT_I3_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BkpBatEn, BKP_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         if ( noHeadFitted )
         {
            GpioInit( &HeadPowerOn, CN_HEAD_PWR_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
            GpioInit( &HeadTx, CN_HEAD_TX, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
            GpioInit( &HeadRx, CN_HEAD_RX, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0 );
         }

         break;
      
      case BASE_IOU_E:
         /* Port A */
         GpioInit( &DismantleTamper, HEAD_TMP_FLT, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &IODRlyEn, IOD_RLY_EN_IOD_R1R2, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &FaultDet1, CN_FAULT_DET_1_IN, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery1, BAT_I1_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery2, BAT_I2_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &IODSet, IOD_SET, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &DetR1, DET_R1, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port B */
         GpioInit( &PrimaryBatIrq, PRI_BAT_IRQ, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &PriBatteryEn, PRI_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &TamperEnable, TAMPER_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &DetR1, DET_R1, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_DOWN_E, 0 );
         GpioInit( &EnDet1, EN_DET_1, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BatMonEn, BAT_MON_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port C */
         GpioInit( &Battery3, BAT_I3_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &IODReset, IOD_RESET, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BatVMon, BAT_VMON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &DetR2, DET_R2, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_DOWN_E, 0 );
         GpioInit( &FaultDet2, CN_FAULT_DET_2_IN, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BkpBatEn, BKP_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         break;
      
      case BASE_MCP_E:
         /* Port A */
         GpioInit( &InstallationTamper, BASE_TMP_FLT, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery1, BAT_I1_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery2, BAT_I2_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port B */
         GpioInit( &BatMonEn, BAT_MON_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &PriBatteryEn, PRI_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &TamperEnable, TAMPER_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port C */
         GpioInit( &BkpBatEn, BKP_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery3, BAT_I3_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BatVMon, BAT_VMON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &FirstAidMCP, CN_FA_MCP_IN, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );
         /* Port D */
         GpioInit( &FireMCP, CN_FIRE_MCP_IN, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );
         break;
      
      case BASE_CONSTRUCTION_E:
         /* Port A */
         GpioInit( &DismantleTamper, HEAD_TMP_FLT, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &InstallationTamper, BASE_TMP_FLT, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &PA2, AN_BKP_CELL_1_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &PA3, AN_BKP_BAT_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &SndrBeaconOn, SNDR_BEACON_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &SVIPowerOn, SVI_PWR_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &I2CPullupsOn, I2C_PULLUPS_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port B */
         GpioInit( &PrimaryBatIrq, PRI_BAT_IRQ, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &PriBatteryEn, PRI_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &VBkpBatMonEn, BKP_BAT_MON_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &TamperEnable, TAMPER_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &VbatMonEn, CONST_PRI_BAT_MON_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port C */
         GpioInit( &AnVbatMon, AN_VBAT_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Pir, CONSTR_PIR_IN, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );
         GpioInit( &FirstAidMCP, CN_FA_MCP_IN, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );
         GpioInit( &BkpBatEn, BKP_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port D */
         GpioInit( &FireMCP, CN_FIRE_MCP_IN, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 1 );
         break;
      
      case BASE_REPEATER_E:
         /* Port A */
         GpioInit( &InstallationTamper, BASE_TMP_FLT, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery1, BAT_I1_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery2, BAT_I2_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port B */
         GpioInit( &PrimaryBatIrq, PRI_BAT_IRQ, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BatMonEn, BAT_MON_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &PriBatteryEn, PRI_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &TamperEnable, TAMPER_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         /* Port C */
         GpioInit( &BatVMon, BAT_VMON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &Battery3, BAT_I3_MON, PIN_ALTERNATE_FCT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &BkpBatEn, BKP_BAT_EN, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );
         GpioInit( &HeadPowerOn, CN_HEAD_PWR_ON, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E,0 );//blanking plate tamper
         GpioInit( &HeadTx, CN_HEAD_TX, PIN_OUTPUT_E, PIN_PUSH_PULL_E, PIN_NO_PULL_E, 0 );//blanking plate tamper
         GpioInit( &HeadRx, CN_HEAD_RX, PIN_INPUT_E, PIN_PUSH_PULL_E, PIN_PULL_UP_E, 0); //blanking plate tamper
         break;
      
      case BASE_NCU_E:
         break;
      
      default:
         break;
   }
}



   


   
   
#ifdef USE_FULL_ASSERT
/*
 * Function Name  : assert_failed
 * Description    : Reports the name of the source file and the source line number
 *                  where the assert_param error has occurred.
 * Input          : - file: pointer to the source file name
 *                  - line: assert_param error line source number
 * Output         : None
 * Return         : None
 */
void assert_failed( uint8_t* file, uint32_t line )
{
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while( 1 )
    {
    }
}
#endif
