# Rust C/C++ Profiler - CMake Configuration
# Rust C/C++ 分析器 - CMake 配置
# Version: 0.1.0-MVP

cmake_minimum_required(VERSION 3.20)
project(RustCCppProfiler 
    VERSION 0.1.0
    LANGUAGES CXX
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

# Find LLVM and Clang / 查找 LLVM 和 Clang
find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "Found Clang in: ${CLANG_INSTALL_PREFIX}")

# LLVM definitions and include dirs / LLVM 定义和包含目录
include_directories(${LLVM_INCLUDE_DIRS})
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Project include directories / 项目包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# Compiler warnings / 编译器警告
if(RCC_ENABLE_WARNINGS)
    if(MSVC)
        add_compile_options(/W4 /WX)
    else()
        add_compile_options(-Wall -Wextra -Wpedantic -Werror)
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

