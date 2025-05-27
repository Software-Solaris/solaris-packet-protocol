# Solaris Packet Protocol (SPP)

Designed for the edge of space, optimized for the embedded world — a packet protocol library for projects that fly high and fail never.

## Description

The Solaris Packet Protocol (SPP) library provides a robust framework for handling data packets in embedded and real-time systems. It includes:

- **Core**: Basic initialization and management functionalities
- **Services**: Specialized services like DataBank for packet pool management
- **Drivers**: Hardware driver interface (extensible)
- **Components**: Additional modular components (extensible)
- **Third Party**: Integration with FreeRTOS and other operating systems

## Features

- ✅ Static library compilation
- ✅ Full cross-compilation support
- ✅ CMake 3.16+ compatible
- ✅ C99 standard
- ✅ Modular architecture
- ✅ Efficient packet pool
- ✅ Robust error handling
- ✅ ESP32-S3 and ESP-IDF support
- ✅ FreeRTOS integration

## Project Structure

```
spp/
├── CMakeLists.txt              # Main CMake file
├── build.sh                    # Automated build script
├── README.md                   # This file
├── CROSS_COMPILATION.md        # Cross-compilation guide
├── .gitmodules                 # Submodules configuration
├── core/                       # Core module
│   ├── CMakeLists.txt
│   ├── core.h
│   ├── core.c
│   ├── macros.h
│   └── returntypes.h
├── services/                   # Services
│   ├── CMakeLists.txt
│   └── databank/              # DataBank service
│       ├── CMakeLists.txt
│       ├── databank.h
│       └── databank.c
├── components/                 # Components (extensible)
├── drivers/                    # Drivers (extensible)
├── examples/                   # Usage examples
│   ├── basic_usage.c
│   ├── CMakeLists.txt
│   └── CMakeLists_example.txt
├── third_party/               # External dependencies
│   └── OS/
│       └── FreeRTOS/          # FreeRTOS Kernel (submodule)
└── cmake/                      # CMake configuration
    ├── spp-config.cmake.in
    └── toolchains/            # Toolchain files
        ├── arm-linux-gnueabihf.cmake
        ├── riscv64-linux-gnu.cmake
        └── esp32s3.cmake
```

## Requirements

### Basic Requirements
- CMake 3.16 or higher
- C99-compatible compiler (GCC, Clang)
- Make or Ninja
- Git (for submodules)

### For Cross-Compilation
- Cross-compilation toolchain for the target architecture
- Supported architectures:
  - **ARM**: `arm-linux-gnueabihf-gcc`
  - **RISC-V**: `riscv64-linux-gnu-gcc`
  - **ESP32-S3**: ESP-IDF toolchain

### For ESP32-S3
- ESP-IDF v4.4 or higher
- Xtensa toolchain (included in ESP-IDF)
- `IDF_PATH` environment variable configured

## Quick Start - Compiling and Running Examples on Linux

### Step 1: Initialize Submodules and Build Library

```bash
# Clone the repository (if not already done)
git clone --recursive <repository-url>
cd spp

# Or initialize submodules if already cloned
git submodule update --init --recursive

# Build the library with Debug mode for better debugging
./build.sh -s -t Debug -v
```

### Step 2: Enable and Build Examples

```bash
# Navigate to build directory
cd build

# Enable examples compilation
cmake -DSPP_BUILD_EXAMPLES=ON ..

# Build examples
make

# Verify the example was built
ls examples/spp_basic_example
```

### Step 3: Run the Example

```bash
# Run the basic example
./examples/spp_basic_example
```

**Expected Output:**
```
Initializing Solaris Packet Protocol...
Initializing Data Bank
Data Bank initialized with 100 packets
SPP initialized successfully.
Packet obtained from databank.
Packet configured: ID=42, Timestamp=1234567890, Data=0xDEADBEEF
Packet returned to databank.
SPP example completed successfully.
```

### Alternative: One-Command Build and Run

```bash
# From the spp directory
./build.sh -s -t Debug && cd build && cmake -DSPP_BUILD_EXAMPLES=ON .. && make && ./examples/spp_basic_example
```

