// Rust C/C++ Profiler - Lifetime Rules Implementation
// Rust C/C++ 分析器 - 生命周期规则实现

#include "tcc/LifetimeRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ExprCXX.h>
#include <clang/AST/StmtCXX.h>
#include <clang/Basic/SourceManager.h>
#include <string>
#include <vector>

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

//=============================================================================
// TCC-ITER-001: IteratorInvalidationRule
// Detects modification of a container while iterating over it.
// 检测在迭代容器时对其进行修改。
//=============================================================================

namespace {

// Mutating methods that invalidate iterators for sequence containers.
// 使序列容器迭代器失效的修改方法。
static const std::vector<std::string>& invalidatingMethods() {
    static const std::vector<std::string> methods = {
        "push_back", "emplace_back", "push_front", "emplace_front",
        "insert", "emplace", "erase", "clear", "resize", "reserve",
        "pop_back", "pop_front", "assign", "swap"
    };
    return methods;
}

// Extract the base VarDecl from a member call expression like `v.push_back(x)`.
// 从成员调用表达式（如 `v.push_back(x)`）中提取基础 VarDecl。
static const clang::VarDecl* getCallBase(const clang::CXXMemberCallExpr* call) {
    if (!call) return nullptr;
    const clang::Expr* obj =
        call->getImplicitObjectArgument()->IgnoreParenImpCasts();
    if (const auto* dr = clang::dyn_cast<clang::DeclRefExpr>(obj)) {
        return clang::dyn_cast<clang::VarDecl>(dr->getDecl());
    }
    return nullptr;
}

// Walks the body of a loop looking for invalidating calls on `container`.
// 遍历循环体，查找对 `container` 的无效化调用。
class LoopBodyChecker
    : public clang::RecursiveASTVisitor<LoopBodyChecker> {
public:
    LoopBodyChecker(const clang::VarDecl* container,
                    IteratorInvalidationRule* rule,
                    clang::ASTContext& ctx,
                    DiagnosticEngine& diags)
        : container_(container), rule_(rule), ctx_(ctx), diags_(diags) {}

    bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr* call) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(call->getBeginLoc())) return true;

        const clang::VarDecl* base = getCallBase(call);
        if (base != container_) return true;

        const clang::CXXMethodDecl* method = call->getMethodDecl();
        if (!method) return true;
        std::string mname = method->getNameAsString();

        for (const auto& m : invalidatingMethods()) {
            if (mname == m) {
                emitDiag(diags_, call->getBeginLoc(), sm,
                         rule_->getCategory(), rule_->getId(),
                         "Calling '" + mname + "' on '" +
                         container_->getNameAsString() +
                         "' during iteration invalidates iterators / "
                         "在迭代期间对 '" + container_->getNameAsString() +
                         "' 调用 '" + mname + "' 会使迭代器失效",
                         Severity::Error);
                break;
            }
        }
        return true;
    }

private:
    const clang::VarDecl* container_;
    IteratorInvalidationRule* rule_;
    clang::ASTContext& ctx_;
    DiagnosticEngine& diags_;
};

class IterInvalidVisitor
    : public clang::RecursiveASTVisitor<IterInvalidVisitor> {
public:
    explicit IterInvalidVisitor(IteratorInvalidationRule* rule,
                                clang::ASTContext& ctx,
                                DiagnosticEngine& diags)
        : rule_(rule), ctx_(ctx), diags_(diags) {}

    // Range-based for: for (auto& x : container) { ... }
    // 基于范围的 for 循环：for (auto& x : container) { ... }
    bool VisitCXXForRangeStmt(clang::CXXForRangeStmt* stmt) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(stmt->getBeginLoc())) return true;

        const clang::Expr* range =
            stmt->getRangeInit()->IgnoreParenImpCasts();
        const auto* dr = clang::dyn_cast<clang::DeclRefExpr>(range);
        if (!dr) return true;
        const auto* container =
            clang::dyn_cast<clang::VarDecl>(dr->getDecl());
        if (!container) return true;

        LoopBodyChecker checker(container, rule_, ctx_, diags_);
        checker.TraverseStmt(stmt->getBody());
        return true;
    }

    // Iterator-based for: for (auto it = v.begin(); ...) { ... }
    // 基于迭代器的 for 循环：for (auto it = v.begin(); ...) { ... }
    bool VisitForStmt(clang::ForStmt* stmt) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(stmt->getBeginLoc())) return true;

        const clang::VarDecl* container = extractIterContainer(stmt->getInit());
        if (!container) return true;

        LoopBodyChecker checker(container, rule_, ctx_, diags_);
        checker.TraverseStmt(stmt->getBody());
        return true;
    }

private:
    // Detect `auto it = container.begin()` in the init of a for statement.
    // 在 for 语句的初始化中检测 `auto it = container.begin()`。
    const clang::VarDecl* extractIterContainer(clang::Stmt* init) {
        if (!init) return nullptr;
        const auto* ds = clang::dyn_cast<clang::DeclStmt>(init);
        if (!ds || !ds->isSingleDecl()) return nullptr;
        const auto* var = clang::dyn_cast<clang::VarDecl>(ds->getSingleDecl());
        if (!var || !var->hasInit()) return nullptr;

        const clang::Expr* initExpr = var->getInit()->IgnoreParenImpCasts();
        const auto* call =
            clang::dyn_cast<clang::CXXMemberCallExpr>(initExpr);
        if (!call) return nullptr;
        const clang::CXXMethodDecl* method = call->getMethodDecl();
        if (!method) return nullptr;
        std::string mname = method->getNameAsString();
        if (mname != "begin" && mname != "cbegin" && mname != "rbegin") {
            return nullptr;
        }
        return getCallBase(call);
    }

    IteratorInvalidationRule* rule_;
    clang::ASTContext& ctx_;
    DiagnosticEngine& diags_;
};

} // anonymous namespace

