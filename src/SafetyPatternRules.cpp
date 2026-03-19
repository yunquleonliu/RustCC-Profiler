// Rust C/C++ Profiler - Safety Pattern Rules Implementation
// Rust C/C++ 分析器 - 安全模式规则实现

#include "tcc/SafetyPatternRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>

namespace tcc {

//=============================================================================
// EnforceNullCheckRule Implementation
//=============================================================================

void EnforceNullCheckRule::check(clang::ASTContext& context,
                                DiagnosticEngine& diagnostics) {
    // Traverse AST to find pointer dereferences
    // 遍历 AST 查找指针解引用
    class NullCheckVisitor : public clang::RecursiveASTVisitor<NullCheckVisitor> {
    public:
        NullCheckVisitor(EnforceNullCheckRule* rule,
                        clang::ASTContext& ctx,
                        DiagnosticEngine& diag)
            : rule_(rule), context_(ctx), diagnostics_(diag) {}
        
        bool VisitUnaryOperator(clang::UnaryOperator* op) {
            if (op->getOpcode() == clang::UO_Deref) {
                // Check if pointer is checked before deref
                // 检查指针在解引用前是否被检查
                if (rule_->isUsedWithoutCheck(op->getSubExpr(), context_)) {
                    emitDiag(diagnostics_, op->getLocation(),
                             context_.getSourceManager(),
                             rule_->getCategory(), rule_->getId(),
                             "Pointer dereferenced without null check / "
                             "指针在未检查空值的情况下被解引用",
                             Severity::Error);
                }
            }
            return true;
        }
        
    private:
        EnforceNullCheckRule* rule_;
        clang::ASTContext& context_;
        DiagnosticEngine& diagnostics_;
    };
    
    NullCheckVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool EnforceNullCheckRule::isUsedWithoutCheck(const clang::Expr* expr,
                                             clang::ASTContext& context) const {
    // Simplified check - in real implementation, would need data flow analysis
    // 简化检查 - 在实际实现中需要数据流分析
    // Check if expr is preceded by null check in same scope
    // 检查表达式在同一作用域中是否有前置的空值检查
    return false;  // Placeholder - assume checked for now / 占位符 - 暂时假设已检查
}

bool EnforceNullCheckRule::shouldUseOptional(const clang::QualType& type) const {
    // Recommend std::optional for nullable return types
    // 对可空返回类型推荐使用 std::optional
    return type->isPointerType() && !type->isFunctionPointerType();
}

//=============================================================================
// PreferOptionalRule Implementation
//=============================================================================

void PreferOptionalRule::check(clang::ASTContext& context,
                              DiagnosticEngine& diagnostics) {
    // Check function return types / 检查函数返回类型
    for (auto* decl : context.getTranslationUnitDecl()->decls()) {
        if (auto* func = clang::dyn_cast<clang::FunctionDecl>(decl)) {
            if (returnsNullablePointer(func)) {
                emitDiag(diagnostics, func->getLocation(),
                         context.getSourceManager(),
                         getCategory(), getId(),
                         "Consider using std::optional instead of nullable pointer / "
                         "考虑使用 std::optional 代替可空指针",
                         Severity::Warning);
            }
        }
    }
}

bool PreferOptionalRule::returnsNullablePointer(
    const clang::FunctionDecl* func) const {
    // Check if function returns a pointer that could be null
    // 检查函数是否返回可能为空的指针
    clang::QualType returnType = func->getReturnType();
    return returnType->isPointerType() && !returnType->isFunctionPointerType();
}

//=============================================================================
// EnforceResultHandlingRule Implementation
//=============================================================================

void EnforceResultHandlingRule::check(clang::ASTContext& context,
                                     DiagnosticEngine& diagnostics) {
    // Find call expressions with Result-type returns
    // 查找返回 Result 类型的调用表达式
    class ResultCheckVisitor : public clang::RecursiveASTVisitor<ResultCheckVisitor> {
    public:
        ResultCheckVisitor(EnforceResultHandlingRule* rule,
                          clang::ASTContext& ctx,
                          DiagnosticEngine& diag)
            : rule_(rule), context_(ctx), diagnostics_(diag) {}
        
        bool VisitCallExpr(clang::CallExpr* call) {
            if (rule_->isResultType(call->getType())) {
                if (rule_->isResultIgnored(call)) {
                    emitDiag(diagnostics_, call->getBeginLoc(),
                             context_.getSourceManager(),
                             rule_->getCategory(), rule_->getId(),
                             "Result type must be handled / Result 类型必须被处理",
                             Severity::Error);
                }
            }
            return true;
        }
        
    private:
        EnforceResultHandlingRule* rule_;
        clang::ASTContext& context_;
        DiagnosticEngine& diagnostics_;
    };
    
    ResultCheckVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool EnforceResultHandlingRule::isResultIgnored(const clang::CallExpr* call) const {
    // Check if call result is used / 检查调用结果是否被使用
    // In expression statement without assignment = ignored
    // 在表达式语句中未赋值 = 被忽略
    return false;  // Placeholder / 占位符
}

bool EnforceResultHandlingRule::isResultType(const clang::QualType& type) const {
    // Check if type is std::variant<T, E> or similar Result type
    // 检查类型是否为 std::variant<T, E> 或类似的 Result 类型
    std::string typeName = type.getAsString();
    return typeName.find("std::variant") != std::string::npos;
}

//=============================================================================
// UncheckedErrorReturnRule Implementation
//=============================================================================

void UncheckedErrorReturnRule::check(clang::ASTContext& context,
                                    DiagnosticEngine& diagnostics) {
    // Check [[nodiscard]] attributes / 检查 [[nodiscard]] 属性
    class NodiscardVisitor : public clang::RecursiveASTVisitor<NodiscardVisitor> {
    public:
        NodiscardVisitor(UncheckedErrorReturnRule* rule,
                        clang::ASTContext& ctx,
                        DiagnosticEngine& diag)
            : rule_(rule), context_(ctx), diagnostics_(diag) {}
        
        bool VisitCallExpr(clang::CallExpr* call) {
            if (rule_->isNodiscardViolated(call)) {
                emitDiag(diagnostics_, call->getBeginLoc(),
                         context_.getSourceManager(),
                         rule_->getCategory(), rule_->getId(),
                         "Function return value must be checked / "
                         "函数返回值必须被检查",
                         Severity::Error);
            }
            return true;
        }
        
    private:
        UncheckedErrorReturnRule* rule_;
        clang::ASTContext& context_;
        DiagnosticEngine& diagnostics_;
    };
    
    NodiscardVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

void UncheckedErrorReturnRule::markMustUse(const clang::FunctionDecl* func) {
    // Mark function as requiring checked return
    // 标记函数需要检查返回值
}

bool UncheckedErrorReturnRule::isNodiscardViolated(
    const clang::CallExpr* call) const {
    // Check if [[nodiscard]] function result is ignored
    // 检查 [[nodiscard]] 函数结果是否被忽略
    if (auto* func = call->getDirectCallee()) {
        return func->hasAttr<clang::WarnUnusedResultAttr>();
    }
    return false;
}

//=============================================================================
// DetectUnsafeUnwrapRule Implementation
//=============================================================================

void DetectUnsafeUnwrapRule::check(clang::ASTContext& context,
                                  DiagnosticEngine& diagnostics) {
    // Detect .value() or .get() without checking
    // 检测未经检查的 .value() 或 .get()
    class UnwrapVisitor : public clang::RecursiveASTVisitor<UnwrapVisitor> {
    public:
        UnwrapVisitor(DetectUnsafeUnwrapRule* rule,
                     clang::ASTContext& ctx,
                     DiagnosticEngine& diag)
            : rule_(rule), context_(ctx), diagnostics_(diag) {}
        
        bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr* call) {
            if (rule_->isUnsafeUnwrap(call)) {
                emitDiag(diagnostics_, call->getBeginLoc(),
                         context_.getSourceManager(),
                         rule_->getCategory(), rule_->getId(),
                         "Unsafe unwrap detected - check value first / "
                         "检测到不安全的 unwrap - 请先检查值",
                         Severity::Warning);
            }
            return true;
        }
        
    private:
        DetectUnsafeUnwrapRule* rule_;
        clang::ASTContext& context_;
        DiagnosticEngine& diagnostics_;
    };
    
    UnwrapVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool DetectUnsafeUnwrapRule::isUnsafeUnwrap(
    const clang::CXXMemberCallExpr* call) const {
    // Check if method is value() without prior check
    // 检查方法是否为 value() 且未事先检查
    if (auto* method = call->getMethodDecl()) {
        std::string name = method->getNameAsString();
        return (name == "value" || name == "get");
    }
    return false;
}

//=============================================================================
// ForbidPanicRule Implementation
//=============================================================================

void ForbidPanicRule::check(clang::ASTContext& context,
                           DiagnosticEngine& diagnostics) {
    // Detect calls to panic/abort functions
    // 检测对 panic/abort 函数的调用
    class PanicVisitor : public clang::RecursiveASTVisitor<PanicVisitor> {
    public:
        PanicVisitor(ForbidPanicRule* rule,
                    clang::ASTContext& ctx,
                    DiagnosticEngine& diag)
            : rule_(rule), context_(ctx), diagnostics_(diag) {}
        
        bool VisitCallExpr(clang::CallExpr* call) {
            if (auto* func = call->getDirectCallee()) {
                if (rule_->isPanicFunction(func)) {
                    emitDiag(diagnostics_, call->getBeginLoc(),
                             context_.getSourceManager(),
                             rule_->getCategory(), rule_->getId(),
                             "Direct panic/abort call forbidden / "
                             "禁止直接调用 panic/abort",
                             Severity::Error);
                }
            }
            return true;
        }
        
    private:
        ForbidPanicRule* rule_;
        clang::ASTContext& context_;
        DiagnosticEngine& diagnostics_;
    };
    
    PanicVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool ForbidPanicRule::isPanicFunction(const clang::FunctionDecl* func) const {
    std::string name = func->getNameAsString();
    return (name == "abort" || name == "exit" || 
            name == "terminate" || name == "_Exit");
}

//=============================================================================
// EnforceBoundsCheckRule Implementation
//=============================================================================

void EnforceBoundsCheckRule::check(clang::ASTContext& context,
                                  DiagnosticEngine& diagnostics) {
    // Check array subscript operations
    // 检查数组下标操作
    class BoundsVisitor : public clang::RecursiveASTVisitor<BoundsVisitor> {
    public:
        BoundsVisitor(EnforceBoundsCheckRule* rule,
                     clang::ASTContext& ctx,
                     DiagnosticEngine& diag)
            : rule_(rule), context_(ctx), diagnostics_(diag) {}
        
        bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr) {
            if (!rule_->isBoundsChecked(expr, context_)) {
                emitDiag(diagnostics_, expr->getBeginLoc(),
                         context_.getSourceManager(),
                         rule_->getCategory(), rule_->getId(),
                         "Array access requires bounds check / "
                         "数组访问需要边界检查",
                         Severity::Warning);
            }
            return true;
        }
        
        bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* expr) {
            if (rule_->shouldUseAtMethod(expr)) {
                emitDiag(diagnostics_, expr->getBeginLoc(),
                         context_.getSourceManager(),
                         rule_->getCategory(), rule_->getId(),
                         "Consider using .at() for bounds-checked access / "
                         "考虑使用 .at() 进行边界检查访问",
                         Severity::Note);
            }
            return true;
        }
        
    private:
        EnforceBoundsCheckRule* rule_;
        clang::ASTContext& context_;
        DiagnosticEngine& diagnostics_;
    };
    
    BoundsVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

bool EnforceBoundsCheckRule::isBoundsChecked(
    const clang::ArraySubscriptExpr* expr,
    clang::ASTContext& context) const {
    // Check if index is validated / 检查索引是否已验证
    // This would require data flow analysis in full implementation
    // 在完整实现中需要数据流分析
    return false;  // Placeholder / 占位符
}

bool EnforceBoundsCheckRule::shouldUseAtMethod(
    const clang::CXXOperatorCallExpr* expr) const {
    // Check if it's vector::operator[] / 检查是否为 vector::operator[]
    if (expr->getOperator() == clang::OO_Subscript) {
        // Check if base is std::vector / 检查基础类型是否为 std::vector
        return true;  // Simplified / 简化版
    }
    return false;
}

} // namespace tcc

