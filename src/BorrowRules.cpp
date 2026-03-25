// Rust C/C++ Profiler - Borrow Checker Rules Implementation
// Rust C/C++ 分析器 - 借用检查规则实现

#include "tcc/BorrowRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/Basic/SourceManager.h>
#include <map>
#include <vector>
#include <string>

namespace tcc {

// -------------------------------------------------------------------------
// Local alias tracking struct (VarDecl-typed for precision)
// -------------------------------------------------------------------------
struct AliasEntry {
    const clang::VarDecl* borrower;
    const clang::VarDecl* owner;
    BorrowType type;
    clang::SourceLocation location;
};

using AliasMap = std::map<const clang::VarDecl*, std::vector<AliasEntry>>;

// -------------------------------------------------------------------------
// Helper: determine borrow type from a pointer type
// -------------------------------------------------------------------------
static BorrowType borrowTypeFromPtrQT(clang::QualType qt) {
    if (qt->isPointerType()) {
        return qt->getPointeeType().isConstQualified()
               ? BorrowType::Immutable : BorrowType::Mutable;
    }
    return BorrowType::None;
}

// -------------------------------------------------------------------------
// AliasBuildVisitor: single pass to collect owner -> [borrows] map.
// Only tracks DIRECT patterns: T* p = &x;  T& r = x;  and p = &x (assignment).
// Method-returned references are deliberately NOT tracked to avoid false
// positives on examples that use idiomatic C++ accessor patterns.
// -------------------------------------------------------------------------
class AliasBuildVisitor
    : public clang::RecursiveASTVisitor<AliasBuildVisitor> {
public:
    explicit AliasBuildVisitor(clang::ASTContext& ctx)
        : ctx_(ctx), sm_(ctx.getSourceManager()) {}

    AliasMap& getMap() { return map_; }

    // VarDecl init: T* p = &x;  or  T& r = x;
    bool VisitVarDecl(clang::VarDecl* var) {
        if (!sm_.isInMainFile(var->getLocation())) return true;
        auto qt = var->getType();
        if (!qt->isPointerType() && !qt->isReferenceType()) return true;
        if (!var->hasInit()) return true;

        auto* init = var->getInit()->IgnoreParenImpCasts();

        if (qt->isPointerType()) {
            // T* p = &x
            if (auto* unary = clang::dyn_cast<clang::UnaryOperator>(init)) {
                if (unary->getOpcode() == clang::UO_AddrOf) {
                    auto* sub = unary->getSubExpr()->IgnoreParenImpCasts();
                    if (auto* dre = clang::dyn_cast<clang::DeclRefExpr>(sub)) {
                        if (auto* owner = clang::dyn_cast<clang::VarDecl>(
                                dre->getDecl())) {
                            if (sm_.isInMainFile(owner->getLocation())) {
                                map_[owner].push_back({var, owner,
                                    borrowTypeFromPtrQT(qt),
                                    var->getLocation()});
                            }
                        }
                    }
                }
            }
        } else {
            // T& r = x  (reference binding)
            if (auto* dre = clang::dyn_cast<clang::DeclRefExpr>(init)) {
                if (auto* owner = clang::dyn_cast<clang::VarDecl>(
                        dre->getDecl())) {
                    if (sm_.isInMainFile(owner->getLocation())) {
                        bool isConst = qt->getPointeeType().isConstQualified();
                        map_[owner].push_back({var, owner,
                            isConst ? BorrowType::Immutable : BorrowType::Mutable,
                            var->getLocation()});
                    }
                }
            }
        }
        return true;
    }

    // Assignment: p = &x
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        if (!sm_.isInMainFile(op->getOperatorLoc())) return true;
        if (!op->isAssignmentOp()) return true;

        auto* lhsDRE = clang::dyn_cast<clang::DeclRefExpr>(
            op->getLHS()->IgnoreParenImpCasts());
        if (!lhsDRE) return true;
        auto* lhsVar = clang::dyn_cast<clang::VarDecl>(lhsDRE->getDecl());
        if (!lhsVar || !lhsVar->getType()->isPointerType()) return true;

        auto* rhs = op->getRHS()->IgnoreParenImpCasts();
        if (auto* unary = clang::dyn_cast<clang::UnaryOperator>(rhs)) {
            if (unary->getOpcode() == clang::UO_AddrOf) {
                auto* sub = unary->getSubExpr()->IgnoreParenImpCasts();
                if (auto* dre = clang::dyn_cast<clang::DeclRefExpr>(sub)) {
                    if (auto* owner = clang::dyn_cast<clang::VarDecl>(
                            dre->getDecl())) {
                        if (sm_.isInMainFile(owner->getLocation())) {
                            map_[owner].push_back({lhsVar, owner,
                                borrowTypeFromPtrQT(lhsVar->getType()),
                                op->getOperatorLoc()});
                        }
                    }
                }
            }
        }
        return true;
    }

private:
    clang::ASTContext& ctx_;
    const clang::SourceManager& sm_;
    AliasMap map_;
};

