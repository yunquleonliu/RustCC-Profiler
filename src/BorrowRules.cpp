// Rust C/C++ Profiler - Borrow Checker Rules Implementation
// Rust C/C++ 分析器 - 借用检查规则实现

#include "tcc/BorrowRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <map>
#include <vector>
#include <string>

namespace tcc {

// ---------------------------------------------------------------------------
// Shared alias-tracking visitor
// 共享别名追踪访问器
//
// Builds an alias map: owner VarDecl* → list of BorrowInfo
// Detects borrows from:
//   1. T* p = &x;     (UnaryOperator UO_AddrOf → DeclRefExpr)
//   2. T& r = x;      (reference binding → DeclRefExpr)
//   3. p = &x;        (assignment where RHS is AddrOf)
//   4. const T& r = x (immutable borrow from reference init)
// ---------------------------------------------------------------------------
struct BorrowEntry {
    const clang::VarDecl* borrower;
    const clang::VarDecl* owner;
    BorrowType            type;
    clang::SourceLocation location;
};

// Returns: Immutable if ptr/ref is to const, else Mutable.
// 返回：如果 ptr/ref 指向 const，则为 Immutable，否则为 Mutable。
static BorrowType classifyBorrow(clang::QualType qt) {
    if (qt->isReferenceType()) {
        return qt->getPointeeType().isConstQualified()
               ? BorrowType::Immutable : BorrowType::Mutable;
    }
    if (qt->isPointerType()) {
        return qt->getPointeeType().isConstQualified()
               ? BorrowType::Immutable : BorrowType::Mutable;
    }
    return BorrowType::None;
}

// Extract the VarDecl being pointed/referred to from init expression.
// 从初始化表达式中提取被指向/引用的 VarDecl。
static const clang::VarDecl* extractOwner(const clang::Expr* expr) {
    if (!expr) return nullptr;
    expr = expr->IgnoreParenImpCasts();

    // &x → x
    if (const auto* uo = clang::dyn_cast<clang::UnaryOperator>(expr)) {
        if (uo->getOpcode() == clang::UO_AddrOf) {
            expr = uo->getSubExpr()->IgnoreParenImpCasts();
        }
    }

    if (const auto* dr = clang::dyn_cast<clang::DeclRefExpr>(expr)) {
        return clang::dyn_cast<clang::VarDecl>(dr->getDecl());
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// BorrowMapBuilder: single-pass visitor that produces the alias map.
// BorrowMapBuilder：生成别名映射的单遍访问器。
// ---------------------------------------------------------------------------
class BorrowMapBuilder
    : public clang::RecursiveASTVisitor<BorrowMapBuilder> {
public:
    explicit BorrowMapBuilder(clang::ASTContext& ctx) : ctx_(ctx) {}

    using AliasMap = std::map<const clang::VarDecl*, std::vector<BorrowEntry>>;

    bool VisitVarDecl(clang::VarDecl* var) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(var->getLocation())) return true;

        clang::QualType qt = var->getType();
        if (!qt->isPointerType() && !qt->isReferenceType()) return true;

        BorrowType bt = classifyBorrow(qt);
        if (bt == BorrowType::None) return true;

        if (!var->hasInit()) return true;
        const clang::VarDecl* owner = extractOwner(var->getInit());
        if (!owner) return true;
        if (!owner->hasLocalStorage() && !owner->isLocalVarDecl()) return true;

        aliasMap_[owner].push_back({var, owner, bt, var->getLocation()});
        return true;
    }

    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(op->getBeginLoc())) return true;
        if (!op->isAssignmentOp()) return true;

        // p = &x  or  p = q  where q already in aliasMap_
        const clang::Expr* lhs = op->getLHS()->IgnoreParenImpCasts();
        const clang::Expr* rhs = op->getRHS()->IgnoreParenImpCasts();

        const auto* lhsDecl = clang::dyn_cast_or_null<clang::DeclRefExpr>(lhs);
        if (!lhsDecl) return true;
        const auto* borrower =
            clang::dyn_cast<clang::VarDecl>(lhsDecl->getDecl());
        if (!borrower) return true;
        if (!borrower->getType()->isPointerType()) return true;

        BorrowType bt = classifyBorrow(borrower->getType());
        const clang::VarDecl* owner = extractOwner(rhs);
        if (!owner) return true;
        if (!owner->hasLocalStorage() && !owner->isLocalVarDecl()) return true;

        aliasMap_[owner].push_back(
            {borrower, owner, bt, op->getBeginLoc()});
        return true;
    }

    const AliasMap& aliasMap() const { return aliasMap_; }

private:
    clang::ASTContext& ctx_;
    AliasMap aliasMap_;
};

// ---------------------------------------------------------------------------
// WriteFinder: finds write (modification) uses of variables in main file.
// WriteFinder：查找主文件中变量的写入（修改）使用。
// ---------------------------------------------------------------------------
class WriteFinder : public clang::RecursiveASTVisitor<WriteFinder> {
public:
    explicit WriteFinder(clang::ASTContext& ctx) : ctx_(ctx) {}

