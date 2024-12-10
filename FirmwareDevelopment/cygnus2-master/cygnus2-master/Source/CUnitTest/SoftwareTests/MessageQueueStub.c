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
* Notes: To test a function that uses a message queue, place the following lines 
*        in the first unit test, Then use the mempool/queue as you do
*        in the project code.  This example is for the MeshQ.
*
*        MeshQ = osMessageCreate(osMessageQ(MeshQ), NULL);
*        MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
*
*************************************************************************************/
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "MessageQueueStub.h"
#include "CO_SerialDebug.h"


bool bosMessageGetCalled = false;
bool bSemaphoreReleaseCalled = false;
bool bSemaphoreWaitCalled = false;
bool bThreadInitCalled = true;
bool bThreadInitFail = false;



osMessageQDef(MeshQ, MESH_Q_SIZE, &CO_Message_t);
osMessageQId(MeshQ);

osMessageQDef(DCHQ, DCH_Q_SIZE, uint32_t);
osMessageQId(DCHQ);

osMessageQDef(RACHPQ, RACH_Q_SIZE, uint32_t);
osMessageQId(RACHPQ);

osMessageQDef(RACHSQ, RACH_Q_SIZE, uint32_t);
osMessageQId(RACHSQ);

osMessageQDef(ACKQ, RACH_Q_SIZE, uint32_t);
osMessageQId(ACKQ);

osMessageQDef(DLCCHQ, DLCCH_Q_SIZE, uint32_t);
osMessageQId(DLCCHQ);

osMessageQDef(DLCCHPQ, DLCCH_Q_SIZE, uint32_t);
osMessageQId(DLCCHPQ);

osMessageQDef(DLCCHSQ, DLCCH_Q_SIZE, uint32_t);
osMessageQId(DLCCHSQ);

osMessageQDef(HeadQ, HEAD_Q_SIZE, uint32_t);
osMessageQId(HeadQ);

osMessageQDef(AppQ, APP_Q_SIZE, CO_Message_t);
osMessageQId(AppQ);

osMessageQDef(ATHandleQ, AT_Q_SIZE, CO_Message_t);
osMessageQId(ATHandleQ);

osPoolDef(AppPool, APP_POOL_SIZE, CO_Message_t);
osPoolId AppPool;

osPoolDef(MeshPool, MESH_POOL_SIZE, CO_Message_t);
osPoolId(MeshPool);

osPoolDef(ATHandlePool, AT_POOL_SIZE, CO_Message_t);
osPoolId ATHandlePool;

#define osPool(name) &os_pool_def_##name

osSemaphoreDef(HeadInterfaceSemaphore);
osSemaphoreId HeadInterfaceSemaphoreId;

osSemaphoreDef(MACSem);
osSemaphoreId MACSemId;

osSemaphoreDef(GpioSem);
osSemaphoreId GpioSemId;





/*************************************************************************************/
/**
* osPoolCreate
* Create a memory pool.
* Usage example: MeshPool = osPoolCreate((const osPoolDef_t *)osPool(MeshPool));
*
* @param pool_def    The name of the pool to create.
*
* @return - osPoolId The pool ID
*/
osPoolId osPoolCreate(const osPoolDef_t *pool_def)
{
   osPoolId pId = NULL;
   if (pool_def)
   {
      if (pool_def->pool)
      {
         pId = (osPoolId)pool_def;
         for (uint32_t index = 0; index < pool_def->pool_sz; index++)
         {
            uint8_t* pSlot = pool_def->pool;
            pSlot += ((pool_def->item_sz) * index);
            *pSlot = OSPOOL_UNUSED;
         }

      }
   }
   return pId;
}

