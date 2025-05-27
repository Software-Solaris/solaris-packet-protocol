# CMakeLists.txt for SPP examples

# Only build examples if enabled
if(SPP_BUILD_EXAMPLES)
    
    # Basic usage example
    add_executable(spp_basic_example basic_usage.c)
    
    # Link with SPP library
    target_link_libraries(spp_basic_example
        PRIVATE
            spp
    )
    
    # Configure include directories
    target_include_directories(spp_basic_example
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/../
    )
    
    # Configure executable properties
    set_target_properties(spp_basic_example PROPERTIES
        OUTPUT_NAME "spp_basic_example"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
    
    # Install examples (optional)
    install(TARGETS spp_basic_example
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
    )
    
    # Show informative message
    message(STATUS "SPP Examples will be built")
    
endif() 