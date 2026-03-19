// Rust C/C++ Profiler - Ownership Rules
// Rust C/C++ 分析器 - 所有权规则
//
// Rules for detecting ownership violations
// 用于检测所有权违规的规则

#pragma once

#include "tcc/Rule.h"
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>

namespace tcc {

// Rule: Forbid 'new' operator / 规则：禁止 'new' 操作符
class ForbidNewRule : public OwnershipRule {
public:
    ForbidNewRule()
        : OwnershipRule("TCC-OWN-001", 
                       "Use of 'new' operator forbidden / 禁止使用 'new' 操作符") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check specific new expression / 检查特定的 new 表达式
    void checkNewExpr(clang::CXXNewExpr* expr,
                     clang::ASTContext& context,
                     DiagnosticEngine& diagnostics);
};

// Rule: Forbid 'delete' operator / 规则：禁止 'delete' 操作符
class ForbidDeleteRule : public OwnershipRule {
public:
    ForbidDeleteRule()
        : OwnershipRule("TCC-OWN-002",
                       "Use of 'delete' operator forbidden / 禁止使用 'delete' 操作符") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check specific delete expression / 检查特定的 delete 表达式
    void checkDeleteExpr(clang::CXXDeleteExpr* expr,
                        clang::ASTContext& context,
                        DiagnosticEngine& diagnostics);
};

// Rule: Forbid malloc/free / 规则：禁止 malloc/free
class ForbidMallocFreeRule : public OwnershipRule {
public:
    ForbidMallocFreeRule()
        : OwnershipRule("TCC-OWN-003",
                       "Use of malloc/free forbidden / 禁止使用 malloc/free") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
};

// Rule: Detect raw owning pointers / 规则：检测原始所有权指针
class RawOwningPointerRule : public OwnershipRule {
public:
    RawOwningPointerRule()
        : OwnershipRule("TCC-OWN-004",
                       "Raw owning pointer detected / 检测到原始所有权指针") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if function returns raw pointer (potential ownership)
    // 检查函数是否返回原始指针（潜在所有权）
    bool isOwningPointerReturn(clang::FunctionDecl* decl) const;
};

} // namespace tcc