    // Collect variables that are written to.
    // 收集被写入的变量。
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(op->getBeginLoc())) return true;
        if (!op->isAssignmentOp() && !op->isCompoundAssignmentOp()) return true;

        if (const auto* dr = clang::dyn_cast<clang::DeclRefExpr>(
                op->getLHS()->IgnoreParenImpCasts())) {
            if (const auto* var =
                    clang::dyn_cast<clang::VarDecl>(dr->getDecl())) {
                writes_[var].push_back(op->getBeginLoc());
            }
        }
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator* op) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(op->getBeginLoc())) return true;
        auto oc = op->getOpcode();
        if (oc != clang::UO_PreInc && oc != clang::UO_PostInc &&
            oc != clang::UO_PreDec && oc != clang::UO_PostDec) return true;

        if (const auto* dr = clang::dyn_cast<clang::DeclRefExpr>(
                op->getSubExpr()->IgnoreParenImpCasts())) {
            if (const auto* var =
                    clang::dyn_cast<clang::VarDecl>(dr->getDecl())) {
                writes_[var].push_back(op->getBeginLoc());
            }
        }
        return true;
    }

    const std::map<const clang::VarDecl*,
                   std::vector<clang::SourceLocation>>& writes() const {
        return writes_;
    }

private:
    clang::ASTContext& ctx_;
    std::map<const clang::VarDecl*,
             std::vector<clang::SourceLocation>> writes_;
};

//=============================================================================
// TCC-BORROW-001: ConflictingBorrowRule
// Fires when the same owner variable has both a mutable and an immutable borrow.
// 当同一个所有者变量同时有可变和不可变借用时触发。
//=============================================================================

void ConflictingBorrowRule::check(clang::ASTContext& context,
                                  DiagnosticEngine& diagnostics) {
    const auto& sm = context.getSourceManager();
    BorrowMapBuilder builder(context);
    builder.TraverseDecl(context.getTranslationUnitDecl());

    for (const auto& [owner, borrows] : builder.aliasMap()) {
        clang::SourceLocation mutLoc, immLoc;
        bool hasMut = false, hasImm = false;

        for (const auto& b : borrows) {
            if (b.type == BorrowType::Mutable) {
                if (!hasMut) mutLoc = b.location;
                hasMut = true;
            } else if (b.type == BorrowType::Immutable) {
                if (!hasImm) immLoc = b.location;
                hasImm = true;
            }
        }

        if (hasMut && hasImm) {
            // Report at the later of the two conflicting borrow sites.
            // 在两个冲突借用站点中较晚的那个处报告。
            clang::SourceLocation reportLoc =
                sm.isBeforeInTranslationUnit(mutLoc, immLoc) ? immLoc : mutLoc;

            emitDiag(diagnostics, reportLoc, sm,
                     getCategory(), getId(),
                     "Cannot borrow '" + owner->getNameAsString() +
                     "' as both mutable and immutable / "
                     "不能同时以可变和不可变方式借用 '" +
                     owner->getNameAsString() + "'",
                     Severity::Error);
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
    for (const auto& [owner, borrows] : borrow_graph_) {
        bool hasMut = false, hasImm = false;
        for (const auto& b : borrows) {
            if (!b.is_active) continue;
            if (b.type == BorrowType::Mutable) hasMut = true;
            else if (b.type == BorrowType::Immutable) hasImm = true;
        }
        if ((hasMut && hasImm) || /* two mutable */ false) return false;
    }
    return true;
}

//=============================================================================
// TCC-BORROW-002: BorrowOutlivesOwnerRule
// Detects: pointer declared in outer scope, assigned &localVar in inner scope.
// 检测：在外部作用域声明的指针，在内部作用域赋值 &localVar。
//=============================================================================

namespace {

class OutlivesVisitor : public clang::RecursiveASTVisitor<OutlivesVisitor> {
public:
    explicit OutlivesVisitor(BorrowOutlivesOwnerRule* rule,
                             clang::ASTContext& ctx,
                             DiagnosticEngine& diags)
        : rule_(rule), ctx_(ctx), diags_(diags) {}

    // Track pointer declarations (outer scope candidates).
    // 追踪指针声明（外部作用域候选）。
    bool VisitVarDecl(clang::VarDecl* var) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(var->getLocation())) return true;
        if (!var->getType()->isPointerType()) return true;
        outerPtrs_.push_back(var);
        return true;
    }

    // Detect p = &x where p is an outer pointer, x is an inner local.
    // 检测 p = &x，其中 p 是外部指针，x 是内部局部变量。
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(op->getBeginLoc())) return true;
        if (op->getOpcode() != clang::BO_Assign) return true;

        const auto* lhsRef = clang::dyn_cast<clang::DeclRefExpr>(
            op->getLHS()->IgnoreParenImpCasts());
        if (!lhsRef) return true;
        const auto* lhsVar =
            clang::dyn_cast<clang::VarDecl>(lhsRef->getDecl());
        if (!lhsVar || !lhsVar->getType()->isPointerType()) return true;

        const clang::VarDecl* owner =
            extractOwner(op->getRHS());
        if (!owner || !owner->hasLocalStorage()) return true;

        // owner is inner if its decl location is after lhsVar's decl location.
        // 如果 owner 的声明位置在 lhsVar 的声明位置之后，则 owner 是内部的。
        if (sm.isBeforeInTranslationUnit(lhsVar->getLocation(),
                                          owner->getLocation())) {
            emitDiag(diags_, op->getBeginLoc(), sm,
                     rule_->getCategory(), rule_->getId(),
                     "Pointer '" + lhsVar->getNameAsString() +
                     "' will outlive its target '" +
                     owner->getNameAsString() +
                     "' / 指针 '" + lhsVar->getNameAsString() +
                     "' 将超出其目标 '" +
                     owner->getNameAsString() + "' 的生命周期",
                     Severity::Error);
        }
        return true;
    }

