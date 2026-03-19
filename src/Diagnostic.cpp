// Rust C/C++ Profiler - Diagnostic Implementation
// Rust C/C++ 分析器 - 诊断实现

#include "tcc/Diagnostic.h"
#include <iostream>
#include <sstream>

namespace tcc {

Diagnostic::Diagnostic(Severity severity,
                       std::string message,
                       SourceLocation location,
                       RuleCategory category,
                       std::string ruleId)
    : severity_(severity)
    , message_(std::move(message))
    , location_(std::move(location))
    , category_(category)
    , ruleId_(std::move(ruleId)) {}

void Diagnostic::addFixHint(std::string hint) {
    fixHints_.push_back(std::move(hint));
}

void Diagnostic::addEscapePath(std::string escape) {
    escapePaths_.push_back(std::move(escape));
}

std::string Diagnostic::format() const {
    std::ostringstream oss;
    
    // Location / 位置
    oss << location_.filename << ":" 
        << location_.line << ":" 
        << location_.column << ": ";
    
    // Severity / 严重程度
    switch (severity_) {
        case Severity::Error:
            oss << "error / 错误: ";
            break;
        case Severity::Warning:
            oss << "warning / 警告: ";
            break;
        case Severity::Note:
            oss << "note / 提示: ";
            break;
    }
    
    // Message and rule ID / 消息和规则ID
    oss << message_ << " [" << ruleId_ << "]\n";
    
    // Fix hints / 修复建议
    if (!fixHints_.empty()) {
        oss << "  Fix suggestions / 修复建议:\n";
        for (const auto& hint : fixHints_) {
            oss << "    → " << hint << "\n";
        }
    }
    
    // Escape paths / 逃生路径
    if (!escapePaths_.empty()) {
        oss << "  Opt-out options / 退出选项:\n";
        for (const auto& escape : escapePaths_) {
            oss << "    ⚠ " << escape << "\n";
        }
    }
    
    return oss.str();
}

void DiagnosticEngine::report(Diagnostic diag) {
    diagnostics_.push_back(std::move(diag));
}

bool DiagnosticEngine::hasErrors() const {
    for (const auto& diag : diagnostics_) {
        if (diag.getSeverity() == Severity::Error) {
            return true;
        }
    }
    return false;
}

size_t DiagnosticEngine::getErrorCount() const {
    size_t count = 0;
    for (const auto& diag : diagnostics_) {
        if (diag.getSeverity() == Severity::Error) {
            ++count;
        }
    }
    return count;
}

void DiagnosticEngine::printAll(std::ostream& os) const {
    for (const auto& diag : diagnostics_) {
        os << diag.format() << "\n";
    }
}

void DiagnosticEngine::clear() {
    diagnostics_.clear();
}

} // namespace tcc

