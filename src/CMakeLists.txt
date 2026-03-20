# Rust C/C++ Profiler - Source CMake
# Rust C/C++ 分析器 - 源码 CMake

# Collect all source files / 收集所有源文件
set(RCC_SOURCES
    main.cpp
    Diagnostic.cpp
    Rule.cpp
    FileDetector.cpp
    RuleEngine.cpp
    ASTVisitor.cpp
    OwnershipRules.cpp
    LifetimeRules.cpp
    ConcurrencyRules.cpp
    # Rust-inspired abstraction rules / Rust 风格抽象规则
    MoveSemanticRules.cpp
    BorrowRules.cpp
    SafetyPatternRules.cpp
)

set(CLANG_LIBS
    clangTooling
    clangFrontend
    clangDriver
    clangSerialization
    clangParse
    clangSema
    clangAnalysis
    clangAST
    clangBasic
    clangEdit
    clangLex
    clangRewrite
)

# Build executable / 构建可执行文件
add_executable(rcc-check ${RCC_SOURCES})

# Link libraries / 链接库
target_link_libraries(rcc-check PRIVATE
    ${CLANG_LIBS}
)

# Set output name / 设置输出名称
set_target_properties(rcc-check PROPERTIES
    OUTPUT_NAME "rcc-check"
)

