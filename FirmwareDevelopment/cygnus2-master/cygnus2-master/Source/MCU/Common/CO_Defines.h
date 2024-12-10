/*************************************************************************************
*  CONFIDENTIAL Bull Products Ltd.
**************************************************************************************
*  Cygnus2
*  Copyright 2018 Bull Products Ltd. All rights reserved.
*  Beacon House, 4 Beacon Rd
*  Rotherwas Industrial Estate
*  Hereford HR2 6JF UK
*  Tel: <44> 1432806806
*
**************************************************************************************
*  File         : CO_Defines.h
*
*  Description  : System Definitions 
*
*************************************************************************************/

#ifndef CO_DEFINES_H
#define CO_DEFINES_H


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "CO_SerialDebug.h"


/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/



/* Public Functions Prototypes
*************************************************************************************/



/* Public Constants
*************************************************************************************/
/* General constants */
#define BITS_PER_BYTE                  8
#define BYTES_PER_WORD                 4
#define SECONDS_IN_ONE_DAY             86400
#define SECONDS_IN_ONE_HOUR            3600
// These three macros are tightly coupled.  Don't change any one of them without reviewing use and values of all three.
#define BITS_PER_WORD                  32    // If this is changed, the code that uses BITS_PER_WORD_SHIFT and BITS_PER_WORD_MASK must be reviewed.
#define BITS_PER_WORD_SHIFT            5     // This macro only valid if BITS_PER_WORD is 32.  Use >> BITS_PER_WORD_SHIFT instead of / BITS_PER_WORD.
#define BITS_PER_WORD_MASK             0X1F  // This macro only valid if BITS_PER_WORD is 32.  Use  & BITS_PER_WORD_MASK  instead of % BITS_PER_WORD.

/* System constants */
#define MAX_DEVICES_PER_SYSTEM         512
#define CO_BAD_NODE_ID                 0xffff
#define NO_TIMESTAMP_RECORDED          0xFFFFFFFFU
#define LPTIM_TICKS_PER_SEC            16384U    // 512 / 31.25 mS
#define LOW_POWER_SLEEP_ENABLE                     // The definitions below set the default times for the low power sleep cycle when no heartbeat is received on start-up.
#define LOW_POWER_INITIAL_AWAKE_DURATION (72u)     // 72 hours.  The time to stay in continuous receive after start-up
#define LOW_POWER_AWAKE_DURATION         (1800u)   // 30 minutes in seconds.  The time to stay awake and in continuous Rx during the start-up sleep cycle
#define LOW_POWER_PHASE2_DURATION        (14u)     // 14 days.  The duration of phase 2 of the start-up sleep cycle.
#define LOW_POWER_PHASE2_SLEEP_DURATION  (3u)      // 3 hours.  The duration of sleep during the start-up sleep cycle phase 2
#define LOW_POWER_PHASE3_SLEEP_DURATION  (6u)      // 6 hours.  The duration of sleep during the start-up sleep cycle phase 3
#define MAX_LOW_POWER_INITIAL_AWAKE_DURATION (99u)
#define MAX_LOW_POWER_SLEEP_DURATION     (24u)
#define MAX_BATTERY_TEST_INTERVAL         49700  //days.  Max that can be multiplied into seconds and fit in 32 bits.
#define MAX_BATTERY_S_FAIL_LOW_THRESH    (4000U) /* For SmartNet     */
#define MAX_BATTERY_C_FAIL_LOW_THRESH    (9500U) /* For Construction */
#define ADDRESS_GLOBAL                    0xFFF
#define PPU_ADDRESS_GLOBAL                0xFFFF
#define ADDRESS_NCU                       0
#define SYSTEM_ID_GLOBAL                  0xFFFFFFFF
#define ZONE_GLOBAL                       0xFF
#define NON_LATCHING_MCP_RESET_PERIOD     5 // seconds.  How long a non latching call point should be held down to cancel the alarm.
#define NON_LATCHING_MCP_TIMEOUT_PERIOD   10  //seconds. How long the alarm signal is blocked after each alarm message
#define MAX_OTA_AT_COMMAND_LENGTH         46
#define OTA_AT_MESSAGE_LENGTH             32
/* TDM structure */
#define SLOT_DURATION                  620U  // In LP timer counter ticks
#define SLOTS_PER_SHORT_FRAME          40U   // number of slots in a short frame
#define DCH_SLOTS_PER_SHORT_FRAME      4U
#define FIRST_DCH_SLOT_IN_SHORT_FRAME  0U
#define FIRST_PRACH_ACK_SLOT_IDX       2U
#define FIRST_SRACH_ACK_SLOT_IDX       6U
#define DULCH_SLOT_IDX                 6U
#define MAX_SHORT_FRAMES_PER_LONG_FRAME 128U
#define LONG_FRAMES_PER_SUPER_FRAME    16U       // Number of long frames in a super frame.

