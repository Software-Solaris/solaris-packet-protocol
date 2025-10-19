# FreeRTOS OSAL Implementation Example CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# FreeRTOS OSAL Implementation Library
add_library(spp_osal_freertos_impl STATIC
    freertos_osal_impl.c
)

# Add include directories
target_include_directories(spp_osal_freertos_impl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

# Link with OSAL library
target_link_libraries(spp_osal_freertos_impl PUBLIC 
    spp_osal
    spp_core
)

# FreeRTOS-specific configuration
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/OS/FreeRTOS")
    target_include_directories(spp_osal_freertos_impl PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/OS/FreeRTOS/Source/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/OS/FreeRTOS/Source/portable
    )
    target_compile_definitions(spp_osal_freertos_impl PUBLIC SPP_FREERTOS_AVAILABLE=1)
    
    # Link with FreeRTOS if available
    if(TARGET freertos)
        target_link_libraries(spp_osal_freertos_impl PUBLIC freertos)
    endif()
else()
    target_compile_definitions(spp_osal_freertos_impl PUBLIC SPP_FREERTOS_AVAILABLE=0)
endif()

# ESP32-specific FreeRTOS support
if(ESP_PLATFORM)
    target_link_libraries(spp_osal_freertos_impl PUBLIC
        idf::freertos
        idf::esp_common
        idf::log
    )
    target_compile_definitions(spp_osal_freertos_impl PUBLIC 
        SPP_FREERTOS_AVAILABLE=1
        ESP_PLATFORM=1
    )
endif()

# Set properties
set_target_properties(spp_osal_freertos_impl PROPERTIES
    VERSION ${PROJECT_VERSION}
    EXPORT_NAME osal_freertos_impl
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS spp_osal_freertos_impl
    EXPORT spp-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(FILES freertos_osal_impl.h
    DESTINATION include/spp/examples/osal/freertos
) 