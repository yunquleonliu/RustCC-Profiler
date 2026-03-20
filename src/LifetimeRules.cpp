// Rust C/C++ Profiler - Lifetime Rules Implementation
// Rust C/C++ 分析器 - 生命周期规则实现

#include "tcc/LifetimeRules.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

namespace tcc {

// Helper: Check if expression refers to local variable
// 辅助函数：检查表达式是否引用局部变量
static bool refersToLocal(clang::Expr* expr) {
    if (!expr) {
        return false;
    }
    
    // Strip casts and parentheses / 去除类型转换和括号
    expr = expr->IgnoreParenCasts();
    
    // Check for DeclRefExpr to local variable / 检查对局部变量的 DeclRefExpr
    if (auto* declRef = llvm::dyn_cast<clang::DeclRefExpr>(expr)) {
        if (auto* varDecl = llvm::dyn_cast<clang::VarDecl>(declRef->getDecl())) {
            return varDecl->hasLocalStorage();
        }
    }
    
    // Check for UnaryOperator (address-of) / 检查 UnaryOperator（取地址）
    if (auto* unary = llvm::dyn_cast<clang::UnaryOperator>(expr)) {
        if (unary->getOpcode() == clang::UO_AddrOf) {
            return refersToLocal(unary->getSubExpr());
        }
    }
    
    return false;
}

// ForbidDanglingRefRule Implementation / ForbidDanglingRefRule 实现

class DanglingRefFinder : public clang::RecursiveASTVisitor<DanglingRefFinder> {
public:
    explicit DanglingRefFinder(ForbidDanglingRefRule* rule,
                              clang::ASTContext& context,
                              DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitFunctionDecl(clang::FunctionDecl* func) {
        if (!func || !func->hasBody() || !isInMainFile(func->getLocation())) {
            return true;
        }
        
        currentFunction_ = func;
        return true;
    }
    
    bool VisitReturnStmt(clang::ReturnStmt* stmt) {
        if (!stmt || !currentFunction_ || !isInMainFile(stmt->getReturnLoc())) {
            return true;
        }
        
        rule_->checkReturnStmt(stmt, currentFunction_, context_, diagnostics_);
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    ForbidDanglingRefRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
    clang::FunctionDecl* currentFunction_ = nullptr;
};

void ForbidDanglingRefRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    DanglingRefFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

void ForbidDanglingRefRule::checkReturnStmt(clang::ReturnStmt* stmt,
                                           clang::FunctionDecl* func,
                                           clang::ASTContext& context,
                                           DiagnosticEngine& diagnostics) {
    auto returnType = func->getReturnType();
    
    // Only check if returning reference / 仅在返回引用时检查
    if (!returnType->isReferenceType()) {
        return;
    }
    
    auto* returnValue = stmt->getRetValue();
    if (!returnValue) {
        return;
    }
    
    // Check if returning reference to local / 检查是否返回局部变量的引用
    if (refersToLocal(returnValue)) {
        const auto& sm = context.getSourceManager();
        auto loc = stmt->getReturnLoc();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(
            presumedLoc.getFilename(),
            presumedLoc.getLine(),
            presumedLoc.getColumn()
        );
        
        Diagnostic diag(
            Severity::Error,
            "Returning reference to local variable (dangling reference) / "
            "返回局部变量的引用（悬空引用）",
            srcLoc,
            RuleCategory::Lifetime,
            getId()
        );
        
        diag.addFixHint("Return by value instead of by reference / "
                       "按值返回而不是按引用返回");
        diag.addFixHint("Return reference to member variable or parameter / "
                       "返回成员变量或参数的引用");
        diag.addFixHint("Use std::unique_ptr or std::shared_ptr for heap allocation / "
                       "对堆分配使用 std::unique_ptr 或 std::shared_ptr");
        
        diag.addEscapePath("Remove @tcc annotation / 移除 @tcc 注解");
        
        diagnostics.report(std::move(diag));
    }
}

// ForbidDanglingPtrRule Implementation / ForbidDanglingPtrRule 实现

class DanglingPtrFinder : public clang::RecursiveASTVisitor<DanglingPtrFinder> {
public:
    explicit DanglingPtrFinder(ForbidDanglingPtrRule* rule,
                              clang::ASTContext& context,
                              DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitFunctionDecl(clang::FunctionDecl* func) {
        if (!func || !func->hasBody() || !isInMainFile(func->getLocation())) {
            return true;
        }
        
        currentFunction_ = func;
        return true;
    }
    
    bool VisitReturnStmt(clang::ReturnStmt* stmt) {
        if (!stmt || !currentFunction_ || !isInMainFile(stmt->getReturnLoc())) {
            return true;
        }
        
        rule_->checkReturnStmt(stmt, currentFunction_, context_, diagnostics_);
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    ForbidDanglingPtrRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
    clang::FunctionDecl* currentFunction_ = nullptr;
};

void ForbidDanglingPtrRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    DanglingPtrFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

void ForbidDanglingPtrRule::checkReturnStmt(clang::ReturnStmt* stmt,
                                           clang::FunctionDecl* func,
                                           clang::ASTContext& context,
                                           DiagnosticEngine& diagnostics) {
    auto returnType = func->getReturnType();
    
    // Only check if returning pointer / 仅在返回指针时检查
    if (!returnType->isPointerType()) {
        return;
    }
    
    auto* returnValue = stmt->getRetValue();
    if (!returnValue) {
        return;
    }
    
    // Check if returning pointer to local / 检查是否返回局部变量的指针
    if (refersToLocal(returnValue)) {
        const auto& sm = context.getSourceManager();
        auto loc = stmt->getReturnLoc();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(
            presumedLoc.getFilename(),
            presumedLoc.getLine(),
            presumedLoc.getColumn()
        );
        
        Diagnostic diag(
            Severity::Error,
            "Returning pointer to local variable (dangling pointer) / "
            "返回局部变量的指针（悬空指针）",
            srcLoc,
            RuleCategory::Lifetime,
            getId()
        );
        
        diag.addFixHint("Return std::unique_ptr<T> instead / "
                       "返回 std::unique_ptr<T>");
        diag.addFixHint("Return by value / 按值返回");
        diag.addFixHint("Allocate on heap with smart pointer / "
                       "使用智能指针在堆上分配");
        
        diag.addEscapePath("Remove @tcc annotation / 移除 @tcc 注解");
        
        diagnostics.report(std::move(diag));
    }
}

// ForbidRawPtrContainerRule Implementation / ForbidRawPtrContainerRule 实现

class RawPtrContainerFinder : public clang::RecursiveASTVisitor<RawPtrContainerFinder> {
public:
    explicit RawPtrContainerFinder(ForbidRawPtrContainerRule* rule,
                                  clang::ASTContext& context,
                                  DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitVarDecl(clang::VarDecl* decl) {
        if (!decl || !isInMainFile(decl->getLocation())) {
            return true;
        }
        
        if (rule_->isRawPointerContainer(decl->getType())) {
            reportViolation(decl);
        }
        
        return true;
    }
    
    bool VisitFieldDecl(clang::FieldDecl* decl) {
        if (!decl || !isInMainFile(decl->getLocation())) {
            return true;
        }
        
        if (rule_->isRawPointerContainer(decl->getType())) {
            reportViolation(decl);
        }
        
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    void reportViolation(clang::DeclaratorDecl* decl) {
        const auto& sm = context_.getSourceManager();
        auto loc = decl->getLocation();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(
            presumedLoc.getFilename(),
            presumedLoc.getLine(),
            presumedLoc.getColumn()
        );
        
        Diagnostic diag(
            Severity::Error,
            "Container storing raw pointers (lifetime unclear) / "
            "容器存储原始指针（生命周期不明确）",
            srcLoc,
            RuleCategory::Lifetime,
            "TCC-LIFE-003"
        );
        
        diag.addFixHint("Use std::vector<std::unique_ptr<T>> / "
                       "使用 std::vector<std::unique_ptr<T>>");
        diag.addFixHint("Use std::vector<std::shared_ptr<T>> / "
                       "使用 std::vector<std::shared_ptr<T>>");
        diag.addFixHint("Store values instead of pointers / "
                       "存储值而不是指针");
        
        diag.addEscapePath("Remove @tcc annotation / 移除 @tcc 注解");
        
        diagnostics_.report(std::move(diag));
    }
    
    ForbidRawPtrContainerRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

void ForbidRawPtrContainerRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    RawPtrContainerFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

bool ForbidRawPtrContainerRule::isRawPointerContainer(clang::QualType type) const {
    // Check for std::vector<T*>, std::list<T*>, etc.
    // 检查 std::vector<T*>、std::list<T*> 等
    
    const auto* recordType = type->getAsCXXRecordDecl();
    if (!recordType) {
        return false;
    }
    
    std::string typeName = recordType->getNameAsString();
    
    // Check if it's a standard container / 检查是否是标准容器
    if (typeName != "vector" && typeName != "list" && 
        typeName != "deque" && typeName != "set") {
        return false;
    }
    
    // Check if template argument is a pointer / 检查模板参数是否是指针
    if (const auto* templateSpec = llvm::dyn_cast<clang::ClassTemplateSpecializationDecl>(recordType)) {
        const auto& templateArgs = templateSpec->getTemplateArgs();
        if (templateArgs.size() > 0) {
            auto argType = templateArgs[0].getAsType();
            return argType->isPointerType();
        }
    }
    
    return false;
}

// ForbidUntrackedRefMemberRule Implementation / ForbidUntrackedRefMemberRule 实现

class UntrackedRefMemberFinder : public clang::RecursiveASTVisitor<UntrackedRefMemberFinder> {
public:
    explicit UntrackedRefMemberFinder(clang::ASTContext& context,
                                     DiagnosticEngine& diagnostics)
        : context_(context), diagnostics_(diagnostics) {}
    
    bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl) {
        if (!decl || !decl->isCompleteDefinition() || !isInMainFile(decl->getLocation())) {
            return true;
        }
        
        // Check all fields / 检查所有字段
        for (auto* field : decl->fields()) {
            if (field->getType()->isReferenceType()) {
                reportViolation(field);
            }
        }
        
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    void reportViolation(clang::FieldDecl* field) {
        const auto& sm = context_.getSourceManager();
        auto loc = field->getLocation();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(
            presumedLoc.getFilename(),
            presumedLoc.getLine(),
            presumedLoc.getColumn()
        );
        
        Diagnostic diag(
            Severity::Error,
            "Reference member without clear lifetime tracking / "
            "没有明确生命周期跟踪的引用成员",
            srcLoc,
            RuleCategory::Lifetime,
            "TCC-LIFE-004"
        );
        
        diag.addFixHint("Use std::reference_wrapper<T> for clearer semantics / "
                       "使用 std::reference_wrapper<T> 以获得更清晰的语义");
        diag.addFixHint("Store by value if possible / 如果可能按值存储");
        diag.addFixHint("Use pointer with ownership documentation / "
                       "使用指针并记录所有权");
        
        diag.addEscapePath("Document lifetime dependency clearly / "
                          "清楚地记录生命周期依赖关系");
        
        diagnostics_.report(std::move(diag));
    }
    
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

void ForbidUntrackedRefMemberRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    UntrackedRefMemberFinder finder(context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

} // namespace tcc