/* Channel hopping sequence constants */
#define DCH_SEQ_LEN                (16)  
#define MAX_RACH_DLCCH_SEQ_LEN     (64) //(128 shortframe / 2)

/* Session constants */
#define MAX_CHILDREN_PER_PARENT       32U   // There are at most 32 child nodes per RBU
#define MAX_CHILDREN_PER_NCU_PARENT   511U   // There are at most 511 child nodes per NCU
#define DEFAULT_CHILDREN_PER_PARENT   24U   // Default to 24 child nodes per RBU
#define UNASSIGNED_RANK               63  // Used by a node to signal that it hasn't selected its rank.
#define MAX_INITIAL_RANK              (UNASSIGNED_RANK - 1)   // Maximum rank that can be used during the CONFIG_SYNC phase of mesh forming.
#define MAX_RANK                      20         // The maximum rank of an RBU. This equals the min no of hops to the NCU.


/* LPTIM properties */
#define LPTIM_MAX 65536U
#define LPTIM_HALF 32768U
#define LPTIM_MASK 65535U
/* LPTIM macros */
#define SUM_WRAP(x1, x2, mod)       (x1 + x2 < mod ? (x1 + x2) : ((x1 + x2) - mod))  // Returns (x1 + x2) with restricted-range wrapping
#define SUBTRACT_WRAP(x1, x2, mod)  (x1 >= x2      ? (x1 - x2) : ((mod + x1) - x2))  // Returns (x1 - x2) with restricted-range wrapping
#define DIFFERENCE_WRAP(x1, x2, mod)((x1 - x2) > (mod / 2)  ? (x1 - (mod + x2)) : (-(x1 - x2)))

/* Sync module macros */
#define ABS_DIFFERENCE(x1, x2)      (x1 >= x2      ? (x1 - x2) : (x2 - x1))
#define INTEGRATE8(x1,x2)           (((x1 * 7) + x2) / 8)
#define INTEGRATE4(x1,x2)           (((x1 * 3) + x2) / 4)

/* LoRa Modulation and Packet Parameters */
#define LORA_BANDWIDTH_868            1         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_BANDWIDTH_915            2         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_PPU_BANDWIDTH            2         // [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved]
#define LORA_SPREADING_FACTOR         7         // [SF6 .. SF12]
#define LORA_CODINGRATE               1         // [1: 4/5, 2: 4/6 3: 4/7, 4: 4/8]
#define LORA_DCH_PREAMBLE_LENGTH      16        // 7 is the fewest number of preamble symbols that gives reliable reception.
                                                // 6 symbols gives a few missing packets, and noticably more jitter on the
                                                // RBU's DCH transmission, suggesting disruption to the sync process???
                                                // 5 symbols or fewer gives no detected packets.
#define LORA_RACH_PREAMBLE_LENGTH     16        // 16 preamble symbols last 5.18ms: 20 preamble symbols last 6.21ms
#define LORA_ACK_PREAMBLE_LENGTH      16        // 7  preamble symbols last 2.88ms: 12 preamble symbols last 4.16ms
#define LORA_DLCCH_PREAMBLE_LENGTH    20        // 16 preamble symbols last 5.18ms: 20 preamble symbols last 6.21ms

