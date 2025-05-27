# SPP Examples

This directory contains comprehensive examples demonstrating how to use the Solaris Packet Protocol (SPP) library with its Hardware Abstraction Layer (HAL) and Operating System Abstraction Layer (OSAL). These examples show real-world implementations for different platforms and operating systems.

## Overview

The examples demonstrate:

- **Platform Integration**: How to integrate SPP with specific hardware platforms
- **RTOS Integration**: How to use SPP with different real-time operating systems
- **Layered Architecture**: How the Core, Services, HAL, and OSAL layers work together
- **CMake Integration**: How to properly configure build systems for different targets
- **Best Practices**: Proper error handling, resource management, and code organization

## Directory Structure

```
examples/
├── CMakeLists.txt              # Main examples build configuration
├── README.md                   # This file
├── basic_usage.c               # Simple SPP usage example
├── CMakeLists_example.txt      # Template for external projects
│
├── hal/                        # Hardware Abstraction Layer Examples
│   ├── CMakeLists.txt          # HAL examples build configuration
│   └── esp32spi/              # ESP32-S3 SPI implementation
│       ├── CMakeLists.txt      # ESP32 SPI build configuration
│       ├── esp32_spi.h         # ESP32 SPI interface
│       └── esp32_spi.c         # ESP32 SPI implementation
│
└── osal/                       # Operating System Abstraction Layer Examples
    ├── CMakeLists.txt          # OSAL examples build configuration
    └── freertos/              # FreeRTOS implementation
        ├── CMakeLists.txt      # FreeRTOS build configuration
        ├── freertos_osal_impl.h # FreeRTOS OSAL interface
        └── freertos_osal_impl.c # FreeRTOS OSAL implementation
```

## Architecture Integration

The examples demonstrate how the SPP stack layers work together:

```
┌─────────────────────────────────────────┐
│              Examples                   │
│        (basic_usage.c, etc.)            │
├─────────────────────────────────────────┤
│              Services                   │
│            (DataBank)                   │
├─────────────────────────────────────────┤
│               Core                      │
│        (Initialization, Types)          │
├─────────────────────────────────────────┤
│               OSAL                      │
│         (FreeRTOS Example)              │
├─────────────────────────────────────────┤
│               HAL                       │
│         (ESP32-S3 Example)              │
├─────────────────────────────────────────┤
│          Hardware Platform              │
│            (ESP32-S3)                   │
└─────────────────────────────────────────┘
```

## Basic Usage Example

### File: `basic_usage.c`

This example demonstrates the fundamental usage of the SPP library:

```c
#include <stdio.h>
#include "core/core.h"
#include "services/databank/databank.h"

int main() {
    printf("SPP Basic Usage Example\n");
    
    // Initialize SPP Core
    retval_t result = Core_Init();
    if (result != SPP_OK) {
        printf("Core initialization failed: %d\n", result);
        return -1;
    }
    
    printf("SPP Core initialized successfully\n");
    
    // Get a packet from DataBank
    spp_packet_t* packet = DataBank_GetPacket();
    if (packet != NULL) {
        printf("Got packet from DataBank\n");
        
        // Use the packet for your application
        // ... packet processing logic ...
        
        // Return packet to DataBank
        DataBank_ReturnPacket(packet);
        printf("Returned packet to DataBank\n");
    } else {
        printf("No packets available in DataBank\n");
    }
    
    printf("SPP Basic Usage Example completed\n");
    return 0;
}
```

### Key Concepts Demonstrated:

1. **Core Initialization**: How to initialize the SPP library
2. **DataBank Usage**: How to get and return packets
3. **Error Handling**: Proper checking of return values
4. **Resource Management**: Returning packets after use

## HAL Examples

### ESP32-S3 SPI Implementation

**Location**: `examples/hal/esp32spi/`

This example shows how to implement platform-specific HAL functions for ESP32-S3:

#### File: `esp32_spi.h`

```c
#ifndef ESP32_SPI_H
#define ESP32_SPI_H

#include "hal/spi/spi.h"

#ifdef ESP_PLATFORM

// ESP32-specific SPI configuration
typedef struct {
    int miso_pin;
    int mosi_pin;
    int sclk_pin;
    int cs_pin;
    int clock_speed_hz;
    int spi_mode;
} esp32_spi_config_t;

// Initialize ESP32 SPI with configuration
retval_t ESP32_SPI_InitWithConfig(const esp32_spi_config_t *config);

#endif // ESP_PLATFORM

#endif // ESP32_SPI_H
```

