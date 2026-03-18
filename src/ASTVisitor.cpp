// Tough C Profiler - AST Visitor Implementation
// Tough C 分析器 - AST 访问者实现

#include "tcc/ASTVisitor.h"
#include "tcc/DiagnosticHelper.h"
#include "tcc/MoveSemanticRules.h"
#include "tcc/OwnershipRules.h"
#include "tcc/SafetyPatternRules.h"
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

namespace tcc {

bool TCCASTVisitor::VisitFunctionDecl(clang::FunctionDecl* decl) {
    if (!decl || !isInMainFile(decl->getLocation())) {
        return true;
    }
    
    // Function declarations are checked by specific rules
    // 函数声明由特定规则检查
    return true;
}

bool TCCASTVisitor::VisitVarDecl(clang::VarDecl* decl) {
    if (!decl || !isInMainFile(decl->getLocation())) {
        return true;
    }
    
    // Variable declarations are checked by specific rules
    // 变量声明由特定规则检查
    return true;
}

bool TCCASTVisitor::VisitCXXNewExpr(clang::CXXNewExpr* expr) {
    if (!expr || !isInMainFile(expr->getLocation())) {
        return true;
    }
    
    // Found a 'new' expression - let rules handle it
    // 发现 'new' 表达式 - 让规则处理
    for (const auto& rule : rules_) {
        if (auto* ownershipRule = dynamic_cast<ForbidNewRule*>(rule.get())) {
            ownershipRule->checkNewExpr(expr, context_, diagnostics_);
        }
    }
    
    return true;
}

bool TCCASTVisitor::VisitCXXDeleteExpr(clang::CXXDeleteExpr* expr) {
    if (!expr || !isInMainFile(expr->getLocation())) {
        return true;
    }
    
    // Found a 'delete' expression - let rules handle it
    // 发现 'delete' 表达式 - 让规则处理
    for (const auto& rule : rules_) {
        if (auto* ownershipRule = dynamic_cast<ForbidDeleteRule*>(rule.get())) {
            ownershipRule->checkDeleteExpr(expr, context_, diagnostics_);
        }
    }
    
    return true;
}

bool TCCASTVisitor::VisitReturnStmt(clang::ReturnStmt* stmt) {
    if (!stmt || !isInMainFile(stmt->getReturnLoc())) {
        return true;
    }
    
    // Return statements are checked by lifetime rules
    // return 语句由生命周期规则检查
    return true;
}

bool TCCASTVisitor::VisitLambdaExpr(clang::LambdaExpr* expr) {
    if (!expr || !isInMainFile(expr->getLocation())) {
        return true;
    }
    
    // Lambda expressions are checked by concurrency rules
    // Lambda 表达式由并发规则检查
    return true;
}

bool TCCASTVisitor::VisitCXXConstructExpr(clang::CXXConstructExpr* expr) {
    if (!expr || !isInMainFile(expr->getLocation())) {
        return true;
    }
    
    // Check for std::thread construction
    // 检查 std::thread 构造
    const auto* ctorDecl = expr->getConstructor();
    if (!ctorDecl) {
        return true;
    }
    
    const auto* recordDecl = ctorDecl->getParent();
    if (!recordDecl) {
        return true;
    }
    
    // Thread construction is checked by concurrency rules
    // 线程构造由并发规则检查
    return true;
}

SourceLocation TCCASTVisitor::getSourceLocation(clang::SourceLocation loc) const {
    if (loc.isInvalid()) {
        return SourceLocation();
    }
    
    const auto& sm = context_.getSourceManager();
    auto presumedLoc = sm.getPresumedLoc(loc);
    
    return SourceLocation(
        presumedLoc.getFilename(),
        presumedLoc.getLine(),
        presumedLoc.getColumn()
    );
}

bool TCCASTVisitor::isInMainFile(clang::SourceLocation loc) const {
    if (loc.isInvalid()) {
        return false;
    }
    
    const auto& sm = context_.getSourceManager();
    return sm.isInMainFile(loc);
}

} // namespace tcc

