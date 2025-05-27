cmake_minimum_required(VERSION 3.16)

# Solaris Packet Protocol Project
project(SolarisPacketProtocol 
    VERSION 1.0.0
    DESCRIPTION "Solaris Packet Protocol Library"
    LANGUAGES C
)

# C standard configuration
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build options
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(SPP_BUILD_TESTS "Build SPP tests" OFF)
option(SPP_BUILD_EXAMPLES "Build SPP examples" OFF)

# Cross-compilation configuration
if(CMAKE_CROSSCOMPILING)
    message(STATUS "Cross-compiling for ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# Directory configuration
set(SPP_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Include subdirectories
add_subdirectory(core)
add_subdirectory(services)

# Include examples if enabled
if(SPP_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Create main SPP library as interface
add_library(spp INTERFACE)

# Link all sublibraries
target_link_libraries(spp
    INTERFACE
        spp_core
        spp_services
)

# Configure public include directories
target_include_directories(spp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/services>
        $<INSTALL_INTERFACE:include>
)

# Installation configuration
include(GNUInstallDirs)

# Install library
install(TARGETS spp spp_core spp_services spp_databank
    EXPORT spp-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY core/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spp/core
    FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY services/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spp/services
    FILES_MATCHING PATTERN "*.h"
)

# Configure files for find_package
install(EXPORT spp-targets
    FILE spp-targets.cmake
    NAMESPACE spp::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spp
)

# Create configuration file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/spp-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/spp-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/spp-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spp
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/spp-config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/spp-config-version.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spp
)

# Build configuration - only for libraries with implementation
# INTERFACE libraries cannot have these properties

# Cross-compilation specific configurations
if(CMAKE_CROSSCOMPILING)
    # Additional cross-compilation configurations can go here
    message(STATUS "Configuring for cross-compilation")
endif()

# Show configuration information
message(STATUS "SPP Library Configuration:")
message(STATUS "  Version: ${PROJECT_VERSION}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "  Cross-compiling: ${CMAKE_CROSSCOMPILING}") 