## Submodule Initialization

Before building, initialize git submodules:

```bash
# Method 1: Using automated script
./build.sh -s

# Method 2: Manually
git submodule update --init --recursive

# Method 3: When cloning the repository
git clone --recursive <repository-url>
```

## Building

### Basic Build

#### Method 1: Using automated script (Recommended)

```bash
# Basic Release build
./build.sh

# Build with submodules
./build.sh -s

# Debug build with verbose output
./build.sh -t Debug -v

# Clean and rebuild
./build.sh -c

# See all options
./build.sh --help

# List available toolchains
./build.sh -l
```

#### Method 2: Using CMake directly

```bash
# Initialize submodules first
git submodule update --init --recursive

# Create build directory
mkdir build
cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Release ..

# Build
cmake --build . -j$(nproc)

# Install (optional)
sudo cmake --install .
```

### Cross-Compilation

#### For ARM (using automated script)

```bash
# Cross-compilation for ARM
./build.sh -x arm-linux-gnueabihf -s

# Cross-compilation for ARM in Debug mode
./build.sh -x arm-linux-gnueabihf -t Debug -v
```

#### For RISC-V (using automated script)

```bash
# Cross-compilation for RISC-V 64-bit
./build.sh -x riscv64-linux-gnu -s

# Cross-compilation for RISC-V with custom installation
./build.sh -x riscv64-linux-gnu -p /opt/spp-riscv
```

#### For ESP32-S3 (using automated script)

```bash
# Configure ESP-IDF first
source $IDF_PATH/export.sh

# Cross-compilation for ESP32-S3
./build.sh -x esp32s3 -s

# Cross-compilation for ESP32-S3 in Debug mode
./build.sh -x esp32s3 -t Debug -v
```

## Building and Running Examples

### Building Examples for Linux

```bash
# Build library and examples
./build.sh -s -t Debug

# Enable examples in existing build
cd build
cmake -DSPP_BUILD_EXAMPLES=ON ..
make

# Run the basic example
./examples/spp_basic_example
```

**Expected Output:**
```
Initializing Solaris Packet Protocol...
Initializing Data Bank
Data Bank initialized with 100 packets
SPP initialized successfully.
Packet obtained from databank.
Packet configured: ID=42, Timestamp=1234567890, Data=0xDEADBEEF
Packet returned to databank.
SPP example completed successfully.
```

### Building Examples for Cross-Compilation

```bash
# For ESP32-S3
./build.sh -x esp32s3 -t Debug
cd build-esp32s3
cmake -DSPP_BUILD_EXAMPLES=ON ..
make

# The example binary will be generated but cannot run on host
# It's compiled for ESP32-S3 architecture
ls examples/spp_basic_example  # This is an ESP32-S3 binary
```

### Running Cross-Compiled Examples

Cross-compiled examples cannot run directly on the host system. They need to be:

1. **For ESP32-S3**: Integrated into an ESP-IDF project and flashed to hardware
2. **For ARM/RISC-V**: Run on target hardware or using QEMU emulation

#### Using QEMU for ARM (if available)

```bash
# Install QEMU (if not already installed)
sudo apt-get install qemu-user-static

# Run ARM binary (if ARM toolchain was used)
qemu-arm-static -L /usr/arm-linux-gnueabihf/ ./build-arm-linux-gnueabihf/examples/spp_basic_example
```

## Understanding Generated Files

After building, you'll find these important files:

### Library Files

```
build/                          # Native Linux build
├── core/
│   └── libspp_core.a          # Core functionality library
├── services/databank/
│   └── libspp_databank.a      # DataBank service library
├── examples/
│   └── spp_basic_example      # Example executable (Linux)
├── spp-config.cmake           # CMake package configuration
└── spp-config-version.cmake   # Version information

build-esp32s3/                  # ESP32-S3 cross-compiled build
├── core/
│   └── libspp_core.a          # Core library (Xtensa architecture)
├── services/databank/
│   └── libspp_databank.a      # DataBank library (Xtensa architecture)
└── examples/
    └── spp_basic_example      # Example binary (ESP32-S3, cannot run on host)
```

