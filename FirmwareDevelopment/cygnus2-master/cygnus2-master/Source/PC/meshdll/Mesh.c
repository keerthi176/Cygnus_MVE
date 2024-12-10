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
*  File         : Mesh.c
*
*  Description  : DLL containing routines to pack and unpack mesh protocol messages using the
*                 formats described in the following document;
*
*                 HKD-16-0015-D_A37 Mesh Protocol Design
*
*************************************************************************************/


/* System Include Files
*************************************************************************************/
#include <stdio.h>
#include <string.h>


/* User Include Files
*************************************************************************************/
#include "CO_ErrorCode.h"
#include "MC_PUP.h"


/* Private Functions Prototypes
*************************************************************************************/
__declspec(dllexport) int decode(const char *const pin, char *pout);


/* Global Variables
*************************************************************************************/


/* Private Variables
*************************************************************************************/
//#define PRINT_MESSAGE_CONTENT

#define MAX_NUMBER_OF_PARAMS 20
#define MAX_PARAM_STRING_LENGTH 30
#define OUTBUFSIZ    1024

#define PARAM_SEP    " "
#define PARAM_END    ""

/*************************************************************************************/
/**
* decode
* Function to unpack a mesh protocol message. The input is a hex byte string with no spaces
* The output is a string showing the decoded message.
*
* @param - pointer to input string INPUT
*
* @return - pointer to buffer containing output string
*/
int decode(const char *const pin, char *buffer)
{
   char *pout = buffer;
   pout[0] = 0;
   int x;
   int stringlen = (int)strlen(pin);
   int len = stringlen >> 1;
   FrameType_t frameType;
   bool appMsgPresent;
   ApplicationLayerMessageType_t appMsgType;
   CO_MessagePayloadPhyDataInd_t phyDataInd;
   ErrorCode_t status;
   char bytebuff[3];
   int ret = 0;

   /* initialise data */
   frameType = 0;
   appMsgPresent = false;
   appMsgType = 0;
   memset(&phyDataInd, 0, sizeof(CO_MessagePayloadPhyDataInd_t));
   status = 0;


   /* decode hex to binary */
   for (x = 0; x < len; x++)
   {
      /* extract a byte */
      bytebuff[0] = pin[2 * x];
      bytebuff[1] = pin[2 * x + 1];
      bytebuff[2] = 0;
      sscanf_s(bytebuff, "%02hhx", &phyDataInd.Data[x]);
   }

   /* set length */
   phyDataInd.Size = len;

#ifdef PRINT_MESSAGE_CONTENT
   /* put hex bytes into output buffer */
   for (x = 0; x < len; x++)
   {
      pout += sprintf_s(pout, OUTBUFSIZ-strlen(pout)-1, "%02x ", phyDataInd.Data[x]);
   }
   pout += sprintf_s(pout, OUTBUFSIZ-strlen(pout)-1, "\n");
#endif

   status = MC_PUP_GetMessageType(&phyDataInd, &frameType, &appMsgPresent, &appMsgType);
   //printf("frametype=%d appmsgPresent=%d appmsgtype=%d\n", frameType, appMsgPresent, appMsgType);

   if (SUCCESS_E != status)
   {
      pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "message not recognised\n");
   }

   if (SUCCESS_E == status)
   {
      int frameLen = 0;

      /* check frame type */
      switch (frameType)
      {
      case FRAME_TYPE_HEARTBEAT_E:
         frameLen = HEARTBEAT_SIZE_BYTES;
         break;
      case FRAME_TYPE_DATA_E:
         frameLen = DATA_MESSAGE_SIZE_BYTES;
         break;
      case FRAME_TYPE_ACKNOWLEDGEMENT_E:
         frameLen = ACKNOWLEDGEMENT_SIZE_BYTES;
         break;
      case FRAME_TYPE_TEST_MESSAGE_E:
         frameLen = DATA_MESSAGE_SIZE_BYTES;
         break;
      default:
         pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "frametype not recognised\n");
         status = ERR_MESSAGE_TYPE_UNKNOWN_E;
         break;
      }

      if (SUCCESS_E == status)
      {
         /* check frame length */
         if (stringlen != frameLen << 1)
         {
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "incorrect frame length\n");
            status = ERR_MESSAGE_FAIL_E;
         }
      }
   }

   if (SUCCESS_E == status)
   {
      /* unpack message */
      switch (frameType)
      {
      case FRAME_TYPE_HEARTBEAT_E:
      {
         FrameHeartbeat_t heartbeat = { 0 };

         status = MC_PUP_UnpackHeartbeat(&phyDataInd, &heartbeat);
         if (SUCCESS_E == status)
         {
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HEARTBEAT" PARAM_SEP);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "slotIdx=%d" PARAM_SEP, heartbeat.SlotIndex);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ShortFrameIdx=%d" PARAM_SEP, heartbeat.ShortFrameIndex);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "LongFrameIdx=%d" PARAM_SEP, heartbeat.LongFrameIndex);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "State=%d" PARAM_SEP, heartbeat.State);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Rank=%d" PARAM_SEP, heartbeat.Rank);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "NumberOfChildrenIndex=%d" PARAM_SEP, heartbeat.NoOfChildrenIdx);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "NumberOfChildrenOfPrimaryTrackingNode=%d" PARAM_SEP, heartbeat.NoOfChildrenOfPTNIdx);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_END, heartbeat.SystemId);
         }
         else
         {
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HEARTBEAT - unpack failed\n");
         }
         break;
      }
      case FRAME_TYPE_DATA_E:
      {
         char *name = "";

         switch (appMsgType)
         {
         case APP_MSG_TYPE_FIRE_SIGNAL_E:
         case APP_MSG_TYPE_ALARM_SIGNAL_E:
         {
            AlarmSignal_t alarmsignal = { 0 };

            status = MC_PUP_UnpackAlarmSignal(&phyDataInd, &alarmsignal);
            if (SUCCESS_E == status)
            {
               if (APP_MSG_TYPE_FIRE_SIGNAL_E == appMsgType)
               {
                  name = "FIRE SIGNAL";
               }
               else
               {
                  name = "ALARM SIGNAL";
               }
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "%s" PARAM_SEP, name);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, alarmsignal.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, alarmsignal.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, alarmsignal.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, alarmsignal.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, alarmsignal.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, alarmsignal.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, alarmsignal.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "RUChannel=%d" PARAM_SEP, alarmsignal.RUChannel);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SensorValue=%d" PARAM_SEP, alarmsignal.SensorValue);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Zone=%d" PARAM_SEP, alarmsignal.Zone);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_END, alarmsignal.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "%s - unpack failed\n", name);
            }
            break;
         }
         case APP_MSG_TYPE_FAULT_SIGNAL_E:
         {
            FaultSignal_t faultMessage = { 0 };

            status = MC_PUP_UnpackFaultSignal(&phyDataInd, &faultMessage);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FAULT SIGNAL" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, faultMessage.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, faultMessage.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, faultMessage.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, faultMessage.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, faultMessage.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, faultMessage.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, faultMessage.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Zone=%d" PARAM_SEP, faultMessage.Zone);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "StatusFlags=%d" PARAM_SEP, faultMessage.StatusFlags);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DetectorFault=%d" PARAM_SEP, faultMessage.DetectorFault);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "BeaconFault=%d" PARAM_SEP, faultMessage.BeaconFault);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SounderFault=%d" PARAM_SEP, faultMessage.SounderFault);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_END, faultMessage.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FAULT - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
         {
            OutputSignal_t outputsignal = { 0 };

            status = MC_PUP_UnpackOutputSignal(&phyDataInd, &outputsignal);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OUTPUT SIGNAL" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, outputsignal.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, outputsignal.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, outputsignal.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, outputsignal.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, outputsignal.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, outputsignal.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, outputsignal.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Zone=%d" PARAM_SEP, outputsignal.Zone);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputChannel=%d" PARAM_SEP, outputsignal.OutputChannel);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputProfile=%d" PARAM_SEP, outputsignal.OutputProfile);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputsActivated=%d" PARAM_SEP, outputsignal.OutputsActivated);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputDuration=%d" PARAM_SEP, outputsignal.OutputDuration);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_END, outputsignal.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OUTPUT SIGNAL - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_COMMAND_E:
         case APP_MSG_TYPE_RESPONSE_E:
         {
            CommandMessage_t commandmessage = { 0 };

            status = MC_PUP_UnpackCommandMessage(&phyDataInd, &commandmessage);
            if (SUCCESS_E == status)
            {
               if (APP_MSG_TYPE_COMMAND_E == appMsgType)
               {
                  name = "COMMAND";
               }
               else
               {
                  name = "RESPONSE";
               }
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "%s" PARAM_SEP, name);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, commandmessage.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, commandmessage.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, commandmessage.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, commandmessage.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, commandmessage.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, commandmessage.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, commandmessage.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ParameterType=%d" PARAM_SEP, commandmessage.ParameterType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ReadWrite=%d" PARAM_SEP, commandmessage.ReadWrite);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "TransactionID=%d" PARAM_SEP, commandmessage.TransactionID);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "P1=%d" PARAM_SEP, commandmessage.P1);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "P2=%d" PARAM_SEP, commandmessage.P2);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Value=%d" PARAM_SEP, commandmessage.Value);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_END, commandmessage.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "%s - unpack failed\n", name);
            }
            break;
         }
         case APP_MSG_TYPE_LOGON_E:
         {
            LogOnMessage_t logonMessage = { 0 };

            status = MC_PUP_UnpackLogOnMsg(&phyDataInd, &logonMessage);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "LOGON" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, logonMessage.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, logonMessage.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, logonMessage.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, logonMessage.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, logonMessage.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, logonMessage.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, logonMessage.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SerialNumber=%d" PARAM_SEP, logonMessage.SerialNumber);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DeviceCombination=%d" PARAM_SEP, logonMessage.DeviceCombination);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Zone=%d" PARAM_SEP, logonMessage.ZoneNumber);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, logonMessage.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "LOGON - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_STATUS_INDICATION_E:
         {
            StatusIndicationMessage_t statusMessage = { 0 };

            status = MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, &statusMessage);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "STATUS INDICATION" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, statusMessage.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, statusMessage.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, statusMessage.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, statusMessage.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, statusMessage.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, statusMessage.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, statusMessage.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "NumberOfParents=%d" PARAM_SEP, statusMessage.NumberOfParents);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "NumberOfChildren=%d" PARAM_SEP, statusMessage.NumberOfChildren);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "AverageSNRPrimaryParent=%d" PARAM_SEP, statusMessage.AverageSNRPrimaryParent);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "AverageSNRSecondaryParent=%d" PARAM_SEP, statusMessage.AverageSNRSecondaryParent);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "NumberOfTrackingNodes=%d" PARAM_SEP, statusMessage.NumberOfTrackingNodes);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "NumberOfChildrenOfPrimaryTrackingNode=%d" PARAM_SEP, statusMessage.NumberOfChildrenOfPrimaryTrackingNode);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Rank=%d" PARAM_SEP, statusMessage.Rank);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Event=%d" PARAM_SEP, statusMessage.Event);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "EventNodeId=%d" PARAM_SEP, statusMessage.EventNodeId);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, statusMessage.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "STATUS INDICATION - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_ROUTE_ADD_E:
         {
            RouteAddMessage_t routeAdd = { 0 };

            status = MC_PUP_UnpackRouteAddMsg(&phyDataInd, &routeAdd);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ROUTE ADD" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, routeAdd.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, routeAdd.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, routeAdd.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, routeAdd.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, routeAdd.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, routeAdd.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, routeAdd.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Rank=%d" PARAM_SEP, routeAdd.Rank);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "IsPrimary=%d" PARAM_SEP, routeAdd.IsPrimary);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, routeAdd.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ROUTE ADD - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E:
         {
            RouteAddResponseMessage_t routeAddResponse = { 0 };

            status = MC_PUP_UnpackRouteAddResponseMsg(&phyDataInd, &routeAddResponse);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ROUTE ADD RESP" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, routeAddResponse.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, routeAddResponse.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, routeAddResponse.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, routeAddResponse.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, routeAddResponse.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, routeAddResponse.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, routeAddResponse.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Accepted=%d" PARAM_SEP, routeAddResponse.Accepted);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, routeAddResponse.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ROUTE ADD RESP - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_ROUTE_DROP_E:
         {
            RouteDropMessage_t routeDrop = { 0 };

            status = MC_PUP_UnpackRouteDropMsg(&phyDataInd, &routeDrop);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ROUTE DROP" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, routeDrop.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, routeDrop.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, routeDrop.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, routeDrop.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, routeDrop.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, routeDrop.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, routeDrop.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, routeDrop.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ROUTE DROP - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_STATE_SIGNAL_E:
         {
            SetStateMessage_t setStateMessage = { 0 };

            status = MC_PUP_UnpackSetState(&phyDataInd, &setStateMessage);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SET STATE" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, setStateMessage.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, setStateMessage.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, setStateMessage.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, setStateMessage.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, setStateMessage.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, setStateMessage.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, setStateMessage.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "State=%d" PARAM_SEP, setStateMessage.State);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, setStateMessage.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SET STATE - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_LOAD_BALANCE_E:
         {
            LoadBalanceMessage_t loadBalance = { 0 };

            status = MC_PUP_UnpackLoadBalanceMsg(&phyDataInd, &loadBalance);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "LOAD BALANCE" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, loadBalance.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, loadBalance.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, loadBalance.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, loadBalance.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, loadBalance.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, loadBalance.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, loadBalance.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, loadBalance.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "LOAD BALANCE - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E:
         {
            LoadBalanceResponseMessage_t loadBalance = { 0 };

            status = MC_PUP_UnpackLoadBalanceResponseMsg(&phyDataInd, &loadBalance);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "LOAD BALANCE RESP" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, loadBalance.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, loadBalance.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, loadBalance.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, loadBalance.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, loadBalance.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, loadBalance.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, loadBalance.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Accepted=%d" PARAM_SEP, loadBalance.Accepted);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, loadBalance.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "LOAD BALANCE RESP - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_OUTPUT_STATE_REQUEST:
         {
            OutputStateRequest_t outputStateReq = { 0 };
            status = MC_PUP_UnpackOutputStateRequestMessage(&phyDataInd, &outputStateReq);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OUTPUT STATE REQ" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, outputStateReq.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, outputStateReq.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, outputStateReq.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, outputStateReq.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, outputStateReq.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, outputStateReq.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, outputStateReq.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, outputStateReq.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OUTPUT STATE REQ - unpack failed\n");
            }
            break;
         }
         case APP_MSG_TYPE_OUTPUT_STATE:
         {
            OutputStateMessage_t outputState = { 0 };
            status = MC_PUP_UnpackOutputStateMessage(&phyDataInd, &outputState);
            if (SUCCESS_E == status)
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OUTPUT STATE" PARAM_SEP);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, outputState.Header.FrameType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, outputState.Header.MACDestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, outputState.Header.MACSourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "HopCount=%d" PARAM_SEP, outputState.Header.HopCount);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "DestinationAddress=%d" PARAM_SEP, outputState.Header.DestinationAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, outputState.Header.SourceAddress);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MessageType=%d" PARAM_SEP, outputState.Header.MessageType);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Zone=%d" PARAM_SEP, outputState.Zone);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputChannel=%d" PARAM_SEP, outputState.OutputChannel);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputProfile=%d" PARAM_SEP, outputState.OutputProfile);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputActivated=%d" PARAM_SEP, outputState.OutputActivated);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OutputDuration=%d" PARAM_SEP, outputState.OutputDuration);
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_SEP, outputState.SystemId);
            }
            else
            {
               pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "OUTPUT STATE - unpack failed\n");
            }
            break;
         }
         default:
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "No handler for message type %d\n", appMsgType);
            break;
         }

         break;
      }
      case FRAME_TYPE_ACKNOWLEDGEMENT_E:
      {
         FrameAcknowledge_t ackMessage = { 0 };

         status = MC_PUP_UnpackAck(&phyDataInd, &ackMessage);
         if (SUCCESS_E == status)
         {
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ACKNOWLEDGEMENT" PARAM_SEP);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, ackMessage.FrameType);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACDestinationAddress=%d" PARAM_SEP, ackMessage.MACDestinationAddress);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "MACSourceAddress=%d" PARAM_SEP, ackMessage.MACSourceAddress);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_END, ackMessage.SystemId);
         }
         else
         {
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "ACKNOWLEDGEMENT - unpack failed\n");
         }
         break;
      }
      case FRAME_TYPE_TEST_MESSAGE_E:
      {
         FrameTestModeMessage_t testMessage = { 0 };

         status = MC_PUP_UnpackTestSignal(&phyDataInd, &testMessage);
         if (SUCCESS_E == status)
         {
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "TEST MESSAGE" PARAM_SEP);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "FrameType=%d" PARAM_SEP, testMessage.FrameType);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SourceAddress=%d" PARAM_SEP, testMessage.SourceAddress);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Payload[0]=%d" PARAM_SEP, testMessage.Payload[0]);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Payload[1]=%d" PARAM_SEP, testMessage.Payload[1]);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Payload[2]=%d" PARAM_SEP, testMessage.Payload[2]);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "Payload[3]=%d" PARAM_SEP, testMessage.Payload[3]);
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "SystemId=%d" PARAM_END, testMessage.SystemId);
         }
         else
         {
            pout += sprintf_s(pout, OUTBUFSIZ - strlen(pout) - 1, "TEST MESSAGE - unpack failed\n");
         }
         break;
      }
      }
   }


   return len;
}
