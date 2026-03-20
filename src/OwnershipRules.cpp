// Rust C/C++ Profiler - Ownership Rules Implementation
// Rust C/C++ 分析器 - 所有权规则实现

#include "tcc/OwnershipRules.h"
#include "tcc/ASTVisitor.h"

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

namespace tcc {

// Helper visitor for finding new expressions / 用于查找 new 表达式的辅助访问者
class NewExprFinder : public clang::RecursiveASTVisitor<NewExprFinder> {
public:
    explicit NewExprFinder(ForbidNewRule* rule,
                          clang::ASTContext& context,
                          DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitCXXNewExpr(clang::CXXNewExpr* expr) {
        if (expr && isInMainFile(expr->getBeginLoc())) {
            rule_->checkNewExpr(expr, context_, diagnostics_);
        }
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    ForbidNewRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

// Helper visitor for finding delete expressions / 用于查找 delete 表达式的辅助访问者
class DeleteExprFinder : public clang::RecursiveASTVisitor<DeleteExprFinder> {
public:
    explicit DeleteExprFinder(ForbidDeleteRule* rule,
                             clang::ASTContext& context,
                             DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* expr) {
        if (expr && isInMainFile(expr->getBeginLoc())) {
            rule_->checkDeleteExpr(expr, context_, diagnostics_);
        }
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    ForbidDeleteRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

// ForbidNewRule Implementation / ForbidNewRule 实现

void ForbidNewRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    NewExprFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

void ForbidNewRule::checkNewExpr(clang::CXXNewExpr* expr,
                                 clang::ASTContext& context,
                                 DiagnosticEngine& diagnostics) {
    const auto& sm = context.getSourceManager();
    auto loc = expr->getBeginLoc();
    
    if (!sm.isInMainFile(loc)) {
        return;
    }
    
    auto presumedLoc = sm.getPresumedLoc(loc);
    SourceLocation srcLoc(
        presumedLoc.getFilename(),
        presumedLoc.getLine(),
        presumedLoc.getColumn()
    );
    
    // Create diagnostic / 创建诊断
    Diagnostic diag(
        Severity::Error,
        "Use of 'new' operator is forbidden in RCC code / "
        "RCC 代码中禁止使用 'new' 操作符",
        srcLoc,
        RuleCategory::Ownership,
        getId()
    );
    
    // Add fix suggestions / 添加修复建议
    diag.addFixHint("Use std::make_unique<T>() for single objects / "
                   "对单个对象使用 std::make_unique<T>()");
    diag.addFixHint("Use std::make_shared<T>() for shared ownership / "
                   "对共享所有权使用 std::make_shared<T>()");
    diag.addFixHint("Use std::vector<T> or other containers for arrays / "
                   "对数组使用 std::vector<T> 或其他容器");
    
    // Add escape paths / 添加逃生路径
    diag.addEscapePath("Remove @tcc annotation to use raw C++ / "
                      "移除 @tcc 注解以使用原始 C++");
    diag.addEscapePath("Move this code to a non-RCC file / "
                      "将此代码移至非 RCC 文件");
    
    diagnostics.report(std::move(diag));
}

// ForbidDeleteRule Implementation / ForbidDeleteRule 实现

void ForbidDeleteRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    DeleteExprFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

void ForbidDeleteRule::checkDeleteExpr(clang::CXXDeleteExpr* expr,
                                       clang::ASTContext& context,
                                       DiagnosticEngine& diagnostics) {
    const auto& sm = context.getSourceManager();
    auto loc = expr->getBeginLoc();
    
    if (!sm.isInMainFile(loc)) {
        return;
    }
    
    auto presumedLoc = sm.getPresumedLoc(loc);
    SourceLocation srcLoc(
        presumedLoc.getFilename(),
        presumedLoc.getLine(),
        presumedLoc.getColumn()
    );
    
    // Create diagnostic / 创建诊断
    Diagnostic diag(
        Severity::Error,
        "Use of 'delete' operator is forbidden in RCC code / "
        "RCC 代码中禁止使用 'delete' 操作符",
        srcLoc,
        RuleCategory::Ownership,
        getId()
    );
    
    // Add fix suggestions / 添加修复建议
    diag.addFixHint("Use smart pointers (std::unique_ptr, std::shared_ptr) with automatic cleanup / "
                   "使用智能指针（std::unique_ptr、std::shared_ptr）自动清理");
    diag.addFixHint("Use RAII pattern for resource management / "
                   "使用 RAII 模式管理资源");
    
    // Add escape paths / 添加逃生路径
    diag.addEscapePath("Remove @tcc annotation to use raw C++ / "
                      "移除 @tcc 注解以使用原始 C++");
    diag.addEscapePath("Move this code to a non-RCC file / "
                      "将此代码移至非 RCC 文件");
    
    diagnostics.report(std::move(diag));
}

// ForbidMallocFreeRule Implementation / ForbidMallocFreeRule 实现

// Helper visitor for finding malloc/free calls / 用于查找 malloc/free 调用的辅助访问者
class MallocFreeCallFinder : public clang::RecursiveASTVisitor<MallocFreeCallFinder> {
public:
    explicit MallocFreeCallFinder(ForbidMallocFreeRule* rule,
                                 clang::ASTContext& context,
                                 DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitCallExpr(clang::CallExpr* call) {
        if (!call || !isInMainFile(call->getBeginLoc())) {
            return true;
        }
        
        auto* callee = call->getDirectCallee();
        if (!callee) {
            return true;
        }
        
        std::string funcName = callee->getNameAsString();
        
        // Check for malloc, calloc, realloc, free
        // 检查 malloc、calloc、realloc、free
        if (funcName == "malloc" || funcName == "calloc" || 
            funcName == "realloc" || funcName == "free") {
            reportViolation(call, funcName);
        }
        
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    void reportViolation(clang::CallExpr* call, const std::string& funcName) {
        const auto& sm = context_.getSourceManager();
        auto loc = call->getBeginLoc();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(
            presumedLoc.getFilename(),
            presumedLoc.getLine(),
            presumedLoc.getColumn()
        );
        
        Diagnostic diag(
            Severity::Error,
            "Use of '" + funcName + "' is forbidden in RCC code / "
            "RCC 代码中禁止使用 '" + funcName + "'",
            srcLoc,
            RuleCategory::Ownership,
            "TCC-OWN-003"
        );
        
        // Fix suggestions / 修复建议
        if (funcName == "malloc" || funcName == "calloc" || funcName == "realloc") {
            diag.addFixHint("Use std::vector<T> for arrays / "
                           "对数组使用 std::vector<T>");
            diag.addFixHint("Use std::make_unique<T>() for single objects / "
                           "对单个对象使用 std::make_unique<T>()");
            diag.addFixHint("Use standard containers (std::string, std::array, etc.) / "
                           "使用标准容器（std::string、std::array 等）");
        } else {
            diag.addFixHint("Use smart pointers with automatic cleanup / "
                           "使用智能指针自动清理");
            diag.addFixHint("Use RAII pattern / 使用 RAII 模式");
        }
        
        // Escape paths / 逃生路径
        diag.addEscapePath("Remove @tcc annotation to use raw C / "
                          "移除 @tcc 注解以使用原始 C");
        diag.addEscapePath("Move this code to a non-RCC file / "
                          "将此代码移至非 RCC 文件");
        
        diagnostics_.report(std::move(diag));
    }
    
    ForbidMallocFreeRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

void ForbidMallocFreeRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    MallocFreeCallFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

// RawOwningPointerRule Implementation / RawOwningPointerRule 实现

// Helper visitor for analyzing function return types / 用于分析函数返回类型的辅助访问者
class OwningPointerFunctionFinder : public clang::RecursiveASTVisitor<OwningPointerFunctionFinder> {
public:
    explicit OwningPointerFunctionFinder(RawOwningPointerRule* rule,
                                        clang::ASTContext& context,
                                        DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitFunctionDecl(clang::FunctionDecl* decl) {
        if (!decl || !isInMainFile(decl->getLocation())) {
            return true;
        }
        
        // Skip functions without body (declarations only)
        // 跳过没有函数体的函数（仅声明）
        if (!decl->hasBody()) {
            return true;
        }
        
        // Check if function returns raw pointer with ownership semantics
        // 检查函数是否返回具有所有权语义的原始指针
        if (rule_->isOwningPointerReturn(decl)) {
            reportViolation(decl);
        }
        
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    void reportViolation(clang::FunctionDecl* decl) {
        const auto& sm = context_.getSourceManager();
        auto loc = decl->getLocation();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(
            presumedLoc.getFilename(),
            presumedLoc.getLine(),
            presumedLoc.getColumn()
        );
        
        std::string funcName = decl->getNameAsString();
        
        Diagnostic diag(
            Severity::Warning,
            "Function '" + funcName + "' returns raw pointer with ownership semantics / "
            "函数 '" + funcName + "' 返回具有所有权语义的原始指针",
            srcLoc,
            RuleCategory::Ownership,
            "TCC-OWN-004"
        );
        
        // Fix suggestions / 修复建议
        diag.addFixHint("Return std::unique_ptr<T> instead of T* / "
                       "返回 std::unique_ptr<T> 而不是 T*");
        diag.addFixHint("Return std::shared_ptr<T> for shared ownership / "
                       "对共享所有权返回 std::shared_ptr<T>");
        diag.addFixHint("Return by value if the object is small / "
                       "如果对象较小则按值返回");
        
        // Escape paths / 逃生路径
        diag.addEscapePath("Use non-owning raw pointer (document ownership) / "
                          "使用非所有权原始指针（记录所有权）");
        diag.addEscapePath("Remove @tcc annotation / 移除 @tcc 注解");
        
        diagnostics_.report(std::move(diag));
    }
    
    RawOwningPointerRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

void RawOwningPointerRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    OwningPointerFunctionFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

bool RawOwningPointerRule::isOwningPointerReturn(clang::FunctionDecl* decl) const {
    if (!decl) {
        return false;
    }
    
    auto returnType = decl->getReturnType();
    
    // Check if returns a raw pointer / 检查是否返回原始指针
    if (!returnType->isPointerType()) {
        return false;
    }
    
    // Check if function name suggests ownership (e.g., "create", "make")
    // 检查函数名是否暗示所有权（例如 "create"、"make"）
    std::string name = decl->getNameAsString();
    return name.find("create") != std::string::npos ||
           name.find("make") != std::string::npos ||
           name.find("alloc") != std::string::npos;
}

} // namespace tcc

