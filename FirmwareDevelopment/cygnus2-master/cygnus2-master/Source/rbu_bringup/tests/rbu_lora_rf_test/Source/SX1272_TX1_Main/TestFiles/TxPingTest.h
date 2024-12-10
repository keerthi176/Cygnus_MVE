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
*  File         : TxPingTest.h
*
*  Description  : Header for the test that sends a ping to a number of RBUs in turn,
*                 and checks for a matching payload in the response.
*
*************************************************************************************/

#ifndef TEST_TXPINGTEST_H
#define TEST_TXPINGTEST_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>
#include "FlashIAP.h"

/* User Include Files
*************************************************************************************/
#include "TestBase.h"

/* Type Declarations
*************************************************************************************/
using namespace mbed;

#define TXPING_MESSAGE_LENGTH              20
#define TXPING_MESSAGE_LENGTH_WITH_CRC     (TXPING_MESSAGE_LENGTH + 2)

#ifdef BOOTLOADER_BUILD
#define NODE_ID_FLASH_OFFSET_ADDRESS 0x0007a000
#else
#define NODE_ID_FLASH_OFFSET_ADDRESS 0x0007e000
#endif

#define TXPING_MAX_NODES 9

typedef enum
{
   TXPING_EVENT_NONE_E,
   TXPING_EVENT_WAIT_E,
   TXPING_EVENT_TXDONE_E,
   TXPING_EVENT_TXTIMEOUT_E,
   TXPING_EVENT_RXDONE_E,
   TXPING_EVENT_RXTIMEOUT_E,
   TXPING_EVENT_RXERROR_E,
   TXPING_EVENT_RXCRCERROR_E,
   TXPING_EVENT_CADDONE_E,
   TXPING_EVENT_CHANNEL_E,
   TXPING_EVENT_SEND_E,
   TXPING_EVENT_RECEIVE_E
} TxPingEvent_t;

typedef struct 
{
   uint32_t count_addr;
   uint32_t rxt_addr;
} addresses_t;

class RadioBoard
{
   static uint32_t m_TxPacketCount;
   public:
   uint32_t m_NodeID;
   uint32_t m_RxDoneCount;
   uint32_t m_RxTimeoutCount;
   uint32_t m_RxErrorCount;
   uint32_t m_RxCrcFailCount;
   int32_t m_RssiValue;
   int32_t m_SnrValue;
   int32_t m_FError;
   int32_t m_FrequencyDeviation;
   uint32_t m_LostPacketCount;
   uint32_t m_PacketNumber;
   double m_PacketLoss;
   
   bool ParseReceivedPacket(const char * pMessage);
   void UpdatePacketLoss(void);

   public:
    RadioBoard();
   ~RadioBoard();
   
   void SetNodeID(const int32_t node_id ){ m_NodeID = node_id; }
   
   static void IncrementPacketCount(void){ m_TxPacketCount++; }
   static uint32_t GetPacketCount(void) { return m_TxPacketCount; }
   
   uint32_t GetRxDoneCount(void) { return m_RxDoneCount; }
   
   uint32_t GetRxTimeoutCount(void) { return m_RxTimeoutCount; }
   
   uint32_t GetRxErrorCount(void) { return m_RxErrorCount; }
   
   uint32_t GetRxCrCFailCount(void) { return m_RxCrcFailCount; }
   
   uint32_t GetLostPacketCount(void) { return m_LostPacketCount; }
   
   void SetRSSI(const int32_t rssi ){ m_RssiValue = rssi; }
   int32_t GetRSSI(void) { return m_RssiValue; }
   
   void SetSNR(const int32_t snr ){ m_SnrValue = snr; }
   int32_t GetSNR(void) { return m_SnrValue; }
   
   void SetFrequencyDeviation(const int32_t freq_dev ){ m_FrequencyDeviation = freq_dev; }
   int32_t GetFrequencyDeviation(void) { return m_FrequencyDeviation; }
   
   double GetPacketLoss(void) { return m_PacketLoss; }
   
   bool ProcessResponse(const char* pMessage);
   
   bool ProcessLostPacket(const TxPingEvent_t event);
};

class TxPingTest : public TestBase
{
   
   RadioBoard m_RadioBoard[TXPING_MAX_NODES];
   FlashIAP m_Flash;
   addresses_t addr;
   uint32_t m_NodeID;
   uint32_t m_Count;
   TxPingEvent_t m_Event;
   uint32_t m_Frequency;
   uint32_t m_TxPower;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   bool m_LoraCrc;
   uint32_t m_MessageInterval;
   bool m_EnableOutput;
   bool m_LastMessageErrorDetected;
   uint16_t m_CrcValue;
   char m_Buffer[TXPING_MESSAGE_LENGTH_WITH_CRC];
   uint32_t m_TimeNow;
   uint32_t m_NextTxTime;
   bool m_WrapAround;
   bool m_CadDone;
   bool m_LastResponseReceived;
   
   uint32_t m_TxDoneCount;
   uint32_t m_TxTimeoutCount;
   uint32_t m_ChannelChangeCount;
   uint32_t m_RxDoneCount;
   uint32_t m_RxTimeoutCount;
   uint32_t m_RxErrorCount;
   uint32_t m_CadDoneCount;

   uint32_t m_RssiValue;
   uint32_t m_SnrValue;
   uint32_t m_Ferror;
   uint32_t m_FeiValue;

   uint32_t m_EventTime;
   uint32_t m_MaxNodes;
   uint32_t m_ExpectedNode;
   
   public:
   TxPingTest( void );
   virtual ~TxPingTest( void);
   
   virtual bool Initialise( void );
   virtual bool RunProcess( void );
   virtual void OnTxDone( void );
   virtual void OnTxTimeout( void );
   virtual void OnFhssChangeChannel( uint8_t channelIndex );
   virtual void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror);
   virtual void OnRxTimeout( void );
   virtual void OnRxError( void );
   virtual void OnCadDone( bool channelActivityDetected );
   virtual bool ProcessCommand(const char* pCommand);
   virtual void PrintHelp(Serial &serialPort);
   
   private:
   void StartReceiving(const uint32_t timeout);
   void StartNextTransmission(void);
};
#endif // TEST_TXPINGTEST_H
