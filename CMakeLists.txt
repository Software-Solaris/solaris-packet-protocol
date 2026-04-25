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
option(SPP_SERVICE_BMP390     "Compile BMP390 pressure sensor service"  OFF)
option(SPP_SERVICE_ICM20948   "Compile ICM20948 IMU service"            OFF)
option(SPP_SERVICE_DATALOGGER "Compile datalogger (SD card) service"    OFF)
option(SPP_SERVICE_SX1261     "Compile sx1261 service"                  OFF)
option(SPP_SERVICE_KALMAN     "Compile kalman service"                   ON)



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
if(SPP_SERVICE_SX1261)
    list(APPEND SPP_CORE_SOURCES services/sx1261/sx1261.c)
endif()
if(SPP_SERVICE_KALMAN)
    list(APPEND SPP_CORE_SOURCES services/kalman/kalman.c)
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
        ports/hal/stub/halStub.c
    )
    add_library(spp_port STATIC ${SPP_PORT_SOURCES})
    target_include_directories(spp_port PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(spp_port PUBLIC spp pthread)

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

    # spp without hal/dispatch.c — lets mocks.c provide HAL symbols without conflict.
    set(SPP_TESTABLE_SOURCES ${SPP_CORE_SOURCES})
    list(FILTER SPP_TESTABLE_SOURCES EXCLUDE REGEX "hal/dispatch\\.c$")
    add_library(spp_testable STATIC ${SPP_TESTABLE_SOURCES})
    set_property(TARGET spp_testable PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_include_directories(spp_testable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_compile_options(spp_testable PRIVATE ${SPP_TEST_COMPILE_OPTIONS})
    target_link_options(spp_testable PRIVATE --coverage)

    # Port stub linked against spp_testable (not spp) to avoid pulling dispatch.c.
    add_library(spp_port_testable STATIC ports/hal/stub/halStub.c)
    set_property(TARGET spp_port_testable PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_include_directories(spp_port_testable PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(spp_port_testable PUBLIC spp_testable pthread)
    target_compile_options(spp_port_testable PRIVATE ${SPP_TEST_COMPILE_OPTIONS})
    target_link_options(spp_port_testable PRIVATE --coverage)

    add_executable(spp_tests
        tests/run_all_tests.c
        tests/core/test_core.c
        tests/services/services_register_test.c
        tests/services/kalman/test_kalman.c
        tests/mocks.c
    )
    target_include_directories(spp_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CGREEN_INCLUDE_DIRS}
    )
    target_link_libraries(spp_tests PRIVATE spp_port_testable ${CGREEN_LIBRARIES} pthread)
    target_compile_options(spp_tests PRIVATE ${SPP_TEST_COMPILE_OPTIONS})
    target_link_options(spp_tests PRIVATE --coverage)
    add_test(NAME spp_tests COMMAND spp_tests)
endif()