//=============================================================================
// TCC-BORROW-001: ConflictingBorrowRule
// For each owner with ≥1 mutable AND ≥1 immutable borrow, emit error.
//=============================================================================

void ConflictingBorrowRule::check(clang::ASTContext& context,
                                  DiagnosticEngine& diagnostics) {
    AliasBuildVisitor abv(context);
    abv.TraverseDecl(context.getTranslationUnitDecl());
    auto& sm = context.getSourceManager();

    for (auto& [owner, borrows] : abv.getMap()) {
        // Only flag local variables to avoid false positives on globals.
        if (!owner->hasLocalStorage()) continue;

        const AliasEntry* firstMut = nullptr;
        const AliasEntry* firstImm = nullptr;
        for (auto& b : borrows) {
            if (b.type == BorrowType::Mutable && !firstMut) firstMut = &b;
            if (b.type == BorrowType::Immutable && !firstImm) firstImm = &b;
        }
        if (firstMut && firstImm) {
            // Report at the later (conflicting) borrow site.
            auto loc = sm.isBeforeInTranslationUnit(firstMut->location,
                                                    firstImm->location)
                       ? firstImm->location : firstMut->location;
            emitDiag(diagnostics, loc, sm,
                     getCategory(), getId(),
                     "Conflicting mutable and immutable borrows of '" +
                     owner->getNameAsString() + "' / 可变和不可变借用冲突",
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
        int mutCount = 0;
        for (auto& b : borrows) {
            if (!b.is_active) continue;
            if (b.type == BorrowType::Mutable) { hasMut = true; ++mutCount; }
            if (b.type == BorrowType::Immutable) hasImm = true;
        }
        if ((hasMut && hasImm) || mutCount > 1) return true;
    }
    return false;
}

//=============================================================================
// TCC-BORROW-002: BorrowOutlivesOwnerRule
// Detects: pointer declared in outer scope gets assigned &local from inner scope.
//=============================================================================

class OutlivesVisitor
    : public clang::RecursiveASTVisitor<OutlivesVisitor> {
public:
    explicit OutlivesVisitor(clang::ASTContext& ctx, DiagnosticEngine& diags,
                             BorrowOutlivesOwnerRule* rule)
        : ctx_(ctx), sm_(ctx.getSourceManager()), diags_(diags), rule_(rule) {}

    // Track scope stack: push on enter, pop on exit of compound statement.
    bool TraverseCompoundStmt(clang::CompoundStmt* cs) {
        scope_starts_.push_back(cs->getLBracLoc());
        bool result = RecursiveASTVisitor::TraverseCompoundStmt(cs);
        scope_starts_.pop_back();
        return result;
    }

    // VarDecl init in same scope: T* p = &x — only dangerous across scope boundary.
    bool VisitVarDecl(clang::VarDecl* var) {
        if (!sm_.isInMainFile(var->getLocation())) return true;
        if (!var->getType()->isPointerType()) return true;
        if (!var->hasInit()) return true;

        auto* init = var->getInit()->IgnoreParenImpCasts();
        checkAddrOf(var, var->getLocation(), init, /*fromAssign=*/false);
        return true;
    }

    // Assignment pattern: p = &x where p is from outer scope.
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        if (!sm_.isInMainFile(op->getOperatorLoc())) return true;
        if (!op->isAssignmentOp()) return true;

        auto* lhsDRE = clang::dyn_cast<clang::DeclRefExpr>(
            op->getLHS()->IgnoreParenImpCasts());
        if (!lhsDRE) return true;
        auto* lhsVar = clang::dyn_cast<clang::VarDecl>(lhsDRE->getDecl());
        if (!lhsVar || !lhsVar->getType()->isPointerType()) return true;

        auto* rhs = op->getRHS()->IgnoreParenImpCasts();
        checkAddrOf(lhsVar, op->getOperatorLoc(), rhs, /*fromAssign=*/true);
        return true;
    }

private:
    // Check if expr is &local_var and whether ptr outlives local_var's scope.
    void checkAddrOf(const clang::VarDecl* ptr, clang::SourceLocation diagLoc,
                     clang::Expr* expr, bool fromAssign) {
        auto* unary = clang::dyn_cast<clang::UnaryOperator>(expr);
        if (!unary || unary->getOpcode() != clang::UO_AddrOf) return;

        auto* sub = unary->getSubExpr()->IgnoreParenImpCasts();
        auto* dre = clang::dyn_cast<clang::DeclRefExpr>(sub);
        if (!dre) return;
        auto* owner = clang::dyn_cast<clang::VarDecl>(dre->getDecl());
        if (!owner) return;
        if (!sm_.isInMainFile(owner->getLocation())) return;
        if (!owner->hasLocalStorage() || !ptr->hasLocalStorage()) return;

        if (fromAssign) {
            // For assignment p = &x inside a block, check if ptr was declared
            // before this block started (ptr is in outer scope, x in inner scope).
            if (!scope_starts_.empty()) {
                auto blockStart = scope_starts_.back();
                // ptr declared before current block → outer scope
                if (sm_.isBeforeInTranslationUnit(ptr->getLocation(), blockStart) &&
                    sm_.isBeforeInTranslationUnit(blockStart, owner->getLocation())) {
                    emitDiag(diags_, diagLoc, sm_,
                             rule_->getCategory(), rule_->getId(),
                             "Pointer to '" + owner->getNameAsString() +
                             "' escapes its scope — borrow outlives owner / "
                             "借用超出所有者生命周期",
                             Severity::Error);
                }
            }
        } else {
            // For initializer T* p = &x — warn when p is initialized to addr
            // of a variable declared in an inner block relative to p.
            // (Less common but handle: T* p = &x where x is more inner.)
            // For same-scope init this fires only if x's decl is in deeper scope.
            // We skip this for init since it's the same scope in most normal code.
        }
    }

    clang::ASTContext& ctx_;
    const clang::SourceManager& sm_;
    DiagnosticEngine& diags_;
    BorrowOutlivesOwnerRule* rule_;
    std::vector<clang::SourceLocation> scope_starts_;
};

void BorrowOutlivesOwnerRule::check(clang::ASTContext& context,
                                    DiagnosticEngine& diagnostics) {
    OutlivesVisitor v(context, diagnostics, this);
    v.TraverseDecl(context.getTranslationUnitDecl());
}

bool BorrowOutlivesOwnerRule::canOutlive(const clang::ValueDecl* /*borrower*/,
                                         const clang::ValueDecl* /*owner*/,
                                         clang::ASTContext& /*context*/) const {
    return false;
}

//=============================================================================
// TCC-BORROW-003: MultipleMutableBorrowRule
// For each owner with ≥2 mutable borrows, emit error at the second site.
//=============================================================================

void MultipleMutableBorrowRule::check(clang::ASTContext& context,
                                      DiagnosticEngine& diagnostics) {
    AliasBuildVisitor abv(context);
    abv.TraverseDecl(context.getTranslationUnitDecl());
    auto& sm = context.getSourceManager();

    for (auto& [owner, borrows] : abv.getMap()) {
        if (!owner->hasLocalStorage()) continue;

        std::vector<const AliasEntry*> muts;
        for (auto& b : borrows) {
            if (b.type == BorrowType::Mutable) muts.push_back(&b);
        }
        if (muts.size() >= 2) {
            emitDiag(diagnostics, muts[1]->location, sm,
                     getCategory(), getId(),
                     "Multiple mutable borrows of '" + owner->getNameAsString() +
                     "' / 多个可变借用",
                     Severity::Error);
        }
    }
}

bool MultipleMutableBorrowRule::hasMultipleMutableBorrows(
    const clang::ValueDecl* /*owner*/) const {
    return false;
}

//=============================================================================
// TCC-BORROW-004: BorrowDuringModificationRule
// Owner modified (assigned / ++/--) while an immutable borrow of it exists.
//=============================================================================

class BorrowModVisitor
    : public clang::RecursiveASTVisitor<BorrowModVisitor> {
public:
    explicit BorrowModVisitor(clang::ASTContext& ctx, DiagnosticEngine& diags,
                              BorrowDuringModificationRule* rule,
                              const AliasMap& map)
        : ctx_(ctx), sm_(ctx.getSourceManager()), diags_(diags),
          rule_(rule), alias_map_(map) {}

    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        if (!sm_.isInMainFile(op->getOperatorLoc())) return true;
        if (!op->isAssignmentOp() && !op->isCompoundAssignmentOp()) return true;
        checkMod(op->getLHS(), op->getOperatorLoc());
        return true;
    }

    bool VisitUnaryOperator(clang::UnaryOperator* op) {
        if (!sm_.isInMainFile(op->getOperatorLoc())) return true;
        auto oc = op->getOpcode();
        if (oc == clang::UO_PreInc || oc == clang::UO_PreDec ||
            oc == clang::UO_PostInc || oc == clang::UO_PostDec) {
            checkMod(op->getSubExpr(), op->getOperatorLoc());
        }
        return true;
    }

private:
    void checkMod(clang::Expr* lhs, clang::SourceLocation modLoc) {
        auto* dre = clang::dyn_cast<clang::DeclRefExpr>(
            lhs->IgnoreParenImpCasts());
        if (!dre) return;
        auto* var = clang::dyn_cast<clang::VarDecl>(dre->getDecl());
        if (!var || !var->hasLocalStorage()) return;

        auto it = alias_map_.find(var);
        if (it == alias_map_.end()) return;

        // Look for any immutable borrow declared strictly before this modification.
        for (auto& borrow : it->second) {
            if (borrow.type != BorrowType::Immutable) continue;
            if (sm_.isBeforeInTranslationUnit(borrow.location, modLoc)) {
                emitDiag(diags_, modLoc, sm_,
                         rule_->getCategory(), rule_->getId(),
                         "Variable '" + var->getNameAsString() +
                         "' modified while immutably borrowed / "
                         "借用期间修改变量",
                         Severity::Error);
                return; // One diagnostic per modification site is enough.
            }
        }
    }

    clang::ASTContext& ctx_;
    const clang::SourceManager& sm_;
    DiagnosticEngine& diags_;
    BorrowDuringModificationRule* rule_;
    const AliasMap& alias_map_;
};

void BorrowDuringModificationRule::check(clang::ASTContext& context,
                                         DiagnosticEngine& diagnostics) {
    AliasBuildVisitor abv(context);
    abv.TraverseDecl(context.getTranslationUnitDecl());

    BorrowModVisitor bmv(context, diagnostics, this, abv.getMap());
    bmv.TraverseDecl(context.getTranslationUnitDecl());
}

bool BorrowDuringModificationRule::isBorrowedAndModified(
    const clang::Expr* /*expr*/) const {
    return false;
}

} // namespace tcc

