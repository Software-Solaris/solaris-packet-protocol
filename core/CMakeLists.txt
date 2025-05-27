# CMakeLists.txt for SPP Core module

# Define core source files
set(SPP_CORE_SOURCES
    core.c
)

# Define core header files
set(SPP_CORE_HEADERS
    core.h
    macros.h
    returntypes.h
)

# Create core library
add_library(spp_core STATIC ${SPP_CORE_SOURCES})

# Configure include directories
target_include_directories(spp_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../services>
        $<INSTALL_INTERFACE:include/spp/core>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link with dependencies if any
target_link_libraries(spp_core
    PUBLIC
        spp_services
)

# Configure library properties
set_target_properties(spp_core PROPERTIES
    OUTPUT_NAME "spp_core"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Configure compilation definitions
target_compile_definitions(spp_core
    PRIVATE
        SPP_CORE_BUILDING
)

# Configure specific compilation options
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(spp_core PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# Install core headers
install(FILES ${SPP_CORE_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spp/core
) 