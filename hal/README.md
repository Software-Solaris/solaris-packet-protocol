# Hardware Abstraction Layer (HAL)

The Hardware Abstraction Layer (HAL) provides a standardized, platform-independent interface for hardware peripherals. This allows the same application code to run on different hardware platforms without modification, while enabling platform-specific optimizations through weak function overrides.

## Overview

The HAL layer sits between the application/services layer and the actual hardware, providing:

- **Platform Independence**: Write once, run on multiple hardware platforms
- **Standardized API**: Consistent interface across all supported peripherals
- **Weak Function Implementation**: Easy platform-specific overrides
- **Error Handling**: Comprehensive error reporting and status codes
- **Modular Design**: Each peripheral is implemented as a separate module

## Architecture

```
┌─────────────────────────────────────────┐
│         Application / Services          │
├─────────────────────────────────────────┤
│              HAL Interface              │
│    (spi.h, gpio.h, uart.h, etc.)        │
├─────────────────────────────────────────┤
│           Weak Implementations          │
│         (Default/Stub Functions)        │
├─────────────────────────────────────────┤
│        Platform Implementations         │
│     (ESP32, STM32, Linux, etc.)         │
├─────────────────────────────────────────┤
│            Hardware Platform            │
└─────────────────────────────────────────┘
```

## Supported Peripherals

### SPI (Serial Peripheral Interface)
- **Location**: `hal/spi/`
- **Interface**: `spi.h`
- **Implementation**: `spi.c` (weak functions)
- **Features**:
  - Full-duplex communication
  - Configurable clock speed and mode
  - Chip select management
  - Blocking and non-blocking operations
  - Status checking

## Directory Structure

```
hal/
├── CMakeLists.txt              # HAL main build configuration
├── README.md                   # This file
└── spi/                        # SPI peripheral module
    ├── CMakeLists.txt          # SPI build configuration
    ├── spi.h                   # SPI interface definition
    └── spi.c                   # Weak SPI implementations
```

## SPI Interface

### Header File: `hal/spi/spi.h`

The SPI interface provides a complete abstraction for SPI communication:

```c
#include "hal/spi/spi.h"

// Initialize SPI peripheral
retval_t SPI_Init(void);

// Deinitialize SPI peripheral
retval_t SPI_Deinit(void);

// Transmit data
retval_t SPI_Transmit(uint8_t *data, uint16_t size, uint32_t timeout);

// Receive data
retval_t SPI_Receive(uint8_t *data, uint16_t size, uint32_t timeout);

// Transmit and receive simultaneously
retval_t SPI_TransmitReceive(uint8_t *tx_data, uint8_t *rx_data, 
                            uint16_t size, uint32_t timeout);

// Check if SPI is busy
bool SPI_IsBusy(void);

// Control chip select
retval_t SPI_SetChipSelect(bool active);

// Simplified interface functions
retval_t spi_write_data(uint8_t *data, uint16_t size);
retval_t spi_get_data(uint8_t *data, uint16_t size);
```

### Implementation: `hal/spi/spi.c`

All functions are implemented as weak symbols, allowing platform-specific overrides:

```c
__attribute__((weak)) retval_t SPI_Init(void) {
    // Default implementation - returns not supported
    return SPP_ERROR_NOT_SUPPORTED;
}

__attribute__((weak)) retval_t spi_write_data(uint8_t *data, uint16_t size) {
    // Default implementation using standard SPI functions
    return SPI_Transmit(data, size, 1000);
}
```

## Using HAL in Your Project

### 1. Include HAL Headers

```c
#include "hal/spi/spi.h"
```

### 2. Initialize HAL

```c
int main() {
    // Initialize SPI
    retval_t result = SPI_Init();
    if (result != SPP_OK) {
        printf("SPI initialization failed: %d\n", result);
        return -1;
    }
    
    // Your application code here
    
    return 0;
}
```

### 3. Use HAL Functions

