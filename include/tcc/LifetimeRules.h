// Rust C/C++ Profiler - Lifetime Rules
// Rust C/C++ 分析器 - 生命周期规则
//
// Rules for detecting lifetime violations
// 用于检测生命周期违规的规则

#pragma once

#include "tcc/Rule.h"
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Expr.h>

namespace tcc {

// Rule: Forbid returning reference to local variable
// 规则：禁止返回局部变量的引用
class ForbidDanglingRefRule : public LifetimeRule {
public:
    ForbidDanglingRefRule()
        : LifetimeRule("TCC-LIFE-001",
                      "Returning reference to local variable / "
                      "返回局部变量的引用") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check specific return statement / 检查特定的 return 语句
    void checkReturnStmt(clang::ReturnStmt* stmt,
                        clang::FunctionDecl* func,
                        clang::ASTContext& context,
                        DiagnosticEngine& diagnostics);
};

// Rule: Forbid returning pointer to local variable
// 规则：禁止返回局部变量的指针
class ForbidDanglingPtrRule : public LifetimeRule {
public:
    ForbidDanglingPtrRule()
        : LifetimeRule("TCC-LIFE-002",
                      "Returning pointer to local variable / "
                      "返回局部变量的指针") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check specific return statement / 检查特定的 return 语句
    void checkReturnStmt(clang::ReturnStmt* stmt,
                        clang::FunctionDecl* func,
                        clang::ASTContext& context,
                        DiagnosticEngine& diagnostics);
};

// Rule: Forbid containers storing raw pointers
// 规则：禁止容器存储原始指针
class ForbidRawPtrContainerRule : public LifetimeRule {
public:
    ForbidRawPtrContainerRule()
        : LifetimeRule("TCC-LIFE-003",
                      "Container storing raw pointers / "
                      "容器存储原始指针") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if type is a container of raw pointers
    // 检查类型是否是原始指针的容器
    bool isRawPointerContainer(clang::QualType type) const;
};

// Rule: Forbid reference members without clear lifetime
// 规则：禁止没有明确生命周期的引用成员
class ForbidUntrackedRefMemberRule : public LifetimeRule {
public:
    ForbidUntrackedRefMemberRule()
        : LifetimeRule("TCC-LIFE-004",
                      "Reference member without clear lifetime / "
                      "没有明确生命周期的引用成员") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
};

} // namespace tcc

