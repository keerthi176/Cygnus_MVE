from enum import Enum


class FaultId(Enum):
    """
    typedef enum
    {
   CO_INSTALLATION_TAMPER_FAULT_E = 0x01u,
   CO_DISMANTLE_TAMPER_FAULT_E    = 0x02u,
   CO_LOW_BATTERY_E               = 0x04u,
   CO_DETECTOR_FAULT_E            = 0x8u,
   CO_BEACON_FAULT_E              = 0x10u,
   CO_SOUNDER_FAULT_E             = 0x20u,
   CO_IO_INPUT_1_FAULT_E          = 0x40u,
   CO_IO_INPUT_2_FAULT_E          = 0x80u,
   CO_DISMANTLE_HEAD_FAULT_E      = 0x100u,
   CO_MISMATCH_HEAD_FAULT_E       = 0x200u,
   CO_BATTERY_FAULT_E             = 0x400u,
   CO_DEVICE_ID_MISMATCH_E        = 0x800u,
   CO_DIRTY_SENSOR_E              = 0x1000u,
   CO_INTERNAL_FAULT_E            = 0x2000u,
   CO_INPUT_SHORT_CIRCUIT_FAULT_E = 0x4000u,
   CO_INPUT_OPEN_CIRCUIT_FAULT_E  = 0x8000u,
   CO_OUTPUT_FAULT_E              = 0x10000u,
   CO_BIT_FAULT_E                 = 0x20000u,
   CO_FAULT_MASK_E                = 0x3FFFFu
    }CO_FaultId_t;
    """
    INSTALLATION_TAMPER = 0x01
    DISMANTLE_TAMPER = 0x02
    LOW_MAIN_BATTERY = 0x04
    DETECTOR_FAULT = 0x08
    BEACON_FAULT = 0x10
    SOUNDER_FAULT = 0x20
    IO_INPUT_1_FAULT = 0x40
    IO_INPUT_2_FAULT = 0x80
    DISMANTLE_HEAD = 0x100
    MISMATCH_HEAD_FAULT = 0x200
    BATTERY_FAULT = 0x400
    DEVICE_ID_MISMATCH = 0x800
    DIRTY_SENSOR = 0x1000
    INTERNAL_FAULT = 0x2000
    INPUT_SHORT_CIRCUIT_FAULT = 0x4000
    INPUT_OPEN_CIRCUIT_FAULT = 0x8000
    OUTPUT_FAULT = 0x10000
    BIT_FAULT = 0x20000
    FAULT_MASK = 0x3FFFF

class FaultType(Enum):
    """
    typedef enum
    {
       FAULT_SIGNAL_NO_ERROR_E,                     //0
       FAULT_SIGNAL_TYPE_MISMATCH_E,                //1
       FAULT_SIGNAL_DEVICE_ID_MISMATCH_E,           //2
       FAULT_SIGNAL_FAULTY_SENSOR_E,                //3
       FAULT_SIGNAL_DIRTY_SENSOR_E,                 //4
       FAULT_SIGNAL_FAULTY_DIRTY_SENSOR_E,          //5
       FAULT_SIGNAL_INTERNAL_FAULT_E,               //6
       FAULT_SIGNAL_SOUNDER_FAULT_E,                //7
       FAULT_SIGNAL_BEACON_LED_FAULT_E,             //8
       FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E,    //9
       FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E,     //10
       FAULT_SIGNAL_OUTPUT_FAULT_E,                 //11
       FAULT_SIGNAL_INSTALLATION_TAMPER_E,          //12
       FAULT_SIGNAL_DISMANTLE_TAMPER_E,             //13
       FAULT_SIGNAL_LOW_BATTERY_E,                  //14
       FAULT_SIGNAL_BATTERY_FAULT_E,                //15
       FAULT_SIGNAL_LOW_LINK_QUALITY_E,             //16
       FAULT_SIGNAL_LOW_PARENT_COUNT_FAULT_E,       //17
       FAULT_SIGNAL_DEVICE_DROPPED_E,               //18
       FAULT_SIGNAL_HEAD_REMOVED_E,                 //19
       FAULT_SIGNAL_FIRMWARE_VERSION_E,             //20
       FAULT_SIGNAL_MAX_E
    } FaultSignalType_t;
    """
    FAULT_SIGNAL_NO_ERROR_E = 0
    FAULT_SIGNAL_TYPE_MISMATCH_E = 1
    FAULT_SIGNAL_DEVICE_ID_MISMATCH_E = 2
    FAULT_SIGNAL_FAULTY_SENSOR_E = 3
    FAULT_SIGNAL_DIRTY_SENSOR_E = 4
    FAULT_SIGNAL_FAULTY_DIRTY_SENSOR_E = 5
    FAULT_SIGNAL_INTERNAL_FAULT_E = 6
    FAULT_SIGNAL_SOUNDER_FAULT_E = 7
    FAULT_SIGNAL_BEACON_LED_FAULT_E = 8
    FAULT_SIGNAL_INPUT_SHORT_CIRCUIT_FAULT_E = 9
    FAULT_SIGNAL_INPUT_OPEN_CIRCUIT_FAULT_E = 10
    FAULT_SIGNAL_OUTPUT_FAULT_E = 11
    FAULT_SIGNAL_INSTALLATION_TAMPER_E = 12
    FAULT_SIGNAL_DISMANTLE_TAMPER_E = 13
    FAULT_SIGNAL_LOW_BATTERY_E = 14
    FAULT_SIGNAL_BATTERY_FAULT_E = 15
    FAULT_SIGNAL_LOW_LINK_QUALITY_E = 16
    FAULT_SIGNAL_LOW_PARENT_COUNT_FAULT_E = 17
    FAULT_SIGNAL_DEVICE_DROPPED_E = 18
    FAULT_SIGNAL_HEAD_REMOVED_E = 19
    FAULT_SIGNAL_FIRMWARE_VERSION = 20
    FAULT_SIGNAL_MAX_E = 21


