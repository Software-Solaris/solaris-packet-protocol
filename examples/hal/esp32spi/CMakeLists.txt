# ESP32-S3 SPI Implementation Example CMakeLists.txt
cmake_minimum_required(VERSION 3.16)

# ESP32 SPI Implementation Library
add_library(spp_hal_esp32_spi STATIC
    esp32_spi.c
)

# Add include directories
target_include_directories(spp_hal_esp32_spi PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../..
)

# Link with HAL SPI library
target_link_libraries(spp_hal_esp32_spi PUBLIC 
    spp_hal_spi
    spp_core
)

# ESP32-specific configuration
if(ESP_PLATFORM)
    # Link with ESP-IDF components
    target_link_libraries(spp_hal_esp32_spi PUBLIC
        idf::driver
        idf::esp_common
        idf::log
    )
    
    target_compile_definitions(spp_hal_esp32_spi PUBLIC ESP_PLATFORM=1)
else()
    # For cross-compilation testing without ESP-IDF
    target_compile_definitions(spp_hal_esp32_spi PUBLIC ESP_PLATFORM=0)
endif()

# Set properties
set_target_properties(spp_hal_esp32_spi PROPERTIES
    VERSION ${PROJECT_VERSION}
    EXPORT_NAME hal_esp32_spi
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)

# Installation
install(TARGETS spp_hal_esp32_spi
    EXPORT spp-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(FILES esp32_spi.h
    DESTINATION include/spp/examples/hal/esp32spi
) 