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
*  File         : RxSurveyTool.h
*
*  Description  : Header for the Survey Tool Receiver.
*
*************************************************************************************/

#ifndef TEST_RX_SURVEY_TOOL_H
#define TEST_RX_SURVEY_TOOL_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "RxTestBase.h"

/* Type Declarations
*************************************************************************************/



#define STRX_MESSAGE_PAYLOAD_LENGTH      5
#define STRX_MESSAGE_LENGTH              (STRX_MESSAGE_PAYLOAD_LENGTH + 4)  //STRX_MESSAGE_PAYLOAD_LENGTH text + 4 bytes systemID
#define STRX_MESSAGE_LENGTH_WITH_CRC     (STRX_MESSAGE_LENGTH + 2)

typedef enum
{
   STRX_EVENT_NONE_E,
   STRX_EVENT_TIMER_E,
   STRX_EVENT_RXDONE_E,
   STRX_EVENT_RXTIMEOUT_E,
   STRX_EVENT_RXERROR_E,
   STRX_EVENT_RXCRCERROR_E,
   STRX_EVENT_CADDONE_E,
   STRX_EVENT_CHANNEL_E
} RxSurveyToolEvent_t;

typedef enum
{
   STRX_MODE_SITENET_TO_SITENET_E,
   STRX_MODE_SITENET_TO_SMARTNET_E,
   STRX_MODE_SMARTNET_TO_SMARTNET_E
} DeviceMode_t;

typedef struct 
{
   uint32_t SystemID;
   char payload[STRX_MESSAGE_PAYLOAD_LENGTH];
}SurveyToolMessage_t;

class SurveyToolRx : public RxTestBase
{
   uint32_t m_SystemID;
   uint32_t m_Count;
   RxSurveyToolEvent_t m_Event;
   uint32_t m_EventTime;
   uint32_t m_PreviousEventTime;
   char m_Buffer[STRX_MESSAGE_LENGTH_WITH_CRC];
   bool m_LastMessageErrorDetected;
   uint32_t m_Frequency;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   bool m_LoraCrc;
   uint32_t m_MessageInterval;//time between received messages in timer ticks

   uint32_t m_RxDoneCount;
   uint32_t m_RxTimeoutCount;
   uint32_t m_RxErrorCount;
   uint32_t m_CadDoneCount;
   uint32_t m_ChannelChangeCount;
   int32_t m_PayloadNumber ;
   int32_t m_LastReceivedPayload;
   uint32_t m_LostMessageCount;
   uint16_t m_CrcValue;

   int16_t m_RssiValue;
   int8_t m_SnrValue;
   uint32_t m_Ferror;
   uint32_t m_FeiValue;
   
   //private functions
   void StartReceiving(void);
   void ResetRadio(void);
   DeviceMode_t GetDeviceMode(void);
   uint32_t GetRssiIndex(const int16_t rssi);
   uint32_t GetSnrIndex(const int8_t snr);
   void SetLeds(const uint32_t index);
   void TimerCallback(void);
   
   
   public:
   SurveyToolRx( void );
   virtual ~SurveyToolRx( void);
   
   virtual bool Initialise( void );
   virtual bool RunProcess( void );

   virtual void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror);
   virtual void OnRxTimeout( void );
   virtual void OnRxError( void );
   virtual void OnCadDone( bool channelActivityDetected );
   virtual void OnFhssChangeChannel( uint8_t channelIndex );

   virtual bool ProcessCommand(const char* pCommand);
   virtual void PrintHelp(Serial &serialPort);
};
#endif //TEST_RX_SURVEY_TOOL_H