class DetectorFault(Enum):
    """
    typedef enum
    {
       CO_DETECTOR_NO_FAULT,
       CO_DETECTOR_TYPE_MISMATCH,
       CO_DETECTOR_ID_MISMATCH,
       CO_DETECTOR_SENSOR_FAULT,
       CO_DETECTOR_DIRTY_SENSOR,
       CO_DETECTOR_DIRTY_AND_SENSOR_FAULT,
       CO_DETECTOR_INTERNAL_FAULT
    } CO_DetectorFault_t;
    """
    NO_FAULT = 0
    TYPE_MISMATCH = 1
    ID_MISMATCH = 2
    SENSOR_FAULT = 3
    DIRTY_SENSOR = 4
    DIRTY_AND_SENSOR_FAULT = 5
    INTERNAL_FAULT = 6


class ChannelIndex(Enum):
    """
    typedef enum
    {
       CO_CHANNEL_NONE_E,                                 // 0
       CO_CHANNEL_SMOKE_E,                                // 1
       CO_CHANNEL_HEAT_B_E,                               // 2
       CO_CHANNEL_CO_E,                                   // 3
       CO_CHANNEL_PIR_E,                                  // 4
       CO_CHANNEL_SOUNDER_E,                              // 5
       CO_CHANNEL_BEACON_E,                               // 6
       CO_CHANNEL_FIRE_CALLPOINT_E,                       // 7
       CO_CHANNEL_STATUS_INDICATOR_LED_E,                 // 8
       CO_CHANNEL_VISUAL_INDICATOR_E,                     // 9
       CO_CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E,    // 10
       CO_CHANNEL_MEDICAL_CALLPOINT_E,                    // 11
       CO_CHANNEL_EVAC_CALLPOINT_E,                       // 12
       CO_CHANNEL_OUTPUT_ROUTING_E,                       // 13
       CO_CHANNEL_INPUT_1_E,
       CO_CHANNEL_INPUT_2_E,                              // 15
       CO_CHANNEL_INPUT_3_E,
       CO_CHANNEL_INPUT_4_E,
       CO_CHANNEL_INPUT_5_E,
       CO_CHANNEL_INPUT_6_E,
       CO_CHANNEL_INPUT_7_E,                              // 20
       CO_CHANNEL_INPUT_8_E,
       CO_CHANNEL_INPUT_9_E,
       CO_CHANNEL_INPUT_10_E,
       CO_CHANNEL_INPUT_11_E,
       CO_CHANNEL_INPUT_12_E,                             // 25
       CO_CHANNEL_INPUT_13_E,
       CO_CHANNEL_INPUT_14_E,
       CO_CHANNEL_INPUT_15_E,
       CO_CHANNEL_INPUT_16_E,
       CO_CHANNEL_INPUT_17_E,                             // 30
       CO_CHANNEL_INPUT_18_E,
       CO_CHANNEL_INPUT_19_E,
       CO_CHANNEL_INPUT_20_E,
       CO_CHANNEL_INPUT_21_E,
       CO_CHANNEL_INPUT_22_E,                             // 35
       CO_CHANNEL_INPUT_23_E,
       CO_CHANNEL_INPUT_24_E,
       CO_CHANNEL_INPUT_25_E,
       CO_CHANNEL_INPUT_26_E,
       CO_CHANNEL_INPUT_27_E,                             //40
       CO_CHANNEL_INPUT_28_E,
       CO_CHANNEL_INPUT_29_E,
       CO_CHANNEL_INPUT_30_E,
       CO_CHANNEL_INPUT_31_E,
       CO_CHANNEL_INPUT_32_E,                             //45
       CO_CHANNEL_OUTPUT_1_E,
       CO_CHANNEL_OUTPUT_2_E,
       CO_CHANNEL_OUTPUT_3_E,
       CO_CHANNEL_OUTPUT_4_E,
       CO_CHANNEL_OUTPUT_5_E,                             //50
       CO_CHANNEL_OUTPUT_6_E,
       CO_CHANNEL_OUTPUT_7_E,
       CO_CHANNEL_OUTPUT_8_E,
       CO_CHANNEL_OUTPUT_9_E,
       CO_CHANNEL_OUTPUT_10_E,                            //55
       CO_CHANNEL_OUTPUT_11_E,
       CO_CHANNEL_OUTPUT_12_E,
       CO_CHANNEL_OUTPUT_13_E,
       CO_CHANNEL_OUTPUT_14_E,
       CO_CHANNEL_OUTPUT_15_E,                            // 60
       CO_CHANNEL_OUTPUT_16_E,
       CO_CHANNEL_HEAT_A1R_E,
       CO_CHANNEL_RADIO_E,
       CO_CHANNEL_WATER_LEAK_E,
       CO_CHANNEL_MAX_E
    } CO_ChannelIndex_t;
    """
    CHANNEL_NONE_E = 0
    CHANNEL_SMOKE_E = 1
    CHANNEL_HEAT_B_E = 2
    CHANNEL_CO_E = 3
    CHANNEL_PIR_E = 4
    CHANNEL_SOUNDER_E = 5
    CHANNEL_BEACON_E = 6
    CHANNEL_FIRE_CALLPOINT_E = 7
    CHANNEL_STATUS_INDICATOR_LED_E = 8
    CHANNEL_VISUAL_INDICATOR_E = 9
    CHANNEL_SOUNDER_VISUAL_INDICATOR_COMBINED_E = 10
    CHANNEL_MEDICAL_CALLPOINT_E = 11
    CHANNEL_EVAC_CALLPOINT_E = 12
    CHANNEL_OUTPUT_ROUTING_E = 13
    CHANNEL_INPUT_1_E = 14
    CHANNEL_INPUT_2_E = 15
    CHANNEL_INPUT_3_E = 16
    CHANNEL_INPUT_4_E = 17
    CHANNEL_INPUT_5_E = 18
    CHANNEL_INPUT_6_E = 19
    CHANNEL_INPUT_7_E = 20
    CHANNEL_INPUT_8_E = 21
    CHANNEL_INPUT_9_E = 22
    CHANNEL_INPUT_10_E = 23
    CHANNEL_INPUT_11_E = 24
    CHANNEL_INPUT_12_E = 25
    CHANNEL_INPUT_13_E = 26
    CHANNEL_INPUT_14_E = 27
    CHANNEL_INPUT_15_E = 28
    CHANNEL_INPUT_16_E = 29
    CHANNEL_INPUT_17_E = 30
    CHANNEL_INPUT_18_E = 31
    CHANNEL_INPUT_19_E = 32
    CHANNEL_INPUT_20_E = 33
    CHANNEL_INPUT_21_E = 34
    CHANNEL_INPUT_22_E = 35
    CHANNEL_INPUT_23_E = 36
    CHANNEL_INPUT_24_E = 37
    CHANNEL_INPUT_25_E = 38
    CHANNEL_INPUT_26_E = 39
    CHANNEL_INPUT_27_E = 40
    CHANNEL_INPUT_28_E = 41
    CHANNEL_INPUT_29_E = 42
    CHANNEL_INPUT_30_E = 43
    CHANNEL_INPUT_31_E = 44
    CHANNEL_INPUT_32_E = 45
    CHANNEL_OUTPUT_1_E = 46
    CHANNEL_OUTPUT_2_E = 47
    CHANNEL_OUTPUT_3_E = 48
    CHANNEL_OUTPUT_4_E = 49
    CHANNEL_OUTPUT_5_E = 50
    CHANNEL_OUTPUT_6_E = 51
    CHANNEL_OUTPUT_7_E = 52
    CHANNEL_OUTPUT_8_E = 53
    CHANNEL_OUTPUT_9_E = 54
    CHANNEL_OUTPUT_10_E = 55
    CHANNEL_OUTPUT_11_E = 56
    CHANNEL_OUTPUT_12_E = 57
    CHANNEL_OUTPUT_13_E = 58
    CHANNEL_OUTPUT_14_E = 59
    CHANNEL_OUTPUT_15_E = 60
    CHANNEL_OUTPUT_16_E = 61
    CHANNEL_HEAT_A1R_E = 62
    CHANNEL_RADIO = 63
    CHANNEL_WATER_LEAK = 64
    CHANNEL_MAX_E = 65