### File Descriptions

| File | Description | Usage |
|------|-------------|-------|
| `libspp_core.a` | Core SPP functionality | Link this in your project |
| `libspp_databank.a` | Packet pool management | Link this for packet handling |
| `spp-config.cmake` | CMake package config | Used by `find_package(spp)` |
| `spp_basic_example` | Example executable | Reference implementation |

### Verifying Architecture

```bash
# Check native Linux libraries
file build/core/libspp_core.a
# Output: build/core/libspp_core.a: current ar archive

# Check ESP32-S3 libraries
xtensa-esp32s3-elf-objdump -f build-esp32s3/core/libspp_core.a
# Output: architecture: xtensa, flags 0x00000011
```

## ESP32-S3 Configuration

### Prerequisites

1. **Install ESP-IDF**:
   ```bash
   # Clone ESP-IDF
   git clone --recursive https://github.com/espressif/esp-idf.git
   cd esp-idf
   ./install.sh esp32s3
   ```

2. **Configure environment**:
   ```bash
   source $IDF_PATH/export.sh
   ```

### Building for ESP32-S3

```bash
# Verify ESP-IDF is configured
echo $IDF_PATH

# Build SPP for ESP32-S3
./build.sh -x esp32s3 -s -t Release

# Files will be generated in build-esp32s3/
```

### Integration with ESP-IDF Project

1. **Copy libraries**:
   ```bash
   cp build-esp32s3/core/libspp_core.a your_esp_project/components/spp/
   cp build-esp32s3/services/databank/libspp_databank.a your_esp_project/components/spp/
   ```

2. **Copy headers**:
   ```bash
   cp -r core/ your_esp_project/components/spp/include/
   cp -r services/ your_esp_project/components/spp/include/
   ```

3. **ESP-IDF component CMakeLists.txt**:
   ```cmake
   # your_esp_project/components/spp/CMakeLists.txt
   idf_component_register(
       SRCS ""
       INCLUDE_DIRS "include"
       REQUIRES freertos
   )
   
   target_link_libraries(${COMPONENT_LIB} INTERFACE
       ${CMAKE_CURRENT_SOURCE_DIR}/libspp_core.a
       ${CMAKE_CURRENT_SOURCE_DIR}/libspp_databank.a
   )
   ```

## Using SPP in Your Own Project

### Method 1: Using CMake find_package (Recommended)

After installing SPP (`sudo cmake --install .` from build directory):

```cmake
# Your project's CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES C)

# Find SPP library
find_package(spp REQUIRED)

# Create your executable
add_executable(my_app main.c)

# Link with SPP
target_link_libraries(my_app PRIVATE spp::spp)
```

### Method 2: Manual Integration

```cmake
# Your project's CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES C)

# Add SPP include directories
include_directories(/path/to/spp/core)
include_directories(/path/to/spp/services)

# Create your executable
add_executable(my_app main.c)

# Link with SPP libraries
target_link_libraries(my_app 
    PRIVATE 
    /path/to/spp/build/core/libspp_core.a
    /path/to/spp/build/services/databank/libspp_databank.a
)
```

### Method 3: Copy Libraries to Your Project

```bash
# Copy libraries
cp build/core/libspp_core.a your_project/libs/
cp build/services/databank/libspp_databank.a your_project/libs/

# Copy headers
cp -r core/ your_project/include/spp/
cp -r services/ your_project/include/spp/
```

```cmake
# Your CMakeLists.txt
add_executable(my_app main.c)
target_include_directories(my_app PRIVATE include)
target_link_libraries(my_app PRIVATE 
    ${CMAKE_SOURCE_DIR}/libs/libspp_core.a
    ${CMAKE_SOURCE_DIR}/libs/libspp_databank.a
)
```

### Method 4: External Project Example

Create a test project outside the SPP directory:

