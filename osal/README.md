# Operating System Abstraction Layer (OSAL)

The Operating System Abstraction Layer (OSAL) provides a unified, platform-independent interface for operating system primitives. This enables the same application code to work seamlessly across different RTOS implementations, bare-metal systems, or even desktop operating systems.

## Overview

The OSAL layer abstracts common operating system concepts, providing:

- **RTOS Independence**: Write once, run on any supported RTOS
- **Unified API**: Consistent interface across all OS primitives
- **Weak Function Implementation**: Easy RTOS-specific overrides
- **ISR Safety**: Interrupt-safe variants for critical operations
- **Timeout Support**: Configurable timeouts for blocking operations
- **Comprehensive Error Handling**: Detailed status reporting

## Architecture

```
┌─────────────────────────────────────────┐
│         Application / Services          │
├─────────────────────────────────────────┤
│             OSAL Interface              │
│   (Tasks, Queues, Mutexes, etc.)        │
├─────────────────────────────────────────┤
│           Weak Implementations          │
│         (Default/Stub Functions)        │
├─────────────────────────────────────────┤
│          RTOS Implementations           │
│    (FreeRTOS, ThreadX, Zephyr, etc.)    │
├─────────────────────────────────────────┤
│            Operating System             │
└─────────────────────────────────────────┘
```

## Supported Primitives

### Tasks
- **Location**: `osal/task.h`, `osal/task.c`
- **Features**: Create, delete, suspend, resume, delay, priority management

### Queues
- **Location**: `osal/queue.h`, `osal/queue.c`
- **Features**: FIFO message passing, ISR support, blocking/non-blocking operations

### Mutexes
- **Location**: `osal/mutex.h`, `osal/mutex.c`
- **Features**: Mutual exclusion, recursive mutexes, priority inheritance

### Semaphores
- **Location**: `osal/semaphore.h`, `osal/semaphore.c`
- **Features**: Binary and counting semaphores, ISR support

## Directory Structure

```
osal/
├── CMakeLists.txt              # OSAL main build configuration
├── README.md                   # This file
├── osal.h                      # Main OSAL interface
├── osal.c                      # OSAL initialization
├── task.h                      # Task management interface
├── task.c                      # Weak task implementations
├── queue.h                     # Queue management interface
├── queue.c                     # Weak queue implementations
├── mutex.h                     # Mutex management interface
├── mutex.c                     # Weak mutex implementations
├── semaphore.h                 # Semaphore management interface
└── semaphore.c                 # Weak semaphore implementations
```

## Core Types and Constants

### Common Types

```c
// Handle types (platform-specific implementations)
typedef void* OSAL_TaskHandle_t;
typedef void* OSAL_QueueHandle_t;
typedef void* OSAL_MutexHandle_t;
typedef void* OSAL_SemaphoreHandle_t;

// Function pointer types
typedef void (*OSAL_TaskFunction_t)(void *parameters);

// Timeout constants
#define OSAL_WAIT_FOREVER    0xFFFFFFFF
#define OSAL_NO_WAIT         0
```

### Task Priorities

```c
typedef enum {
    OSAL_TASK_PRIORITY_IDLE = 0,
    OSAL_TASK_PRIORITY_LOW = 1,
    OSAL_TASK_PRIORITY_NORMAL = 2,
    OSAL_TASK_PRIORITY_HIGH = 3,
    OSAL_TASK_PRIORITY_CRITICAL = 4
} OSAL_TaskPriority_t;
```

### Task States

```c
typedef enum {
    OSAL_TASK_STATE_READY,
    OSAL_TASK_STATE_RUNNING,
    OSAL_TASK_STATE_BLOCKED,
    OSAL_TASK_STATE_SUSPENDED,
    OSAL_TASK_STATE_DELETED
} OSAL_TaskState_t;
```

## Task Management

### Header File: `osal/task.h`

```c
#include "osal/task.h"

// Create a new task
SppRetVal_t OSAL_TaskCreate(OSAL_TaskHandle_t *task,
                        const char *name,
                        OSAL_TaskFunction_t function,
                        void *parameters,
                        OSAL_TaskPriority_t priority);

// Delete a task
SppRetVal_t OSAL_TaskDelete(OSAL_TaskHandle_t task);

// Suspend a task
SppRetVal_t OSAL_TaskSuspend(OSAL_TaskHandle_t task);

// Resume a task
SppRetVal_t OSAL_TaskResume(OSAL_TaskHandle_t task);

// Delay current task
SppRetVal_t OSAL_TaskDelay(uint32_t milliseconds);

// Yield CPU to other tasks
SppRetVal_t OSAL_TaskYield(void);

// Get current task handle
OSAL_TaskHandle_t OSAL_TaskGetCurrent(void);

// Get task state
OSAL_TaskState_t OSAL_TaskGetState(OSAL_TaskHandle_t task);
```

