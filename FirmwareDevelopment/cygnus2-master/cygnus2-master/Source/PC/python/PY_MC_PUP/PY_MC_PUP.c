#include <Python.h>

#include "..\..\..\MCU\Mesh\MC_PUP.c"


static PyObject *SpamError;
static PyObject * packet_decode(PyObject *self, PyObject *args, PyObject *kwargs);
PyMethodDef module_functions[] = {
   { "packet_decode", (PyCFunction)packet_decode, METH_VARARGS|METH_KEYWORDS, NULL },
   { NULL, NULL, 0, NULL }
};
static struct PyModuleDef moduledef = {
   PyModuleDef_HEAD_INIT,
   "PY_MC_PUP",     /* m_name */
   "Pack/Unpack package",  /* m_doc */
   -1,                  /* m_size */
   module_functions,    /* m_methods */
   NULL,                /* m_reload */
   NULL,                /* m_traverse */
   NULL,                /* m_clear */
   NULL,                /* m_free */
};

uint8_t MC_ShortFramesPerLongFrame(void)
{
   return 16;
}

PyMODINIT_FUNC
PyInit_PY_MC_PUP(void)
{
   return PyModule_Create(&moduledef);
}

static PyObject *
packet_decode(PyObject *self, PyObject *args, PyObject *kwargs)
{
   PyObject *result = PyDict_New();
   static char *kwlist[] = {"packet", NULL};
    FrameType_t frameType;
    bool appMsgPresent;
    ApplicationLayerMessageType_t appMsgType;
   char *packet;
   ErrorCode_t status;
    CO_MessagePayloadPhyDataInd_t phyDataInd;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s", kwlist, &packet)) {
         return NULL;
    }
   char *pos = packet;
   int packet_size = (int)(strlen(packet)/2);
   
      
   /* initialise data */
   frameType = 0;
   appMsgPresent = false;
   appMsgType = 0;
   memset(&phyDataInd, 0, sizeof(CO_MessagePayloadPhyDataInd_t));
   status = 0;
    
    for (size_t count = 0; count < packet_size; count++) {
        sscanf(pos, "%2hhx", &phyDataInd.Data[count]);
        pos += 2;
    }

   /* set length */
   phyDataInd.Size = packet_size;

   status = MC_PUP_GetMessageType(&phyDataInd, &frameType, &appMsgPresent, &appMsgType);

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
            PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", heartbeat.FrameType));
            PyDict_SetItem(result, PyUnicode_FromString("slotIdx"), Py_BuildValue("i", heartbeat.SlotIndex));
            PyDict_SetItem(result, PyUnicode_FromString("ShortFrameIdx"), Py_BuildValue("i", heartbeat.ShortFrameIndex));
            PyDict_SetItem(result, PyUnicode_FromString("LongFrameIdx"), Py_BuildValue("i", heartbeat.LongFrameIndex));
            PyDict_SetItem(result, PyUnicode_FromString("State"), Py_BuildValue("i", heartbeat.State));
            PyDict_SetItem(result, PyUnicode_FromString("Rank"), Py_BuildValue("i", heartbeat.Rank));
            PyDict_SetItem(result, PyUnicode_FromString("ActiveFrameLength"), Py_BuildValue("i", heartbeat.ActiveFrameLength));
            PyDict_SetItem(result, PyUnicode_FromString("NoOfChildrenIdx"), Py_BuildValue("i", heartbeat.NoOfChildrenIdx));
            PyDict_SetItem(result, PyUnicode_FromString("NoOfChildrenOfPTNIdx"), Py_BuildValue("i", heartbeat.NoOfChildrenOfPTNIdx));
            PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", heartbeat.SystemId));
         }
          else
          {
            PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", -1));
          }

          break;       
        }
        case FRAME_TYPE_DATA_E:
       {
          switch (appMsgType)
          {
             case APP_MSG_TYPE_FIRE_SIGNAL_E:
             case APP_MSG_TYPE_ALARM_SIGNAL_E:
             {
               AlarmSignal_t alarmsignal = { 0 };

               status = MC_PUP_UnpackAlarmSignal(&phyDataInd, &alarmsignal);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", alarmsignal.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", alarmsignal.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", alarmsignal.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", alarmsignal.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", alarmsignal.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", alarmsignal.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", alarmsignal.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("RUChannel"), Py_BuildValue("i", alarmsignal.RUChannel));
                  PyDict_SetItem(result, PyUnicode_FromString("SensorValue"), Py_BuildValue("i", alarmsignal.SensorValue));
                  PyDict_SetItem(result, PyUnicode_FromString("Zone"), Py_BuildValue("i", alarmsignal.Zone));
                  PyDict_SetItem(result, PyUnicode_FromString("AlarmActive"), Py_BuildValue("i", alarmsignal.AlarmActive));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", alarmsignal.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_FAULT_SIGNAL_E:
             {
               FaultSignal_t faultMessage = { 0 };

               status = MC_PUP_UnpackFaultSignal(&phyDataInd, &faultMessage);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", faultMessage.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", faultMessage.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", faultMessage.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", faultMessage.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", faultMessage.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", faultMessage.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", faultMessage.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Channel"), Py_BuildValue("i", faultMessage.RUChannelIndex));
                  PyDict_SetItem(result, PyUnicode_FromString("Value"), Py_BuildValue("i", faultMessage.Value));
                  PyDict_SetItem(result, PyUnicode_FromString("FaultType"), Py_BuildValue("i", faultMessage.FaultType));
                  PyDict_SetItem(result, PyUnicode_FromString("OverallFault"), Py_BuildValue("i", faultMessage.OverallFault));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", faultMessage.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_OUTPUT_SIGNAL_E:
             {
               OutputSignal_t outputsignal = { 0 };

               status = MC_PUP_UnpackOutputSignal(&phyDataInd, &outputsignal);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", outputsignal.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", outputsignal.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", outputsignal.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", outputsignal.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", outputsignal.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", outputsignal.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", outputsignal.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Zone"), Py_BuildValue("i", outputsignal.Zone));
                  PyDict_SetItem(result, PyUnicode_FromString("OutputChannel"), Py_BuildValue("i", outputsignal.OutputChannel));
                  PyDict_SetItem(result, PyUnicode_FromString("OutputProfile"), Py_BuildValue("i", outputsignal.OutputProfile));
                  PyDict_SetItem(result, PyUnicode_FromString("OutputsActivated"), Py_BuildValue("i", outputsignal.OutputsActivated));
                  PyDict_SetItem(result, PyUnicode_FromString("OutputDuration"), Py_BuildValue("i", outputsignal.OutputDuration));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", outputsignal.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E:
             {
               AlarmOutputState_t outputsignal = { 0 };

               status = MC_PUP_UnpackAlarmOutputState(&phyDataInd, &outputsignal);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", outputsignal.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", outputsignal.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", outputsignal.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", outputsignal.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", outputsignal.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", outputsignal.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", outputsignal.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Silenceable"), Py_BuildValue("i", outputsignal.Silenceable));
                  PyDict_SetItem(result, PyUnicode_FromString("Unsilenceable"), Py_BuildValue("i", outputsignal.Unsilenceable));
                  PyDict_SetItem(result, PyUnicode_FromString("DelayMask"), Py_BuildValue("i", outputsignal.DelayMask));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", outputsignal.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
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
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", commandmessage.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", commandmessage.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", commandmessage.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", commandmessage.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", commandmessage.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", commandmessage.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", commandmessage.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("ParameterType"), Py_BuildValue("i", commandmessage.ParameterType));
                  PyDict_SetItem(result, PyUnicode_FromString("ReadWrite"), Py_BuildValue("i", commandmessage.ReadWrite));
                  PyDict_SetItem(result, PyUnicode_FromString("TransactionID"), Py_BuildValue("i", commandmessage.TransactionID));
                  PyDict_SetItem(result, PyUnicode_FromString("P1"), Py_BuildValue("i", commandmessage.P1));
                  PyDict_SetItem(result, PyUnicode_FromString("P2"), Py_BuildValue("i", commandmessage.P2));
                  PyDict_SetItem(result, PyUnicode_FromString("Value"), Py_BuildValue("i", commandmessage.Value));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", commandmessage.SystemId)); 
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_LOGON_E:
             {
               LogOnMessage_t logonMessage = { 0 };

               status = MC_PUP_UnpackLogOnMsg(&phyDataInd, &logonMessage);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", logonMessage.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", logonMessage.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", logonMessage.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", logonMessage.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", logonMessage.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", logonMessage.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", logonMessage.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("SerialNumber"), Py_BuildValue("i", logonMessage.SerialNumber));
                  PyDict_SetItem(result, PyUnicode_FromString("DeviceCombination"), Py_BuildValue("i", logonMessage.DeviceCombination));
                  PyDict_SetItem(result, PyUnicode_FromString("ZoneNumber"), Py_BuildValue("i", logonMessage.ZoneNumber));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", logonMessage.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_STATUS_INDICATION_E:
             {
               StatusIndicationMessage_t statusMessage = { 0 };

               status = MC_PUP_UnpackStatusIndicationMsg(&phyDataInd, &statusMessage);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", statusMessage.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", statusMessage.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", statusMessage.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", statusMessage.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", statusMessage.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", statusMessage.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", statusMessage.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("PrimaryParentID"), Py_BuildValue("i", statusMessage.PrimaryParentID));
                  PyDict_SetItem(result, PyUnicode_FromString("SecondaryParentID"), Py_BuildValue("i", statusMessage.SecondaryParentID));
                  PyDict_SetItem(result, PyUnicode_FromString("AverageRssiPrimaryParent"), Py_BuildValue("i", statusMessage.AverageRssiPrimaryParent));
                  PyDict_SetItem(result, PyUnicode_FromString("AverageRssiSecondaryParent"), Py_BuildValue("i", statusMessage.AverageRssiSecondaryParent));
                  PyDict_SetItem(result, PyUnicode_FromString("Rank"), Py_BuildValue("i", statusMessage.Rank));
                  PyDict_SetItem(result, PyUnicode_FromString("Event"), Py_BuildValue("i", statusMessage.Event));
                  PyDict_SetItem(result, PyUnicode_FromString("EventData"), Py_BuildValue("i", statusMessage.EventData));
                  PyDict_SetItem(result, PyUnicode_FromString("OverallFault"), Py_BuildValue("i", statusMessage.OverallFault));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", statusMessage.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_ROUTE_ADD_E:
             {
               RouteAddMessage_t routeAdd = { 0 };

               status = MC_PUP_UnpackRouteAddMsg(&phyDataInd, &routeAdd);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i", routeAdd.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i", routeAdd.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i", routeAdd.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i", routeAdd.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i", routeAdd.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i", routeAdd.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i", routeAdd.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Zone"), Py_BuildValue("i", routeAdd.Zone));
                  PyDict_SetItem(result, PyUnicode_FromString("Rank"), Py_BuildValue("i", routeAdd.Rank));
                  PyDict_SetItem(result, PyUnicode_FromString("IsPrimary"), Py_BuildValue("i", routeAdd.IsPrimary));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i", routeAdd.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E:
             {
               RouteAddResponseMessage_t routeAddResponse = { 0 };

               status = MC_PUP_UnpackRouteAddResponseMsg(&phyDataInd, &routeAddResponse);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",routeAddResponse.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",routeAddResponse.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",routeAddResponse.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",routeAddResponse.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",routeAddResponse.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",routeAddResponse.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",routeAddResponse.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Accepted"), Py_BuildValue("i",routeAddResponse.Accepted));
                  PyDict_SetItem(result, PyUnicode_FromString("DayNight"), Py_BuildValue("i",routeAddResponse.DayNight));
                  PyDict_SetItem(result, PyUnicode_FromString("GlobalDelayEnabled"), Py_BuildValue("i",routeAddResponse.GlobalDelayEnabled));
                  PyDict_SetItem(result, PyUnicode_FromString("ZoneEnabled"), Py_BuildValue("i",routeAddResponse.ZoneEnabled));
                  PyDict_SetItem(result, PyUnicode_FromString("ZoneLowerUpper"), Py_BuildValue("i",routeAddResponse.ZoneLowerUpper));
                  PyDict_SetItem(result, PyUnicode_FromString("ZoneMapWord"), Py_BuildValue("i",routeAddResponse.ZoneMapWord));
                  PyDict_SetItem(result, PyUnicode_FromString("ZoneMapHalfWord"), Py_BuildValue("i",routeAddResponse.ZoneMapHalfWord));
                  PyDict_SetItem(result, PyUnicode_FromString("FaultsEnabled"), Py_BuildValue("i",routeAddResponse.FaultsEnabled));
                  PyDict_SetItem(result, PyUnicode_FromString("GlobalDelayOverride"), Py_BuildValue("i",routeAddResponse.GlobalDelayOverride));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",routeAddResponse.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_ROUTE_DROP_E:
             {
               RouteDropMessage_t routeDrop = { 0 };

               status = MC_PUP_UnpackRouteDropMsg(&phyDataInd, &routeDrop);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",routeDrop.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",routeDrop.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",routeDrop.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",routeDrop.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",routeDrop.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",routeDrop.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",routeDrop.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Reason"), Py_BuildValue("i",routeDrop.Reason));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",routeDrop.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_STATE_SIGNAL_E:
             {
               SetStateMessage_t setStateMessage = { 0 };

               status = MC_PUP_UnpackSetState(&phyDataInd, &setStateMessage);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",setStateMessage.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",setStateMessage.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",setStateMessage.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",setStateMessage.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",setStateMessage.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",setStateMessage.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",setStateMessage.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("State"), Py_BuildValue("i",setStateMessage.State));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",setStateMessage.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_LOAD_BALANCE_E:
             {
               LoadBalanceMessage_t loadBalance = { 0 };

               status = MC_PUP_UnpackLoadBalanceMsg(&phyDataInd, &loadBalance);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",loadBalance.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",loadBalance.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",loadBalance.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",loadBalance.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",loadBalance.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",loadBalance.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",loadBalance.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",loadBalance.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E:
             {
               LoadBalanceResponseMessage_t loadBalance = { 0 };

               status = MC_PUP_UnpackLoadBalanceResponseMsg(&phyDataInd, &loadBalance);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",loadBalance.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",loadBalance.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",loadBalance.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",loadBalance.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",loadBalance.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",loadBalance.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",loadBalance.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Accepted"), Py_BuildValue("i",loadBalance.Accepted));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",loadBalance.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
             case APP_MSG_TYPE_RBU_DISABLE_E:
             {
               RBUDisableMessage_t message = { 0 };

               status = MC_PUP_UnpackRBUDisableMessage(&phyDataInd, &message);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",message.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",message.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",message.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",message.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",message.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",message.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",message.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("UnitAddress"), Py_BuildValue("i",message.UnitAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",message.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
               break;
             }
            case APP_MSG_TYPE_PING_E:
            {
                     PingRequest_t message = { 0 };

                     status = MC_PUP_UnpackPingRequestMessage(&phyDataInd, &message);
                     if (SUCCESS_E == status)
                     {
                        PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",message.Header.FrameType));
                        PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                        PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",message.Header.MACDestinationAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",message.Header.MACSourceAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",message.Header.HopCount));
                        PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",message.Header.DestinationAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",message.Header.SourceAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",message.Header.MessageType));
                        PyDict_SetItem(result, PyUnicode_FromString("Payload"), Py_BuildValue("i",message.Payload));
                        PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",message.SystemId));
                     }
                     else
                     {
                        PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
                     }
               break;
            }
            case APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E:
            {
                     BatteryStatusMessage_t message = { 0 };

                     status = MC_PUP_UnpackBatteryStatusMessage(&phyDataInd, &message);
                     if (SUCCESS_E == status)
                     {
                        PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",message.Header.FrameType));
                        PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                        PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",message.Header.MACDestinationAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",message.Header.MACSourceAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",message.Header.HopCount));
                        PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",message.Header.DestinationAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",message.Header.SourceAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",message.Header.MessageType));
                        PyDict_SetItem(result, PyUnicode_FromString("Primary Battery"), Py_BuildValue("i",message.PrimaryBatteryVoltage));
                        PyDict_SetItem(result, PyUnicode_FromString("Backup Battery"), Py_BuildValue("i",message.BackupBatteryVoltage));
                        PyDict_SetItem(result, PyUnicode_FromString("Device Combination"), Py_BuildValue("i",message.DeviceCombination));
                        PyDict_SetItem(result, PyUnicode_FromString("Zone"), Py_BuildValue("i",message.ZoneNumber));
                        PyDict_SetItem(result, PyUnicode_FromString("Smoke"), Py_BuildValue("i",message.SmokeAnalogueValue));
                        PyDict_SetItem(result, PyUnicode_FromString("Heat"), Py_BuildValue("i",message.HeatAnalogueValue));
                        PyDict_SetItem(result, PyUnicode_FromString("PIR"), Py_BuildValue("i",message.PirAnalogueValue));
                        PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",message.SystemId));
                     }
                     else
                     {
                        PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
                     }
               break;
            }
            case APP_MSG_TYPE_ZONE_ENABLE_E:
            {
                     ZoneEnableMessage_t message = { 0 };

                     status = MC_PUP_UnpackZoneEnableMessage(&phyDataInd, &message);
                     if (SUCCESS_E == status)
                     {
                        PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",message.Header.FrameType));
                        PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                        PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",message.Header.MACDestinationAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",message.Header.MACSourceAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",message.Header.HopCount));
                        PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",message.Header.DestinationAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",message.Header.SourceAddress));
                        PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",message.Header.MessageType));
                        PyDict_SetItem(result, PyUnicode_FromString("LowHigh"), Py_BuildValue("i",message.LowHigh));
                        PyDict_SetItem(result, PyUnicode_FromString("HalfWord"), Py_BuildValue("i",message.HalfWord));
                        PyDict_SetItem(result, PyUnicode_FromString("Word"), Py_BuildValue("i",message.Word));
                        PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",message.SystemId));
                     }
                     else
                     {
                        PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
                     }
               break;
            }
          }

         break;
      }
      case FRAME_TYPE_ACKNOWLEDGEMENT_E:
      {
         FrameAcknowledge_t ackMessage = { 0 };

         status = MC_PUP_UnpackAck(&phyDataInd, &ackMessage);
         if (SUCCESS_E == status)
         {
            PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",ackMessage.FrameType));
            PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",ackMessage.MACDestinationAddress));
            PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",ackMessage.MACSourceAddress));
            PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",ackMessage.SystemId));
         }
         else
         {
            PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
         }
         break;
      }
      case FRAME_TYPE_TEST_MESSAGE_E:
      {
               BatteryStatusMessage_t message = { 0 };

               status = MC_PUP_UnpackBatteryStatusMessage(&phyDataInd, &message);
               if (SUCCESS_E == status)
               {
                  PyDict_SetItem(result, PyUnicode_FromString("FrameType"), Py_BuildValue("i",message.Header.FrameType));
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", appMsgType));
                  PyDict_SetItem(result, PyUnicode_FromString("MACDestinationAddress"), Py_BuildValue("i",message.Header.MACDestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MACSourceAddress"), Py_BuildValue("i",message.Header.MACSourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("HopCount"), Py_BuildValue("i",message.Header.HopCount));
                  PyDict_SetItem(result, PyUnicode_FromString("DestinationAddress"), Py_BuildValue("i",message.Header.DestinationAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("SourceAddress"), Py_BuildValue("i",message.Header.SourceAddress));
                  PyDict_SetItem(result, PyUnicode_FromString("MessageType"), Py_BuildValue("i",message.Header.MessageType));
                  PyDict_SetItem(result, PyUnicode_FromString("Primary Battery"), Py_BuildValue("i",message.PrimaryBatteryVoltage));
                  PyDict_SetItem(result, PyUnicode_FromString("Backup Battery"), Py_BuildValue("i",message.BackupBatteryVoltage));
                  PyDict_SetItem(result, PyUnicode_FromString("Device Combination"), Py_BuildValue("i",message.DeviceCombination));
                  PyDict_SetItem(result, PyUnicode_FromString("Zone"), Py_BuildValue("i",message.ZoneNumber));
                  PyDict_SetItem(result, PyUnicode_FromString("Smoke"), Py_BuildValue("i",message.SmokeAnalogueValue));
                  PyDict_SetItem(result, PyUnicode_FromString("Heat"), Py_BuildValue("i",message.HeatAnalogueValue));
                  PyDict_SetItem(result, PyUnicode_FromString("PIR"), Py_BuildValue("i",message.PirAnalogueValue));
                  PyDict_SetItem(result, PyUnicode_FromString("SystemId"), Py_BuildValue("i",message.SystemId));
               }
               else
               {
                  PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
               }
         break;
      }
      case FRAME_TYPE_PPU_MODE_E:
      {
         PPU_Message_t message = { 0 };

         status = MC_PUP_UnpackPpuMessage(&phyDataInd, &message);
         if (SUCCESS_E == status)
         {
            PyDict_SetItem(result, PyUnicode_FromString("PpuAddress"), Py_BuildValue("i",message.PpuAddress));
            PyDict_SetItem(result, PyUnicode_FromString("RbuAddress"), Py_BuildValue("i", message.RbuAddress));
            PyDict_SetItem(result, PyUnicode_FromString("SystemID"), Py_BuildValue("i",message.SystemID));
            PyDict_SetItem(result, PyUnicode_FromString("Checksum"), Py_BuildValue("i",message.Checksum));
            PyDict_SetItem(result, PyUnicode_FromString("CalculatedChecksum"), Py_BuildValue("i",message.CalculatedChecksum));
            PyDict_SetItem(result, PyUnicode_FromString("PacketLength"), Py_BuildValue("i",message.PacketLength));
            PyDict_SetItem(result, PyUnicode_FromString("Command"), Py_BuildValue("i",message.Command));
            PyDict_SetItem(result, PyUnicode_FromString("Payload"), Py_BuildValue("y",message.Payload));
         }
         else
         {
            PyDict_SetItem(result, PyUnicode_FromString("appMsgType"), Py_BuildValue("i", -1));
         }
      break;
      }
     default:
     {
      Py_INCREF(Py_None);
      return Py_None;
    }
   }
   }
   else
   {
     Py_INCREF(Py_None);
    return Py_None;
   }

   return result;
}

