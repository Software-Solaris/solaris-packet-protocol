cmake_minimum_required(VERSION 3.16)
project(spp C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ----------------------------------------------------------------
# Feature flags (override with -DSPP_NO_RTOS=1 etc.)
# ----------------------------------------------------------------
option(SPP_NO_RTOS    "Disable RTOS task APIs (use baremetal port)" OFF)
option(SPP_NO_MALLOC  "Disable dynamic memory allocation"           OFF)
option(SPP_NO_STORAGE "Disable storage HAL"                         OFF)
option(SPP_BUILD_TESTS "Build Cgreen unit tests (requires host build)" OFF)
option(SPP_PORT "Port to use: posix | freertos | baremetal" "posix")

# ----------------------------------------------------------------
# Core SPP library (platform-agnostic)
# ----------------------------------------------------------------
file(GLOB_RECURSE SPP_CORE_SOURCES
    "src/core/*.c"
    "src/osal/*.c"
    "src/hal/*.c"
    "src/services/*.c"
    "src/util/*.c"
)

add_library(spp STATIC ${SPP_CORE_SOURCES})

target_include_directories(spp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(SPP_NO_RTOS)
    target_compile_definitions(spp PUBLIC SPP_NO_RTOS=1)
endif()
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
    file(GLOB_RECURSE SPP_PORT_SOURCES
        "ports/posix/osal/*.c"
        "ports/posix/hal/*.c"
    )
    add_library(spp_port STATIC ${SPP_PORT_SOURCES})
    target_include_directories(spp_port PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(spp_port PUBLIC spp pthread)

elseif(SPP_PORT STREQUAL "baremetal")
    file(GLOB_RECURSE SPP_PORT_SOURCES "ports/baremetal/osal/*.c")
    add_library(spp_port STATIC ${SPP_PORT_SOURCES})
    target_include_directories(spp_port PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(spp_port PUBLIC spp)
endif()

# ----------------------------------------------------------------
# Unit tests (host build only)
# ----------------------------------------------------------------
if(SPP_BUILD_TESTS)
    enable_testing()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CGREEN REQUIRED cgreen)

    file(GLOB SPP_TEST_SOURCES "tests/unit/*.c")

    add_executable(spp_tests ${SPP_TEST_SOURCES})

    target_include_directories(spp_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CGREEN_INCLUDE_DIRS}
    )

    target_link_libraries(spp_tests PRIVATE
        spp
        spp_port
        ${CGREEN_LIBRARIES}
        pthread
    )

    target_compile_options(spp_tests PRIVATE
        -Wall -Wextra -Wpedantic
        --coverage          # gcov coverage
        -fprofile-arcs
        -ftest-coverage
    )
    target_link_options(spp_tests PRIVATE --coverage)

    add_test(NAME spp_unit_tests COMMAND spp_tests)
endif()