### Usage Example

```c
void my_task_function(void *parameters) {
    int *counter = (int*)parameters;
    
    while (1) {
        (*counter)++;
        printf("Task counter: %d\n", *counter);
        
        // Delay for 1 second
        OSAL_TaskDelay(1000);
    }
}

int main() {
    OSAL_TaskHandle_t task_handle;
    int counter = 0;
    
    // Initialize OSAL
    OSAL_Init();
    
    // Create task
    SppRetVal_t result = OSAL_TaskCreate(&task_handle,
                                     "MyTask",
                                     my_task_function,
                                     &counter,
                                     OSAL_TASK_PRIORITY_NORMAL);
    
    if (result != SPP_OK) {
        printf("Failed to create task: %d\n", result);
        return -1;
    }
    
    // Start scheduler (platform-specific)
    // For FreeRTOS: vTaskStartScheduler();
    
    return 0;
}
```

## Queue Management

### Header File: `osal/queue.h`

```c
#include "osal/queue.h"

// Create a queue
SppRetVal_t OSAL_QueueCreate(OSAL_QueueHandle_t *queue,
                         uint32_t max_items,
                         uint32_t item_size);

// Delete a queue
SppRetVal_t OSAL_QueueDelete(OSAL_QueueHandle_t queue);

// Send item to queue
SppRetVal_t OSAL_QueueSend(OSAL_QueueHandle_t queue,
                       const void *item,
                       uint32_t timeout);

// Send item to queue from ISR
SppRetVal_t OSAL_QueueSendFromISR(OSAL_QueueHandle_t queue,
                              const void *item);

// Receive item from queue
SppRetVal_t OSAL_QueueReceive(OSAL_QueueHandle_t queue,
                          void *item,
                          uint32_t timeout);

// Receive item from queue from ISR
SppRetVal_t OSAL_QueueReceiveFromISR(OSAL_QueueHandle_t queue,
                                 void *item);

// Peek at queue item without removing
SppRetVal_t OSAL_QueuePeek(OSAL_QueueHandle_t queue,
                       void *item,
                       uint32_t timeout);

// Get number of items in queue
uint32_t OSAL_QueueGetCount(OSAL_QueueHandle_t queue);
```

### Usage Example

```c
typedef struct {
    uint32_t id;
    uint8_t data[16];
} message_t;

void producer_task(void *parameters) {
    OSAL_QueueHandle_t *queue = (OSAL_QueueHandle_t*)parameters;
    message_t msg;
    uint32_t counter = 0;
    
    while (1) {
        msg.id = counter++;
        snprintf((char*)msg.data, sizeof(msg.data), "Msg %lu", counter);
        
        SppRetVal_t result = OSAL_QueueSend(*queue, &msg, 1000);
        if (result != SPP_OK) {
            printf("Failed to send message: %d\n", result);
        }
        
        OSAL_TaskDelay(500);
    }
}

void consumer_task(void *parameters) {
    OSAL_QueueHandle_t *queue = (OSAL_QueueHandle_t*)parameters;
    message_t msg;
    
    while (1) {
        SppRetVal_t result = OSAL_QueueReceive(*queue, &msg, OSAL_WAIT_FOREVER);
        if (result == SPP_OK) {
            printf("Received message %lu: %s\n", msg.id, msg.data);
        }
    }
}

int main() {
    OSAL_QueueHandle_t queue;
    OSAL_TaskHandle_t producer, consumer;
    
    // Initialize OSAL
    OSAL_Init();
    
    // Create queue for 10 messages
    SppRetVal_t result = OSAL_QueueCreate(&queue, 10, sizeof(message_t));
    if (result != SPP_OK) {
        printf("Failed to create queue: %d\n", result);
        return -1;
    }
    
    // Create tasks
    OSAL_TaskCreate(&producer, "Producer", producer_task, &queue, OSAL_TASK_PRIORITY_NORMAL);
    OSAL_TaskCreate(&consumer, "Consumer", consumer_task, &queue, OSAL_TASK_PRIORITY_NORMAL);
    
    return 0;
}
```

## Mutex Management

### Header File: `osal/mutex.h`

