cmake_minimum_required(VERSION 3.16)
project(spp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------
# Feature flags
# ----------------------------------------------------------------
option(SPP_NO_MALLOC  "Disable dynamic memory allocation" OFF)
option(SPP_NO_STORAGE "Disable storage HAL"               OFF)
option(SPP_BUILD_TESTS "Build Cgreen unit tests (requires host build)" OFF)
option(SPP_PORT "Port to use: posix | freertos | baremetal" "posix")

# ----------------------------------------------------------------
# Services — opt-in (all ON by default)
# ----------------------------------------------------------------
option(SPP_SERVICE_BMP390     "Compile BMP390 pressure sensor service"  ON)
option(SPP_SERVICE_ICM20948   "Compile ICM20948 IMU service"            ON)
option(SPP_SERVICE_DATALOGGER "Compile datalogger (SD card) service"    ON)

# ----------------------------------------------------------------
# Core SPP library (platform-agnostic)
# Sources are co-located with headers — one directory per module.
# ----------------------------------------------------------------
set(SPP_CORE_SOURCES
    core/core.c
    core/error.c
    hal/dispatch.c
    services/service.c
    services/databank/databank.c
    services/pubsub/pubsub.c
    services/log/log.c
    util/crc.c
)

if(SPP_SERVICE_BMP390)
    list(APPEND SPP_CORE_SOURCES services/bmp390/bmp390.c)
endif()
if(SPP_SERVICE_ICM20948)
    list(APPEND SPP_CORE_SOURCES services/icm20948/icm20948.c)
endif()
if(SPP_SERVICE_DATALOGGER)
    list(APPEND SPP_CORE_SOURCES services/datalogger/datalogger.c)
endif()

add_library(spp STATIC ${SPP_CORE_SOURCES})

# Include root is the parent of spp/ so #include "spp/core/packet.h" resolves correctly.
target_include_directories(spp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(SPP_NO_MALLOC)
    target_compile_definitions(spp PUBLIC SPP_NO_MALLOC=1)
endif()
if(SPP_NO_STORAGE)
    target_compile_definitions(spp PUBLIC SPP_NO_STORAGE=1)
endif()

# ----------------------------------------------------------------
# Port selection
# ----------------------------------------------------------------
if(SPP_PORT STREQUAL "posix")
    set(SPP_PORT_SOURCES
        ports/hal/stub/hal_stub.c
    )
    add_library(spp_port STATIC ${SPP_PORT_SOURCES})
    target_include_directories(spp_port PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(spp_port PUBLIC spp pthread)

elseif(SPP_PORT STREQUAL "freertos")
    set(SPP_PORT_SOURCES
        ports/hal/esp32/hal_esp32.c
    )
    add_library(spp_port STATIC ${SPP_PORT_SOURCES})
    target_include_directories(spp_port PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/ports/hal/esp32
    )
    target_link_libraries(spp_port PUBLIC spp)

elseif(SPP_PORT STREQUAL "baremetal")
    set(SPP_PORT_SOURCES
        ports/hal/esp32/hal_esp32_baremetal.c
    )
    add_library(spp_port STATIC ${SPP_PORT_SOURCES})
    target_include_directories(spp_port PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/ports/hal/esp32
    )
    target_link_libraries(spp_port PUBLIC spp)
endif()

# ----------------------------------------------------------------
# Unit tests (host build only)
# ----------------------------------------------------------------
if(SPP_BUILD_TESTS)
    enable_testing()
    find_package(cgreen REQUIRED)
    find_program(CGREEN_RUNNER cgreen-runner REQUIRED)

    set_property(TARGET spp spp_port PROPERTY POSITION_INDEPENDENT_CODE ON)

    set(SPP_TEST_COMPILE_OPTIONS
        -Wall -Wextra -Wpedantic
        --coverage -fprofile-arcs -ftest-coverage
    )

    # Helper to define a test module as a shared library consumed by cgreen-runner.
    function(spp_add_test_module name src)
        add_library(${name} SHARED ${src})
        target_include_directories(${name} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/..
            ${CGREEN_INCLUDE_DIRS}
        )
        target_link_libraries(${name} PRIVATE spp spp_port ${CGREEN_LIBRARIES} pthread)
        target_compile_options(${name} PRIVATE ${SPP_TEST_COMPILE_OPTIONS})
        target_link_options(${name} PRIVATE --coverage)
        add_test(NAME ${name} COMMAND ${CGREEN_RUNNER} $<TARGET_FILE:${name}>)
    endfunction()

    spp_add_test_module(spp_test_core     tests/core/test_core.c)
    spp_add_test_module(spp_test_util     tests/util/test_crc.c)
    spp_add_test_module(spp_test_databank tests/services/databank/test_databank.c)
    spp_add_test_module(spp_test_pubsub   tests/services/pubsub/test_pubsub.c)
    spp_add_test_module(spp_test_log      tests/services/log/test_log.c)
    spp_add_test_module(spp_test_service  tests/services/test_service.c)

    set_tests_properties(
        spp_test_databank spp_test_pubsub spp_test_log spp_test_service
        PROPERTIES LABELS "services"
    )
endif()