#define LORA_DCH_SYMBOL_TIMEOUT       24        // 16 symbols RxTimeout at symbol duration (64/250000) = 4.096ms.
#define WIDE_DCH_SYMBOL_TIMEOUT       32        // 32 symbols RxTimeout at symbol duration (64/250000) = 8.192ms.
#define LORA_DLCCH_SYMBOL_TIMEOUT     16        // 16 symbols RxTimeout at symbol duration (64/250000) = 4.096ms.
#define LORA_RACH_SYMBOL_TIMEOUT      16        // 16 symbols RxTimeout at symbol duration (64/250000) = 4.096ms.
#define LORA_ACK_SYMBOL_TIMEOUT       16        // 16 symbols RxTimeout at symbol duration (64/250000) = 4.096ms.
#define LORA_TX_TIMEOUT               20        // milliseconds
#define PPU_LORA_TX_TIMEOUT           20       // milliseconds

#define LORA_FIX_LENGTH_PAYLOAD_ON    1
#define LORA_FIX_LENGTH_PAYLOAD_OFF   0
#define LORA_IQ_INVERSION_ON          false
#define LORA_RX_CONTINUOUS_MODE       true
#define LORA_RX_SINGLE_MODE           false
#define CHANNEL_HOPPING_ENABLED       true
#define CHANNEL_HOPPING_DISABLED      false
#define NUM_HOPPING_CHANS             10        // The number of radio channels used by the Cygnus II system.

#define TX_MIN_OUTPUT_POWER           0         // 0dBm
#define TX_LOW_OUTPUT_POWER           2         // 2dBm
#define TX_HIGH_OUTPUT_POWER          7         // 7dBm
#define TX_MAX_OUTPUT_POWER           10        // 10dBm

/* Test mode constants */
#define TEST_MODE_MONITORING_PERIOD  1639    /* 1639  * 61us = 0.099979 seconds */
#define TEST_PAYLOAD_SIZE            13      /* do not increase above 16 without extending the Payload in FrameTestModeMessage_t */
#define TEST_MODE_WATCHDOG_PERIOD    20000  //16390   /* ~ 1 second */

/* Semtech Chip revision ID */
#define SEMTECH_SILICON_DEFAULT_ID  0x22u

//200 hour test bit mapping
#define TEST_200HR_CHANNEL_SOUNDER                          1  //bit 0
#define TEST_200HR_CHANNEL_BEACON                           2  //bit 1
#define TEST_200HR_CHANNEL_SOUND_VISUAL_INDICATOR_COMBINED  4  //bit 2
#define TEST_200HR_CHANNEL_VISUAL_INDICATOR                 8  //bit 3

//Bit map for combined global delays and day/night command payload
#define ATGSET_GLOBAL_DELAY_MAX  0xFF
#define ATGSET_GLOBAL_DELAY1_MASK  0xFF
#define ATGSET_GLOBAL_DELAY2_MASK  0xFF00
#define ATGSET_GLOBAL_DELAY_BIT_LENGTH  8
#define ATGSET_GLOBAL_DELAY_RESOLUTION 30  //30 seconds for each count of the global delay
#define ATGSET_IGNORE_SECURITY_NIGHT_MASK 0x10000
#define ATGSET_IGNORE_SECURITY_DAY_MASK 0x20000
#define ATGSET_IS_DAY_MASK 0x40000
#define ATGSET_GLOBAL_OVERRIDE_MASK 0x80000
#define ATGSET_DELAYS_ENABLE_MASK 0x100000

/* generic def for a bad 32bit value */
#define ERROR_VALUE (0xffffffffu)

#define MAX_INPUT_CHANNELS 32U

#define MAX_OUTPUT_DELAY 600U

/*Battery monitor NVM constants */
#define DM_BAT_PASSIVATION_TIMEOUT 180
#define DM_BAT_PASSIVATION_VOLTAGE_MV 3100
#define DM_BAT_NUMBER_OF_BATTERY_READINGS 3
#define DM_BAT_TIMER_ID 3
//PPU Mode
//#define PPU_EMBEDDED_CHECKSUM
#ifdef PPU_EMBEDDED_CHECKSUM
#define PPU_MESSAGE_HEADER_SIZE 14
#define PPU_CHECKSUM_LOW 8
#define PPU_CHECKSUM_HIGH 9
#else
#define PPU_MESSAGE_HEADER_SIZE 12
#endif
#define PPU_MESSAGE_CHECKSUM_SIZE 2