```c
#include "osal/mutex.h"

// Mutex types
typedef enum {
    OSAL_MUTEX_TYPE_NORMAL,
    OSAL_MUTEX_TYPE_RECURSIVE
} OSAL_MutexType_t;

// Create a mutex
SppRetVal_t OSAL_MutexCreate(OSAL_MutexHandle_t *mutex,
                         OSAL_MutexType_t type);

// Delete a mutex
SppRetVal_t OSAL_MutexDelete(OSAL_MutexHandle_t mutex);

// Take (lock) a mutex
SppRetVal_t OSAL_MutexTake(OSAL_MutexHandle_t mutex,
                       uint32_t timeout);

// Give (unlock) a mutex
SppRetVal_t OSAL_MutexGive(OSAL_MutexHandle_t mutex);

// Try to take mutex without blocking
SppRetVal_t OSAL_MutexTryTake(OSAL_MutexHandle_t mutex);

// Get mutex holder
OSAL_TaskHandle_t OSAL_MutexGetHolder(OSAL_MutexHandle_t mutex);
```

### Usage Example

```c
OSAL_MutexHandle_t shared_resource_mutex;
int shared_counter = 0;

void worker_task(void *parameters) {
    int task_id = *(int*)parameters;
    
    while (1) {
        // Take mutex to protect shared resource
        SppRetVal_t result = OSAL_MutexTake(shared_resource_mutex, 1000);
        if (result == SPP_OK) {
            // Critical section
            int old_value = shared_counter;
            OSAL_TaskDelay(10); // Simulate work
            shared_counter = old_value + 1;
            printf("Task %d: counter = %d\n", task_id, shared_counter);
            
            // Release mutex
            OSAL_MutexGive(shared_resource_mutex);
        } else {
            printf("Task %d: Failed to take mutex\n", task_id);
        }
        
        OSAL_TaskDelay(100);
    }
}

int main() {
    OSAL_TaskHandle_t task1, task2;
    int task1_id = 1, task2_id = 2;
    
    // Initialize OSAL
    OSAL_Init();
    
    // Create mutex
    SppRetVal_t result = OSAL_MutexCreate(&shared_resource_mutex, OSAL_MUTEX_TYPE_NORMAL);
    if (result != SPP_OK) {
        printf("Failed to create mutex: %d\n", result);
        return -1;
    }
    
    // Create worker tasks
    OSAL_TaskCreate(&task1, "Worker1", worker_task, &task1_id, OSAL_TASK_PRIORITY_NORMAL);
    OSAL_TaskCreate(&task2, "Worker2", worker_task, &task2_id, OSAL_TASK_PRIORITY_NORMAL);
    
    return 0;
}
```

## Semaphore Management

### Header File: `osal/semaphore.h`

```c
#include "osal/semaphore.h"

// Semaphore types
typedef enum {
    OSAL_SEMAPHORE_TYPE_BINARY,
    OSAL_SEMAPHORE_TYPE_COUNTING
} OSAL_SemaphoreType_t;

// Create a semaphore
SppRetVal_t OSAL_SemaphoreCreate(OSAL_SemaphoreHandle_t *semaphore,
                             OSAL_SemaphoreType_t type,
                             uint32_t max_count,
                             uint32_t initial_count);

// Delete a semaphore
SppRetVal_t OSAL_SemaphoreDelete(OSAL_SemaphoreHandle_t semaphore);

// Take (wait for) a semaphore
SppRetVal_t OSAL_SemaphoreTake(OSAL_SemaphoreHandle_t semaphore,
                           uint32_t timeout);

// Give (signal) a semaphore
SppRetVal_t OSAL_SemaphoreGive(OSAL_SemaphoreHandle_t semaphore);

// Give semaphore from ISR
SppRetVal_t OSAL_SemaphoreGiveFromISR(OSAL_SemaphoreHandle_t semaphore);

// Get semaphore count
uint32_t OSAL_SemaphoreGetCount(OSAL_SemaphoreHandle_t semaphore);
```

### Usage Example

