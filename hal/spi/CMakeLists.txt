# SPI Hardware Abstraction Layer CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# SPI HAL Library (now with implementation)
add_library(spp_hal_spi STATIC spi.c)

# Add include directories
target_include_directories(spp_hal_spi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
    $<INSTALL_INTERFACE:include>
)

# Link with core library for return types
target_link_libraries(spp_hal_spi PUBLIC spp_core)

# Set properties
set_target_properties(spp_hal_spi PROPERTIES
    VERSION ${PROJECT_VERSION}
    EXPORT_NAME hal_spi
)

# Installation

# Install headers
install(FILES spi.h
    DESTINATION include/spp/hal/spi
) 