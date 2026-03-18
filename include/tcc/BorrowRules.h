// Tough C Profiler - Borrow Checker Rules
// Tough C 分析器 - 借用检查规则
//
// Rules for detecting borrow violations (Rust-style)
// 用于检测借用违规的规则（Rust 风格）

#pragma once

#include "tcc/Rule.h"
#include <clang/AST/Expr.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Stmt.h>
#include <vector>
#include <map>

namespace tcc {

// Borrow type classification / 借用类型分类
enum class BorrowType {
    None,           // Not borrowed / 未借用
    Immutable,      // const T* or const T& / 不可变借用
    Mutable         // T* or T& / 可变借用
};

// Borrow information / 借用信息
struct BorrowInfo {
    const clang::ValueDecl* borrower;     // 借用者
    const clang::ValueDecl* owner;        // 所有者
    BorrowType type;                      // 借用类型
    clang::SourceLocation location;       // 位置
    bool is_active;                       // 是否活跃
};

// Rule: TCC-BORROW-001 - Detect conflicting borrows (mutable + immutable)
// 规则：TCC-BORROW-001 - 检测冲突的借用（可变 + 不可变）
class ConflictingBorrowRule : public LifetimeRule {
public:
    ConflictingBorrowRule()
        : LifetimeRule("TCC-BORROW-001",
                      "Conflicting mutable and immutable borrows / "
                      "可变和不可变借用冲突") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Track borrow operations / 追踪借用操作
    void trackBorrow(const clang::ValueDecl* borrower,
                    const clang::ValueDecl* owner,
                    BorrowType type,
                    clang::SourceLocation loc);
    
    // Detect if borrows conflict / 检测借用是否冲突
    bool detectConflicts() const;
    
private:
    std::vector<BorrowInfo> active_borrows_;
    std::map<const clang::ValueDecl*, std::vector<BorrowInfo>> borrow_graph_;
};

// Rule: TCC-BORROW-002 - Detect borrow outliving owner
// 规则：TCC-BORROW-002 - 检测借用超出所有者生命周期
class BorrowOutlivesOwnerRule : public LifetimeRule {
public:
    BorrowOutlivesOwnerRule()
        : LifetimeRule("TCC-BORROW-002",
                      "Borrow outlives owner / 借用超出所有者生命周期") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if borrow can outlive its owner / 检查借用是否可能超出其所有者
    bool canOutlive(const clang::ValueDecl* borrower,
                   const clang::ValueDecl* owner,
                   clang::ASTContext& context) const;
};

// Rule: TCC-BORROW-003 - Detect multiple mutable borrows
// 规则：TCC-BORROW-003 - 检测多个可变借用
class MultipleMutableBorrowRule : public LifetimeRule {
public:
    MultipleMutableBorrowRule()
        : LifetimeRule("TCC-BORROW-003",
                      "Multiple mutable borrows detected / "
                      "检测到多个可变借用") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if multiple mutable borrows exist simultaneously
    // 检查是否同时存在多个可变借用
    bool hasMultipleMutableBorrows(const clang::ValueDecl* owner) const;
};

// Rule: TCC-BORROW-004 - Detect borrow during modification
// 规则：TCC-BORROW-004 - 检测修改期间的借用
class BorrowDuringModificationRule : public LifetimeRule {
public:
    BorrowDuringModificationRule()
        : LifetimeRule("TCC-BORROW-004",
                      "Borrowed value modified / 借用的值被修改") {}
    
    void check(clang::ASTContext& context, DiagnosticEngine& diagnostics) override;
    
    // Check if borrowed value is being modified / 检查借用的值是否被修改
    bool isBorrowedAndModified(const clang::Expr* expr) const;
};

} // namespace tcc