```c
OSAL_SemaphoreHandle_t resource_semaphore;

void resource_user_task(void *parameters) {
    int task_id = *(int*)parameters;
    
    while (1) {
        // Wait for resource availability
        SppRetVal_t result = OSAL_SemaphoreTake(resource_semaphore, 2000);
        if (result == SPP_OK) {
            printf("Task %d: Got resource, using it...\n", task_id);
            
            // Simulate resource usage
            OSAL_TaskDelay(1000);
            
            printf("Task %d: Releasing resource\n", task_id);
            OSAL_SemaphoreGive(resource_semaphore);
        } else {
            printf("Task %d: Timeout waiting for resource\n", task_id);
        }
        
        OSAL_TaskDelay(500);
    }
}

int main() {
    OSAL_TaskHandle_t task1, task2, task3;
    int task1_id = 1, task2_id = 2, task3_id = 3;
    
    // Initialize OSAL
    OSAL_Init();
    
    // Create counting semaphore for 2 resources
    SppRetVal_t result = OSAL_SemaphoreCreate(&resource_semaphore,
                                          OSAL_SEMAPHORE_TYPE_COUNTING,
                                          2, 2);
    if (result != SPP_OK) {
        printf("Failed to create semaphore: %d\n", result);
        return -1;
    }
    
    // Create tasks that compete for resources
    OSAL_TaskCreate(&task1, "User1", resource_user_task, &task1_id, OSAL_TASK_PRIORITY_NORMAL);
    OSAL_TaskCreate(&task2, "User2", resource_user_task, &task2_id, OSAL_TASK_PRIORITY_NORMAL);
    OSAL_TaskCreate(&task3, "User3", resource_user_task, &task3_id, OSAL_TASK_PRIORITY_NORMAL);
    
    return 0;
}
```

## Platform-Specific Implementation

### Method 1: Override Weak Functions

Create your RTOS-specific implementation file and override the weak functions:

```c
// freertos_osal_impl.c
#include "osal/task.h"
#include "FreeRTOS.h"
#include "task.h"

// Override weak task creation function
SppRetVal_t OSAL_TaskCreate(OSAL_TaskHandle_t *task,
                        const char *name,
                        OSAL_TaskFunction_t function,
                        void *parameters,
                        OSAL_TaskPriority_t priority) {
    
    if (task == NULL || function == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Map OSAL priority to FreeRTOS priority
    UBaseType_t freertos_priority = priority + 1;
    
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)function,
        name ? name : "OSALTask",
        2048, // Stack size
        parameters,
        freertos_priority,
        (TaskHandle_t*)task
    );
    
    return (result == pdPASS) ? SPP_OK : SPP_ERROR_INSUFFICIENT_MEMORY;
}

SppRetVal_t OSAL_TaskDelay(uint32_t milliseconds) {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
    return SPP_OK;
}

// Override other functions as needed...
```

### Method 2: Complete RTOS Implementation

For a complete FreeRTOS implementation, see `examples/osal/freertos/` which provides:

- Full task management implementation
- Queue operations with FreeRTOS queues
- Mutex implementation using FreeRTOS semaphores
- Semaphore implementation with binary and counting types
- ISR-safe variants for interrupt contexts
- Priority mapping between OSAL and FreeRTOS

## CMake Integration

### In Your Project CMakeLists.txt

```cmake
# Find and link SPP OSAL
find_package(spp REQUIRED)

add_executable(my_app main.c my_rtos_osal_impl.c)
target_link_libraries(my_app spp::osal)
```

### For FreeRTOS Projects

```cmake
# Link with FreeRTOS
target_link_libraries(my_app spp::osal freertos_kernel)

# Add FreeRTOS configuration
target_compile_definitions(my_app PRIVATE SPP_FREERTOS_AVAILABLE=1)
```

## Error Handling

All OSAL functions return `SppRetVal_t` status codes:

```c
typedef enum {
    SPP_OK,                        // Success
    SPP_ERROR,                     // General error
    SPP_ERROR_NULL_POINTER,        // NULL pointer passed
    SPP_ERROR_NOT_INITIALIZED,     // OSAL not initialized
    SPP_ERROR_INVALID_PARAMETER,   // Invalid parameter
    SPP_ERROR_TIMEOUT,             // Operation timed out
    SPP_ERROR_INSUFFICIENT_MEMORY, // Not enough memory
    SPP_ERROR_QUEUE_FULL,          // Queue is full
    SPP_ERROR_QUEUE_EMPTY,         // Queue is empty
} SppRetVal_t;
```

### Error Handling Example

```c
SppRetVal_t result = OSAL_TaskCreate(&task, "MyTask", task_func, NULL, OSAL_TASK_PRIORITY_NORMAL);
switch (result) {
    case SPP_OK:
        printf("Task created successfully\n");
        break;
    case SPP_ERROR_INSUFFICIENT_MEMORY:
        printf("Not enough memory to create task\n");
        break;
    case SPP_ERROR_NULL_POINTER:
        printf("Invalid task handle pointer\n");
        break;
    default:
        printf("Task creation failed: %d\n", result);
        break;
}
```

