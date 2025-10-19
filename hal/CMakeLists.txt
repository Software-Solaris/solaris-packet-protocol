# Hardware Abstraction Layer (HAL) CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# HAL Library
add_library(spp_hal INTERFACE)

# Add include directories
target_include_directories(spp_hal INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/..
)

# Add subdirectories
add_subdirectory(spi)

# Link with core library
target_link_libraries(spp_hal INTERFACE spp_hal_spi)

# Set properties
set_target_properties(spp_hal PROPERTIES
    VERSION ${PROJECT_VERSION}
    EXPORT_NAME hal
)

# Installation

# Install headers
install(DIRECTORY $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/
    DESTINATION include/spp/hal
    FILES_MATCHING PATTERN "*.h"
) 