```bash
# Create a new project directory
mkdir my_spp_project
cd my_spp_project

# Create main.c
cat > main.c << 'EOF'
#include <stdio.h>
#include <inttypes.h>
#include "core/core.h"
#include "services/databank/databank.h"

int main() {
    printf("=== My SPP Application ===\n");
    
    retval_t result = Core_Init();
    if (result != SPP_OK) {
        printf("Error initializing SPP: %d\n", result);
        return -1;
    }
    
    spp_packet_t* packet = DataBank_GetPacket();
    if (packet != NULL) {
        packet->packet_id = 123;
        packet->timestamp = 9876543210;
        packet->data = 0xCAFEBABE;
        
        printf("Packet: ID=%d, Timestamp=%" PRIu32 ", Data=0x%" PRIX32 "\n", 
               packet->packet_id, packet->timestamp, packet->data);
        
        DataBank_ReturnPacket(packet);
    }
    
    printf("Application completed successfully!\n");
    return 0;
}
EOF

# Create CMakeLists.txt
cat > CMakeLists.txt << 'EOF'
cmake_minimum_required(VERSION 3.16)
project(MySPPProject LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(my_app main.c)
target_include_directories(my_app PRIVATE /path/to/spp)
target_link_libraries(my_app PRIVATE 
    /path/to/spp/build/core/libspp_core.a
    /path/to/spp/build/services/databank/libspp_databank.a
)
EOF

# Build and run
mkdir build && cd build
cmake .. && make && ./my_app
```

## Library Usage

### Basic Example

```c
#include <stdio.h>
#include <inttypes.h>
#include "core/core.h"
#include "services/databank/databank.h"

int main() {
    // Initialize SPP
    retval_t result = Core_Init();
    if (result != SPP_OK) {
        printf("Error initializing SPP: %d\n", result);
        return -1;
    }
    
    // Get a packet from databank
    spp_packet_t* packet = DataBank_GetPacket();
    if (packet != NULL) {
        // Configure packet
        packet->packet_id = 42;
        packet->timestamp = 1234567890;
        packet->data = 0xDEADBEEF;
        
        printf("Packet configured: ID=%d, Timestamp=%" PRIu32 ", Data=0x%" PRIX32 "\n", 
               packet->packet_id, packet->timestamp, packet->data);
        
        // Return packet when done
        DataBank_ReturnPacket(packet);
    }
    
    return 0;
}
```

### With FreeRTOS

```c
#include "FreeRTOS.h"
#include "task.h"
#include "core/core.h"

void spp_task(void *pvParameters) {
    // Initialize SPP
    Core_Init();
    
    while(1) {
        // Your packet processing logic here
        spp_packet_t* packet = DataBank_GetPacket();
        if (packet != NULL) {
            // Process packet
            DataBank_ReturnPacket(packet);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main() {
    xTaskCreate(spp_task, "SPP Task", 2048, NULL, 5, NULL);
    vTaskStartScheduler();
    return 0;
}
```

## Troubleshooting

### Common Issues

1. **"Toolchain not found"**:
   ```bash
   ./build.sh -l  # List available toolchains
   ```

2. **"IDF_PATH not set"** (ESP32-S3):
   ```bash
   source $IDF_PATH/export.sh
   ```

3. **"Examples not building"**:
   ```bash
   cmake -DSPP_BUILD_EXAMPLES=ON ..
   ```

4. **"Cannot find spp package"**:
   ```bash
   # Install the library first
   cd build && sudo cmake --install .
   ```

5. **"Include files not found"**:
   - Check that you've built the library first: `./build.sh -s`
   - Verify include paths in your CMakeLists.txt
   - Ensure you're using the correct relative paths

6. **"Library files not found"**:
   - Verify library files exist: `ls build/core/libspp_core.a`
   - Check library paths in your CMakeLists.txt
   - Ensure you've built the library before your project

## Contributing

1. Fork the project
2. Create a feature branch (`git checkout -b feature/new-functionality`)
3. Commit your changes (`git commit -am 'Add new functionality'`)
4. Push to the branch (`git push origin feature/new-functionality`)
5. Create a Pull Request

## License

[Specify license here]

## Contact

[Project contact information]