#define MAX_PPU_COMMAND_PAYLOAD_LENGTH 40
#define PPU_ANNOUNCEMENT_MIN_PERIOD 5
#define PPU_ANNOUNCEMENT_PERIOD_SPAN 30
#define PPU_ANNOUNCEMENT_DEFAULT_PERIOD 10
#define PPU_START_DISCONNECTED_MODE 0
#define PPU_ANNOUNCEMENT_SEND 1
#define PPU_START_CONNECTED_MODE 2
#define PPU_SEND_ROUTE_DROP_E 3
//PPU Commands
#define PPU_CMD_PING 0x0200
#define PPU_CMD_PPU_VERSION 0x0202
#define PPU_ANNOUNCEMENT_MSG 0x0206
#define PPU_CMD_CONFIG 0x0208
#define PPU_CMD_PPU_CONNECT 0x0210
#define PPU_CMD_PPU_DISCONNECT 0x0212
#define PPU_CMD_PPU_RESET_TO_BOOT1 0x0280   //Boot for serial firmware update
#define PPU_CMD_PPU_RESET_TO_BOOT2 0x0282   //Boot for radio firmware update
#define PPU_CMD_PPU_RESET_NORMAL 0x0284     //Boot to main application
#define PPU_CMD_PPU_RESET_DISCONNECTED 0x0286  //Boot to main application and go into PPU Connected state
#define PPU_CMD_PPU_RESET_CONNECTED 0x0288  //Boot to main application and go into PPU Connected state
#define PPU_CMD_PPU_FIRMWARE_UPDATE_CONFIG 0x028A
#define PPU_CMD_PPU_FIRMWARE_BIT_REQ 0x028C
#define PPU_CMD_PPU_FIRMWARE_SEND_PACKET 0x028E
#define PPU_CMD_PPU_RESET 0x0FFF



//Status bit masks for command/response field
#define PPU_COMMAND_MASK 0x0FFF
#define PPU_STATUS_BIT_ERROR 0x8000
#define PPU_STATUS_BIT_SECURITY 0x4000
#define PPU_STATUS_BIT_ACK 0x2000
#define PPU_STATUS_BIT_ACK_REQ 0x1000

//Start-up modes
#define STARTUP_NORMAL 0u
#define STARTUP_PPU_CONNECTED 1u
#define STARTUP_PPU_DISCONNECTED 2u
#define STARTUP_SERIAL_BOOTLOADER 3u
#define STARTUP_RADIO_BOOTLOADER 4u

#define PARAM_RESET_EVENTS 1
#define PARAM_RESET_FAULTS 2
#define PARAM_RESET_DULCH 	4


// Amber Flash faults
#define LED_SINGLE_FLASH_FAULTS (CO_INSTALLATION_TAMPER_FAULT_E | \
                                 CO_DISMANTLE_TAMPER_FAULT_E | \
                                 CO_DETECTOR_FAULT_E | \
                                 CO_BEACON_FAULT_E | \
                                 CO_SOUNDER_FAULT_E | \
                                 CO_IO_INPUT_1_FAULT_E | \
                                 CO_IO_INPUT_2_FAULT_E | \
                                 CO_DISMANTLE_HEAD_FAULT_E | \
                                 CO_MISMATCH_HEAD_FAULT_E | \
                                 CO_DEVICE_ID_MISMATCH_E | \
                                 CO_DIRTY_SENSOR_E | \
                                 CO_INTERNAL_FAULT_E | \
                                 CO_INPUT_SHORT_CIRCUIT_FAULT_E | \
                                 CO_INPUT_OPEN_CIRCUIT_FAULT_E | \
                                 CO_OUTPUT_FAULT_E )
/* Debug and Setup Code
*************************************************************************************/
// Do not delete these.  They enable valuable debug and setup information.

//#define COUNT_FALSE_CADS                       // Enabling this will cause a print to the console of the number of false CADs
                                               // (i.e. the CAD has detected activity, but the resulting RxSingle timed-out rather than receiving a packet).

