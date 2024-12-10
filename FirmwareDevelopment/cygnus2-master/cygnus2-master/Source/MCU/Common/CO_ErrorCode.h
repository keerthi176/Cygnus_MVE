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
*  File         : CO_ErrorCode.h
*
*  Description  : Error Codes definitions header file
*
*************************************************************************************/

#ifndef CO_ERROR_CODE_H
#define CO_ERROR_CODE_H


/* System Include Files
*************************************************************************************/



/* User Include Files
*************************************************************************************/



/* Public Structures
*************************************************************************************/ 



/* Public Enumerations
*************************************************************************************/
typedef enum
{
   // SUCCESS : (Value = 0)
   SUCCESS_E = 0,                                       // 0000 - General success indicator for functions/operations

   // WARNING CODES : (values 1, 2, 3...)
   START_OF_WARNING_CODES_E = 1,                        // 0001 - WARNING CODES START AT 1
   // SYNC ALGORITHM
   WARNING_NODE_ALREADY_BEING_TRACKED_E,                // 0002 - An attempt was made to add a tracking node that is already being tracked.
   // PARENT SELECTION ALGORITHM
   WARNING_NO_NEIGHBOUR_FOUND_E,                        // 0003 - There is no neighbour in the neighbour table at the requested rank

   // ERROR CODES : (values 1001, 1002, 1003...)
   START_OF_ERROR_CODES_E = 1000,                       // 1000 - ERROR CODES START AT 1000
   // SYNC ALGORITHM ERROR CODES
   ERR_ALL_TRACKING_NODES_ALLOCATED_E,                  // 1001 - An attempt was made to add more tracking nodes than are supported.
   ERR_TRACKING_NODE_NOT_FOUND_E,                       // 1002 - Specified tracking node not found.
   ERR_SLOT_INDEX_OUT_OF_RANGE_E,                       // 1003 - Call to function unsuccessful as the slot index was an invalid value 
   ERR_UNEXPECTED_ERROR_E,                              // 1004 - There should be a free element in SyncData.aTrackingNode, but code failed to find it.
   ERR_NO_TRACKING_NODES_ALLOCATED_E,                   // 1005 - Function called expects there to be active tracking nodes, but there are none active.
   ERR_NEXT_SLOT_INDEX_OUT_OF_RANGE_E,                  // 1006 - The value of the Next Slot Index is too large.

   // CHANNEL HOPPING SEQUENCE GENERATION ERROR CODES
   ERR_FIRST_ADJACENT_HOP_CLASH_E,                      // 1007 - The channel used for hop(n) is too close to that for hop(n-1).
   ERR_SECOND_ADJACENT_HOP_CLASH_E,                     // 1008 - The channel used for hop(n) is the same as hop(n-2).
   ERR_END_SEQ_EXCEEDS_ARRAY_SIZE_E,                    // 1009 - The end sequence could not be generated because the array size is too small.
   ERR_HOP_SEQ_GEN_HAS_NO_AVAILABLE_CHANNELS_E,         // 1010 - The hop sequence generator has determined that there are no available channels.
   ERR_HOP_SEQ_GEN_REACHED_MAX_ITERATIONS_E,            // 1011 - The hopping sequence generator exceeded max permitted iterations.
   ERR_PRBS_SEED_NOT_SET_CORRECTLY_E,                   // 1012 - The structure containing the PRBS seed value does not contain valid values.
   ERR_RAND_NUM_GENERATOR_NOT_SEEDED_E,                 // 1013 - The PRBS random number generator was called without having first seeded it.
   ERR_TOO_FEW_CHANNEL_OPTIONS_E,                       // 1014 - There must be at least three other channels that can be hopped to.
   ERR_TOO_MANY_CHANNEL_SPECIFIED_E,                    // 1015 - The number of channels specified exceeds the s/w constant NUM_HOPPING_CHANS.
   ERR_HOPPING_SEQUENCE_LENGTH_TOO_SHORT_E,             // 1016 - The shortest hopping sequence that has been tested is 16.


   // GENERAL PURPOSE ERROR CODES
   ERR_EEPROM_WRITE_FAIL_E,                             // 1017 - An attempt to write to EEPROM failed.
   ERR_INVALID_POINTER_E,                               // 1018 - A pointer could not be used e.g. it was passed as NULL to a function
   ERR_OUT_OF_RANGE_E,                                  // 1019 - A property value falls outside the permissable range
   ERR_FAILED_TO_CREATE_THREAD_E,                       // 1020 - Thread creation failed
   ERR_FAILED_TO_CREATE_QUEUE_E,                        // 1021 - Queue creation failed
   ERR_NO_RESOURCE_E,                                   // 1022 - A requested resource was not available e.g. mempool was full
   ERR_MESSAGE_FAIL_E,                                  // 1023 - A message was not processed to completion
   ERR_INIT_FAIL_E,                                     // 1024 - Initialisation of a module failed
   ERR_DATA_INTEGRITY_CHECK_E,                          // 1025 - Data cannot be trusted e.g. failed CRC
   ERR_TIMEOUT_E,                                       // 1026 - A timeout occurred
   ERR_MESSAGE_TYPE_UNKNOWN_E,                          // 1027 - An unknown message type was encountered
   ERR_BUILT_IN_TEST_FAIL_E,                            // 1028 - BIT test failed.
   ERR_MESSAGE_DUPLICATE_E,                             // 1029 - Message not sent because it is a duplicate on one already queued
   ERR_NOT_FOUND_E,                                     // 1030 - A requested value was not found.
   ERR_SYSTEM_NOT_ACTIVE_E,                             // 1031 - Command blocked because the network is not in ACTIVE state
   ERR_INVALID_PARAMETER_E,                             // 1032 - A function parameter could not be used e.g. null pointer
   ERR_NOT_SUPPORTED_E,                                 // 1033 - The request failed because it is not supported e.g. sounder command if no sounder is fitted.
   ERR_QUEUE_OVERFLOW_E,                                // 1034 - An attempt to add a message to a queue failed because to queue was full
   ERR_NO_MEMORY_E,                                     // 1035 - Memory pool request failed because all elements were occupied
   ERR_CONFIG_DISABLED_E,                               // 1036 - A command failed because the target has been disabled i.e. zone, device or channel disabled
   ERR_OPERATION_FAIL_E,                                // 1037 - An operation did not run to completion due to an error in processing
   ERR_BUSY_E,                                          // 1038 - A module was unavailable because it was processing another operation 
   ERR_NO_SYNC_LOCK_E,                                  // 1039 - Could not set a new sync node because there was no record of it
   ERR_FAILED_TO_CREATE_MEMPOOL_E,                      // 1040 - Memory Pool creation failed
   ERR_SIGNAL_LOST_E,                                   // 1041 - Mesh forming and healing, node heartbeat signal lost
   ERR_ACCESS_DENIED_E                                  // 1042 - A resource is protected from access

} ErrorCode_t;



/* Public Functions Prototypes
*************************************************************************************/



/* Public Constants
*************************************************************************************/



/* Macros
*************************************************************************************/



#endif // CO_ERROR_CODE_H