```c
void send_spi_data(void) {
    uint8_t tx_data[] = {0x01, 0x02, 0x03, 0x04};
    uint8_t rx_data[4];
    
    // Simple write
    retval_t result = spi_write_data(tx_data, sizeof(tx_data));
    if (result != SPP_OK) {
        printf("SPI write failed: %d\n", result);
        return;
    }
    
    // Read data
    result = spi_get_data(rx_data, sizeof(rx_data));
    if (result != SPP_OK) {
        printf("SPI read failed: %d\n", result);
        return;
    }
    
    // Full-duplex communication
    result = SPI_TransmitReceive(tx_data, rx_data, sizeof(tx_data), 1000);
    if (result != SPP_OK) {
        printf("SPI transfer failed: %d\n", result);
        return;
    }
}
```

## Platform-Specific Implementation

### Method 1: Override Weak Functions

Create your platform-specific implementation file and override the weak functions:

```c
// my_platform_spi.c
#include "hal/spi/spi.h"
#include "my_platform_spi_driver.h"

// Override the weak SPI_Init function
retval_t SPI_Init(void) {
    // Your platform-specific SPI initialization
    if (my_platform_spi_init() == 0) {
        return SPP_OK;
    }
    return SPP_ERROR;
}

// Override the weak spi_write_data function
retval_t spi_write_data(uint8_t *data, uint16_t size) {
    if (data == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    // Your platform-specific SPI write
    if (my_platform_spi_write(data, size) == 0) {
        return SPP_OK;
    }
    return SPP_ERROR;
}

// Override other functions as needed...
```

### Method 2: Complete Implementation

For complex platforms, you might want to implement all functions:

```c
// esp32_spi_implementation.c
#include "hal/spi/spi.h"
#include "driver/spi_master.h"

static spi_device_handle_t spi_handle = NULL;

retval_t SPI_Init(void) {
    spi_bus_config_t bus_config = {
        .miso_io_num = GPIO_NUM_19,
        .mosi_io_num = GPIO_NUM_23,
        .sclk_io_num = GPIO_NUM_18,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };
    
    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 1000000,
        .mode = 0,
        .spics_io_num = GPIO_NUM_5,
        .queue_size = 1,
    };
    
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return SPP_ERROR;
    
    ret = spi_bus_add_device(SPI2_HOST, &dev_config, &spi_handle);
    if (ret != ESP_OK) return SPP_ERROR;
    
    return SPP_OK;
}

retval_t SPI_Transmit(uint8_t *data, uint16_t size, uint32_t timeout) {
    if (data == NULL || spi_handle == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    
    spi_transaction_t trans = {
        .length = size * 8,
        .tx_buffer = data,
        .rx_buffer = NULL,
    };
    
    esp_err_t ret = spi_device_transmit(spi_handle, &trans);
    return (ret == ESP_OK) ? SPP_OK : SPP_ERROR;
}
```

## CMake Integration

### In Your Project CMakeLists.txt

```cmake
# Find and link SPP HAL
find_package(spp REQUIRED)

add_executable(my_app main.c my_platform_spi.c)
target_link_libraries(my_app spp::hal)
```

### For ESP-IDF Projects

```cmake
# components/my_app/CMakeLists.txt
idf_component_register(
    SRCS "main.c" "esp32_spi_implementation.c"
    INCLUDE_DIRS "."
    REQUIRES spp_hal driver
)
```

## Error Handling

All HAL functions return `retval_t` status codes defined in `core/returntypes.h`:

```c
typedef enum {
    SPP_OK,                        // Success
    SPP_ERROR,                     // General error
    SPP_ERROR_NULL_POINTER,        // NULL pointer passed
    SPP_ERROR_NOT_INITIALIZED,     // Peripheral not initialized
    SPP_ERROR_INVALID_PARAMETER,   // Invalid parameter
    SPP_ERROR_TIMEOUT,             // Operation timed out
    SPP_ERROR_BUSY,                // Peripheral is busy
    SPP_ERROR_NOT_SUPPORTED,       // Operation not supported
} retval_t;
```

