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
*  File         : PacketIncrementingPayload.h
*
*  Description  : Header for the test that sends a packet with an incrementing
*                 number in the payload, represented as an ascii string.
*
*************************************************************************************/

#ifndef TEST_PACKETINCREMENTINGPPUPAYLOAD_H
#define TEST_PACKETINCREMENTINGPPUPAYLOAD_H

/* System Include Files
*************************************************************************************/
#include <stdint.h>


/* User Include Files
*************************************************************************************/
#include "TestBase.h"

/* Definitions
*************************************************************************************/
#define EMBEDDED_CHECKSUM //Comment-out to tag checksum onto the end of messages

#define PPUB_MAX_PAYLOAD_LENGTH 242U

#ifdef EMBEDDED_CHECKSUM
#define PPUB_MESSAGE_HEADER_LENGTH 14U
#define PPUB_MAX_MESSAGE_LENGTH (PPUB_MESSAGE_HEADER_LENGTH+PPUB_MAX_PAYLOAD_LENGTH)
#else
#define PPUB_MESSAGE_HEADER_LENGTH 12U
#define PPUB_MAX_MESSAGE_LENGTH (PPUB_MESSAGE_HEADER_LENGTH+PPUB_MAX_PAYLOAD_LENGTH + 2)
#endif


#define PPU_MASTER_ID 512u


//Messages for PPU mode
#define PPUB_CMD_PING 0x0200
#define PPUB_CMD_PPU_VERSION 0x0202
#define PPUB_ANNOUNCEMENT_MSG 0x0206
#define PPUB_CMD_CONFIG 0x0208
#define PPUB_CMD_PPU_CONNECT 0x0210
#define PPUB_CMD_PPU_RESET_TO_BOOT1 0x0280   //Boot for serial firmware update
#define PPUB_CMD_PPU_RESET_TO_BOOT2 0x0282   //Boot for radio firmware update
#define PPUB_CMD_PPU_RESET_NORMAL 0x0284     //Boot to main application
#define PPUB_CMD_PPU_FIRMWARE_UPDATE_CONFIG 0x0286
#define PPUB_CMD_PPU_FIRMWARE_BIT_REQ 0x0288
#define PPUB_CMD_PPU_FIRMWARE_SEND_PACKET 0x028A
#define PPUB_CMD_PPU_RESET 0x0FFF


#define PPUP_RESPONSE_PING_ACKNOWLEDGE 0x2200
#define PPUB_RESPONSE_PPU_VERSION 0x0101
#define PPUB_RESPONSE_FIRMWARE_BIT_REQ 0x8212

//Response mesage lengths
#ifdef EMBEDDED_CHECKSUM
#define PPUB_RESPONSE_PING_LENGTH 6
#define PPUB_RESPONSE_VERSION_LENGTH 24
#define PPUB_RESPONSE_BIT_REQ_LENGTH 14
#define PPUB_ANNOUNCEMENT_LENGTH 14
#else
#define PPUB_RESPONSE_PING_LENGTH 8
#define PPUB_RESPONSE_VERSION_LENGTH 26
#define PPUB_RESPONSE_BIT_REQ_LENGTH 16
#define PPUB_ANNOUNCEMENT_LENGTH 16
#endif
/* Type Declarations
*************************************************************************************/
typedef enum
{
   PPU_EVENT_NONE_E,
   PPU_EVENT_TXDONE_E,
   PPU_EVENT_TXTIMEOUT_E,
   PPU_EVENT_RXDONE_E,
   PPU_EVENT_RXTIMEOUT_E,
   PPU_EVENT_RXERROR_E,
   PPU_EVENT_RXCRCERROR_E,
   PPU_EVENT_CHANNEL_E,
   PPU_EVENT_USART_CMD_E
} PacketPpuIncrementingPayloadEvent_t;

typedef enum
{
   PPU_STATE_IDLE_E,
   PPU_STATE_WAITING_FOR_RADIO_RESPONSE_E,
   PPU_STATE_WAITING_FOR_RADIO_SEND_CONFIRMATION_E
} PpuState_t;

typedef struct
{
   uint16_t PpuId;
   uint16_t DeviceId;
   uint32_t SystemID;//Serial Number
   uint16_t Checksum;
   uint16_t PacketLength;
   uint16_t Command;
   uint8_t Payload[PPUB_MAX_PAYLOAD_LENGTH];
   uint16_t CalculatedChecksum;
   
   void Serialise(uint8_t* pBuffer);
   bool Deserialise(const uint8_t* pBuffer);
} PPU_Message_t;

typedef struct
{
   uint16_t Source;
   uint16_t Destination;
   uint32_t SystemID;
   uint16_t PacketLength;
   uint16_t Command;
   uint8_t Payload[PPUB_MAX_PAYLOAD_LENGTH];
}UsartPpuCommand_t;


class PacketPpuCommand : public TestBase
{
   uint32_t m_Count;
   PacketPpuIncrementingPayloadEvent_t m_Event;
   PpuState_t m_State;
   uint32_t m_Frequency;
   uint32_t m_TxPower;
   uint32_t m_SpreadingFactor;
   uint32_t m_CodingRate;
   bool m_LoraCrc;
   uint32_t m_MessageInterval;
   uint32_t m_TxDoneCount;
   uint32_t m_TxTimeoutCount;
   uint32_t m_ChannelChangeCount;
   UsartPpuCommand_t m_UsartCommand;
   PPU_Message_t m_PpuTxMessage;
   PPU_Message_t m_PpuRxMessage;
   uint16_t m_ExpectedResponse;
   uint8_t m_Buffer[PPUB_MAX_MESSAGE_LENGTH];
   
   uint32_t m_RssiValue;
   uint32_t m_SnrValue;
   uint32_t m_Ferror;
   uint32_t m_FeiValue;

   bool ReadPpuCommand(const char* pValue);
   bool IsHexChar(const char* pChar);
   uint8_t CharToHexValue(const char* pChar);
   uint8_t HexTextToUint8(const char* pText);
   uint16_t HexTextToUint16(const char* pText);
   uint32_t HexTextToUint32(const char* pText);
   void ProcessUsartCommand(void);
   void ProcessRadioMessage(void);
   void SetRadioForTransmit(const bool forPpu);
   void StartReceive(void);
   uint32_t GetExpectedResponseSize(void);
   void SendEnterPPUMode(const uint16_t destination);
   void SendAnnouncement(void);
   bool SendPing(const uint16_t nodeID);
   bool SendPpuConnect(const uint16_t nodeID);
   bool SendPpuVersionRequest(const uint16_t nodeID);
   bool SendPpuReset(const uint16_t nodeID);
   bool SendPpuResetToRadioUpdate(const uint16_t nodeID);
   bool SendPpuUpdate(const uint16_t nodeID, const uint16_t packetSize, const uint16_t numberOfPackets);
   bool SendPpuLostPacketRequest(const uint16_t nodeID);
   bool SendAtCommand(const uint16_t nodeID, const char* command);
   
   public:
   PacketPpuCommand( void );
   virtual ~PacketPpuCommand( void);
   
   virtual bool Initialise( void );
   virtual bool RunProcess( void );
   virtual void OnTxDone( void );
   virtual void OnTxTimeout( void );
   virtual void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr, uint32_t FeiVal, float ferror);
   virtual void OnRxTimeout( void );
   virtual void OnRxError( void );
   virtual void OnFhssChangeChannel( uint8_t channelIndex );
   virtual bool ProcessCommand(const char* pCommand);
   virtual void PrintHelp(Serial &serialPort);
};
#endif