/* Define this to use the radio board to emulate a Sound and Visual Indicator (SVI) */
//#define I2C_SVI_BOARD_SIMULATION

#ifndef I2C_SVI_BOARD_SIMULATION

   /* Enable stop mode during idle periods */
   #define ENABLE_STOP_MODE

   /* Enable debugging during stop mode - only for DEBUG configuration */
   #ifdef _DEBUG
   #define ENABLE_DBG_STOP_MODE
   #endif

   /* This enables the Independant Watchdog */
   #define IWDG_ENABLE
   /* Independent watchdog timeout value in ms */
   #define IWDG_TIMEOUT_PERIOD_MS      (3000u)
#endif /* I2C_SVI_BOARD_SIMULATION */

/* End of Debug and Setup Code Defines **********************************************/



/* Halt the Watchdog during Debugging (breakpoint halt) - only for DEBUG configuration */
#ifdef _DEBUG
   #define IWDG_DEBUG_FREEZE
#endif


/* Macros
*************************************************************************************/
#define DIV_ROUNDUP(n,d)            ((n + d - 1)/d)

#define MIN(a, b)                   (((a) < (b)) ? (a) : (b))
#define MAX(a, b)                   (((a) > (b)) ? (a) : (b))

// Data type conversions
#define CONV_BOOL_TO_INT(x)    (x ? 1 : 0)

/* Assertions - used to check assumptions and guard against bad parameters */
#define CO_ASSERT_VOID(a)   if (!(a)) {return;}
#define CO_ASSERT_RET(a,b)  if (!(a)) {return b;}
/* enabled in DEBUG configuration but disabled in RELEASE configuration */
#ifdef _DEBUG
#define CO_ASSERT_VOID_MSG(a,str)   if (!(a)) {CO_PRINT_B_1(DBG_ERROR_E, "%s\n\r",str); return;}
#define CO_ASSERT_RET_MSG(a,b,str)  if (!(a)) {CO_PRINT_B_1(DBG_ERROR_E, "%s\n\r",str); return b;}
#else
#define CO_ASSERT_VOID_MSG(a,str)   if (!(a)) {CO_PRINT_A_1(DBG_ERROR_E, "%s\n\r",str); return;}
#define CO_ASSERT_RET_MSG(a,b,str)  if (!(a)) {CO_PRINT_A_1(DBG_ERROR_E, "%s\n\r",str); return b;}
#endif

/* Error Checks - used to check for error conditions */
/* enabled in both DEBUG and RELEASE configurations */
#define CO_ERROR_CHECK(a)        if (!(a)) {return ERR_OUT_OF_RANGE_E;}
#define CO_ERROR_CHECK_VOID(a)   if (!(a)) {return;}
#define CO_ERROR_CHECK_RET(a,b)  if (!(a)) {return b;}
#define CO_ERROR_CHECK_VOID_MSG(a,str)   if (!(a)) {CO_PRINT_B_1(DBG_ERROR_E, "%s\n\r",str); return;}
#define CO_ERROR_CHECK_RET_MSG(a,b,str)  if (!(a)) {CO_PRINT_B_1(DBG_ERROR_E, "%s\n\r",str); return b;}


/* compiler switches */

/* compile switch for encryption.  Comment-out to disable encryption */
#define USE_ENCPRYPTED_PAYLOAD

// TEST MODE for crystal timing
// MC_MAC.c sets the RED LED output high for the Tx period, and BLUE LED high for the Rx period.
// Timing between two radio boards (Tx and Rx) can be examined on a dual channel scope.
// 12 second pulse sent out on the RED LED when ATMODE=7 is sent over the usart
//#define TXRX_TIMING_PULSE
#ifdef TXRX_TIMING_PULSE
//Enable this if you want the CAD to show on the BLUE LED
#define TXRX_TIMING_PULSE_INCLUDE_CAD
//Enable this to show the slot wake-ups on the GREEN LED
#define SHOW_SLOT_WAKEUPS
//Enable this to show the frame sync on the GREEN LED
#define TXRX_TIMING_FRAME_SYNC
#endif

