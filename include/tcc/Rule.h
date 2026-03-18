// Tough C Profiler - Rule System
// Tough C 分析器 - 规则系统
//
// Base classes and interfaces for TCC safety rules
// TCC 安全规则的基类和接口

#pragma once

#include "tcc/Core.h"
#include "tcc/Diagnostic.h"
#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <memory>
#include <string>
#include <vector>

namespace tcc {

// Base class for all TCC rules / 所有 TCC 规则的基类
class Rule {
public:
    explicit Rule(std::string id, std::string description, RuleCategory category)
        : id_(std::move(id))
        , description_(std::move(description))
        , category_(category) {}
    
    virtual ~Rule() = default;
    
    // Rule identification / 规则标识
    const std::string& getId() const { return id_; }
    const std::string& getDescription() const { return description_; }
    RuleCategory getCategory() const { return category_; }
    
    // Check if rule applies to this AST node / 检查规则是否适用于此 AST 节点
    virtual void check(clang::ASTContext& context, 
                      DiagnosticEngine& diagnostics) = 0;

protected:
    std::string id_;
    std::string description_;
    RuleCategory category_;
};

// Rule for ownership checking / 所有权检查规则
class OwnershipRule : public Rule {
public:
    explicit OwnershipRule(std::string id, std::string description)
        : Rule(std::move(id), std::move(description), RuleCategory::Ownership) {}
};

// Rule for lifetime checking / 生命周期检查规则
class LifetimeRule : public Rule {
public:
    explicit LifetimeRule(std::string id, std::string description)
        : Rule(std::move(id), std::move(description), RuleCategory::Lifetime) {}
};

// Rule for concurrency checking / 并发检查规则
class ConcurrencyRule : public Rule {
public:
    explicit ConcurrencyRule(std::string id, std::string description)
        : Rule(std::move(id), std::move(description), RuleCategory::Concurrency) {}
};

// Rule for safety patterns (Option, Result, etc.) / 安全模式规则
class SafetyRule : public Rule {
public:
    explicit SafetyRule(std::string id, std::string description)
        : Rule(std::move(id), std::move(description), RuleCategory::Safety) {}
};

// Rule registry / 规则注册表
class RuleRegistry {
public:
    static RuleRegistry& instance();
    
    // Register a rule / 注册规则
    void registerRule(std::unique_ptr<Rule> rule);
    
    // Get all rules / 获取所有规则
    const std::vector<std::unique_ptr<Rule>>& getRules() const { return rules_; }
    
    // Get rules by category / 按类别获取规则
    std::vector<Rule*> getRulesByCategory(RuleCategory category) const;

private:
    RuleRegistry() = default;
    std::vector<std::unique_ptr<Rule>> rules_;
};

} // namespace tcc
