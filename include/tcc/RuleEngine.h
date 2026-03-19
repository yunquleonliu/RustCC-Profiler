// Rust C/C++ Profiler - Rule Engine
// Rust C/C++ 分析器 - 规则引擎
//
// Orchestrates rule checking across the AST
// 协调整个 AST 的规则检查

#pragma once

#include "tcc/Core.h"
#include "tcc/Diagnostic.h"
#include "tcc/Rule.h"
#include <clang/AST/ASTContext.h>
#include <string>
#include <vector>

namespace tcc {

// Main rule engine / 主规则引擎
class RuleEngine {
public:
    RuleEngine();
    
    // Initialize with default rules / 使用默认规则初始化
    void initializeDefaultRules();
    
    // Add custom rule / 添加自定义规则
    void addRule(std::unique_ptr<Rule> rule);
    
    // Run all rules on AST / 在 AST 上运行所有规则
    void analyze(clang::ASTContext& context, DiagnosticEngine& diagnostics);
    
    // Enable/disable rule categories / 启用/禁用规则类别
    void enableCategory(RuleCategory category, bool enabled = true);
    bool isCategoryEnabled(RuleCategory category) const;
    
    // Get statistics / 获取统计信息
    size_t getRuleCount() const;
    size_t getActiveRuleCount() const;

    // Performance: stop after this many errors (0 = unlimited)
    // 性能：在达到此错误数后停止（0 表示不限制）
    void setMaxErrors(size_t n) { maxErrors_ = n; }
    size_t getMaxErrors() const { return maxErrors_; }

private:
    std::vector<std::unique_ptr<Rule>> rules_;
    bool ownershipEnabled_    = true;
    bool lifetimeEnabled_     = true;
    bool concurrencyEnabled_  = true;
    bool safetyEnabled_       = true;  // Option/Result/Borrow / Rust 安全模式
    size_t maxErrors_         = 100;  // Stop after this many errors / 达到此错误数后停止
};

} // namespace tcc