#### File: `esp32_spi.c`

```c
#include "esp32_spi.h"

#ifdef ESP_PLATFORM

#include "driver/spi_master.h"
#include "driver/gpio.h"

static spi_device_handle_t spi_handle = NULL;
static bool spi_initialized = false;

// Override weak SPI_Init function
retval_t SPI_Init(void) {
    esp32_spi_config_t default_config = {
        .miso_pin = GPIO_NUM_19,
        .mosi_pin = GPIO_NUM_23,
        .sclk_pin = GPIO_NUM_18,
        .cs_pin = GPIO_NUM_5,
        .clock_speed_hz = 1000000,
        .spi_mode = 0
    };
    
    return ESP32_SPI_InitWithConfig(&default_config);
}

retval_t ESP32_SPI_InitWithConfig(const esp32_spi_config_t *config) {
    if (config == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (spi_initialized) {
        return SPP_OK; // Already initialized
    }
    
    // Configure SPI bus
    spi_bus_config_t bus_config = {
        .miso_io_num = config->miso_pin,
        .mosi_io_num = config->mosi_pin,
        .sclk_io_num = config->sclk_pin,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        return SPP_ERROR;
    }
    
    // Configure SPI device
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = config->clock_speed_hz,
        .mode = config->spi_mode,
        .spics_io_num = config->cs_pin,
        .queue_size = 1,
    };
    
    ret = spi_bus_add_device(SPI2_HOST, &dev_config, &spi_handle);
    if (ret != ESP_OK) {
        spi_bus_free(SPI2_HOST);
        return SPP_ERROR;
    }
    
    spi_initialized = true;
    return SPP_OK;
}

// Override weak SPI_Transmit function
retval_t SPI_Transmit(uint8_t *data, uint16_t size, uint32_t timeout) {
    if (data == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (!spi_initialized || spi_handle == NULL) {
        return SPP_ERROR_NOT_INITIALIZED;
    }
    
    spi_transaction_t trans = {
        .length = size * 8,
        .tx_buffer = data,
        .rx_buffer = NULL,
    };
    
    esp_err_t ret = spi_device_transmit(spi_handle, &trans);
    return (ret == ESP_OK) ? SPP_OK : SPP_ERROR;
}

// Override other SPI functions...

#endif // ESP_PLATFORM
```

#### CMake Configuration: `esp32spi/CMakeLists.txt`

```cmake
# ESP32 SPI HAL Implementation
cmake_minimum_required(VERSION 3.16)

# Only build if targeting ESP32
if(ESP_PLATFORM)
    # Create ESP32 SPI implementation library
    add_library(spp_hal_esp32spi STATIC
        esp32_spi.c
    )
    
    # Include directories
    target_include_directories(spp_hal_esp32spi PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    )
    
    # Link with SPP HAL and ESP-IDF components
    target_link_libraries(spp_hal_esp32spi PUBLIC
        spp_hal_spi
        idf::driver
    )
    
    # Set properties
    set_target_properties(spp_hal_esp32spi PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
    )
    
    # Install
    install(TARGETS spp_hal_esp32spi
        EXPORT spp-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    
    install(FILES esp32_spi.h
        DESTINATION include/spp/examples/hal/esp32spi
    )
endif()
```

### Key HAL Concepts Demonstrated:

1. **Weak Function Override**: How to override weak HAL functions
2. **Platform-Specific Code**: Using conditional compilation for ESP32
3. **Configuration Structure**: Flexible configuration approach
4. **Error Handling**: Proper ESP-IDF error code mapping
5. **Resource Management**: Proper initialization and cleanup

## OSAL Examples

### FreeRTOS Implementation

**Location**: `examples/osal/freertos/`

This example shows how to implement OSAL functions for FreeRTOS:

#### File: `freertos_osal_impl.h`

