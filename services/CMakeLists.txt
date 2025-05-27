# CMakeLists.txt for SPP Services module

# Include service subdirectories
add_subdirectory(databank)

# Create services library that groups all services
add_library(spp_services INTERFACE)

# Link all services
target_link_libraries(spp_services
    INTERFACE
        spp_databank
)

# Configure include directories
target_include_directories(spp_services
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../core>
        $<INSTALL_INTERFACE:include/spp/services>
)

# Configure library properties
set_target_properties(spp_services PROPERTIES
    OUTPUT_NAME "spp_services"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
) 