void IteratorInvalidationRule::check(clang::ASTContext& context,
                                     DiagnosticEngine& diagnostics) {
    IterInvalidVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

//=============================================================================
// TCC-LIFE-005: CrossFunctionLifetimeRule
// Pattern A: return ref/ptr to by-value parameter
// Pattern B: reference member bound to value parameter in ctor initializer
// Pattern C: global/static/out-ptr receives &localVar
// 模式 A：返回对值参数的引用/指针
// 模式 B：在构造函数初始化列表中，引用成员绑定到值参数
// 模式 C：全局/静态/出参指针获得 &localVar
//=============================================================================

namespace {

class CrossLifetimeVisitor
    : public clang::RecursiveASTVisitor<CrossLifetimeVisitor> {
public:
    explicit CrossLifetimeVisitor(CrossFunctionLifetimeRule* rule,
                                  clang::ASTContext& ctx,
                                  DiagnosticEngine& diags)
        : rule_(rule), ctx_(ctx), diags_(diags) {}

    // Pattern A: function returning T& or T* where return val refers to
    //            a by-value parameter.
    // 模式 A：函数返回 T& 或 T*，其中返回值引用了值参数。
    bool VisitReturnStmt(clang::ReturnStmt* ret) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(ret->getReturnLoc())) return true;
        if (!currentFunc_) return true;

        clang::QualType retTy = currentFunc_->getReturnType();
        if (!retTy->isReferenceType() && !retTy->isPointerType()) return true;

        const clang::Expr* rv = ret->getRetValue();
        if (!rv) return true;
        rv = rv->IgnoreParenImpCasts();

        // Unwrap & operator for pointer returns
        if (const auto* uo = clang::dyn_cast<clang::UnaryOperator>(rv)) {
            if (uo->getOpcode() == clang::UO_AddrOf) {
                rv = uo->getSubExpr()->IgnoreParenImpCasts();
            }
        }

        if (const auto* dr = clang::dyn_cast<clang::DeclRefExpr>(rv)) {
            if (const auto* param =
                    clang::dyn_cast<clang::ParmVarDecl>(dr->getDecl())) {
                if (!param->getType()->isReferenceType() &&
                    !param->getType()->isPointerType()) {
                    emitDiag(diags_, ret->getReturnLoc(), sm,
                             rule_->getCategory(), rule_->getId(),
                             "Returning reference/pointer to by-value parameter '" +
                             param->getNameAsString() +
                             "' which is destroyed on return / "
                             "返回对值参数 '" + param->getNameAsString() +
                             "' 的引用/指针，该参数在返回时被销毁",
                             Severity::Error);
                }
            }
        }
        return true;
    }

    bool VisitFunctionDecl(clang::FunctionDecl* func) {
        if (func->hasBody()) currentFunc_ = func;
        return true;
    }

    // Pattern C: global or static pointer assigned &localVar
    // 模式 C：全局或静态指针被赋值 &localVar
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        const auto& sm = ctx_.getSourceManager();
        if (!sm.isInMainFile(op->getBeginLoc())) return true;
        if (op->getOpcode() != clang::BO_Assign) return true;

        const auto* lhsDR = clang::dyn_cast<clang::DeclRefExpr>(
            op->getLHS()->IgnoreParenImpCasts());
        if (!lhsDR) return true;
        const auto* lhsVar =
            clang::dyn_cast<clang::VarDecl>(lhsDR->getDecl());
        if (!lhsVar) return true;
        // Only flag global or static storage duration on the LHS
        bool lhsIsNonLocal =
            lhsVar->hasGlobalStorage() || lhsVar->isStaticLocal();
        if (!lhsIsNonLocal) return true;
        if (!lhsVar->getType()->isPointerType()) return true;

        const clang::Expr* rhs = op->getRHS()->IgnoreParenImpCasts();
        if (const auto* uo = clang::dyn_cast<clang::UnaryOperator>(rhs)) {
            if (uo->getOpcode() == clang::UO_AddrOf) {
                const clang::Expr* inner =
                    uo->getSubExpr()->IgnoreParenImpCasts();
                if (const auto* dr =
                        clang::dyn_cast<clang::DeclRefExpr>(inner)) {
                    if (const auto* localVar =
                            clang::dyn_cast<clang::VarDecl>(dr->getDecl())) {
                        if (localVar->hasLocalStorage() &&
                            !localVar->isStaticLocal()) {
                            emitDiag(diags_, op->getBeginLoc(), sm,
                                     rule_->getCategory(), rule_->getId(),
                                     "Storing address of local variable '" +
                                     localVar->getNameAsString() +
                                     "' in non-local pointer '" +
                                     lhsVar->getNameAsString() +
                                     "' — dangling after scope exit / "
                                     "将局部变量 '" + localVar->getNameAsString() +
                                     "' 的地址存储到非局部指针 '" +
                                     lhsVar->getNameAsString() +
                                     "' — 作用域退出后成为悬空指针",
                                     Severity::Error);
                        }
                    }
                }
            }
        }
        return true;
    }

private:
    CrossFunctionLifetimeRule* rule_;
    clang::ASTContext& ctx_;
    DiagnosticEngine& diags_;
    clang::FunctionDecl* currentFunc_ = nullptr;
};

} // anonymous namespace

void CrossFunctionLifetimeRule::check(clang::ASTContext& context,
                                      DiagnosticEngine& diagnostics) {
    CrossLifetimeVisitor visitor(this, context, diagnostics);
    visitor.TraverseDecl(context.getTranslationUnitDecl());
}

} // namespace tcc

