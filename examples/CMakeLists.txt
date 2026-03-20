# Rust C/C++ Profiler - Examples CMake
# Rust C/C++ 分析器 - 示例 CMake

# Keep example source files available as analysis inputs.
# These files include both safe and intentionally unsafe patterns,
# so they are not compiled as binaries here.
file(GLOB RCC_EXAMPLE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cc")

if(RCC_EXAMPLE_SOURCES)
    add_custom_target(rcc-examples ALL
        COMMENT "Examples available in ${CMAKE_CURRENT_SOURCE_DIR}"
        SOURCES ${RCC_EXAMPLE_SOURCES}
    )

    install(FILES ${RCC_EXAMPLE_SOURCES}
        DESTINATION share/rcc-check/examples
    )
endif()
