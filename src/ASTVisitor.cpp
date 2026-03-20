// Rust C/C++ Profiler - AST Visitor Implementation
// Rust C/C++ 分析器 - AST 访问者实现

#include "tcc/ASTVisitor.h"
#include "tcc/DiagnosticHelper.h"
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
    if (!expr || !isInMainFile(expr->getBeginLoc())) {
        return true;
    }
    // Ownership rules handle 'new' via their own visitors in check()
    // 所有权规则通过 check() 中的独立访问器处理 'new'
    return true;
}

bool TCCASTVisitor::VisitCXXDeleteExpr(clang::CXXDeleteExpr* expr) {
    if (!expr || !isInMainFile(expr->getBeginLoc())) {
        return true;
    }
    // Ownership rules handle 'delete' via their own visitors in check()
    // 所有权规则通过 check() 中的独立访问器处理 'delete'
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
    if (!expr || !isInMainFile(expr->getBeginLoc())) {
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