class SounderFault(Enum):
    """
    typedef enum
    {
       CO_SOUNDER_NO_FAULT,
       CO_SOUNDER_TYPE_MISMATCH,
       CO_SOUNDER_ID_MISMATCH,
       CO_SOUNDER_INTERNAL_FAULT
    } CO_SounderFault_t;
    """
    NO_FAULT = 0
    TYPE_MISMATCH = 1
    ID_MISMATCH = 2
    INTERNAL_FAULT = 3


class MeshEvent(Enum):
    """
    typedef enum
    {
        CO_MESH_EVENT_NONE_E, //just a status update, not initiated by an event
        CO_MESH_EVENT_CHILD_NODE_ADDED_E,
        CO_MESH_EVENT_CHILD_NODE_DROPPED_E,
        CO_MESH_EVENT_PRIMARY_PARENT_ADDED_E,
        CO_MESH_EVENT_PRIMARY_PARENT_DROPPED_E,
        CO_MESH_EVENT_SECONDARY_PARENT_ADDED_E,
        CO_MESH_EVENT_SECONDARY_PARENT_DROPPED_E,
        CO_MESH_EVENT_PRIMARY_TRACKING_NODE_ADDED_E,
        CO_MESH_EVENT_PRIMARY_TRACKING_NODE_DROPPED_E,
        CO_MESH_EVENT_SECONDARY_TRACKING_NODE_ADDED_E,
        CO_MESH_EVENT_SECONDARY_TRACKING_NODE_DROPPED_E,
        CO_MESH_EVENT_SECONDARY_PARENT_PROMOTED_E,
        CO_MESH_EVENT_PRIMARY_TRACKING_NODE_PROMOTED_E,
        CO_MESH_EVENT_REPORT_NODES,
        CO_MESH_EVENT_ZONE_CHANGE,
        CO_MESH_EVENT_STATE_CHANGE
        CO_MESH_EVENT_BATTERY_STATUS_E,
        CO_MESH_EVENT_APPLICATION_REQUEST_E,
        CO_MESH_EVENT_INITIAL_TRK_NODE_LOST_E,
        CO_MESH_EVENT_DAY_NIGHT_CHANGE_E
        CO_MESH_EVENT_SYNCH_LOSS_E,
        CO_MESH_EVENT_COMMANDED_RESET_E,
        CO_MESH_EVENT_PPU_MODE_REQUESTED_E,
        CO_MESH_EVENT_COMMS_FAIL_E
    }CO_MeshEvent_t;  /* if you change this list, update MESH_EVENT_STR below */
    """
    NONE = 0
    CHILD_NODE_ADDED = 1
    CHILD_NODE_DROPPED = 2
    PRIMARY_PARENT_ADDED = 3
    PRIMARY_PARENT_DROPPED = 4
    SECONDARY_PARENT_ADDED = 5
    SECONDARY_PARENT_DROPPED = 6
    PRIMARY_TRACKING_NODE_ADDED = 7
    PRIMARY_TRACKING_NODE_DROPPED = 8
    SECONDARY_TRACKING_NODE_ADDED = 9
    SECONDARY_TRACKING_NODE_DROPPED = 10
    SECONDARY_PARENT_PROMOTED = 11
    PRIMARY_TRACKING_NODE_PROMOTED = 12
    REPORT_NODES = 13
    ZONE_CHANGE = 14
    STATE_CHANGE = 15
    BATTERY_STATUS = 16
    APPLICATION_REQUEST = 17
    INITIAL_TRK_NODE_LOST = 18
    DAY_NIGHT_CHANGE = 19
    MESH_EVENT_SYNCH_LOSS = 20
    MESH_EVENT_COMMANDED_RESET = 21
    MESH_EVENT_PPU_MODE_REQUESTED = 22
    MESH_EVENT_COMMS_FAIL = 23