/*************************************************************************************/
/**
* osPoolEntry
* Test utility function.
* Returns a pointer to a slot in the memory pool so that your unit test can examine
* the contents.
* Usage example: pMessage = (CO_Message_t*)osPoolEntry(MeshPool, 3);
*
* @param pool_id    The name of the memory pool.
* @param index      The index of the memory pool slot that you want (zero based).
*
* @return -          Pointer to the memory slot.
*/
uint32_t* osPoolEntry(osPoolId pool_id, uint32_t index)
{
   uint32_t* pReturn = NULL;
   osPoolDef_t* pool_def = (osPoolDef_t*)pool_id;
   if (pool_def)
   {
      if (pool_def->pool)
      {
         if (index < pool_def->pool_sz)
         {
            uint8_t* pSlot = pool_def->pool;
            pSlot += ((pool_def->item_sz) * index);
            pReturn = (uint32_t*)pSlot;
         }
      }
   }
   return pReturn;
}

/*************************************************************************************/
/**
* osPoolSize
* Test utility function.
* Returns the size of the memory pool in allocation units.
* Usage example: uint32_t poolSize = osPoolSize(osPool(MeshPool));
*
* @param pool_def   The memory pool instance.
*
* @return -         The total number of allocation units in the pool.
*/
uint32_t osPoolSize(const osPoolDef_t* pool_def)
{
   uint32_t poolSize = 0;

   if (pool_def)
   {
      if (pool_def->pool)
      {
         poolSize = pool_def->pool_sz;
      }
   }
   return poolSize;
}

/*************************************************************************************/
/**
* osPoolFreeSpace
* Test utility function.
* Returns the number of unsused allocation units in the memory pool.
* Usage example: uint32_t poolSpace = osPoolFreeSpace(MeshPool);
*
* @param pool_def   The memory pool ID.
*
* @return -         The number of unsused allocation units in the pool.
*/
uint32_t osPoolFreeSpace(osPoolId pool_id)
{
   uint32_t poolSpace = 0;
   osPoolDef_t* pool_def = (osPoolDef_t*)pool_id;
   if (pool_def)
   {
      if (pool_def->pool)
      {
         for (uint32_t index = 0; index < pool_def->pool_sz; index++)
         {
            uint8_t* pSlot = pool_def->pool;
            pSlot += ((pool_def->item_sz) * index);
            if (*pSlot == OSPOOL_UNUSED)
            {
               poolSpace++;
            }
         }
      }
   }
   return poolSpace;
}

/*************************************************************************************/
/**
* osPoolBlocksAllocatedCount
* Test utility function.
* Returns the number of used allocation units in the memory pool.
* Usage example: uint32_t count = osPoolBlocksAllocatedCount(MeshPool);
*
* @param pool_def   The memory pool ID.
*
* @return -         The number of used allocation units in the pool.
*/
uint32_t osPoolBlocksAllocatedCount(osPoolId pool_id)
{
   uint32_t block_count = 0;
   osPoolDef_t* pool_def = (osPoolDef_t*)pool_id;
   if (pool_def)
   {
      if (pool_def->pool)
      {
         for (uint32_t index = 0; index < pool_def->pool_sz; index++)
         {
            uint8_t* pSlot = pool_def->pool;
            pSlot += ((pool_def->item_sz) * index);
            if (*pSlot != OSPOOL_UNUSED)
            {
               block_count++;
            }
         }
      }
   }
   return block_count;
}

/*************************************************************************************/
/**
* osPoolAlloc
* Allocates a slot in the memory pool and returns a pointer to it.
* Usage example:  CO_Message_t* pFireSignalReq = osPoolAlloc(MeshPool);
*
* @param pool_def   The memory pool ID.
*
* @return -         A pointer to the allocated memory slot, or NULL if the pool is full.
*/
void* osPoolAlloc(osPoolId pool_id)
{
   void* allocation_slot = NULL;
   if (pool_id)
   {
      osPoolDef_t* pool_def = (osPoolDef_t*)pool_id;
      for (uint32_t index = 0; index < pool_def->pool_sz; index++)
      {
         uint8_t* pSlot = pool_def->pool;
         pSlot += ((pool_def->item_sz) * index);
         if (OSPOOL_UNUSED == *pSlot)
         {
            allocation_slot = pSlot;
            *pSlot = 0x00;
            break;
         }
      }
   }
   return allocation_slot;
}