```c
#ifndef FREERTOS_OSAL_IMPL_H
#define FREERTOS_OSAL_IMPL_H

#include "osal/osal.h"

#ifdef SPP_FREERTOS_AVAILABLE

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// FreeRTOS-specific type mappings
#define OSAL_TaskHandle_t        TaskHandle_t
#define OSAL_QueueHandle_t       QueueHandle_t
#define OSAL_MutexHandle_t       SemaphoreHandle_t
#define OSAL_SemaphoreHandle_t   SemaphoreHandle_t

// Priority mapping functions
UBaseType_t OSAL_PriorityToFreeRTOS(OSAL_TaskPriority_t osal_priority);
OSAL_TaskPriority_t FreeRTOSPriorityToOSAL(UBaseType_t freertos_priority);

#endif // SPP_FREERTOS_AVAILABLE

#endif // FREERTOS_OSAL_IMPL_H
```

#### File: `freertos_osal_impl.c`

```c
#include "freertos_osal_impl.h"

#ifdef SPP_FREERTOS_AVAILABLE

// Priority mapping
UBaseType_t OSAL_PriorityToFreeRTOS(OSAL_TaskPriority_t osal_priority) {
    switch (osal_priority) {
        case OSAL_TASK_PRIORITY_IDLE:     return 0;
        case OSAL_TASK_PRIORITY_LOW:      return 1;
        case OSAL_TASK_PRIORITY_NORMAL:   return 2;
        case OSAL_TASK_PRIORITY_HIGH:     return 3;
        case OSAL_TASK_PRIORITY_CRITICAL: return 4;
        default:                          return 2; // Default to normal
    }
}

// Override weak OSAL_TaskCreate function
retval_t OSAL_TaskCreate(OSAL_TaskHandle_t *task,
                        const char *name,
                        OSAL_TaskFunction_t function,
                        void *parameters,
                        OSAL_TaskPriority_t priority) {
    
    if (task == NULL || function == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    UBaseType_t freertos_priority = OSAL_PriorityToFreeRTOS(priority);
    
    BaseType_t result = xTaskCreate(
        (TaskFunction_t)function,
        name ? name : "OSALTask",
        2048, // Stack size in words
        parameters,
        freertos_priority,
        task
    );
    
    return (result == pdPASS) ? SPP_OK : SPP_ERROR_INSUFFICIENT_MEMORY;
}

// Override weak OSAL_TaskDelay function
retval_t OSAL_TaskDelay(uint32_t milliseconds) {
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
    return SPP_OK;
}

// Override weak OSAL_QueueCreate function
retval_t OSAL_QueueCreate(OSAL_QueueHandle_t *queue,
                         uint32_t max_items,
                         uint32_t item_size) {
    
    if (queue == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    if (max_items == 0 || item_size == 0) {
        return SPP_ERROR_INVALID_PARAMETER;
    }
    
    *queue = xQueueCreate(max_items, item_size);
    return (*queue != NULL) ? SPP_OK : SPP_ERROR_INSUFFICIENT_MEMORY;
}

// Override weak OSAL_QueueSend function
retval_t OSAL_QueueSend(OSAL_QueueHandle_t queue,
                       const void *item,
                       uint32_t timeout) {
    
    if (queue == NULL || item == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    TickType_t freertos_timeout = (timeout == OSAL_WAIT_FOREVER) ? 
                                  portMAX_DELAY : pdMS_TO_TICKS(timeout);
    
    BaseType_t result = xQueueSend(queue, item, freertos_timeout);
    
    switch (result) {
        case pdPASS:
            return SPP_OK;
        case errQUEUE_FULL:
            return SPP_ERROR_QUEUE_FULL;
        default:
            return SPP_ERROR_TIMEOUT;
    }
}

// Override weak OSAL_MutexCreate function
retval_t OSAL_MutexCreate(OSAL_MutexHandle_t *mutex,
                         OSAL_MutexType_t type) {
    
    if (mutex == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    switch (type) {
        case OSAL_MUTEX_TYPE_NORMAL:
            *mutex = xSemaphoreCreateMutex();
            break;
        case OSAL_MUTEX_TYPE_RECURSIVE:
            *mutex = xSemaphoreCreateRecursiveMutex();
            break;
        default:
            return SPP_ERROR_INVALID_PARAMETER;
    }
    
    return (*mutex != NULL) ? SPP_OK : SPP_ERROR_INSUFFICIENT_MEMORY;
}

// Override other OSAL functions...

#endif // SPP_FREERTOS_AVAILABLE
```