## Best Practices

### 1. Always Initialize OSAL
```c
int main() {
    // Initialize OSAL before using any OSAL functions
    SppRetVal_t result = OSAL_Init();
    if (result != SPP_OK) {
        printf("OSAL initialization failed: %d\n", result);
        return -1;
    }
    
    // Now safe to use OSAL functions
}
```

### 2. Check Return Values
```c
SppRetVal_t result = OSAL_QueueSend(queue, &data, 1000);
if (result != SPP_OK) {
    // Handle error appropriately
    printf("Queue send failed: %d\n", result);
}
```

### 3. Use Appropriate Timeouts
```c
// For critical operations, use longer timeouts
SppRetVal_t result = OSAL_MutexTake(critical_mutex, 5000);

// For non-critical operations, use shorter timeouts
SppRetVal_t result = OSAL_QueueSend(log_queue, &msg, 100);

// For polling, use no wait
SppRetVal_t result = OSAL_QueueReceive(queue, &data, OSAL_NO_WAIT);
```

### 4. Clean Up Resources
```c
// Always delete created objects when done
OSAL_TaskDelete(task_handle);
OSAL_QueueDelete(queue_handle);
OSAL_MutexDelete(mutex_handle);
OSAL_SemaphoreDelete(semaphore_handle);
```

### 5. Use ISR-Safe Functions in Interrupts
```c
void my_interrupt_handler(void) {
    // Use ISR-safe variants in interrupt context
    OSAL_QueueSendFromISR(queue, &data);
    OSAL_SemaphoreGiveFromISR(semaphore);
    
    // Don't use regular functions in ISR!
    // OSAL_QueueSend(queue, &data, 0); // WRONG!
}
```

## Examples

See the `examples/osal/` directory for complete RTOS implementations:

- **FreeRTOS**: `examples/osal/freertos/`
  - Complete FreeRTOS OSAL implementation
  - Demonstrates proper FreeRTOS integration
  - Shows priority mapping and configuration

## Adding Support for New RTOS

To add support for a new RTOS:

1. **Create implementation directory**: `examples/osal/new_rtos/`
2. **Create implementation files**: 
   - `new_rtos_osal_impl.h`
   - `new_rtos_osal_impl.c`
3. **Override weak functions**: Implement all OSAL functions
4. **Map types and priorities**: Convert between OSAL and RTOS types
5. **Handle ISR context**: Implement ISR-safe variants
6. **Create CMakeLists.txt**: Build configuration
7. **Test thoroughly**: Verify all functionality

### Template Structure

```c
// new_rtos_osal_impl.h
#ifndef NEW_RTOS_OSAL_IMPL_H
#define NEW_RTOS_OSAL_IMPL_H

#include "osal/osal.h"
#include "new_rtos.h"

// RTOS-specific type mappings
#define OSAL_TaskHandle_t        new_rtos_task_t*
#define OSAL_QueueHandle_t       new_rtos_queue_t*
#define OSAL_MutexHandle_t       new_rtos_mutex_t*
#define OSAL_SemaphoreHandle_t   new_rtos_semaphore_t*

#endif // NEW_RTOS_OSAL_IMPL_H
```

## Troubleshooting

### Common Issues

1. **OSAL not initialized**
   - Always call `OSAL_Init()` before using OSAL functions
   - Check return value of initialization

2. **Timeout errors**
   - Increase timeout values for slow operations
   - Use `OSAL_WAIT_FOREVER` for blocking operations
   - Use `OSAL_NO_WAIT` for polling

3. **Memory allocation failures**
   - Check available heap memory
   - Reduce stack sizes or queue sizes
   - Implement proper error handling

4. **Priority inversion**
   - Use priority inheritance mutexes
   - Design proper priority schemes
   - Avoid long critical sections

### Debug Tips

```c
// Add debug prints to track OSAL operations
SppRetVal_t OSAL_TaskCreate(...) {
    printf("Creating task: %s\n", name);
    SppRetVal_t result = /* implementation */;
    printf("Task creation result: %d\n", result);
    return result;
}
```

## Contributing

When contributing to the OSAL:

1. Follow the existing code style and patterns
2. Implement all functions in the interface
3. Use proper error handling and return codes
4. Add comprehensive documentation
5. Provide example implementations
6. Test on target RTOS platforms
7. Ensure ISR safety where applicable

For more information, see the main project README and examples directory. 