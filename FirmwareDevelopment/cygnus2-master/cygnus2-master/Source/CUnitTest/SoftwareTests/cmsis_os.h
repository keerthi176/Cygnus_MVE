
#ifndef CMSIS_OS
#define CMSIS_OS

#include <stdint.h>


#define osWaitForever     0xFFFFFFFFU    ///< wait forever timeout value

/// Status code values returned by CMSIS-RTOS functions.
typedef enum {
	osOK = 0,       ///< function completed; no error or event occurred.
	osEventSignal = 0x08,       ///< function completed; signal event occurred.
	osEventMessage = 0x10,       ///< function completed; message event occurred.
	osEventMail = 0x20,       ///< function completed; mail event occurred.
	osEventTimeout = 0x40,       ///< function completed; timeout occurred.
	osErrorParameter = 0x80,       ///< parameter error: a mandatory parameter was missing or specified an incorrect object.
	osErrorResource = 0x81,       ///< resource not available: a specified resource was not available.
	osErrorTimeoutResource = 0xC1,       ///< resource not available within given time: a specified resource was not available within the timeout period.
	osErrorISR = 0x82,       ///< not allowed in ISR context: the function cannot be called from interrupt service routines.
	osErrorISRRecursive = 0x83,       ///< function called multiple times from ISR with same object.
	osErrorPriority = 0x84,       ///< system cannot determine priority or thread has illegal priority.
	osErrorNoMemory = 0x85,       ///< system is out of memory: it was impossible to allocate or reserve memory for the operation.
	osErrorValue = 0x86,       ///< value of a parameter is out of range.
	osErrorOS = 0xFF,       ///< unspecified RTOS error: run-time error but no other error message fits.
	os_status_reserved = 0x7FFFFFFF  ///< prevent from enum down-size compiler optimization.
} osStatus;




/// Entry point of a thread.
typedef void(*os_pthread) (void const *argument);
/// Priority used for thread control.
typedef enum {
	osPriorityIdle = -3,          ///< priority: idle (lowest)
	osPriorityLow = -2,          ///< priority: low
	osPriorityBelowNormal = -1,          ///< priority: below normal
	osPriorityNormal = 0,          ///< priority: normal (default)
	osPriorityAboveNormal = +1,          ///< priority: above normal
	osPriorityHigh = +2,          ///< priority: high
	osPriorityRealtime = +3,          ///< priority: realtime (highest)
	osPriorityError = 0x84,       ///< system cannot determine priority or thread has illegal priority
	os_priority_reserved = 0x7FFFFFFF  ///< prevent from enum down-size compiler optimization.
} osPriority;

/// Thread Definition structure contains startup information of a thread.
typedef struct os_thread_def {
	os_pthread               pthread;    ///< start address of thread function
	osPriority             tpriority;    ///< initial thread priority
	uint32_t               instances;    ///< maximum number of instances of that thread function
	uint32_t               stacksize;    ///< stack size requirements in bytes; 0 is default stack size
} osThreadDef_t;

typedef struct osThreadDef_t *osThreadId;

/// Definition structure for memory block allocation.
typedef struct os_pool_def {
	uint32_t                 pool_sz;    ///< number of items (elements) in the pool
	uint32_t                 item_sz;    ///< size of an item
	void                       *pool;    ///< pointer to memory for pool
} osPoolDef_t;

/// Definition structure for message queue.
typedef struct os_messageQ_def {
	uint32_t                queue_sz;    ///< number of elements in the queue
	void                       *pool;    ///< memory array for messages
} osMessageQDef_t;

#define osMessageQDef(name, queue_sz, type)   \
uint32_t os_messageQ_q_##name[4+(queue_sz)] = { 0 }; \
const osMessageQDef_t os_messageQ_def_##name = \
{ (queue_sz), (os_messageQ_q_##name) }

#define osMessageQ(name) &os_messageQ_def_##name

typedef struct osMessageQDef_t *osMessageQId;

#define osPoolDef(name, no, type)   \
uint32_t os_pool_m_##name[3+((sizeof(type)+3)/4)*(no)]; \
const osPoolDef_t os_pool_def_##name = \
{ (no), sizeof(type), (os_pool_m_##name) }

#define osPool(name) \
&os_pool_def_##name

typedef struct osPoolDef_t *osPoolId;

/// Mutex Definition structure contains setup information for a mutex.
typedef struct os_mutex_def {
	void                      *mutex;    ///< pointer to internal data
} osMutexDef_t;

/// Semaphore Definition structure contains setup information for a semaphore.
typedef struct os_semaphore_def {
	void                  *semaphore;    ///< pointer to internal data
} osSemaphoreDef_t;

/// Mutex ID identifies the mutex (pointer to a mutex control block).
typedef struct osMutexDef_t *osMutexId;

typedef struct osSemaphoreDef_t *osSemaphoreId;

#define osSemaphoreDef(name)  \
uint32_t os_semaphore_cb_##name[2] = { 0 }; \
osSemaphoreDef_t os_semaphore_def_##name = { (os_semaphore_cb_##name) }

#define osSemaphore(name)  &os_semaphore_def_##name

#define osMutexDef(name)  \
uint32_t os_mutex_cb_##name[4] = { 0 }; \
const osMutexDef_t os_mutex_def_##name = { (os_mutex_cb_##name) }

/// Access a Mutex definition.
/// \param         name          name of the mutex object.
#define osMutex(name)  \
&os_mutex_def_##name

typedef void *osMailQId;

/// Event structure contains detailed information about an event.
typedef struct {
	osStatus                 status;     ///< status code: event or error information
	union {
		uint32_t                    v;     ///< message as 32-bit value
		void                       *p;     ///< message or mail as void pointer
		int32_t               signals;     ///< signal flags
	} value;                             ///< event value
	union {
		osMailQId             mail_id;     ///< mail id obtained by \ref osMailCreate
		osMessageQId       message_id;     ///< message id obtained by \ref osMessageCreate
	} def;                               ///< event definition
} osEvent;

#define osKernelSysTickMicroSec(microsec)  microsec

#define osThreadDef(name, priority, instances, stacksz)  \
const osThreadDef_t os_thread_def_##name = \
{ (name), (priority), (instances), (stacksz)  }

#define osThread(name)  \
&os_thread_def_##name

uint32_t osKernelSysTick(void);
void* osPoolAlloc(osPoolId pool_id);
osMessageQId osMessageCreate(const osMessageQDef_t *queue_def, osThreadId thread_id);
osStatus osMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec);
osEvent osMessageGet(osMessageQId queue_id, uint32_t millisec);
osThreadId osThreadCreate(const osThreadDef_t *thread_def, void *argument);
osStatus osSemaphoreRelease(osSemaphoreId semaphore_id);
osStatus osPoolFree(osPoolId pool_id, void *block);
void *osPoolCAlloc(osPoolId pool_id);
osMutexId osMutexCreate(const osMutexDef_t *mutex_def);
void osDelay(uint32_t timeout_ms);
osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec);
osStatus osMutexRelease(osMutexId mutex_id);
osPoolId osPoolCreate(const osPoolDef_t *pool_def);
#endif