class MessageType(Enum):
    """
    typedef enum
    {
        CO_MESSAGE_GENERATE_HEARTBEAT_E,
        CO_MESSAGE_PHY_DATA_IND_E,
        CO_MESSAGE_PHY_DATA_REQ_E,
        CO_MESSAGE_HEARTBEAT_E,
        CO_MESSAGE_GENERATE_FIRE_SIGNAL_E,
        CO_MESSAGE_GENERATE_ALARM_SIGNAL_E,            //5
        CO_MESSAGE_GENERATE_OUTPUT_SIGNAL_E,
        CO_MESSAGE_GENERATE_LOGON_REQ,
        CO_MESSAGE_GENERATE_FAULT_SIGNAL_E,
        CO_MESSAGE_GENERATE_COMMAND_SIGNAL_E,
        CO_MESSAGE_GENERATE_RESPONSE_SIGNAL_E,         //10
        CO_MESSAGE_MAC_EVENT_E,
        CO_MESSAGE_GENERATE_TEST_MESSAGE_E,
        CO_MESSAGE_GENERATE_SET_STATE_E,
        CO_MESSAGE_GENERATE_ROUTE_ADD_RESPONSE_E,
        CO_MESSAGE_GENERATE_ROUTE_ADD_REQUEST_E,       //15
        CO_MESSAGE_GENERATE_RBU_DISABLE_MESSAGE_E,
        CO_MESSAGE_GENERATE_RBU_REPORT_MESSAGE_E,
        CO_MESSAGE_GENERATE_STATUS_INDICATION_E,
        CO_MESSAGE_GENERATE_ROUTE_DROP_REQUEST_E,
        CO_MESSAGE_RUN_BUILT_IN_TESTS_E,               //20
        CO_MESSAGE_MESH_STATUS_EVENT_E,
        CO_MESSAGE_TIMER_EVENT_E,
        CO_MESSAGE_GENERATE_PPU_DEV_LIST_E,
        CO_MESSAGE_BROADCAST_PP_FREQ_E,
        CO_MESSAGE_BROADCAST_PP_SYS_ID_E,              //25
        CO_MESSAGE_BROADCAST_PP_MODE_REQ_E,
        CO_MESSAGE_CONFIRMATION_E,
        CO_MESSAGE_CIE_COMMAND_E,
        CO_MESSAGE_SERVICE_TX_BUFFER_QUEUE_E,
        CO_MESSAGE_APPLICATION_REQUEST_E,              //30
        CO_MESSAGE_GENERATE_ALARM_OUTPUT_STATE_E,
        CO_MESSAGE_GENERATE_PING_REQUEST_E,
        CO_MESSAGE_GENERATE_BATTERY_STATUS_E,
        CO_MESSAGE_BATTERY_STATUS_MESSAGE_E,
        CO_MESSAGE_RESET_EVENT_E,                      //35
        CO_MESSAGE_GENERATE_ZONE_ENABLE_E,
        CO_MESSAGE_SET_OUTPUT_E,
        CO_MESSAGE_REQUEST_SENSOR_DATA_E,
        CO_MESSAGE_SENSOR_DATA_E,
        CO_MESSAGE_GENERATE_ATTX_COMMAND_E,             //40
        CO_MESSAGE_GENERATE_ATTX_RESPONSE_E,
        CO_MESSAGE_GENERATE_START_OTA_AT_MODE_E,
        CO_MESSAGE_GENERATE_PPU_MODE_REQUEST_E,
        CO_MESSAGE_PPU_MODE_REQUEST_E,
        CO_MESSAGE_PPU_COMMAND_E,                       //45
        CO_MESSAGE_PPU_RESPONSE_E,
        CO_MESSAGE_NVM_CLEANUP_E,
        CO_MESSAGE_MAX_E
    } CO_MessageType_t;
    """
    GENERATE_HEARTBEAT = 0
    PHY_DATA_IND = 1
    PHY_DATA_REQ = 2
    HEARTBEAT = 3
    GENERATE_FIRE_SIGNAL = 4
    GENERATE_ALARM_SIGNAL = 5
    GENERATE_OUTPUT_SIGNAL = 6
    GENERATE_LOGON_REQ = 7
    GENERATE_FAULT_SIGNAL = 8
    GENERATE_COMMAND_SIGNAL = 9
    GENERATE_RESPONSE_SIGNAL = 10
    MAC_EVENT = 11
    GENERATE_TEST_MESSAGE = 12
    GENERATE_SET_STATE = 13
    GENERATE_ROUTE_ADD_RESPONSE = 14
    GENERATE_ROUTE_ADD_REQUEST = 15
    GENERATE_RBU_DISABLE_MESSAGE = 16
    GENERATE_RBU_REPORT_MESSAGE = 17
    GENERATE_STATUS_INDICATION = 18
    GENERATE_ROUTE_DROP_REQUEST = 19
    RUN_BUILT_IN_TESTS = 20
    MESH_STATUS_EVENT = 21
    TIMER_EVENT = 22
    GENERATE_PPU_DEV_LIST = 23
    BROADCAST_PP_FREQ = 24
    BROADCAST_PP_SYS_ID = 25
    BROADCAST_PP_MODE_REQ = 26
    CONFIRMATION = 27
    CIE_COMMAND = 28
    SERVICE_TX_BUFFER_QUEUE = 29
    APPLICATION_REQUEST = 30
    GENERATE_ALARM_OUTPUT_STATE = 31
    GENERATE_PING_REQUEST = 32
    GENERATE_BATTERY_STATUS = 33
    BATTERY_STATUS_MESSAGE = 34
    RESET_EVENT = 35
    GENERATE_ZONE_ENABLE = 36
    SET_OUTPUT = 37
    REQUEST_SENSOR_DATA = 38
    SENSOR_DATA = 39
    GENERATE_ATTX_COMMAND = 40
    GENERATE_ATTX_RESPONSE = 41
    GENERATE_START_OTA_AT_MODE = 42
    GENERATE_PPU_MODE_REQUEST = 43
    PPU_MODE_REQUEST = 44
    PPU_COMMAND = 45
    PPU_RESPONSE = 46
    MESSAGE_PPU_RESPONSE = 47
    MESSAGE_NVM_CLEANUP = 48
    MESSAGE_MAX = 49