#### CMake Configuration: `freertos/CMakeLists.txt`

```cmake
# FreeRTOS OSAL Implementation
cmake_minimum_required(VERSION 3.16)

# Only build if FreeRTOS is available
if(SPP_FREERTOS_AVAILABLE)
    # Create FreeRTOS OSAL implementation library
    add_library(spp_osal_freertos STATIC
        freertos_osal_impl.c
    )
    
    # Include directories
    target_include_directories(spp_osal_freertos PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../..
    )
    
    # Link with SPP OSAL and FreeRTOS
    target_link_libraries(spp_osal_freertos PUBLIC
        spp_osal
        freertos_kernel
    )
    
    # Add FreeRTOS compile definition
    target_compile_definitions(spp_osal_freertos PUBLIC
        SPP_FREERTOS_AVAILABLE=1
    )
    
    # Set properties
    set_target_properties(spp_osal_freertos PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
    )
    
    # Install
    install(TARGETS spp_osal_freertos
        EXPORT spp-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )
    
    install(FILES freertos_osal_impl.h
        DESTINATION include/spp/examples/osal/freertos
    )
endif()
```

### Key OSAL Concepts Demonstrated:

1. **Type Mapping**: How to map OSAL types to FreeRTOS types
2. **Priority Conversion**: Converting between priority systems
3. **Timeout Handling**: Mapping OSAL timeouts to FreeRTOS ticks
4. **Error Code Mapping**: Converting FreeRTOS errors to SPP errors
5. **Conditional Compilation**: Building only when FreeRTOS is available

## CMake Integration

### Main Examples CMakeLists.txt

```cmake
# Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Build basic usage example
add_executable(spp_basic_usage basic_usage.c)
target_link_libraries(spp_basic_usage 
    spp_core 
    spp_databank
)

# Add subdirectories for platform-specific examples
add_subdirectory(hal)
add_subdirectory(osal)

# Install examples
install(TARGETS spp_basic_usage
    RUNTIME DESTINATION bin/examples
)

install(FILES 
    basic_usage.c
    CMakeLists_example.txt
    DESTINATION share/spp/examples
)
```

### HAL Examples CMakeLists.txt

```cmake
# HAL Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Add ESP32 SPI example if targeting ESP32
if(ESP_PLATFORM OR CMAKE_TOOLCHAIN_FILE MATCHES "esp32")
    add_subdirectory(esp32spi)
endif()

# Add other HAL examples here
# if(STM32_PLATFORM)
#     add_subdirectory(stm32spi)
# endif()
```

### OSAL Examples CMakeLists.txt

```cmake
# OSAL Examples CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# Add FreeRTOS example if FreeRTOS is available
if(SPP_FREERTOS_AVAILABLE OR TARGET freertos_kernel)
    set(SPP_FREERTOS_AVAILABLE ON)
    add_subdirectory(freertos)
endif()

# Add other OSAL examples here
# if(SPP_THREADX_AVAILABLE)
#     add_subdirectory(threadx)
# endif()
```

## Using Examples in Your Project

### Method 1: Copy Example Files

```bash
# Copy the example you need
cp examples/hal/esp32spi/* your_project/hal_impl/
cp examples/osal/freertos/* your_project/osal_impl/

# Include in your CMakeLists.txt
add_subdirectory(hal_impl)
add_subdirectory(osal_impl)
target_link_libraries(your_app spp_hal_esp32spi spp_osal_freertos)
```

### Method 2: Use as Template

Use `CMakeLists_example.txt` as a template for your project:

```cmake
# Your project CMakeLists.txt (based on CMakeLists_example.txt)
cmake_minimum_required(VERSION 3.16)
project(my_spp_project)

# Find SPP package
find_package(spp REQUIRED)

# Your application
add_executable(my_app 
    main.c
    my_hal_impl.c
    my_osal_impl.c
)

# Link with SPP
target_link_libraries(my_app 
    spp::core
    spp::databank
    spp::hal
    spp::osal
)

# Platform-specific linking
if(ESP_PLATFORM)
    target_link_libraries(my_app idf::driver)
endif()

if(SPP_FREERTOS_AVAILABLE)
    target_link_libraries(my_app freertos_kernel)
endif()
```

