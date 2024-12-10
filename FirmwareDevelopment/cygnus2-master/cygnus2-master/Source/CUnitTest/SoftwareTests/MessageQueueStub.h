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
*  File         : MessageQueueStub.h
*
*  Description  : Header for CUnit stub of message queues and message pools
*
* Notes: To test a function that uses a message queue, include this header file.
*        At the beginning of your test, call osMessageCreate.
*        This example is for the MeshQ.
*
*        MeshQ = osMessageCreate(osMessageQ(MeshQ), NULL);
*
*        You will also need the Mesh Pool.  Add this to the test.
*
*        MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
*
*        If you need a semaphore...
*
*        MACSemId = osSemaphoreCreate(osSemaphore(MACSem), 1);
*
*************************************************************************************/

#ifndef MESSAGE_QUEUE_STUB_H
#define MESSAGE_QUEUE_STUB_H

#include <inttypes.h>
#include <time.h>
#include "cmsis_os.h"
#include "CO_Message.h"
#include "DM_SerialPort.h"


#define MESH_POOL_SIZE 10
#define MESH_Q_SIZE    4
#define DCH_Q_SIZE 1
#define RACH_Q_SIZE 10
#define ACK_Q_SIZE 1
#define DLCCH_Q_SIZE 10
#define HEAD_Q_SIZE 1
#define APP_POOL_SIZE 5
#define APP_Q_SIZE    3
#define AT_Q_SIZE    3
#define AT_POOL_SIZE    5
#define OSPOOL_UNUSED 0xff
#define CIE_POOL_SIZE 130

typedef struct test_sem_release
{
   osSemaphoreId sem;
   uint32_t timeout;
   clock_t start_time;
} test_sem_release_t;

extern bool bosMessageGetCalled;
extern bool bSemaphoreReleaseCalled;
extern bool bSemaphoreWaitCalled;
extern bool bThreadInitCalled;
extern bool bThreadInitFail;

extern osPoolId MeshPool;
extern const osPoolDef_t os_pool_def_MeshPool;

extern osPoolId AppPool;
extern const osPoolDef_t os_pool_def_AppPool;

extern osPoolId ATHandlePool;
extern const osPoolDef_t os_pool_def_ATHandlePool;

extern osPoolId CieMsgPool;
extern const osPoolDef_t os_pool_def_CieMsgPool;

extern osMessageQId(AppQ);
extern const osMessageQDef_t os_messageQ_def_AppQ;

extern osMessageQId(ATHandleQ);
extern const osMessageQDef_t os_messageQ_def_ATHandleQ;

extern osMessageQId(MeshQ);
extern const osMessageQDef_t os_messageQ_def_MeshQ;

extern osMessageQId(DCHQ);
extern const osMessageQDef_t os_messageQ_def_DCHQ;

extern osMessageQId(RACHPQ);
extern const osMessageQDef_t os_messageQ_def_RACHPQ;

extern osMessageQId(RACHSQ);
extern const osMessageQDef_t os_messageQ_def_RACHSQ;

extern osMessageQId(ACKQ);
extern const osMessageQDef_t os_messageQ_def_ACKQ;

extern osMessageQId(DLCCHPQ);
extern const osMessageQDef_t os_messageQ_def_DLCCHPQ;

extern osMessageQId(DLCCHSQ);
extern const osMessageQDef_t os_messageQ_def_DLCCHSQ;

extern osMessageQId(HeadQ);
extern const osMessageQDef_t os_messageQ_def_HeadQ;

extern osMessageQId(CieFireQ);
extern const osMessageQDef_t os_messageQ_def_CieFireQ;

extern osMessageQId(CieAlarmQ);
extern const osMessageQDef_t os_messageQ_def_CieAlarmQ;

extern osMessageQId(CieFaultQ);
extern const osMessageQDef_t os_messageQ_def_CieFaultQ;

extern osMessageQId(CieMiscQ);
extern const osMessageQDef_t os_messageQ_def_CieMiscQ;

extern osMessageQId(CieTxBufferQ);
extern const osMessageQDef_t os_messageQ_def_CieTxBufferQ;


extern osSemaphoreId HeadInterfaceSemaphoreId;
extern osSemaphoreDef_t os_semaphore_def_HeadInterfaceSemaphore;

extern osSemaphoreId MACSemId;
extern osSemaphoreDef_t os_semaphore_def_MACSem;

extern osSemaphoreId  GpioSemId;
extern osSemaphoreDef_t os_semaphore_def_GpioSem;


osPoolId osPoolCreate(const osPoolDef_t *pool_def);
uint32_t osPoolSize(const osPoolDef_t* pool_def);
uint32_t osPoolFreeSpace(osPoolId pool_id);
void* osPoolAlloc(osPoolId pool_id);
osStatus osPoolFree(osPoolId pool_id, void *block);
uint32_t osPoolBlocksAllocatedCount(osPoolId pool_id);
uint32_t* osPoolEntry(osPoolId pool_id, uint32_t index);
void osPoolFlush(osPoolId pool_id);

osMessageQId osMessageCreate(const osMessageQDef_t *queue_def, osThreadId thread_id);
osStatus osMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec);
osEvent osMessageGet(osMessageQId queue_id, uint32_t millisec);
uint32_t osMessageCount(osMessageQId queue_id);
uint32_t osMessageQSize(osMessageQId queue_id);
void osMessageQFlush(osMessageQId queue_id);

osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t *semaphore_def, int32_t count);
int32_t osSemaphoreWait(osSemaphoreId semaphore_id, uint32_t timeout_ms);
osStatus osSemaphoreRelease(osSemaphoreId semaphore_id);

void osDelay(uint32_t timeout_ms);

SerialPortOperationResult_t SerialPortWriteTxBuffer(uint8_t uart_port, uint8_t* pData, uint16_t length);

extern bool bSerialDebugPrintToConsole;
extern bool bSerialDebugPrintCalled;
extern int SerialDebugPrintCount;
extern char SerialDebugLastPrint[256];
SerialPortOperationResult_t SerialPortDebugPrint(const char *fmt, ...);

#endif // MESSAGE_QUEUE_STUB_H