class RBUSensorType(Enum):
    """
    typedef enum
    {
       CO_NONE_E,
       CO_SMOKE_E,
       CO_HEAT_A1R_E,
       CO_HEAT_B_E,
       CO_SMOKE_AND_HEAT_A1R_E,
       CO_SMOKE_AND_PIR_E,
       CO_PIR_E,
       CO_FIRE_CALLPOINT_E,
       CO_FIRST_AID_CALLPOINT_E,
       CO_SOUNDER_E,
       CO_BEACON_W_2_4_7_5_E,
       CO_BEACON_W_3_1_11_3_E,
       CO_BEACON_C_3_8_9_E,
       CO_BEACON_C_3_15_E,
       CO_VISUAL_INDICATOR_E,
       CO_REMOTE_INDICATOR_E,
       CO_IO_UNIT_INPUT_1_E,
       CO_IO_UNIT_OUTPUT_1_E,
       CO_IO_UNIT_INPUT_2_E,
       CO_IO_UNIT_OUTPUT_2_E
    } CO_RBUSensorType_t;
    """
    NONE = 0
    SMOKE = 1
    HEAT_A1R = 2
    HEAT_B = 3
    SMOKE_AND_HEAT_A1R = 4
    SMOKE_AND_PIR = 5
    PIR = 6
    FIRE_CALLPOINT = 7
    FIRST_AID_CALLPOINT = 8
    SOUNDER = 9
    BEACON_W_2_4_7_5 = 10
    BEACON_W_3_1_11_3 = 11
    BEACON_C_3_8_9 = 12
    BEACON_C_3_15 = 13
    VISUAL_INDICATOR = 14
    REMOTE_INDICATOR = 15
    IO_UNIT_INPUT_1 = 16
    IO_UNIT_OUTPUT_1 = 17
    IO_UNIT_INPUT_2 = 18
    IO_UNIT_OUTPUT_2_E = 19


class OutputProfile(Enum):
    """
    typedef enum
    {
       CO_PROFILE_FIRE_E,
       CO_PROFILE_FIRST_AID_E,
       CO_PROFILE_EVACUATE_E,
       CO_PROFILE_SECURITY_E,
       CO_PROFILE_GENERAL_E,
       CO_PROFILE_FAULT_E,
       CO_PROFILE_ROUTING_ACK_E,
       CO_PROFILE_TEST_E,
       CO_PROFILE_SILENT_TEST_E,
       CO_PROFILE_MAX_E
    } CO_OutputProfile_t;
    """
    FIRE = 0
    FIRST_AID = 1
    EVACUATE = 2
    SECURITY = 3
    GENERAL = 4
    FAULT = 5
    ROUTING = 6
    TEST = 7
    SILENT_TEST = 8

