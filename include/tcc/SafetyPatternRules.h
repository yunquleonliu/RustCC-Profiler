// Rust C/C++ Profiler - Safety Pattern Rules
// Rust C/C++ 分析器 - 安全模式规则
//
// Rules for enforcing Rust-style safety patterns (Option, Result, etc.)
// 用于强制执行 Rust 风格安全模式的规则（Option、Result 等）

#pragma once

#include "tcc/Rule.h"
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Type.h>
#include <string>
#include <set>

namespace tcc {

// Rule: TCC-OPTION-001 - Enforce explicit null checks
// 规则：TCC-OPTION-001 - 强制显式空值检查
class EnforceNullCheckRule : public SafetyRule {
public:
    EnforceNullCheckRule()
        : SafetyRule("TCC-OPTION-001",
                    "Pointer must be checked before use / "
                    "指针在使用前必须检查") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if pointer is used without null check / 检查指针是否未经空值检查使用
    bool isUsedWithoutCheck(const clang::Expr* expr,
                           clang::ASTContext& context) const;
    
    // Recommend using std::optional / 推荐使用 std::optional
    bool shouldUseOptional(const clang::QualType& type) const;
};

// Rule: TCC-OPTION-002 - Prefer std::optional over nullable pointers
// 规则：TCC-OPTION-002 - 优先使用 std::optional 而非可空指针
class PreferOptionalRule : public SafetyRule {
public:
    PreferOptionalRule()
        : SafetyRule("TCC-OPTION-002",
                    "Use std::optional instead of nullable pointer / "
                    "使用 std::optional 而非可空指针") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if function returns nullable pointer / 检查函数是否返回可空指针
    bool returnsNullablePointer(const clang::FunctionDecl* func) const;
};

// Rule: TCC-RESULT-001 - Enforce error handling with Result type
// 规则：TCC-RESULT-001 - 使用 Result 类型强制错误处理
class EnforceResultHandlingRule : public SafetyRule {
public:
    EnforceResultHandlingRule()
        : SafetyRule("TCC-RESULT-001",
                    "Result type must be handled / "
                    "Result 类型必须被处理") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if Result type is ignored / 检查 Result 类型是否被忽略
    bool isResultIgnored(const clang::CallExpr* call) const;
    
    // Check if type is a Result type / 检查类型是否为 Result 类型
    bool isResultType(const clang::QualType& type) const;
};

// Rule: TCC-RESULT-002 - Detect unchecked error returns
// 规则：TCC-RESULT-002 - 检测未检查的错误返回
class UncheckedErrorReturnRule : public SafetyRule {
public:
    UncheckedErrorReturnRule()
        : SafetyRule("TCC-RESULT-002",
                    "Function return value must be checked / "
                    "函数返回值必须被检查") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Mark functions that must have checked returns
    // 标记必须检查返回值的函数
    void markMustUse(const clang::FunctionDecl* func);
    
    // Check if [[nodiscard]] is violated / 检查是否违反 [[nodiscard]]
    bool isNodiscardViolated(const clang::CallExpr* call) const;
};

// Rule: TCC-PANIC-001 - Detect unwrap/panic patterns that can fail
// 规则：TCC-PANIC-001 - 检测可能失败的 unwrap/panic 模式
class DetectUnsafeUnwrapRule : public SafetyRule {
public:
    DetectUnsafeUnwrapRule()
        : SafetyRule("TCC-PANIC-001",
                    "Unsafe unwrap detected / 检测到不安全的 unwrap") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Detect patterns like .value() without checking
    // 检测未经检查的 .value() 等模式
    bool isUnsafeUnwrap(const clang::CXXMemberCallExpr* call) const;
};

// Rule: TCC-PANIC-002 - Forbid direct panic/abort calls
// 规则：TCC-PANIC-002 - 禁止直接调用 panic/abort
class ForbidPanicRule : public SafetyRule {
public:
    ForbidPanicRule()
        : SafetyRule("TCC-PANIC-002",
                    "Direct panic/abort call forbidden / "
                    "禁止直接调用 panic/abort") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Detect calls to abort(), exit(), std::terminate(), etc.
    // 检测对 abort()、exit()、std::terminate() 等的调用
    bool isPanicFunction(const clang::FunctionDecl* func) const;
};

// Rule: TCC-SAFE-001 - Enforce bounds checking for array access
// 规则：TCC-SAFE-001 - 对数组访问强制边界检查
class EnforceBoundsCheckRule : public SafetyRule {
public:
    EnforceBoundsCheckRule()
        : SafetyRule("TCC-SAFE-001",
                    "Array access requires bounds check / "
                    "数组访问需要边界检查") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if array subscript is bounds-checked / 检查数组下标是否经过边界检查
    bool isBoundsChecked(const clang::ArraySubscriptExpr* expr,
                        clang::ASTContext& context) const;
    
    // Recommend using .at() instead of [] for vectors
    // 推荐对 vector 使用 .at() 而非 []
    bool shouldUseAtMethod(const clang::CXXOperatorCallExpr* expr) const;
};

} // namespace tcc