### Error Handling Example

```c
retval_t result = SPI_Init();
switch (result) {
    case SPP_OK:
        printf("SPI initialized successfully\n");
        break;
    case SPP_ERROR_NOT_SUPPORTED:
        printf("SPI not supported on this platform\n");
        break;
    case SPP_ERROR:
    default:
        printf("SPI initialization failed\n");
        break;
}
```

## Best Practices

### 1. Always Check Return Values
```c
retval_t result = SPI_Init();
if (result != SPP_OK) {
    // Handle error appropriately
    return result;
}
```

### 2. Validate Parameters
```c
retval_t my_spi_function(uint8_t *data, uint16_t size) {
    if (data == NULL) {
        return SPP_ERROR_NULL_POINTER;
    }
    if (size == 0) {
        return SPP_ERROR_INVALID_PARAMETER;
    }
    // Continue with implementation
}
```

### 3. Use Timeouts for Blocking Operations
```c
// Use reasonable timeouts
retval_t result = SPI_Transmit(data, size, 1000); // 1 second timeout
```

### 4. Check Busy Status
```c
if (SPI_IsBusy()) {
    return SPP_ERROR_BUSY;
}
```

## Examples

See the `examples/hal/` directory for complete platform implementations:

- **ESP32-S3 SPI**: `examples/hal/esp32spi/`
  - Complete ESP-IDF SPI driver implementation
  - Demonstrates proper ESP32-S3 integration
  - Shows configuration and usage patterns

## Adding New Peripherals

To add a new peripheral to the HAL:

1. **Create peripheral directory**: `hal/new_peripheral/`
2. **Create interface header**: `hal/new_peripheral/new_peripheral.h`
3. **Create weak implementation**: `hal/new_peripheral/new_peripheral.c`
4. **Create CMakeLists.txt**: `hal/new_peripheral/CMakeLists.txt`
5. **Update main HAL CMakeLists.txt**: Add subdirectory
6. **Create example implementation**: `examples/hal/platform_new_peripheral/`

### Template Structure

```c
// hal/new_peripheral/new_peripheral.h
#ifndef NEW_PERIPHERAL_H
#define NEW_PERIPHERAL_H

#include "core/returntypes.h"

// Function declarations
retval_t NEW_PERIPHERAL_Init(void);
retval_t NEW_PERIPHERAL_Deinit(void);
// Add more functions as needed

#endif // NEW_PERIPHERAL_H
```

```c
// hal/new_peripheral/new_peripheral.c
#include "new_peripheral.h"

__attribute__((weak)) retval_t NEW_PERIPHERAL_Init(void) {
    return SPP_ERROR_NOT_SUPPORTED;
}

__attribute__((weak)) retval_t NEW_PERIPHERAL_Deinit(void) {
    return SPP_ERROR_NOT_SUPPORTED;
}
```

## Troubleshooting

### Common Issues

1. **Linker errors about multiple definitions**
   - Make sure you're not defining the same function in multiple files
   - Use weak functions properly

2. **Functions not being overridden**
   - Ensure your implementation file is being compiled and linked
   - Check that function signatures match exactly

3. **Platform-specific headers not found**
   - Add proper include directories to your CMakeLists.txt
   - Use conditional compilation for platform-specific code

### Debug Tips

```c
// Add debug prints to verify function calls
retval_t SPI_Init(void) {
    printf("Platform-specific SPI_Init called\n");
    // Your implementation
    return SPP_OK;
}
```

## Contributing

When contributing to the HAL:

1. Follow the existing code style and patterns
2. Implement all functions in the interface
3. Use proper error handling and return codes
4. Add comprehensive documentation
5. Provide example implementations
6. Test on target platforms

For more information, see the main project README and examples directory. 