class HeadMessageType(Enum):
    """
    typedef enum
    {
       HEAD_WRITE_ENABLED_E,
       HEAD_WRITE_PREALARM_THRESHOLD_E,
       HEAD_WRITE_ALARM_THRESHOLD_E,
       HEAD_WRITE_FAULT_THRESHOLD_E,
       HEAD_WRITE_FLASH_RATE_E,
       HEAD_WRITE_TONE_SELECTION_E,
       HEAD_WRITE_TEST_MODE_E,
       HEAD_WRITE_OUTPUT_ACTIVATED_E,
       HEAD_WRITE_INDICATOR_LED_E,
       HEAD_WRITE_IDENTIFICATION_NUMBER_E,
       HEAD_READ_DEVICE_TYPE_E,
       HEAD_READ_DEVICE_TYPE_AND_CLASS_E,
       HEAD_READ_ENABLED_E,
       HEAD_READ_PREALARM_THRESHOLD_E,
       HEAD_READ_ALARM_THRESHOLD_E,
       HEAD_READ_FAULT_THRESHOLD_E,
       HEAD_READ_FLASH_RATE_E,
       HEAD_READ_TONE_SELECTION_E,
       HEAD_READ_TEST_MODE_E,
       HEAD_READ_ANALOGUE_VALUE_E,
       HEAD_READ_SERIAL_NUMBER_E,
       HEAD_READ_FIRMWARE_VERSION_E,
       HEAD_READ_INDICATOR_LED_E,
       HEAD_READ_FAULT_TYPE_E,
       HEAD_READ_IDENTIFICATION_NUMBER_E,
       HEAD_EVENT_PREALARM_THRESHOLD_EXCEEDED_E,
       HEAD_EVENT_ALARM_THRESHOLD_EXCEEDED_E,
       HEAD_EVENT_FAULT_THRESHOLD_EXCEEDED_E,
       HEAD_RESET_PIR_E,
       HEAD_DAY_NIGHT_SETTING_E,
       HEAD_WAKE_UP_SIGNAL_E,
       HEAD_SLEEP_MODE_E,
       HEAD_INTERNAL_MESSAGE_E
    } HeadMessageType_t;
    """
    HEAD_WRITE_ENABLED = 0
    HEAD_WRITE_PREALARM_THRESHOLD = 1
    HEAD_WRITE_ALARM_THRESHOLD = 2
    HEAD_WRITE_FAULT_THRESHOLD = 3
    HEAD_WRITE_FLASH_RATE = 4
    HEAD_WRITE_TONE_SELECTION = 5
    HEAD_WRITE_TEST_MODE = 6
    HEAD_WRITE_OUTPUT_ACTIVATED = 7
    HEAD_WRITE_INDICATOR_LED = 8
    HEAD_WRITE_IDENTIFICATION_NUMBER = 9
    HEAD_READ_DEVICE_TYPE = 10
    HEAD_READ_DEVICE_TYPE_AND_CLASS = 11
    HEAD_READ_ENABLED = 12
    HEAD_READ_PREALARM_THRESHOLD = 13
    HEAD_READ_ALARM_THRESHOLD = 14
    HEAD_READ_FAULT_THRESHOLD = 15
    HEAD_READ_FLASH_RATE = 16
    HEAD_READ_TONE_SELECTION = 17
    HEAD_READ_TEST_MODE = 18
    HEAD_READ_ANALOGUE_VALUE = 19
    HEAD_READ_SERIAL_NUMBER = 20
    HEAD_READ_FIRMWARE_VERSION = 21
    HEAD_READ_INDICATOR_LED = 22
    HEAD_READ_FAULT_TYPE = 23
    HEAD_READ_IDENTIFICATION_NUMBER = 24
    HEAD_EVENT_PREALARM_THRESHOLD_EXCEEDED = 25
    HEAD_EVENT_ALARM_THRESHOLD_EXCEEDED = 26
    HEAD_EVENT_FAULT_THRESHOLD_EXCEEDED = 27
    HEAD_RESET_PIR = 28
    HEAD_DAY_NIGHT_SETTING = 29
    HEAD_WAKE_UP_SIGNAL = 30
    HEAD_SLEEP_MODE = 31
    HEAD_INTERNAL_MESSAGE = 32


class FrameType(Enum):
    """
    typedef enum
    {
       FRAME_TYPE_HEARTBEAT_E,
       FRAME_TYPE_DATA_E,
       FRAME_TYPE_ACKNOWLEDGEMENT_E,
       FRAME_TYPE_AT_E,
       FRAME_TYPE_PPU_MODE_E,
       FRAME_TYPE_TEST_MESSAGE_E,
       FRAME_TYPE_ERROR_E,
       FRAME_TYPE_MAX_E
    } FrameType_t;
    """
    UNKNOWN = -1
    HEARTBEAT = 0
    DATA = 1
    ACKNOWLEDGEMENT = 2
    AT = 3
    PPU_MODE = 4
    TEST_MESSAGE = 5
    ERROR = 6



# See ApplicationLayerMessageType_t enum  in CO_Message.h
class ApplicationLayerMessageType(Enum):
    """
    typedef enum
    {
       APP_MSG_TYPE_FIRE_SIGNAL_E,
       APP_MSG_TYPE_ALARM_SIGNAL_E,
       APP_MSG_TYPE_FAULT_SIGNAL_E,
       APP_MSG_TYPE_OUTPUT_SIGNAL_E,
       APP_MSG_TYPE_COMMAND_E,
       APP_MSG_TYPE_RESPONSE_E,
       APP_MSG_TYPE_LOGON_E,
       APP_MSG_TYPE_STATUS_INDICATION_E,
       APP_MSG_TYPE_APP_FIRMWARE_E,
       APP_MSG_TYPE_ROUTE_ADD_E,
       APP_MSG_TYPE_ROUTE_ADD_RESPONSE_E,
       APP_MSG_TYPE_ROUTE_DROP_E,
       APP_MSG_TYPE_TEST_MODE_E,
       APP_MSG_TYPE_TEST_SIGNAL_E,
       APP_MSG_TYPE_STATE_SIGNAL_E,
       APP_MSG_TYPE_LOAD_BALANCE_E,
       APP_MSG_TYPE_LOAD_BALANCE_RESPONSE_E,
       APP_MSG_TYPE_ACK_E,
       APP_MSG_TYPE_HEARTBEAT_E,
       APP_MSG_TYPE_RBU_DISABLE_E,
       APP_MSG_TYPE_STATUS_SIGNAL_E,
       APP_MSG_TYPE_UNKNOWN_E,
       APP_MSG_TYPE_ALARM_OUTPUT_STATE_SIGNAL_E,
       APP_MSG_TYPE_PING_E,
       APP_MSG_TYPE_BATTERY_STATUS_SIGNAL_E
       APP_MSG_TYPE_DAY_NIGHT_STATUS_E,
       APP_MSG_TYPE_ZONE_ENABLE_E,
       APP_MSG_TYPE_AT_COMMAND_E,
       APP_MSG_TYPE_PPU_MODE_E,
       APP_MSG_TYPE_PPU_COMMAND_E,
       APP_MSG_TYPE_GLOBAL_DELAYS_E,
       APP_MSG_TYPE_RESET_E,
       APP_MSG_TYPE_ADD_NODE_LINK_E,
       APP_MSG_TYPE_DROP_NODE_LINK_E,
       APP_MSG_CHECK_TDM_SYNCH_E,
       APP_MSG_TYPE_MAX_E
    } ApplicationLayerMessageType_t;
    """
    FIRE_SIGNAL = 0
    ALARM_SIGNAL = 1
    FAULT_SIGNAL = 2
    OUTPUT_SIGNAL = 3
    COMMAND = 4
    RESPONSE = 5
    LOGON = 6
    STATUS_INDICATION = 7
    APP_FIRMWARE = 8
    ROUTE_ADD = 9
    ROUTE_ADD_RESPONSE = 10
    ROUTE_DROP = 11
    TEST_MODE = 12
    TEST_SIGNAL = 13
    STATE_SIGNAL = 14
    LOAD_BALANCE = 15
    LOAD_BALANCE_RESPONSE = 16
    ACK = 17
    HEARTBEAT = 18
    RBU_DISABLE = 19
    STATUS_SIGNAL = 20
    UNKNOWN = 21
    ALARM_OUTPUT_STATE_SIGNAL = 22
    PING = 23
    BATTERY_STATUS_SIGNAL = 24
    DAY_NIGHT_STATUS = 25
    ZONE_ENABLE = 26
    AT_COMMAND = 27
    PPU_MODE = 28
    PPU_COMMAND = 29
    GLOBAL_DELAYS = 30
    RESET = 31
    ADD_NODE_LINK = 32
    DROP_NODE_LINK = 33
    CHECK_TDM_SYNCH = 24

