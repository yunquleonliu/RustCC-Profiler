// Tough C Profiler - Core Definitions
// Tough C 分析器 - 核心定义
// 
// Core types and constants for the Tough C profiler system
// Tough C 分析器系统的核心类型和常量

#pragma once

#include <string>
#include <vector>
#include <memory>

namespace tcc {

// Version information / 版本信息
constexpr const char* VERSION = "0.1.0-MVP";
constexpr int VERSION_MAJOR = 0;
constexpr int VERSION_MINOR = 1;
constexpr int VERSION_PATCH = 0;

// Exit codes / 退出码
enum class ExitCode : int {
    Success = 0,              // All checks passed / 所有检查通过
    RuleViolation = 1,        // TCC rules violated / 违反 TCC 规则
    CompilationError = 2,     // C++ syntax errors / C++ 语法错误
    InternalError = 3,        // Tool internal error / 工具内部错误
    InvalidArguments = 4,     // Invalid command arguments / 无效命令参数
    FileNotFound = 5          // Input file not found / 输入文件未找到
};

// Severity levels for diagnostics / 诊断严重程度级别
enum class Severity {
    Error,    // Must fix / 必须修复
    Warning,  // Should fix / 应该修复
    Note      // Informational / 信息提示
};

// Rule categories / 规则类别
enum class RuleCategory {
    Ownership,     // Ownership and smart pointers / 所有权和智能指针
    Lifetime,      // Reference and pointer lifetime / 引用和指针生命周期
    Concurrency,   // Thread safety / 线程安全
    Safety,        // Safety patterns (Option, Result, etc.) / 安全模式
    TypeSafety     // Type system safety / 类型系统安全 (future)
};

// Forward declarations / 前向声明
class Diagnostic;
class RuleEngine;
class ASTVisitor;

} // namespace tcc