/*************************************************************************************/
/**
* osPoolCAlloc
* Allocates a slot in the memory pool, zeros the block and returns a pointer to it.
* Usage example:  CO_Message_t* pFireSignalReq = osPoolCAlloc(MeshPool);
*
* @param pool_def   The memory pool ID.
*
* @return -         A pointer to the allocated memory slot, or NULL if the pool is full.
*/
void* osPoolCAlloc(osPoolId pool_id)
{
   void* allocation_slot = NULL;
   if (pool_id)
   {
      osPoolDef_t* pool_def = (osPoolDef_t*)pool_id;
      for (uint32_t index = 0; index < pool_def->pool_sz; index++)
      {
         uint8_t* pSlot = pool_def->pool;
         pSlot += ((pool_def->item_sz) * index);
         if (OSPOOL_UNUSED == *pSlot)
         {
            allocation_slot = pSlot;
            memset(pSlot, 0, pool_def->item_sz);
            break;
         }
      }
   }
   return allocation_slot;
}

/*************************************************************************************/
/**
* osPoolFree
* Frees a used slot in the memory pool..
* Usage example:  osStatus status = osPoolFree(MeshPool, pFireSignalReq);
*
* @param pool_def   The memory pool ID.
* @param block      The address of the slot to free ( use the pointer returned by osPoolAlloc(pool_id) ).
*
* @return -         osOK on success.  osErrorResource on failure.
*/
osStatus osPoolFree(osPoolId pool_id, void *block)
{
   osStatus status = osErrorResource;
   osPoolDef_t* pool_def = (osPoolDef_t*)pool_id;
   uint8_t* blockAddress = block;
   for (uint32_t index = 0; index < pool_def->pool_sz; index++)
   {
      uint8_t* pSlot = pool_def->pool;
      pSlot += ((pool_def->item_sz) * index);
      if (blockAddress == pSlot)
      {
         *pSlot = OSPOOL_UNUSED;
         status = osOK;
         break;
      }
   }
   return status;
}

/*************************************************************************************/
/**
* osPoolFlush
* Empties the memory pool.
* Usage example:  osPoolFlush(MeshPool);
*
* @param pool_def   The memory pool ID.
*
* @return -         None.
*/
void osPoolFlush(osPoolId pool_id)
{
   osPoolDef_t* pool_def = (osPoolDef_t*)pool_id;
   if (pool_def)
   {
      for (uint32_t index = 0; index < pool_def->pool_sz; index++)
      {
         uint8_t* pSlot = pool_def->pool;
         pSlot += ((pool_def->item_sz) * index);
         *pSlot = OSPOOL_UNUSED;
      }
   }
}

/*************************************************************************************/
/**
* osMessagePut
* Adds a message to a message queue.
* Usage example:  osStatus osStat = osMessagePut(MeshQ, (uint32_t)pFireSignalReq, 0);
*
* @param queue_id    The message queue ID.
* @param info        The address of the message to be added.
* @param millisec    Not used.  Retained for cmis compatability.
*
* @return -         osOK on success.  osErrorResource on failure.
*/
osStatus osMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
   (void)millisec;
   osStatus status = osErrorResource;
   osMessageQDef_t* queue_def = (osMessageQDef_t*)queue_id;
   if (queue_def)
   {
      for (uint32_t index = 0; index < queue_def->queue_sz; index++)
      {
         uint32_t* pSlot = (void*)queue_def->pool;
         pSlot += index;
         if (0 == *pSlot)
         {
            osEvent* pEvent = (void*)malloc(sizeof(osEvent));
            if (pEvent)
            {
               pEvent->status = osEventMessage;
               pEvent->value.p = (void*)info;
               *pSlot = (uint32_t)pEvent;
               status = osOK;
               break;
            }
         }
      }
   }
   return status;
}