### Method 3: Include Examples Directly

```cmake
# Include SPP examples in your build
add_subdirectory(path/to/spp/examples)

# Link with example implementations
target_link_libraries(your_app 
    spp_core
    spp_databank
    spp_hal_esp32spi      # If using ESP32
    spp_osal_freertos     # If using FreeRTOS
)
```

## Building Examples

### Build All Examples

```bash
# Configure with examples enabled
cmake -DSPP_BUILD_EXAMPLES=ON ..

# Build
cmake --build .

# Examples will be in build/examples/
```

### Build for ESP32-S3

```bash
# Configure ESP-IDF environment
source $IDF_PATH/export.sh

# Build with ESP32 toolchain
./build.sh -x esp32s3 -DSPP_BUILD_EXAMPLES=ON

# ESP32 examples will be in build-esp32s3/examples/
```

### Build with FreeRTOS

```bash
# Configure with FreeRTOS
cmake -DSPP_BUILD_EXAMPLES=ON -DSPP_FREERTOS_AVAILABLE=ON ..

# Build
cmake --build .
```

## Testing Examples

### Basic Usage Test

```bash
# Run basic usage example
./build/examples/spp_basic_usage

# Expected output:
# SPP Basic Usage Example
# SPP Core initialized successfully
# Got packet from DataBank
# Returned packet to DataBank
# SPP Basic Usage Example completed
```

### ESP32-S3 Test

```bash
# Flash to ESP32-S3 (if using ESP-IDF)
idf.py flash monitor

# Or test compilation
./build.sh -x esp32s3 -DSPP_BUILD_EXAMPLES=ON
```

## Best Practices Demonstrated

### 1. Error Handling

```c
retval_t result = SPI_Init();
if (result != SPP_OK) {
    printf("SPI initialization failed: %d\n", result);
    return -1;
}
```

### 2. Resource Management

```c
spp_packet_t* packet = DataBank_GetPacket();
if (packet != NULL) {
    // Use packet
    DataBank_ReturnPacket(packet); // Always return
}
```

### 3. Conditional Compilation

```c
#ifdef ESP_PLATFORM
    // ESP32-specific code
#endif

#ifdef SPP_FREERTOS_AVAILABLE
    // FreeRTOS-specific code
#endif
```

### 4. Configuration Structures

```c
esp32_spi_config_t config = {
    .miso_pin = GPIO_NUM_19,
    .mosi_pin = GPIO_NUM_23,
    // ... other configuration
};
ESP32_SPI_InitWithConfig(&config);
```

### 5. Type Safety

```c
// Use proper type casting
BaseType_t result = xTaskCreate(
    (TaskFunction_t)function,  // Explicit cast
    name,
    stack_size,
    parameters,
    priority,
    task_handle
);
```

## Extending Examples

### Adding New HAL Implementation

1. Create directory: `examples/hal/new_platform/`
2. Implement HAL functions for your platform
3. Create CMakeLists.txt with proper conditionals
4. Update `examples/hal/CMakeLists.txt`

### Adding New OSAL Implementation

1. Create directory: `examples/osal/new_rtos/`
2. Implement OSAL functions for your RTOS
3. Create proper type mappings
4. Handle ISR contexts appropriately
5. Update `examples/osal/CMakeLists.txt`

## Troubleshooting

### Common Issues

1. **Examples not building**
   - Ensure `SPP_BUILD_EXAMPLES=ON`
   - Check platform-specific dependencies

2. **ESP32 examples failing**
   - Verify ESP-IDF is properly configured
   - Check `ESP_PLATFORM` is defined

3. **FreeRTOS examples failing**
   - Ensure FreeRTOS is available
   - Set `SPP_FREERTOS_AVAILABLE=ON`

4. **Linking errors**
   - Check library dependencies
   - Verify target platform configuration

### Debug Tips

```c
// Add debug output to track execution
printf("Initializing platform-specific implementation\n");
retval_t result = platform_init();
printf("Platform init result: %d\n", result);
```

## Contributing

When contributing examples:

1. Follow existing code style and patterns
2. Add comprehensive error handling
3. Include proper CMake configuration
4. Test on target platforms
5. Document platform-specific requirements
6. Provide clear usage instructions

For more information, see the main project README and individual layer documentation. 