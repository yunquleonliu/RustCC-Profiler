// Rust C/C++ Profiler - Concurrency Rules
// Rust C/C++ 分析器 - 并发规则
//
// Rules for detecting thread safety violations
// 用于检测线程安全违规的规则

#pragma once

#include "tcc/Rule.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>

namespace tcc {

// Rule: Forbid unsynchronized shared mutable state
// 规则：禁止非同步共享可变状态
class ForbidUnsyncSharedStateRule : public ConcurrencyRule {
public:
    ForbidUnsyncSharedStateRule()
        : ConcurrencyRule("TCC-CONC-001",
                         "Unsynchronized shared mutable state / "
                         "非同步共享可变状态") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if variable is mutable and potentially shared
    // 检查变量是否可变且可能被共享
    bool isMutableSharedState(clang::VarDecl* decl) const;
};

// Rule: Forbid capturing non-const references in thread lambda
// 规则：禁止在线程 lambda 中捕获非 const 引用
class ForbidNonConstLambdaCaptureRule : public ConcurrencyRule {
public:
    ForbidNonConstLambdaCaptureRule()
        : ConcurrencyRule("TCC-CONC-002",
                         "Capturing non-const reference in thread lambda / "
                         "在线程 lambda 中捕获非 const 引用") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
};

// Rule: Forbid sharing raw pointers across threads
// 规则：禁止跨线程共享原始指针
class ForbidRawPtrThreadSharingRule : public ConcurrencyRule {
public:
    ForbidRawPtrThreadSharingRule()
        : ConcurrencyRule("TCC-CONC-003",
                         "Sharing raw pointer across threads / "
                         "跨线程共享原始指针") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
};

// Rule: Require std::atomic for shared counters
// 规则：共享计数器需要 std::atomic
class RequireAtomicForSharedCounterRule : public ConcurrencyRule {
public:
    RequireAtomicForSharedCounterRule()
        : ConcurrencyRule("TCC-CONC-004",
                         "Non-atomic shared counter / "
                         "非原子共享计数器") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
};

} // namespace tcc

