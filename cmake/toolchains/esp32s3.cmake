# Toolchain file for ESP32-S3
# Usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/esp32s3.cmake ..

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR xtensa)

# Configure ESP-IDF variables
if(NOT DEFINED ENV{IDF_PATH})
    message(FATAL_ERROR "IDF_PATH environment variable is not set. Please source the ESP-IDF environment.")
endif()

set(IDF_PATH $ENV{IDF_PATH})
set(IDF_TARGET esp32s3)

# Configure Xtensa cross-compiler
set(CMAKE_C_COMPILER xtensa-esp32s3-elf-gcc)
set(CMAKE_CXX_COMPILER xtensa-esp32s3-elf-g++)
set(CMAKE_ASM_COMPILER xtensa-esp32s3-elf-gcc)

# Configure additional tools
set(CMAKE_AR xtensa-esp32s3-elf-ar)
set(CMAKE_RANLIB xtensa-esp32s3-elf-ranlib)
set(CMAKE_OBJCOPY xtensa-esp32s3-elf-objcopy)
set(CMAKE_OBJDUMP xtensa-esp32s3-elf-objdump)
set(CMAKE_SIZE xtensa-esp32s3-elf-size)

# Configure search paths
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# ESP32-S3 specific configurations
set(ESP32S3_CPU_FLAGS 
    "-mlongcalls"
    "-mtext-section-literals"
    "-ffunction-sections"
    "-fdata-sections"
    "-fstrict-volatile-bitfields"
    "-Wall"
    "-Werror=all"
    "-Wno-error=unused-function"
    "-Wno-error=unused-variable"
    "-Wno-error=deprecated-declarations"
    "-Wextra"
    "-Wno-unused-parameter"
    "-Wno-sign-compare"
    "-ggdb"
    "-Os"
    "-freorder-blocks"
    "-fstack-protector"
    "-fno-jump-tables"
    "-fno-tree-switch-conversion"
)

# Apply compilation flags
string(JOIN " " ESP32S3_CPU_FLAGS_STR ${ESP32S3_CPU_FLAGS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ESP32S3_CPU_FLAGS_STR}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ESP32S3_CPU_FLAGS_STR}")

# Configure C-specific flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wno-old-style-declaration")

# Configure C++-specific flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -fno-exceptions -fno-rtti")

# Configure linker flags
set(ESP32S3_LINKER_FLAGS
    "-nostdlib"
    "-Wl,--gc-sections"
    "-Wl,--cref"
    "-Wl,--Map=output.map"
    "-fno-lto"
    "-Wl,--start-group"
    "-lc"
    "-lm"
    "-lgcc"
    "-lstdc++"
    "-Wl,--end-group"
)

string(JOIN " " ESP32S3_LINKER_FLAGS_STR ${ESP32S3_LINKER_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${ESP32S3_LINKER_FLAGS_STR}")

# Preprocessor definitions for ESP32-S3
add_definitions(
    -DESP_PLATFORM
    -DESP32S3
    -DCONFIG_IDF_TARGET_ESP32S3
    -DCONFIG_FREERTOS_HZ=1000
    -DCONFIG_ESP32S3_DEFAULT_CPU_FREQ_MHZ=240
    -DCONFIG_ESP32S3_SPIRAM_SUPPORT
    -DWITH_POSIX
)

# Configure ESP-IDF include directories (basic)
set(ESP_IDF_INCLUDES
    "${IDF_PATH}/components/freertos/include"
    "${IDF_PATH}/components/freertos/port/xtensa/include"
    "${IDF_PATH}/components/esp_common/include"
    "${IDF_PATH}/components/esp_hw_support/include"
    "${IDF_PATH}/components/esp_system/include"
    "${IDF_PATH}/components/hal/include"
    "${IDF_PATH}/components/hal/esp32s3/include"
    "${IDF_PATH}/components/soc/include"
    "${IDF_PATH}/components/soc/esp32s3/include"
    "${IDF_PATH}/components/xtensa/include"
    "${IDF_PATH}/components/xtensa/esp32s3/include"
    "${IDF_PATH}/components/newlib/platform_include"
    "${IDF_PATH}/components/log/include"
)

# Function to configure an ESP32-S3 target
function(configure_esp32s3_target target_name)
    target_include_directories(${target_name} SYSTEM PRIVATE ${ESP_IDF_INCLUDES})
    
    # Configure target-specific properties
    set_target_properties(${target_name} PROPERTIES
        SUFFIX ".elf"
        C_STANDARD 99
        CXX_STANDARD 11
    )
    
    # Add target-specific definitions
    target_compile_definitions(${target_name} PRIVATE
        ESP_PLATFORM=1
        ESP32S3=1
        CONFIG_IDF_TARGET_ESP32S3=1
    )
endfunction()

# Configure CMAKE to not try to run compiled binaries
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Toolchain information
message(STATUS "ESP32-S3 Toolchain Configuration:")
message(STATUS "  IDF_PATH: ${IDF_PATH}")
message(STATUS "  Target: ${IDF_TARGET}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  CXX Compiler: ${CMAKE_CXX_COMPILER}")

# Verify compiler exists
find_program(ESP32S3_GCC_FOUND ${CMAKE_C_COMPILER})
if(NOT ESP32S3_GCC_FOUND)
    message(FATAL_ERROR "ESP32-S3 toolchain not found. Please install ESP-IDF and ensure the toolchain is in PATH.")
endif() 