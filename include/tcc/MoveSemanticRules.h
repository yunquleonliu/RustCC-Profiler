// Tough C Profiler - Move Semantic Rules
// Tough C 分析器 - 移动语义规则
//
// Rules for detecting use-after-move violations (Rust-style)
// 用于检测移动后使用违规的规则（Rust 风格）

#pragma once

#include "tcc/Rule.h"
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/ExprCXX.h>
#include <map>
#include <set>

namespace tcc {

// Variable state tracking for move semantics
// 移动语义的变量状态追踪
enum class VariableState {
    Owned,      // Has ownership / 拥有所有权
    Moved,      // Has been moved / 已移动
    Borrowed,   // Is borrowed / 被借用
    Invalid     // Invalid (freed or post-move) / 无效
};

// Rule: TCC-OWN-005 - Detect use after move
// 规则：TCC-OWN-005 - 检测移动后使用
class UseAfterMoveRule : public OwnershipRule {
public:
    UseAfterMoveRule()
        : OwnershipRule("TCC-OWN-005",
                       "Use after move detected / 检测到移动后使用") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Track variable states / 追踪变量状态
    void trackMoveOperation(const clang::CXXConstructExpr* expr);
    void trackMoveCall(const clang::CallExpr* expr);
    
    // Check if variable is accessed after move / 检查变量是否在移动后被访问
    bool isUsedAfterMove(const clang::DeclRefExpr* ref) const;
    
private:
    // State tracking / 状态追踪
    std::map<const clang::VarDecl*, VariableState> variable_states_;
    std::set<const clang::VarDecl*> moved_variables_;
};

// Rule: TCC-OWN-006 - Detect double move
// 规则：TCC-OWN-006 - 检测双重移动
class DoubleMoveRule : public OwnershipRule {
public:
    DoubleMoveRule()
        : OwnershipRule("TCC-OWN-006",
                       "Variable moved more than once / 变量被多次移动") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Detect multiple move operations on same variable
    // 检测同一变量的多次移动操作
    bool isMovedMultipleTimes(const clang::VarDecl* var) const;
};

// Rule: TCC-OWN-007 - Enforce move semantics for RAII types
// 规则：TCC-OWN-007 - 对 RAII 类型强制移动语义
class EnforceMoveSemanticsRule : public OwnershipRule {
public:
    EnforceMoveSemanticsRule()
        : OwnershipRule("TCC-OWN-007",
                       "RAII type requires move semantics / "
                       "RAII 类型需要移动语义") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if type should have move semantics / 检查类型是否应具有移动语义
    bool requiresMovSemantics(const clang::CXXRecordDecl* record) const;
    
    // Check if type has proper move constructor/assignment
    // 检查类型是否有正确的移动构造/赋值
    bool hasProperMoveSemantics(const clang::CXXRecordDecl* record) const;
};

} // namespace tcc
