// Rust C/C++ Profiler - Concurrency Rules Implementation
// Rust C/C++ 分析器 - 并发规则实现

#include "tcc/ConcurrencyRules.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>

namespace tcc {

// ForbidUnsyncSharedStateRule Implementation

class UnsyncSharedStateFinder : public clang::RecursiveASTVisitor<UnsyncSharedStateFinder> {
public:
    explicit UnsyncSharedStateFinder(ForbidUnsyncSharedStateRule* rule,
                                    clang::ASTContext& context,
                                    DiagnosticEngine& diagnostics)
        : rule_(rule), context_(context), diagnostics_(diagnostics) {}
    
    bool VisitVarDecl(clang::VarDecl* decl) {
        if (!decl || !isInMainFile(decl->getLocation())) {
            return true;
        }
        
        if (rule_->isMutableSharedState(decl)) {
            reportViolation(decl);
        }
        
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    void reportViolation(clang::VarDecl* decl) {
        const auto& sm = context_.getSourceManager();
        auto loc = decl->getLocation();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(presumedLoc.getFilename(), presumedLoc.getLine(), presumedLoc.getColumn());
        
        Diagnostic diag(
            Severity::Error,
            "Global/static mutable state without synchronization / "
            "全局/静态可变状态没有同步",
            srcLoc, RuleCategory::Concurrency, "TCC-CONC-001"
        );
        
        diag.addFixHint("Use std::atomic<T> for simple types / 对简单类型使用 std::atomic<T>");
        diag.addFixHint("Use std::mutex for complex state / 对复杂状态使用 std::mutex");
        diag.addFixHint("Use thread_local for thread-specific state / 对线程特定状态使用 thread_local");
        diag.addEscapePath("Document thread-safety explicitly / 明确记录线程安全性");
        
        diagnostics_.report(std::move(diag));
    }
    
    ForbidUnsyncSharedStateRule* rule_;
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

void ForbidUnsyncSharedStateRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    UnsyncSharedStateFinder finder(this, context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

bool ForbidUnsyncSharedStateRule::isMutableSharedState(clang::VarDecl* decl) const {
    if (!decl) return false;
    
    // Check if global or static / 检查是否是全局或静态
    bool isShared = decl->hasGlobalStorage() || decl->isStaticLocal();
    if (!isShared) return false;

    // Thread-local storage is NOT shared between threads / 线程局部存储不在线程间共享
    if (decl->getTLSKind() != clang::VarDecl::TLS_None) return false;

    // Check if mutable (not const) / 检查是否可变（非 const）
    if (decl->getType().isConstQualified()) return false;
    
    // Check if it's atomic (safe) / 检查是否是 atomic（安全）
    std::string typeName = decl->getType().getAsString();
    if (typeName.find("atomic") != std::string::npos) return false;
    if (typeName.find("mutex") != std::string::npos) return false;
    
    return true;
}

// ForbidNonConstLambdaCaptureRule Implementation

class NonConstLambdaCaptureFinder : public clang::RecursiveASTVisitor<NonConstLambdaCaptureFinder> {
public:
    explicit NonConstLambdaCaptureFinder(clang::ASTContext& context, DiagnosticEngine& diagnostics)
        : context_(context), diagnostics_(diagnostics) {}
    
    bool VisitLambdaExpr(clang::LambdaExpr* lambda) {
        if (!lambda || !isInMainFile(lambda->getBeginLoc())) {
            return true;
        }
        
        // Check each capture / 检查每个捕获
        for (auto capture : lambda->captures()) {
            if (capture.capturesVariable()) {
                auto* var = capture.getCapturedVar();
                if (var && !var->getType().isConstQualified() && 
                    capture.getCaptureKind() == clang::LCK_ByRef) {
                    // Only flag fundamental/pointer types - class types manage their own sync
                    // 仅标记基础/指针类型，类类型通常自行管理同步
                    auto varType = var->getType().getCanonicalType();
                    if (varType->isBuiltinType() || varType->isPointerType() ||
                        varType->isEnumeralType()) {
                        reportViolation(lambda, var);
                    }
                }
            }
        }
        
        return true;
    }
    
private:
    bool isInMainFile(clang::SourceLocation loc) const {
        return context_.getSourceManager().isInMainFile(loc);
    }
    
    void reportViolation(clang::LambdaExpr* lambda, clang::VarDecl* var) {
        const auto& sm = context_.getSourceManager();
        auto loc = lambda->getBeginLoc();
        auto presumedLoc = sm.getPresumedLoc(loc);
        
        SourceLocation srcLoc(presumedLoc.getFilename(), presumedLoc.getLine(), presumedLoc.getColumn());
        
        Diagnostic diag(
            Severity::Error,
            "Lambda captures non-const reference (potential data race) / "
            "Lambda 捕获非 const 引用（潜在数据竞争）",
            srcLoc, RuleCategory::Concurrency, "TCC-CONC-002"
        );
        
        diag.addFixHint("Capture by value instead: [=] / 改为按值捕获：[=]");
        diag.addFixHint("Capture as const reference if read-only / 如果只读则捕获为 const 引用");
        diag.addFixHint("Use std::atomic or mutex for shared state / 对共享状态使用 std::atomic 或 mutex");
        diag.addEscapePath("Remove @tcc annotation / 移除 @tcc 注解");
        
        diagnostics_.report(std::move(diag));
    }
    
    clang::ASTContext& context_;
    DiagnosticEngine& diagnostics_;
};

void ForbidNonConstLambdaCaptureRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    NonConstLambdaCaptureFinder finder(context, diagnostics);
    finder.TraverseDecl(context.getTranslationUnitDecl());
}

// ForbidRawPtrThreadSharingRule Implementation

void ForbidRawPtrThreadSharingRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    // Basic implementation - can be enhanced / 基本实现 - 可以增强
    // Currently covered by other rules / 目前由其他规则覆盖
}

// RequireAtomicForSharedCounterRule Implementation

void RequireAtomicForSharedCounterRule::check(clang::ASTContext& context, DiagnosticEngine& diagnostics) {
    // Enhanced by ForbidUnsyncSharedStateRule / 由 ForbidUnsyncSharedStateRule 增强
}

} // namespace tcc

