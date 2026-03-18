// Tough C Profiler - Rule Engine Implementation
// Tough C 分析器 - 规则引擎实现

#include "tcc/RuleEngine.h"
#include "tcc/OwnershipRules.h"
#include "tcc/LifetimeRules.h"
#include "tcc/ConcurrencyRules.h"
#include "tcc/MoveSemanticRules.h"
#include "tcc/BorrowRules.h"
#include "tcc/SafetyPatternRules.h"
#include "tcc/ASTVisitor.h"

namespace tcc {

RuleEngine::RuleEngine() {
    // Constructor / 构造函数
}

void RuleEngine::initializeDefaultRules() {
    // Register ownership rules / 注册所有权规则
    if (ownershipEnabled_) {
        addRule(std::make_unique<ForbidNewRule>());
        addRule(std::make_unique<ForbidDeleteRule>());
        addRule(std::make_unique<ForbidMallocFreeRule>());
        addRule(std::make_unique<RawOwningPointerRule>());
    }
    
    // Register lifetime rules / 注册生命周期规则
    if (lifetimeEnabled_) {
        addRule(std::make_unique<ForbidDanglingRefRule>());
        addRule(std::make_unique<ForbidDanglingPtrRule>());
        addRule(std::make_unique<ForbidRawPtrContainerRule>());
        addRule(std::make_unique<ForbidUntrackedRefMemberRule>());
    }
    
    // Register concurrency rules / 注册并发规则
    if (concurrencyEnabled_) {
        addRule(std::make_unique<ForbidUnsyncSharedStateRule>());
        addRule(std::make_unique<ForbidNonConstLambdaCaptureRule>());
        addRule(std::make_unique<ForbidRawPtrThreadSharingRule>());
        addRule(std::make_unique<RequireAtomicForSharedCounterRule>());
    }

    // Register Rust-inspired safety rules / 注册 Rust 风格安全规则
    if (ownershipEnabled_) {
        // Move semantics rules (extend ownership category)
        // 移动语义规则（扩展所有权类别）
        addRule(std::make_unique<UseAfterMoveRule>());
        addRule(std::make_unique<DoubleMoveRule>());
        addRule(std::make_unique<EnforceMoveSemanticsRule>());
    }
    if (lifetimeEnabled_) {
        // Borrow checker rules (extend lifetime category)
        // 借用检查器规则（扩展生命周期类别）
        addRule(std::make_unique<ConflictingBorrowRule>());
        addRule(std::make_unique<BorrowOutlivesOwnerRule>());
        addRule(std::make_unique<MultipleMutableBorrowRule>());
        addRule(std::make_unique<BorrowDuringModificationRule>());
    }
    if (safetyEnabled_) {
        // Option / Result / panic-safety rules
        // Option / Result / panic 安全规则
        addRule(std::make_unique<EnforceNullCheckRule>());
        addRule(std::make_unique<PreferOptionalRule>());
        addRule(std::make_unique<EnforceResultHandlingRule>());
        addRule(std::make_unique<UncheckedErrorReturnRule>());
        addRule(std::make_unique<DetectUnsafeUnwrapRule>());
        addRule(std::make_unique<ForbidPanicRule>());
        addRule(std::make_unique<EnforceBoundsCheckRule>());
    }
}

void RuleEngine::addRule(std::unique_ptr<Rule> rule) {
    rules_.push_back(std::move(rule));
}

void RuleEngine::analyze(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    // Create AST visitor / 创建 AST 访问者
    TCCASTVisitor visitor(context, rules_, diagnostics);
    
    // Traverse the entire AST / 遍历整个 AST
    visitor.TraverseDecl(context.getTranslationUnitDecl());
    
    // Run all enabled rules / 运行所有启用的规则
    for (const auto& rule : rules_) {
        // Check if rule category is enabled / 检查规则类别是否启用
        if (!isCategoryEnabled(rule->getCategory())) {
            continue;
        }

        // Performance: skip remaining rules if error cap is reached
        // 性能：如果已达到错误上限，跳过剩余规则
        if (maxErrors_ > 0 && diagnostics.getErrorCount() >= maxErrors_) {
            break;
        }
        
        // Execute rule / 执行规则
        rule->check(context, diagnostics);
    }
}

void RuleEngine::enableCategory(RuleCategory category, bool enabled) {
    switch (category) {
        case RuleCategory::Ownership:
            ownershipEnabled_ = enabled;
            break;
        case RuleCategory::Lifetime:
            lifetimeEnabled_ = enabled;
            break;
        case RuleCategory::Concurrency:
            concurrencyEnabled_ = enabled;
            break;
        case RuleCategory::Safety:
            safetyEnabled_ = enabled;
            break;
        default:
            break;
    }
}

bool RuleEngine::isCategoryEnabled(RuleCategory category) const {
    switch (category) {
        case RuleCategory::Ownership:
            return ownershipEnabled_;
        case RuleCategory::Lifetime:
            return lifetimeEnabled_;
        case RuleCategory::Concurrency:
            return concurrencyEnabled_;
        case RuleCategory::Safety:
            return safetyEnabled_;
        default:
            return false;
    }
}

size_t RuleEngine::getRuleCount() const {
    return rules_.size();
}

size_t RuleEngine::getActiveRuleCount() const {
    size_t count = 0;
    for (const auto& rule : rules_) {
        if (isCategoryEnabled(rule->getCategory())) {
            ++count;
        }
    }
    return count;
}

} // namespace tcc
