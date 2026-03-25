// Rust C/C++ Profiler - Move Semantic Rules Implementation
// Rust C/C++ 分析器 - 移动语义规则实现

#include "tcc/MoveSemanticRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/StmtVisitor.h>
#include <clang/Basic/SourceManager.h>
#include <map>

namespace tcc {

// Visitor to track move operations / 追踪移动操作的访问器
class MoveTrackingVisitor : public clang::RecursiveASTVisitor<MoveTrackingVisitor> {
public:
    explicit MoveTrackingVisitor(UseAfterMoveRule* rule,
                                clang::ASTContext& context,
                                DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}

    // Record the source var of std::move(x) at x's own DeclRefExpr location.
    // VisitCallExpr fires BEFORE children's VisitDeclRefExpr in data-recursion order,
    // so moved_at_[x] == x.loc when VisitDeclRefExpr processes 'x' inside std::move(x).
    // isBeforeInTranslationUnit(x.loc, x.loc) returns false => skip the move-source ref.
    bool VisitCallExpr(clang::CallExpr* call) {
        const auto& sm = context_.getSourceManager();
        if (!sm.isInMainFile(call->getBeginLoc())) return true;
        if (auto* func = call->getDirectCallee()) {
            std::string name = func->getQualifiedNameAsString();
            if (name == "std::move" && call->getNumArgs() > 0) {
                if (auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(
                        call->getArg(0)->IgnoreParenImpCasts())) {
                    if (auto* varDecl = clang::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
                        // Store location of 'x' in std::move(x), not the call's BeginLoc
                        moved_at_[varDecl] = declRef->getLocation();
                    }
                }
            }
        }
        return true;
    }

    // Visit variable references to detect use-after-move / 访问变量引用以检测移动后使用
    bool VisitDeclRefExpr(clang::DeclRefExpr* ref) {
        const auto& sm = context_.getSourceManager();
        if (!sm.isInMainFile(ref->getLocation())) return true;
        if (auto* varDecl = clang::dyn_cast<clang::VarDecl>(ref->getDecl())) {
            auto it = moved_at_.find(varDecl);
            if (it != moved_at_.end()) {
                // Only flag if this use is strictly after the move source.
                // When L == M (the ref inside std::move(x)), returns false => skip.
                if (sm.isBeforeInTranslationUnit(it->second, ref->getLocation())) {
                    emitDiag(diagnostics_, ref->getLocation(),
                             sm,
                             rule_->getCategory(), rule_->getId(),
                             "Variable used after move / 变量在移动后被使用",
                             Severity::Error);
                }
            }
        }
        return true;
    }

private:
    UseAfterMoveRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
    std::map<const clang::VarDecl*, clang::SourceLocation> moved_at_;
};

//=============================================================================
// UseAfterMoveRule Implementation
//=============================================================================

void UseAfterMoveRule::check(clang::ASTContext& context,
                             DiagnosticEngine& diagnostics) {
    MoveTrackingVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

//=============================================================================
// DoubleMoveRule Implementation
//=============================================================================

void DoubleMoveRule::check(clang::ASTContext& context,
                          DiagnosticEngine& diagnostics) {
    // Implementation similar to UseAfterMoveRule
    // Track move counts and report double moves
    // 实现类似于 UseAfterMoveRule
    // 追踪移动次数并报告双重移动
}

bool DoubleMoveRule::isMovedMultipleTimes(const clang::VarDecl* var) const {
    // Check if variable has been moved more than once
    // 检查变量是否被移动超过一次
    return false;  // Placeholder / 占位符
}

//=============================================================================
// EnforceMoveSemanticsRule Implementation
//=============================================================================

void EnforceMoveSemanticsRule::check(clang::ASTContext& context,
                                    DiagnosticEngine& diagnostics) {
    // Visit all class definitions in the main file / 访问主文件中的所有类定义
    const auto& sm = context.getSourceManager();
    for (auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (auto* record = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (!sm.isInMainFile(record->getLocation())) continue;
            if (record->isCompleteDefinition() && requiresMovSemantics(record)) {
                if (!hasProperMoveSemantics(record)) {
                    emitDiag(diagnostics, record->getLocation(), sm,
                             getCategory(), getId(),
                             "RAII type missing move semantics / RAII 类型缺少移动语义",
                             Severity::Warning);
                }
            }
        }
    }
}

bool EnforceMoveSemanticsRule::requiresMovSemantics(
    const clang::CXXRecordDecl* record) const {
    // Check if type is RAII (has destructor with cleanup)
    // 检查类型是否为 RAII（有清理析构函数）
    return record->hasUserDeclaredDestructor();
}

bool EnforceMoveSemanticsRule::hasProperMoveSemantics(
    const clang::CXXRecordDecl* record) const {
    // Check for move constructor and move assignment
    // 检查移动构造和移动赋值
    bool hasMoveConstructor = false;
    bool hasMoveAssignment = false;
    
    for (auto* ctor : record->ctors()) {
        if (ctor->isMoveConstructor()) {
            hasMoveConstructor = true;
            break;
        }
    }
    
    for (auto* method : record->methods()) {
        if (method->isMoveAssignmentOperator()) {
            hasMoveAssignment = true;
            break;
        }
    }
    
    return hasMoveConstructor && hasMoveAssignment;
}

//=============================================================================
// TCC-OWN-008: DoubleFreeViaAliasRule
// Detects: two raw pointers aliasing the same new-expression, both deleted.
// 检测：两个原始指针指向同一个 new 表达式，都被 delete。
//=============================================================================

namespace {

class AliasFreeVisitor : public clang::RecursiveASTVisitor<AliasFreeVisitor> {
public:
    explicit AliasFreeVisitor(DoubleFreeViaAliasRule* rule,
                              clang::ASTContext& ctx,
                              DiagnosticEngine& diags)
        : rule_(rule), ctx_(ctx), diags_(diags) {}

    // Track: T* p = new T  →  alloc_[p] = newExpr
    // 追踪: T* p = new T  →  alloc_[p] = newExpr
    bool VisitVarDecl(clang::VarDecl* var) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(var->getLocation())) return true;
        if (!var->getType()->isPointerType()) return true;
        if (!var->hasInit()) return true;

        const clang::Expr* init = var->getInit()->IgnoreParenImpCasts();
        if (clang::isa<clang::CXXNewExpr>(init)) {
            alloc_[var] = init;
        }
        return true;
    }

    // Track: q = p  where p is in alloc_ → q aliases p's allocation
    // 追踪: q = p，其中 p 在 alloc_ 中 → q 是 p 的分配的别名
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(op->getBeginLoc())) return true;
        if (op->getOpcode() != clang::BO_Assign) return true;

