# CMakeLists.txt for SPP DataBank service

# Define databank source files
set(SPP_DATABANK_SOURCES
    databank.c
)

# Define databank header files
set(SPP_DATABANK_HEADERS
    databank.h
)

# Create databank library
add_library(spp_databank STATIC ${SPP_DATABANK_SOURCES})

# Configure include directories
target_include_directories(spp_databank
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../core>
        $<INSTALL_INTERFACE:include/spp/services/databank>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Configure library properties
set_target_properties(spp_databank PROPERTIES
    OUTPUT_NAME "spp_databank"
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Configure compilation definitions
target_compile_definitions(spp_databank
    PRIVATE
        SPP_DATABANK_BUILDING
    PUBLIC
        DATA_BANK_SIZE=100  # Define default data bank size
)

# Configure specific compilation options
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(spp_databank PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
    )
endif()

# Install databank headers
install(FILES ${SPP_DATABANK_HEADERS}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/spp/services/databank
) 