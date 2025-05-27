# Operating System Abstraction Layer (OSAL) CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# OSAL Library - Changed from INTERFACE to STATIC to include source files
add_library(spp_osal STATIC
    osal.c
    task.c
    queue.c
    mutex.c
    semaphore.c
)

# Add include directories
target_include_directories(spp_osal PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/..
)

# Link with core library for return types
target_link_libraries(spp_osal PUBLIC spp_core)

# Add subdirectories for platform-specific implementations

# Set properties
set_target_properties(spp_osal PROPERTIES
    VERSION ${PROJECT_VERSION}
    EXPORT_NAME osal
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Installation

# Install headers
install(DIRECTORY $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/
    DESTINATION include/spp/osal
    FILES_MATCHING PATTERN "*.h"
) 