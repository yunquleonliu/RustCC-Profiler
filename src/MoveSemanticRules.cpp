// Rust C/C++ Profiler - Move Semantic Rules Implementation
// Rust C/C++ 分析器 - 移动语义规则实现

#include "tcc/MoveSemanticRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/StmtVisitor.h>

namespace tcc {

// Visitor to track move operations / 追踪移动操作的访问器
class MoveTrackingVisitor : public clang::RecursiveASTVisitor<MoveTrackingVisitor> {
public:
    explicit MoveTrackingVisitor(UseAfterMoveRule* rule,
                                clang::ASTContext& context,
                                DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    // Visit move constructor calls / 访问移动构造函数调用
    bool VisitCXXConstructExpr(clang::CXXConstructExpr* expr) {
        if (expr->getConstructor()->isMoveConstructor()) {
            rule_->trackMoveOperation(expr);
        }
        return true;
    }
    
    // Visit std::move calls / 访问 std::move 调用
    bool VisitCallExpr(clang::CallExpr* call) {
        if (auto* func = call->getDirectCallee()) {
            std::string name = func->getQualifiedNameAsString();
            if (name == "std::move") {
                rule_->trackMoveCall(call);
            }
        }
        return true;
    }
    
    // Visit variable references / 访问变量引用
    bool VisitDeclRefExpr(clang::DeclRefExpr* ref) {
        if (rule_->isUsedAfterMove(ref)) {
            emitDiag(diagnostics_, ref->getLocation(),
                     context_.getSourceManager(),
                     rule_->getCategory(), rule_->getId(),
                     "Variable used after move / 变量在移动后被使用",
                     Severity::Error);
        }
        return true;
    }

private:
    UseAfterMoveRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

//=============================================================================
// UseAfterMoveRule Implementation
//=============================================================================

void UseAfterMoveRule::check(clang::ASTContext& context,
                             DiagnosticEngine& diagnostics) {
    MoveTrackingVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

void UseAfterMoveRule::trackMoveOperation(const clang::CXXConstructExpr* expr) {
    // Track the source of the move / 追踪移动的源
    if (expr->getNumArgs() > 0) {
        if (auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(
                expr->getArg(0)->IgnoreParenImpCasts())) {
            if (auto* varDecl = clang::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
                variable_states_[varDecl] = VariableState::Moved;
                moved_variables_.insert(varDecl);
            }
        }
    }
}

void UseAfterMoveRule::trackMoveCall(const clang::CallExpr* expr) {
    // Track std::move() argument / 追踪 std::move() 的参数
    if (expr->getNumArgs() > 0) {
        if (auto* declRef = clang::dyn_cast<clang::DeclRefExpr>(
                expr->getArg(0)->IgnoreParenImpCasts())) {
            if (auto* varDecl = clang::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
                variable_states_[varDecl] = VariableState::Moved;
                moved_variables_.insert(varDecl);
            }
        }
    }
}

bool UseAfterMoveRule::isUsedAfterMove(const clang::DeclRefExpr* ref) const {
    if (auto* varDecl = clang::dyn_cast<clang::VarDecl>(ref->getDecl())) {
        auto it = variable_states_.find(varDecl);
        if (it != variable_states_.end() && it->second == VariableState::Moved) {
            return true;
        }
    }
    return false;
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
    // Visit all class definitions / 访问所有类定义
    for (auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (auto* record = clang::dyn_cast<clang::CXXRecordDecl>(decl)) {
            if (record->isCompleteDefinition() && requiresMovSemantics(record)) {
                if (!hasProperMoveSemantics(record)) {
                    auto& sm = context.getSourceManager();
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