private:
    BorrowOutlivesOwnerRule* rule_;
    clang::ASTContext& ctx_;
    DiagnosticEngine& diags_;
    std::vector<const clang::VarDecl*> outerPtrs_;
};

} // anonymous namespace

void BorrowOutlivesOwnerRule::check(clang::ASTContext& context,
                                    DiagnosticEngine& diagnostics) {
    OutlivesVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool BorrowOutlivesOwnerRule::canOutlive(const clang::ValueDecl* borrower,
                                         const clang::ValueDecl* owner,
                                         clang::ASTContext& context) const {
    const auto& sm = context.getSourceManager();
    return sm.isBeforeInTranslationUnit(borrower->getLocation(),
                                        owner->getLocation());
}

//=============================================================================
// TCC-BORROW-003: MultipleMutableBorrowRule
// Fires when the same owner variable has ≥2 mutable borrows.
// 当同一个所有者变量有 ≥2 个可变借用时触发。
//=============================================================================

void MultipleMutableBorrowRule::check(clang::ASTContext& context,
                                       DiagnosticEngine& diagnostics) {
    const auto& sm = context.getSourceManager();
    BorrowMapBuilder builder(context);
    builder.TraverseDecl(context.getTranslationUnitDecl());

    for (const auto& [owner, borrows] : builder.aliasMap()) {
        std::vector<clang::SourceLocation> mutLocs;
        for (const auto& b : borrows) {
            if (b.type == BorrowType::Mutable) {
                mutLocs.push_back(b.location);
            }
        }
        if (mutLocs.size() >= 2) {
            emitDiag(diagnostics, mutLocs[1], sm,
                     getCategory(), getId(),
                     "Cannot borrow '" + owner->getNameAsString() +
                     "' as mutable more than once / "
                     "不能多次以可变方式借用 '" +
                     owner->getNameAsString() + "'",
                     Severity::Error);
        }
    }
}

bool MultipleMutableBorrowRule::hasMultipleMutableBorrows(
    const clang::ValueDecl* owner) const {
    return false;
}

//=============================================================================
// TCC-BORROW-004: BorrowDuringModificationRule
// Fires when owner is modified (written) while a const borrow exists.
// 当存在 const 借用时，所有者被修改（写入）时触发。
//=============================================================================

void BorrowDuringModificationRule::check(clang::ASTContext& context,
                                          DiagnosticEngine& diagnostics) {
    const auto& sm = context.getSourceManager();

    BorrowMapBuilder borrowBuilder(context);
    borrowBuilder.TraverseDecl(context.getTranslationUnitDecl());

    WriteFinder writeFind(context);
    writeFind.TraverseDecl(context.getTranslationUnitDecl());

    for (const auto& [owner, borrows] : borrowBuilder.aliasMap()) {
        // Only care about immutable (const) borrows for this rule.
        // 只关心不可变（const）借用。
        bool hasConstBorrow = false;
        for (const auto& b : borrows) {
            if (b.type == BorrowType::Immutable) { hasConstBorrow = true; break; }
        }
        if (!hasConstBorrow) continue;

        auto wit = writeFind.writes().find(owner);
        if (wit == writeFind.writes().end()) continue;

        for (auto writeLoc : wit->second) {
            emitDiag(diagnostics, writeLoc, sm,
                     getCategory(), getId(),
                     "Cannot assign to '" + owner->getNameAsString() +
                     "' because it is borrowed / "
                     "无法赋值给 '" + owner->getNameAsString() +
                     "'，因为它已被借用",
                     Severity::Error);
        }
    }
}

bool BorrowDuringModificationRule::isBorrowedAndModified(
    const clang::Expr* expr) const {
    return false;
}

} // namespace tcc