//Uncomment this to show the radio frequency deviation for received messages in the debug log.
//#define SHOW_FREQUENCY_DEVIATION

//Comment this line to enforce the manual discard after reading a CIE queue
#define AUTO_DISCARD_CIE_QUEUE_MSG_AFTER_READ

//Uncomment this line to send the MESH 0 (new link) and MESH 1 (dropped link) updates to the control panel
#define SEND_MESH_LINK_UPDATES

//Uncomment this line to send crc fail messages to the debug output
//#define OUTPUT_CRC_FAIL_INFO

//Comment this out to prevent the LoRa chip from sleeping
#define ALLOW_RADIO_SLEEP

//Enable this to allow the NCU cache to be updated via the debug port using ATSTM
//#define ENABLE_NCU_CACHE_DEBUG

//Set this to true to enable battery checks
#define ENABLE_BATTERY_CHECKS true
//enable this to send dummy battery readings when battery checks are disabled above
//#define SEND_DUMMY_BATTERY_READINGS

//Enable this to do the SmartNet primary battery current test under continuous load
//#define ENABLE_CONTINUOUS_LOAD_BATTERY_CURRENT_TEST

//Enable this to calculate slot length based on the measured average frame length
#define USE_AVERAGE_SYNC_FRAMES

#define ENABLE_CHANNEL_HOPPING                  // Comment out this definition to disable channel hopping

//comment this out to enable sleeping through unused slots
//#define WAKE_FOR_ALL_SLOTS

//Enable this to activate the MESH:1,<parent>,<child> CIE reports
//#define ENABLE_VERBOSE_CIE_MESH_UPDATES

//Enable this to prevent selection of the NCU as a parent
//#define DISABLE_NCU_SELECTION_WHEN_FORMING

//Enable this to use the ATANL and ATDNL commands to add or remove a link from the session manager
//#define ENABLE_LINK_ADD_OR_REMOVE

//Comment this out to use original Hb frquncy hop (each long frame)
#define USE_DCH_HOPPING_FORMULA
#ifdef USE_DCH_HOPPING_FORMULA
//Enable one of these only
//#define USE_DCH_HOPPING_FORMULA_1 //Hb freq hop each short frame
#define USE_DCH_HOPPING_FORMULA_2 //Hb freq hop each Hb slot
#endif

//Enable this to have a log on message sent on establishing a sync node
//#define SEND_LOG_ON

//Enable this to reverse the order of the dulch slots
//#define DULCH_USE_BIT_REVERSE

//Enable this to send route add messages on DULCH
#define SEND_ROUTE_ADD_ON_DULCH

//Enable this to record and display the LPTIM count for heartbeat slot wake-up and send/receive
//#define SHOW_HB_WAKEUP_TIME_STAMPS

#define USE_DULCH_WRAP_AROUND

//enable this to skip the wait between sysnc and parent selection
#define FAST_TRACK_FORMING

//Enable this to wake up the ATBV command for reading battery voltages out to the debug log
#define ENABLE_BATTERY_VOLTAGE_COMMAND

//Enable this to allow ATTIM to move message timin in the slot
//#define ENABLE_TDM_CALIBRATION

//Enable this to make the LED flicker when the device receives a downlink message
//#define SHOW_DOWNLINK_LED

//Enable this to wait for the next long frame to apply synch correction
//#define DEFER_SYNCH_TO_NEXT_LONG_FRAME

//Enable this to check for duplicate messages in the Ack Mgr queues
#define DUPLICATE_MESSAGE_REPLACEMENT_ENABLED

//Enable this to send fire signals on S-RACH in addition to P-RACH
//#define DUPLICATE_FIRE_SIGNAL_ON_SRACH

//Enable this when using rechargeable lipo batteries
//#define SE_BATTERY

//#define SHOW_HEAD_TRAFFIC_ON_PPU_USART

//Enable this to wake up the ATROFF command (adds specified offset to a node's rssi)
//#define APPLY_RSSI_OFFSET

//#define USE_NEW_SYNC_NODE_RESELECT

//Enable this to activate the ATSHB (skip a number of hearbeat transmissions)
//#define ENABLE_HEARTBEAT_TX_SKIP