class ParameterType(Enum):
    """
    typedef enum
    {
       PARAM_TYPE_ANALOGUE_VALUE_E,              //0
       PARAM_TYPE_NEIGHBOUR_INFO_E,              //1
       PARAM_TYPE_STATUS_FLAGS_E,                //2
       PARAM_TYPE_DEVICE_COMBINATION_E,          //3
       PARAM_TYPE_ALARM_THRESHOLD_E,             //4
       PARAM_TYPE_PRE_ALARM_THRESHOLD_E,         //5
       PARAM_TYPE_FAULT_THRESHOLD_E,             //6
       PARAM_TYPE_FLASH_RATE_E,                  //7
       PARAM_TYPE_TONE_SELECTION_E,              //8
       PARAM_TYPE_RBU_SERIAL_NUMBER_E,           //9
       PARAM_TYPE_PLUGIN_SERIAL_NUMBER_E,        //10
       PARAM_TYPE_RBU_ENABLE_E,                  //11
       PARAM_TYPE_PLUGIN_ENABLE_E,               //12
       PARAM_TYPE_MODULATION_BANDWIDTH_E,        //13
       PARAM_TYPE_SPREADING_FACTOR_E,            //14
       PARAM_TYPE_FREQUENCY_E,                   //15
       PARAM_TYPE_CODING_RATE_E,                 //16
       PARAM_TYPE_TX_POWER_E,                    //17
       PARAM_TYPE_TEST_MODE_E,                   //18
       PARAM_TYPE_PLUGIN_TEST_MODE_E,            //19
       PARAM_TYPE_FIRMWARE_INFO_E,               //20
       PARAM_TYPE_FIRMWARE_ACTIVE_IMAGE_E,       //21
       PARAM_TYPE_REBOOT_E,                      //22
       PARAM_TYPE_SVI_E,                         //23
       PARAM_TYPE_RBU_DISABLE_E,                 //24
       PARAM_TYPE_TX_POWER_LOW_E,                //25
       PARAM_TYPE_TX_POWER_HIGH_E,               //26
       PARAM_TYPE_OUTPUT_STATE_E,                //27
       PARAM_TYPE_ZONE_E,                        //28
       PARAM_TYPE_CRC_INFORMATION_E,             //29
       PARAM_TYPE_PIR_RESET_E,                   //30
       PARAM_TYPE_EEPROM_INFORMATION_E,          //31
       PARAM_TYPE_FAULT_TYPE_E,                  //32
       PARAM_TYPE_MESH_STATUS_E,                 //33
       PARAM_TYPE_INDICATOR_LED_E,               //34
       PARAM_TYPE_PLUGIN_ID_NUMBER_E,            //35
       PARAM_TYPE_PLUGIN_FIRMWARE_INFO_E,        //36
       PARAM_TYPE_DAY_NIGHT_SETTING_E,           //37
       PARAM_TYPE_ALARM_ACK_E,                   //38
       PARAM_TYPE_EVACUATE_E,                    //39
       PARAM_TYPE_GLOBAL_DELAY_E,                //40
       PARAM_TYPE_DISABLE_OUTPUTS_E,             //41
       PARAM_TYPE_ALARM_RESET_E,                 //42
       PARAM_TYPE_SOUND_LEVEL_E,                 //43
       PARAM_TYPE_TEST_ONE_SHOT_E,               //44
       PARAM_TYPE_CONFIRMED_E,                   //45
       PARAM_TYPE_ALARM_CONFIG_E,                //46
       PARAM_TYPE_ALARM_OPTION_FLAGS_E,          //47
       PARAM_TYPE_ALARM_DELAYS_E,                //48
       PARAM_TYPE_CHANNEL_FLAGS_E,               //49
       PARAM_TYPE_MAX_RANK_E,                    //50
       PARAM_TYPE_200_HOUR_TEST_E,               //51
       PARAM_TYPE_CHECK_FIRMWARE_E,              //52
       PARAM_TYPE_SET_LOCAL_GLOBAL_DELAYS_E,     //53
       PARAM_TYPE_GLOBAL_ALARM_DELAY_VALUES_E,   //54
       PARAM_TYPE_GLOBAL_ALARM_OVERRIDE_E,       //55
       PARAM_TYPE_BATTERY_STATUS_REQUEST_E,      //56
       PARAM_TYPE_GLOBAL_DELAY_COMBINED_E,       //57
       PARAM_TYPE_PLUGIN_DETECTOR_TYPE_E,        //58
       PARAM_TYPE_PLUGIN_TYPE_AND_CLASS_E,       //69
       PARAM_TYPE_SCAN_FOR_DEVICES_E,            //60
       PARAM_TYPE_SENSOR_VALUES_REQUEST_E,       //61
       PARAM_TYPE_DELAYED_OUTPUT_E,              //62
       PARAM_TYPE_BATTERY_TEST_E,                //63
       PARAM_TYPE_PRODUCT_CODE_E,                //64
       PARAM_TYPE_PPU_MODE_ENABLE_E,             //65
       PARAM_TYPE_DEPASSIVATION_SETTINGS_E,      //66
       PARAM_TYPE_ENTER_PPU_MODE_E,              //67
       PARAM_TYPE_MAX_E
    } ParameterType_t;
    """
    ANALOGUE_VALUE = 0
    NEIGHBOUR_INFO = 1
    STATUS_FLAGS = 2
    DEVICE_COMBINATION = 3
    ALARM_THRESHOLD = 4
    PRE_ALARM_THRESHOLD = 5
    FAULT_THRESHOLD = 6
    FLASH_RATE = 7
    TONE_SELECTION = 8
    RBU_SERIAL_NUMBER = 9
    PLUGIN_SERIAL_NUMBER = 10
    RBU_ENABLE = 11
    PLUGIN_ENABLE = 12
    MODULATION_BANDWIDTH = 13
    SPREADING_FACTOR = 14
    FREQUENCY = 15
    CODING_RATE = 16
    TX_POWER = 17
    TEST_MODE = 18
    PLUGIN_TEST_MODE = 19
    FIRMWARE_INFO = 20
    FIRMWARE_ACTIVE_IMAGE = 21
    REBOOT = 22
    SVI = 23
    RBU_DISABLE = 24
    TX_POWER_LOW = 25
    TX_POWER_HIGH = 26
    OUTPUT_STATE = 27
    ZONE = 28
    CRC_INFORMATION = 29
    PIR_RESET = 30
    EEPROM_INFORMATION = 31
    FAULT_TYPE = 32
    MESH_STATUS = 33
    INDICATOR_LED = 34
    PLUGIN_ID_NUMBER = 35
    PLUGIN_FIRMWARE_INFO = 36
    DAY_NIGHT_SETTING = 37
    ALARM_ACK = 38
    EVACUATE = 39
    GLOBAL_DELAY = 40
    DISABLE_OUTPUTS = 41
    ALARM_RESET = 42
    SOUND_LEVEL = 43
    TEST_ONE_SHOT = 44
    ALARM_CONF = 45
    ALARM_CONFIG = 46
    ALARM_OPTION_FLAGS = 47
    ALARM_DELAYS = 48
    CHANNEL_FLAGS = 49
    MAX_RANK = 50
    TWO_HUNDRED_HOUR_TEST = 51
    CHECK_FIRMWARE = 52
    SET_LOCAL_GLOBAL_DELAYS_E = 53
    GLOBAL_ALARM_DELAY_VALUES_E = 54
    GLOBAL_ALARM_OVERRIDE_E = 55
    BATTERY_STATUS_REQUEST_E = 56
    GLOBAL_DELAY_COMBINED_E = 57
    PLUGIN_DETECTOR_TYPE_E = 58
    PLUGIN_TYPE_AND_CLASS_E = 59
    SCAN_FOR_DEVICES_E = 60
    SENSOR_VALUES_REQUEST_E = 61
    DELAYED_OUTPUT_E = 62
    BATTERY_TEST_E = 63
    PRODUCT_CODE_E = 64
    PPU_MODE_ENABLE = 65
    DEPASSIVATION_SETTINGS = 66
    ENTER_PPU_MODE_E = 67

