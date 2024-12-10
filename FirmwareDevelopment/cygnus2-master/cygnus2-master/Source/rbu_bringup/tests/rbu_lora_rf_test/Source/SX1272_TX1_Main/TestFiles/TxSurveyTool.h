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
*  File         : TxSurveyTool.h
*
*  Description  : Header for the transmitter for the survey tool.
*
*************************************************************************************/

#ifndef TX_SURVEY_TOOL_H
#define TX_SURVEY_TOOL_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "TestBase.h"

/* defines
*************************************************************************************/
#define STTX_MESSAGE_PAYLOAD_LENGTH 5
#define STTX_MESSAGE_LENGTH (STTX_MESSAGE_PAYLOAD_LENGTH + 4)
#define STTX_MESSAGE_LENGTH_WITH_CRC  (STTX_MESSAGE_LENGTH + 2)

/* Type Declarations
*************************************************************************************/
typedef enum
{
   STTX_EVENT_NONE_E,
   STTX_EVENT_TXDONE_E,
   STTX_EVENT_TXTIMEOUT_E,
   STTX_EVENT_CHANNEL_E
} TxSurveyToolEvent_t;


typedef struct
{
   uint32_t SystemID;
   char Payload[STTX_MESSAGE_PAYLOAD_LENGTH];
} SurveyToolMessage_t;


class TxSurveyTool : public TestBase
{
   uint32_t m_SystemID;
   uint32_t m_Count;
   TxSurveyToolEvent_t m_Event;
   uint32_t m_Frequency;
   uint32_t m_TxPower;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   bool m_LoraCrc;
   uint32_t m_MessageInterval;
   
   uint32_t m_TxDoneCount;
   uint32_t m_TxTimeoutCount;
   uint32_t m_ChannelChangeCount;
   
   public:
      TxSurveyTool( void );
      virtual ~TxSurveyTool( void);
      
      virtual bool Initialise( void );
      virtual bool RunProcess( void );
      virtual void OnTxDone( void );
      virtual void OnTxTimeout( void );
      virtual void OnFhssChangeChannel( uint8_t channelIndex );
      virtual bool ProcessCommand(const char* pCommand);
      virtual void PrintHelp(Serial &serialPort);
};
#endif //TX_SURVEY_TOOL_H
