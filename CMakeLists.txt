# Rust C/C++ Profiler - CMake Configuration
# Rust C/C++ 分析器 - CMake 配置
# Version: 0.1.0-MVP

cmake_minimum_required(VERSION 3.20)
project(RustCCppProfiler 
    VERSION 0.1.0
    LANGUAGES C CXX
    DESCRIPTION "Pre-compilation safety profiler for C/C++ - 面向 C/C++ 的预编译安全分析器"
)

# C++17 required for modern features / 需要 C++17 支持现代特性
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for tooling / 导出编译命令供工具使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Build options / 构建选项
option(RCC_BUILD_TESTS "Build test suite / 构建测试套件" ON)
option(RCC_BUILD_EXAMPLES "Build examples / 构建示例" ON)
option(RCC_ENABLE_WARNINGS "Enable all compiler warnings / 启用所有编译器警告" ON)
option(RCC_WARNINGS_AS_ERRORS "Treat warnings as errors / 将警告视为错误" OFF)

# Windows auto-discovery for LLVM development package.
# Windows 自动发现 LLVM 开发包。
if(WIN32 AND NOT LLVM_DIR)
    file(GLOB _rcc_user_llvm_dirs "$ENV{USERPROFILE}/llvm-dev/clang+llvm-*-windows-msvc")

    set(_rcc_llvm_roots
        "C:/Program Files/LLVM"
        "C:/Program Files (x86)/LLVM"
        "$ENV{LLVM_HOME}"
    )
    list(APPEND _rcc_llvm_roots ${_rcc_user_llvm_dirs})

    foreach(_rcc_root IN LISTS _rcc_llvm_roots)
        if(_rcc_root AND EXISTS "${_rcc_root}/lib/cmake/llvm/LLVMConfig.cmake")
            set(LLVM_DIR "${_rcc_root}/lib/cmake/llvm" CACHE PATH "Path to LLVM CMake config" FORCE)
            break()
        endif()
    endforeach()
endif()

# Find LLVM and Clang / 查找 LLVM 和 Clang
find_package(LLVM REQUIRED CONFIG)

# If LLVM_DIR is provided, infer sibling Clang_DIR when missing.
# 如果提供了 LLVM_DIR，在缺失时推导同级 Clang_DIR。
if(NOT Clang_DIR AND LLVM_DIR)
    get_filename_component(_rcc_llvm_cmake_parent "${LLVM_DIR}" DIRECTORY)
    set(_rcc_inferred_clang_dir "${_rcc_llvm_cmake_parent}/clang")
    if(EXISTS "${_rcc_inferred_clang_dir}/ClangConfig.cmake")
        set(Clang_DIR "${_rcc_inferred_clang_dir}" CACHE PATH "Path to Clang CMake config" FORCE)
    endif()
endif()

find_package(Clang REQUIRED CONFIG)

# Some prebuilt LLVM packages on Windows encode an absolute dependency on
# Enterprise DIA SDK path. Remap it to BuildTools path when needed.
# 某些 Windows 预编译 LLVM 包会硬编码 Enterprise DIA SDK 路径；
# 当需要时重映射到 BuildTools 路径。
if(WIN32)
    set(_rcc_bad_dia "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/DIA SDK/lib/amd64/diaguids.lib")
    set(_rcc_good_dia "C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/DIA SDK/lib/amd64/diaguids.lib")

    if(NOT EXISTS "${_rcc_bad_dia}" AND EXISTS "${_rcc_good_dia}")
        if(TARGET LLVMDebugInfoPDB)
            get_target_property(_rcc_pdb_links LLVMDebugInfoPDB INTERFACE_LINK_LIBRARIES)
            if(_rcc_pdb_links)
                string(REPLACE "${_rcc_bad_dia}" "${_rcc_good_dia}" _rcc_pdb_links_patched "${_rcc_pdb_links}")
                if(NOT _rcc_pdb_links_patched STREQUAL _rcc_pdb_links)
                    set_target_properties(LLVMDebugInfoPDB PROPERTIES
                        INTERFACE_LINK_LIBRARIES "${_rcc_pdb_links_patched}"
                    )
                endif()
            endif()
        endif()
    endif()
endif()

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Using ClangConfig.cmake in: ${Clang_DIR}")

# LLVM definitions and include dirs / LLVM 定义和包含目录
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Project include directories / 项目包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# Compiler warnings / 编译器警告
if(RCC_ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4)
        if(RCC_WARNINGS_AS_ERRORS)
            add_compile_options(/WX)
        endif()
    else()
        add_compile_options(-Wall -Wextra -Wpedantic)
        if(RCC_WARNINGS_AS_ERRORS)
            add_compile_options(-Werror)
        endif()
    endif()
endif()

# Core library / 核心库
add_subdirectory(src)

# Tests / 测试
if(RCC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples / 示例
if(RCC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation / 安装配置
install(TARGETS rcc-check
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