namespace tcc {

// ─── Step 2: Move-semantics visitors ───────────────────────────────────────

bool TCCASTVisitor::VisitCallExpr(clang::CallExpr* expr) {
    if (!expr || !isInMainFile(expr->getBeginLoc())) {
        return true;
    }
    // Track std::move() for use-after-move detection
    // 追踪 std::move() 以检测移动后使用
    for (const auto& rule : rules_) {
        if (auto* moveRule = dynamic_cast<UseAfterMoveRule*>(rule.get())) {
            if (auto* callee = expr->getDirectCallee()) {
                if (callee->getQualifiedNameAsString() == "std::move") {
                    moveRule->trackMoveCall(expr);
                }
            }
        }
    }
    return true;
}

bool TCCASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr* expr) {
    if (!expr || !isInMainFile(expr->getLocation())) {
        return true;
    }
    // Detect use-after-move violations
    // 检测移动后使用违规
    for (const auto& rule : rules_) {
        if (auto* moveRule = dynamic_cast<UseAfterMoveRule*>(rule.get())) {
            if (moveRule->isUsedAfterMove(expr)) {
                auto& sm = context_.getSourceManager();
                auto loc = expr->getLocation();
                // Unique key to avoid duplicate diagnostics on this pass
                auto key = std::make_pair(sm.getSpellingLineNumber(loc),
                                          sm.getSpellingColumnNumber(loc));
                if (reportedMoveUses_.insert(key).second) {
                    emitDiag(diagnostics_, loc, sm,
                             moveRule->getCategory(), moveRule->getId(),
                             "Variable used after move / 变量在移动后被使用",
                             Severity::Error);
                }
            }
        }
    }
    return true;
}

// ─── Step 2: Safety-pattern visitors ───────────────────────────────────────

bool TCCASTVisitor::VisitCXXMemberCallExpr(clang::CXXMemberCallExpr* expr) {
    if (!expr || !isInMainFile(expr->getBeginLoc())) {
        return true;
    }
    // Detect unsafe .value() / .get() on std::optional
    // 检测对 std::optional 的不安全 .value() / .get() 调用
    for (const auto& rule : rules_) {
        if (auto* unwrapRule = dynamic_cast<DetectUnsafeUnwrapRule*>(rule.get())) {
            if (unwrapRule->isUnsafeUnwrap(expr)) {
                emitDiag(diagnostics_, expr->getBeginLoc(),
                         context_.getSourceManager(),
                         unwrapRule->getCategory(), unwrapRule->getId(),
                         "Unsafe unwrap detected - check value first / "
                         "检测到不安全的 unwrap - 请先检查值",
                         Severity::Warning);
            }
        }
    }
    return true;
}

bool TCCASTVisitor::VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr) {
    if (!expr || !isInMainFile(expr->getBeginLoc())) {
        return true;
    }
    // Detect unchecked array subscript access
    // 检测未经检查的数组下标访问
    for (const auto& rule : rules_) {
        if (auto* boundsRule = dynamic_cast<EnforceBoundsCheckRule*>(rule.get())) {
            if (!boundsRule->isBoundsChecked(expr, context_)) {
                emitDiag(diagnostics_, expr->getBeginLoc(),
                         context_.getSourceManager(),
                         boundsRule->getCategory(), boundsRule->getId(),
                         "Array access requires bounds check / "
                         "数组访问需要边界检查",
                         Severity::Warning);
            }
        }
    }
    return true;
}

bool TCCASTVisitor::VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* expr) {
    if (!expr || !isInMainFile(expr->getBeginLoc())) {
        return true;
    }
    // Suggest .at() over operator[] for std::vector
    // 对 std::vector 建议使用 .at() 而非 operator[]
    for (const auto& rule : rules_) {
        if (auto* boundsRule = dynamic_cast<EnforceBoundsCheckRule*>(rule.get())) {
            if (boundsRule->shouldUseAtMethod(expr)) {
                emitDiag(diagnostics_, expr->getBeginLoc(),
                         context_.getSourceManager(),
                         boundsRule->getCategory(), boundsRule->getId(),
                         "Consider using .at() for bounds-checked access / "
                         "考虑使用 .at() 进行边界检查访问",
                         Severity::Note);
            }
        }
    }
    return true;
}

} // namespace tcc