class ResetReason(Enum):
    """
    typedef enum 
    {
       CO_RESET_REASON_UNKNOWN_E,
       CO_RESET_REASON_NO_PARENTS_LEFT_E,
       CO_RESET_REASON_DROPPED_BY_SYNC_NODE_E,
       CO_RESET_REASON_NODE_STOPPED_RESPONDING_E,
       CO_RESET_REASON_MAX_CONNECTION_ATTEMPTS_E,
       CO_RESET_REASON_FAILED_TO_SEND_ROUTE_ADD_E,
       CO_RESET_REASON_LINK_REMAKE_FAILED_E,
       CO_RESET_REASON_FREE_RUNNING_E,
       CO_RESET_REASON_CIRCULAR_PATH_E
       CO_RESET_REASON_RESET_COMMAND_E,
       CO_RESET_REASON_PPU_MODE_REQUESTED_E,
    } ResetReason_t;
    """
    REASON_UNKNOWN = 0
    NO_PARENTS_LEFT = 1
    DROPPED_BY_SYNC_NODE = 2
    NODE_STOPPED_RESPONDING = 3
    MAX_CONNECTION_ATTEMPTS = 4
    FAILED_TO_SEND_ROUTE_ADD = 5
    LINK_REMAKE_FAILED = 6
    FREE_RUNNING = 7
    CIRCULAR_PATH = 8
    RESET_COMMAND = 9
    PPU_MODE_REQUESTED = 10