//Enable this to print the dch hopping channels for each base frequency on start-up
//#define SHOW_ALL_HOPPING_CHANNELS

//Enable this for old Construction boards which used the opposite logic for Bkp Batt enable
//#define USE_OLD_CONSTRUCTION_BKP_BATTERY_PIN_LOGIC

//Enable this to fix the number of downlink repeats to DEFAULT_NUMBER_OF_RESENDS (ignore NVM set by ATMAXDL)
#define IGNORE_MAX_RESEND_SETTING

//enable this to send the rxd hex messages to the debug log
//#define REPORT_RXD_MESSAGES_RAW_BINARY

//Enable this to show the secondary rach link utilisation
//#define SHOW_NETWORK_USAGE

//Enable this to print debug messages on allocation and freeing of MeshPool slots for memory leak investigation
//#define DEBUG_MESH_QUEUE
#ifdef DEBUG_MESH_QUEUE
#define ALLOCMESHPOOL osPoolAlloc(MeshPool);meshpoolcount++;CO_PRINT_B_3(DBG_INFO_E,"Meshpool++ %d, file %s, line %d\r\n",meshpoolcount,__FILE__,__LINE__);
#define FREEMESHPOOL(x) osPoolFree(MeshPool, x);meshpoolcount--;CO_PRINT_B_3(DBG_INFO_E,"Meshpool-- %d, file %s, line %d\r\n",meshpoolcount,__FILE__,__LINE__);
extern uint32_t meshpoolcount;
#else
#define ALLOCMESHPOOL osPoolAlloc(MeshPool);
#define FREEMESHPOOL(x) osPoolFree(MeshPool, x);
#endif

//Enable this to print debug messages on allocation and freeing of ATHandlePool slots for memory leak investigation
//#define DEBUG_ATHANDLE_POOL
#ifdef DEBUG_ATHANDLE_POOL
extern uint32_t gAtHandlePoolCount;
#define CALLOC_ATHANDLE_POOL osPoolCAlloc(ATHandlePool);gAtHandlePoolCount++;CO_PRINT_B_3(DBG_INFO_E,"ATHandlePool++ %d, file %s, line %d\r\n",gAtHandlePoolCount,__FILE__,__LINE__);
#define ALLOC_ATHANDLE_POOL osPoolAlloc(ATHandlePool);gAtHandlePoolCount++;CO_PRINT_B_3(DBG_INFO_E,"ATHandlePool++ %d, file %s, line %d\r\n",gAtHandlePoolCount,__FILE__,__LINE__);
#define FREE_ATHANDLE_POOL(x) osPoolFree(ATHandlePool, x);gAtHandlePoolCount--;CO_PRINT_B_3(DBG_INFO_E,"ATHandlePool-- %d, file %s, line %d\r\n",gAtHandlePoolCount,__FILE__,__LINE__);
#else
#define CALLOC_ATHANDLE_POOL osPoolCAlloc(ATHandlePool);
#define ALLOC_ATHANDLE_POOL osPoolAlloc(ATHandlePool);
#define FREE_ATHANDLE_POOL(x) osPoolFree(ATHandlePool, x);
#endif


//Comment this out to maintain outputs ON when receiving a system reset message
#define CANCEL_OUTPUTS_ON_RESET_MESSAGE

//Comment this out to disable the >20 raw ADC check for SmartNet currrent test
//#define DM_BAT_DO_SMARTNET_CURRENT_TEST

//Comment this out to disable the 1%-99% battery contribution test on the three SmartNet primary batteries
//#define DM_BAT_DO_SMARTNET_CURRENT_WINDOW_TEST

//Enable this to do battery voltage readings at 50ms intervals (default is 1 second)
#define DM_BAT_DO_50ms_TEST

//Enable this to print the reason that the device is being kept awake
//#define REPORT_WHY_CANT_SLEEP

//Enable this to put the RBU into sleep mode when the battery is depleted
//#define DEEP_SLEEP_ON_BATTERY_END_OF_LIFE

//Enable this to send the global delays message on DULCH
#define SEND_GLOBAL_DELAY_ON_DULCH

#endif // CO_DEFINES_H