/*************************************************************************************/
/**
* osMessageGet
* Reads a message from a message queue.
* Usage example:  osEvent event = osMessageGet(MeshQ, timeout);
*
* @param queue_id    The message queue ID.
* @param millisec    Timeout, in milliseconds, to wait for a message to arrive.
*
* @return -          An osEvent object containing the message.
*                    The event.status attribute is set to osEventMessage if a message was
*                    retrieved,  osErrorParameter if the supplied queue_id was not defined
*                    or  osEventTimeout if no message was found.
*/
osEvent osMessageGet(osMessageQId queue_id, uint32_t millisec)
{
   static osEvent event;
   osMessageQDef_t* queue_def = (osMessageQDef_t*)queue_id;

   event.status = osErrorParameter;

   bosMessageGetCalled = true;

   clock_t start = clock();
   clock_t timeout = start + millisec;

   if (queue_def)
   {
      bool bMessageFound = false;
      do
      {
         uint32_t pool = *(uint32_t*)queue_def->pool;
         osEvent* ePtr = (osEvent*)pool;
         if (ePtr)
         {
            if (osEventMessage == ePtr->status)
            {
               event.status = ePtr->status;
               event.value.p = ePtr->value.p;
               uint32_t* pMsg = queue_def->pool;
               uint32_t* pNextMsg = pMsg + 1;

               free((void*)*pMsg);

               uint32_t count = 1;
               while (count < queue_def->queue_sz)
               {
                  *pMsg = *pNextMsg;
                  pMsg++;
                  pNextMsg++;
                  count++;
               }
               *pMsg = 0;
               bMessageFound = true;
            }
         }
      } while (!bMessageFound && timeout > clock());

      if (!bMessageFound)
      {
         event.status = osEventTimeout;
      }
   }
   return event;
}

/*************************************************************************************/
/**
* osMessageCreate
* Creates and initialises a new message queue.
* Usage example:  MeshQ = osMessageCreate(osMessageQ(MeshQ), NULL);
*
* @param queue_def   The message queue definition.
* @param thread_id   Not used.  Retained for cmis compatability.
*
* @return -         The message queue ID.
*/
osMessageQId osMessageCreate(const osMessageQDef_t *queue_def, osThreadId thread_id)
{
   (void)thread_id;

   osMessageQId qId = NULL;
   if (queue_def)
   {
      if (queue_def->pool)
      {
         qId = (osMessageQId)queue_def;

         uint32_t* pSlot = queue_def->pool;
         for (uint32_t index = 0; index < queue_def->queue_sz; index++)
         {
            if (pSlot != 0x00)
            {
               *pSlot = 0x00;
            }
         }
      }
   }
   return qId;
}

/*************************************************************************************/
/**
* osMessageQSize
* Test utility function.
* Returns the total queue buffer size in terms of number of messages.
* Usage example:  uint32_t queueSize = osMessageQSize(MeshQ);
*
* @param queue_id    The message queue ID.
*
* @return -          The size of the queue.
*/
uint32_t osMessageQSize(osMessageQId queue_id)
{
   osMessageQDef_t* queue_def = (osMessageQDef_t*)queue_id;
   uint32_t qSize = 0;
   if (queue_def)
   {
      if (queue_def->pool)
      {
         qSize = queue_def->queue_sz;
      }
   }
   return qSize;
}

/*************************************************************************************/
/**
* osMessageCount
* Test utility function.
* Returns the total number of messages waiting in the queue.
* Usage example:  uint32_t messageCount = osMessageCount(MeshQ);
*
* @param queue_id    The message queue ID.
*
* @return -          The number of queued messages.
*/
uint32_t osMessageCount(osMessageQId queue_id)
{
   osMessageQDef_t* queue_def = (osMessageQDef_t*)queue_id;
   uint32_t messageCount = 0;
   uint32_t* pSlot = queue_def->pool;
   for (uint32_t index = 0; index < queue_def->queue_sz; index++)
   {
      if (*pSlot != 0x00)
      {
         messageCount++;
      }
      pSlot++;
   }
   return messageCount;
}

/*************************************************************************************/
/**
* osMessageQFlush
* Test utility function.
* Discards all messages waiting in the queue.
* Usage example:  osMessageFlush(MeshQ);
*
* @param queue_id    The message queue ID.
*
* @return -          None.
*/
void osMessageQFlush(osMessageQId queue_id)
{
   osMessageQDef_t* queue_def = (osMessageQDef_t*)queue_id;
   if (queue_def)
   {
      uint32_t* pSlot = queue_def->pool;
      for (uint32_t index = 0; index < queue_def->queue_sz; index++)
      {
         if (pSlot != 0x00)
         {
            free((void*)*pSlot);
            *pSlot = 0x00;
         }
         pSlot++;
      }
   }
}



osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t *semaphore_def, int32_t count)
{
   osSemaphoreId sid = NULL;
   if (semaphore_def)
   {
      sid = (osSemaphoreId)semaphore_def;
   }

   return sid;
}

/*************************************************************************************/
/**
* osSemaphoreRelease
* Releases the specified semaphore.
* Usage example:  osSemaphoreRelease(HeadInterfaceSemaphoreId);
*
* @param semaphore_id    The semaphore ID.
*
* @return -              osOK on success, osErrorResource if the semaphore is undefined.
*/
osStatus osSemaphoreRelease(osSemaphoreId semaphore_id)
{
   bSemaphoreReleaseCalled = true;
   osStatus status = osErrorResource;
   osSemaphoreDef_t* semaphore_def = (osSemaphoreDef_t*)semaphore_id;

   if (semaphore_def)
   {
      uint32_t* pSem = (uint32_t*)semaphore_def->semaphore;
      if (pSem)
      {
         *pSem = 1;
         status = osOK;
      }
   }

   return status;
}

int32_t osSemaphoreWait(osSemaphoreId semaphore_id, uint32_t timeout_ms)
{
   bSemaphoreWaitCalled = true;
   int32_t status = -1;
   osSemaphoreDef_t* semaphore_def = (osSemaphoreDef_t*)semaphore_id;


   clock_t start = clock();
   clock_t timeout = start + timeout_ms;

   if (semaphore_def)
   {
      uint32_t* pSem = (uint32_t*)semaphore_def->semaphore;
      if (pSem)
      {
         *pSem = 0;
         while (*pSem == 0 && (timeout_ms == osWaitForever || timeout > clock()))
         {
         }
         status = (int32_t)*pSem;
      }
   }

   return status;
}

osThreadId osThreadCreate(const osThreadDef_t *thread_def, void *argument)
{
   bThreadInitCalled = true;
   if (bThreadInitFail)
   {
      return (osThreadId)0;
   }
   return (osThreadId)1;
}


void osDelay(uint32_t timeout_ms)
{
   clock_t start = clock();
   clock_t timeout = start + timeout_ms;

   while (clock() < timeout)
   {
   };
}

/*************************************************************************************
* Serial Port Stubs
*/
bool bSerialDebugPrintToConsole = false; //Set this to 'true' in your test to have SerialDebugPrint write to the console.
bool bSerialDebugPrintCalled = false;
int SerialDebugPrintCount = 0;
char SerialDebugLastPrint[256];
SerialPortOperationResult_t SerialDebug_Print(debug_category_t cat, const char *fmt, ...)
{
   bSerialDebugPrintCalled = true;
   SerialDebugPrintCount++;

   SerialPortOperationResult_t result = UART_SUCCESS;
   va_list argp;
   switch (cat)
   {
   case DBG_SYSTEM:
	   strcpy(SerialDebugLastPrint, "+SYS:");
	   break;
   case DBG_INFO:
	   strcpy(SerialDebugLastPrint, "+INF:");
	   break;
   case DBG_BIT:
	   strcpy(SerialDebugLastPrint, "+BIT:");
	   break;
   case DBG_ERROR:
	   strcpy(SerialDebugLastPrint, "+ERR:");
	   break;
   default:
	   /* We should never be here */
	   strcpy(SerialDebugLastPrint, "");
	   break;
   }
   va_start(argp, fmt);
   int msg_length = vsprintf_s(&SerialDebugLastPrint[strlen(SerialDebugLastPrint)], sizeof(SerialDebugLastPrint), fmt, argp);
   if (0 < msg_length  && bSerialDebugPrintToConsole)
   {
      printf("%s", SerialDebugLastPrint);
   }
   va_end(argp);


   return UART_SUCCESS;
}