        const auto* lhsDR = clang::dyn_cast<clang::DeclRefExpr>(
            op->getLHS()->IgnoreParenImpCasts());
        const auto* rhsDR = clang::dyn_cast<clang::DeclRefExpr>(
            op->getRHS()->IgnoreParenImpCasts());
        if (!lhsDR || !rhsDR) return true;

        const auto* lhs = clang::dyn_cast<clang::VarDecl>(lhsDR->getDecl());
        const auto* rhs = clang::dyn_cast<clang::VarDecl>(rhsDR->getDecl());
        if (!lhs || !rhs) return true;

        if (alloc_.count(rhs)) {
            // lhs aliases rhs's allocation
            aliases_[lhs] = rhs;
        }
        return true;
    }

    // Track: delete p  →  deleted_[p] = loc
    // 追踪: delete p  →  deleted_[p] = loc
    bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* del) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(del->getBeginLoc())) return true;

        const clang::Expr* arg = del->getArgument()->IgnoreParenImpCasts();
        if (const auto* dr = clang::dyn_cast<clang::DeclRefExpr>(arg)) {
            if (const auto* var =
                    clang::dyn_cast<clang::VarDecl>(dr->getDecl())) {
                if (deleted_.count(var)) {
                    // Already deleted — report
                    emitDiag(diags_, del->getBeginLoc(), sm,
                             rule_->getCategory(), rule_->getId(),
                             "Deleting '" + var->getNameAsString() +
                             "' which may already be deleted (direct alias) / "
                             "删除可能已被删除的 '" + var->getNameAsString() + "'（直接别名）",
                             Severity::Error);
                } else {
                    deleted_[var] = del->getBeginLoc();
                    // Check if any alias of this var was already deleted.
                    // 检查此变量的任何别名是否已被删除。
                    checkAliasDelete(var, del->getBeginLoc(), sm);
                }
            }
        }
        return true;
    }

private:
    void checkAliasDelete(const clang::VarDecl* var,
                          clang::SourceLocation loc,
                          const clang::SourceManager& sm) {
        // Direct: var is alias of another → check if that other was deleted.
        // 直接：var 是另一个变量的别名 → 检查该变量是否已被删除。
        auto ait = aliases_.find(var);
        if (ait != aliases_.end()) {
            const clang::VarDecl* orig = ait->second;
            if (deleted_.count(orig)) {
                emitDiag(diags_, loc, sm,
                         rule_->getCategory(), rule_->getId(),
                         "Potential double-free: '" + var->getNameAsString() +
                         "' aliases '" + orig->getNameAsString() +
                         "' which was already deleted / "
                         "潜在双重释放：'" + var->getNameAsString() +
                         "' 是已被删除的 '" + orig->getNameAsString() +
                         "' 的别名",
                         Severity::Error);
            }
        }
        // Reverse: another var was aliased from var → check if that other was deleted.
        // 反向：另一个变量是 var 的别名 → 检查该变量是否已被删除。
        for (const auto& [alias, orig] : aliases_) {
            if (orig == var && deleted_.count(alias)) {
                emitDiag(diags_, loc, sm,
                         rule_->getCategory(), rule_->getId(),
                         "Potential double-free: '" + var->getNameAsString() +
                         "' is aliased by '" + alias->getNameAsString() +
                         "' which was already deleted / "
                         "潜在双重释放：'" + var->getNameAsString() +
                         "' 被已删除的 '" + alias->getNameAsString() +
                         "' 引用",
                         Severity::Error);
            }
        }
    }

    DoubleFreeViaAliasRule* rule_;
    clang::ASTContext& ctx_;
    DiagnosticEngine& diags_;

    // owner ptr → new expression
    std::map<const clang::VarDecl*, const clang::Expr*> alloc_;
    // alias ptr → original owner ptr
    std::map<const clang::VarDecl*, const clang::VarDecl*> aliases_;
    // ptr → first delete location
    std::map<const clang::VarDecl*, clang::SourceLocation> deleted_;
};

} // anonymous namespace

void DoubleFreeViaAliasRule::check(clang::ASTContext& context,
                                   DiagnosticEngine& diagnostics) {
    AliasFreeVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

} // namespace tcc

