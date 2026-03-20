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

} // namespace tcc

