// Rust C/C++ Profiler - Diagnostic System
// Rust C/C++ 分析器 - 诊断系统
//
// System for collecting and reporting rule violations
// 用于收集和报告规则违规的系统

#pragma once

#include "tcc/Core.h"
#include <string>
#include <vector>

namespace tcc {

// Source location / 源码位置
struct SourceLocation {
    std::string filename;      // File path / 文件路径
    unsigned line;             // Line number (1-based) / 行号（从1开始）
    unsigned column;           // Column number (1-based) / 列号（从1开始）
    
    SourceLocation() : line(0), column(0) {}
    SourceLocation(std::string file, unsigned l, unsigned c)
        : filename(std::move(file)), line(l), column(c) {}
};

// Diagnostic message / 诊断消息
class Diagnostic {
public:
    Diagnostic(Severity severity, 
               std::string message,
               SourceLocation location,
               RuleCategory category,
               std::string ruleId);
    
    // Accessors / 访问器
    Severity getSeverity() const { return severity_; }
    const std::string& getMessage() const { return message_; }
    const SourceLocation& getLocation() const { return location_; }
    RuleCategory getCategory() const { return category_; }
    const std::string& getRuleId() const { return ruleId_; }
    
    // Add suggestion for fixing / 添加修复建议
    void addFixHint(std::string hint);
    const std::vector<std::string>& getFixHints() const { return fixHints_; }
    
    // Add escape path suggestion / 添加逃生路径建议
    void addEscapePath(std::string escape);
    const std::vector<std::string>& getEscapePaths() const { return escapePaths_; }
    
    // Format for output / 格式化输出
    std::string format() const;

private:
    Severity severity_;
    std::string message_;
    SourceLocation location_;
    RuleCategory category_;
    std::string ruleId_;
    std::vector<std::string> fixHints_;      // How to fix / 如何修复
    std::vector<std::string> escapePaths_;   // How to opt-out / 如何退出
};

// Diagnostic collector / 诊断收集器
class DiagnosticEngine {
public:
    DiagnosticEngine() = default;
    
    // Add diagnostic / 添加诊断
    void report(Diagnostic diag);
    
    // Query diagnostics / 查询诊断
    const std::vector<Diagnostic>& getDiagnostics() const { return diagnostics_; }
    
    // Check if any errors / 检查是否有错误
    bool hasErrors() const;
    
    // Get error count / 获取错误数量
    size_t getErrorCount() const;
    
    // Print all diagnostics / 打印所有诊断
    void printAll(std::ostream& os) const;
    
    // Clear all / 清空
    void clear();

private:
    std::vector<Diagnostic> diagnostics_;
};

} // namespace tcc

