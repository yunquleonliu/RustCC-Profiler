// Tough C Profiler - Borrow Checker Rules Implementation
// Tough C 分析器 - 借用检查规则实现

#include "tcc/BorrowRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>

namespace tcc {

// Visitor to track borrow operations / 追踪借用操作的访问器
class BorrowTrackingVisitor : public clang::RecursiveASTVisitor<BorrowTrackingVisitor> {
public:
    explicit BorrowTrackingVisitor(ConflictingBorrowRule* rule,
                                  clang::ASTContext& context,
                                  DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    // Visit variable declarations / 访问变量声明
    bool VisitVarDecl(clang::VarDecl* var) {
        if (var->getType()->isPointerType() || var->getType()->isReferenceType()) {
            // Track pointer/reference declarations
            // 追踪指针/引用声明
            if (var->hasInit()) {
                trackBorrowFromInit(var, var->getInit());
            }
        }
        return true;
    }
    
    // Visit assignments / 访问赋值
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        if (op->isAssignmentOp()) {
            // Track borrow assignments / 追踪借用赋值
            // Left side is borrower, right side is owner
            // 左侧是借用者，右侧是所有者
        }
        return true;
    }

private:
    void trackBorrowFromInit(clang::VarDecl* borrower, clang::Expr* init) {
        // Determine borrow type / 确定借用类型
        BorrowType type = BorrowType::None;
        if (borrower->getType().isConstQualified() ||
            (borrower->getType()->isReferenceType() &&
             borrower->getType()->getPointeeType().isConstQualified())) {
            type = BorrowType::Immutable;
        } else {
            type = BorrowType::Mutable;
        }
        
        // Find owner / 查找所有者
        if (auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(
                init->IgnoreParenImpCasts())) {
            if (auto* owner = declRef->getDecl()) {
                rule_->trackBorrow(borrower, owner, type, borrower->getLocation());
            }
        }
    }
    
    ConflictingBorrowRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

//=============================================================================
// ConflictingBorrowRule Implementation
//=============================================================================

void ConflictingBorrowRule::check(clang::ASTContext& context,
                                 DiagnosticEngine& diagnostics) {
    BorrowTrackingVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
    
    // Check for conflicts / 检查冲突
    if (!detectConflicts()) {
        // Report conflicts found by visitor
        // 报告访问器发现的冲突
        for (const auto& [owner, borrows] : borrow_graph_) {
            bool hasMutable = false;
            bool hasImmutable = false;
            
            for (const auto& borrow : borrows) {
                if (borrow.type == BorrowType::Mutable) {
                    hasMutable = true;
                } else if (borrow.type == BorrowType::Immutable) {
                    hasImmutable = true;
                }
            }
            
            if (hasMutable && hasImmutable) {
                auto& sm = context.getSourceManager();
                emitDiag(diagnostics, borrows.front().location, sm,
                         getCategory(), getId(),
                         "Conflicting mutable and immutable borrows / "
                         "可变和不可变借用冲突",
                         Severity::Error);
            }
        }
    }
}

void ConflictingBorrowRule::trackBorrow(const clang::ValueDecl* borrower,
                                       const clang::ValueDecl* owner,
                                       BorrowType type,
                                       clang::SourceLocation loc) {
    BorrowInfo info{borrower, owner, type, loc, true};
    active_borrows_.push_back(info);
    borrow_graph_[owner].push_back(info);
}

bool ConflictingBorrowRule::detectConflicts() const {
    // Check if any owner has conflicting borrows
    // 检查是否有所有者存在冲突的借用
    for (const auto& [owner, borrows] : borrow_graph_) {
        int mutableCount = 0;
        int immutableCount = 0;
        
        for (const auto& borrow : borrows) {
            if (!borrow.is_active) continue;
            
            if (borrow.type == BorrowType::Mutable) {
                mutableCount++;
            } else if (borrow.type == BorrowType::Immutable) {
                immutableCount++;
            }
        }
        
        // Conflict: both mutable and immutable exist
        // 冲突：同时存在可变和不可变借用
        if (mutableCount > 0 && immutableCount > 0) {
            return false;
        }
        
        // Conflict: multiple mutable borrows
        // 冲突：多个可变借用
        if (mutableCount > 1) {
            return false;
        }
    }
    
    return true;
}

//=============================================================================
// BorrowOutlivesOwnerRule Implementation
//=============================================================================

void BorrowOutlivesOwnerRule::check(clang::ASTContext& context,
                                   DiagnosticEngine& diagnostics) {
    // Implement lifetime analysis
    // 实现生命周期分析
    // Check if references outlive their referents
    // 检查引用是否超出其引用对象的生命周期
}

bool BorrowOutlivesOwnerRule::canOutlive(const clang::ValueDecl* borrower,
                                        const clang::ValueDecl* owner,
                                        clang::ASTContext& context) const {
    // Compare scopes / 比较作用域
    // If borrower's scope extends beyond owner's, return true
    // 如果借用者的作用域超出所有者的作用域，返回 true
    return false;  // Placeholder / 占位符
}

//=============================================================================
// MultipleMutableBorrowRule Implementation
//=============================================================================

void MultipleMutableBorrowRule::check(clang::ASTContext& context,
                                     DiagnosticEngine& diagnostics) {
    // Detect multiple mutable references to same object
    // 检测对同一对象的多个可变引用
}

bool MultipleMutableBorrowRule::hasMultipleMutableBorrows(
    const clang::ValueDecl* owner) const {
    // Check borrow graph for multiple mutable borrows
    // 检查借用图中的多个可变借用
    return false;  // Placeholder / 占位符
}

//=============================================================================
// BorrowDuringModificationRule Implementation
//=============================================================================

void BorrowDuringModificationRule::check(clang::ASTContext& context,
                                        DiagnosticEngine& diagnostics) {
    // Detect modifications while borrowed
    // 检测借用期间的修改
}

bool BorrowDuringModificationRule::isBorrowedAndModified(
    const clang::Expr* expr) const {
    // Check if expression is being modified while borrowed
    // 检查表达式是否在借用期间被修改
    return false;  // Placeholder / 占位符
}

} // namespace tcc
