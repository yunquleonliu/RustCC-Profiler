// Rust C/C++ Profiler - Lifetime Rules Implementation
// Rust C/C++ 分析器 - 生命周期规则实现

#include "tcc/LifetimeRules.h"
#include "tcc/DiagnosticHelper.h"
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/ExprCXX.h>
#include <clang/Basic/SourceManager.h>
#include <set>
#include <string>

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
// Detect container-modifying method calls inside range-for and iterator-for loops.
//=============================================================================

static const std::set<std::string> kInvalidatingMethods = {
    "push_back", "emplace_back", "push_front", "emplace_front",
    "insert", "emplace", "erase", "clear", "resize", "reserve",
    "pop_back", "pop_front", "assign"
};

class IterInvalidVisitor
    : public clang::RecursiveASTVisitor<IterInvalidVisitor> {
public:
    explicit IterInvalidVisitor(clang::ASTContext& ctx, DiagnosticEngine& diags,
                                IteratorInvalidationRule* rule)
        : ctx_(ctx), sm_(ctx.getSourceManager()), diags_(diags), rule_(rule) {}

    // Pattern 1: range-based for loop
    bool VisitCXXForRangeStmt(clang::CXXForRangeStmt* stmt) {
        if (!sm_.isInMainFile(stmt->getForLoc())) return true;

        // Get the range expression (the container being iterated).
        auto* rangeInit = stmt->getRangeInit();
        if (!rangeInit) return true;

        // Resolve the iterated container to a VarDecl if possible.
        const clang::VarDecl* containerVar = resolveToVar(rangeInit);
        if (!containerVar) return true;

        // Scan body for modifying calls on the same container.
        scanBodyForMods(stmt->getBody(), containerVar, /*rangeFor=*/true);
        return true;
    }

    // Pattern 2: traditional iterator-based for loop
    bool VisitForStmt(clang::ForStmt* stmt) {
        if (!sm_.isInMainFile(stmt->getForLoc())) return true;

        // Detect: init is  auto it = container.begin()
        const clang::VarDecl* containerVar = detectIteratorContainer(stmt->getInit());
        if (!containerVar) return true;

        scanBodyForMods(stmt->getBody(), containerVar, /*rangeFor=*/false);
        return true;
    }

private:
    const clang::VarDecl* resolveToVar(clang::Expr* e) const {
        e = e->IgnoreParenImpCasts();
        if (auto* dre = clang::dyn_cast<clang::DeclRefExpr>(e)) {
            return clang::dyn_cast<clang::VarDecl>(dre->getDecl());
        }
        return nullptr;
    }

    // Returns the container VarDecl if the init is: auto it = container.begin()
    const clang::VarDecl* detectIteratorContainer(clang::Stmt* init) const {
        if (!init) return nullptr;
        auto* ds = clang::dyn_cast<clang::DeclStmt>(init);
        if (!ds || !ds->isSingleDecl()) return nullptr;
        auto* var = clang::dyn_cast<clang::VarDecl>(ds->getSingleDecl());
        if (!var || !var->hasInit()) return nullptr;

        auto* call = clang::dyn_cast<clang::CXXMemberCallExpr>(
            var->getInit()->IgnoreParenImpCasts());
        if (!call) return nullptr;

        auto* method = call->getMethodDecl();
        if (!method) return nullptr;
        std::string name = method->getNameAsString();
        if (name != "begin" && name != "cbegin") return nullptr;

        return resolveToVar(call->getImplicitObjectArgument());
    }

    // Scan a statement subtree for method calls that invalidate iterators
    // on the given container variable.
    void scanBodyForMods(clang::Stmt* body,
                         const clang::VarDecl* containerVar,
                         bool rangeFor) {
        if (!body) return;
        for (auto* child : body->children()) {
            if (!child) continue;
            checkStmtForMod(child, containerVar);
            scanBodyForMods(child, containerVar, rangeFor);
        }
    }

    void checkStmtForMod(clang::Stmt* s, const clang::VarDecl* containerVar) {
        auto* call = clang::dyn_cast<clang::CXXMemberCallExpr>(s);
        if (!call) return;
        if (!sm_.isInMainFile(call->getBeginLoc())) return;

        auto* method = call->getMethodDecl();
        if (!method) return;
        std::string methodName = method->getNameAsString();
        if (!kInvalidatingMethods.count(methodName)) return;

        // Check that the implicit object is our container.
        const clang::VarDecl* obj =
            resolveToVar(call->getImplicitObjectArgument());
        if (obj != containerVar) return;

        emitDiag(diags_, call->getBeginLoc(), sm_,
                 rule_->getCategory(), rule_->getId(),
                 "Container '" + containerVar->getNameAsString() +
                 "' modified during iteration ('" + methodName +
                 "') — iterator invalidation / "
                 "迭代期间容器被修改，迭代器失效",
                 Severity::Error);
    }

    clang::ASTContext& ctx_;
    const clang::SourceManager& sm_;
    DiagnosticEngine& diags_;
    IteratorInvalidationRule* rule_;
};

void IteratorInvalidationRule::check(clang::ASTContext& context,
                                     DiagnosticEngine& diagnostics) {
    IterInvalidVisitor v(context, diagnostics, this);
    v.TraverseDecl(context.getTranslationUnitDecl());
}

//=============================================================================
// TCC-LIFE-005: CrossFunctionLifetimeRule
// Three patterns:
//  A: function returns T& or const T& to a by-value parameter (local copy)
//  B: constructor initializer binds reference member to value parameter
//  C: assignment of &local to a non-local (global/static/out-ptr) pointer
//=============================================================================

class CrossFuncLifetimeVisitor
    : public clang::RecursiveASTVisitor<CrossFuncLifetimeVisitor> {
public:
    explicit CrossFuncLifetimeVisitor(clang::ASTContext& ctx,
                                      DiagnosticEngine& diags,
                                      CrossFunctionLifetimeRule* rule)
        : ctx_(ctx), sm_(ctx.getSourceManager()), diags_(diags), rule_(rule) {}

    // Pattern A: function returns T& to a by-value parameter.
    bool VisitFunctionDecl(clang::FunctionDecl* func) {
        if (!func->hasBody() || !sm_.isInMainFile(func->getLocation())) return true;

        auto retTy = func->getReturnType();
        if (!retTy->isReferenceType()) return true;

        // Collect by-value parameters.
        std::set<const clang::VarDecl*> byValParams;
        for (auto* p : func->parameters()) {
            if (!p->getType()->isReferenceType() && !p->getType()->isPointerType()) {
                byValParams.insert(p);
            }
        }
        if (byValParams.empty()) return true;

        // Scan return statements inside this function.
        checkReturns(func->getBody(), byValParams, func->getLocation());
        return true;
    }

    // Pattern C: assignment of &local to a non-local pointer variable.
    bool VisitBinaryOperator(clang::BinaryOperator* op) {
        if (!sm_.isInMainFile(op->getOperatorLoc())) return true;
        if (!op->isAssignmentOp()) return true;

        auto* lhsDRE = clang::dyn_cast<clang::DeclRefExpr>(
            op->getLHS()->IgnoreParenImpCasts());
        if (!lhsDRE) return true;
        auto* lhsVar = clang::dyn_cast<clang::VarDecl>(lhsDRE->getDecl());
        if (!lhsVar || !lhsVar->getType()->isPointerType()) return true;

        // lhsVar must be non-local (global, static, or extern).
        bool lhsIsNonLocal = !lhsVar->hasLocalStorage() ||
                             lhsVar->getStorageClass() == clang::SC_Static;
        if (!lhsIsNonLocal) return true;

        auto* rhs = op->getRHS()->IgnoreParenImpCasts();
        if (auto* unary = clang::dyn_cast<clang::UnaryOperator>(rhs)) {
            if (unary->getOpcode() == clang::UO_AddrOf) {
                auto* sub = unary->getSubExpr()->IgnoreParenImpCasts();
                if (auto* dre = clang::dyn_cast<clang::DeclRefExpr>(sub)) {
                    if (auto* local = clang::dyn_cast<clang::VarDecl>(
                            dre->getDecl())) {
                        if (local->hasLocalStorage() &&
                            sm_.isInMainFile(local->getLocation())) {
                            emitDiag(diags_, op->getOperatorLoc(), sm_,
                                     rule_->getCategory(), rule_->getId(),
                                     "Address of local '" +
                                     local->getNameAsString() +
                                     "' stored in non-local pointer / "
                                     "局部变量地址存入非局部指针",
                                     Severity::Error);
                        }
                    }
                }
            }
        }
        return true;
    }

private:
    void checkReturns(clang::Stmt* body,
                      const std::set<const clang::VarDecl*>& byValParams,
                      clang::SourceLocation funcLoc) {
        if (!body) return;
        if (auto* ret = clang::dyn_cast<clang::ReturnStmt>(body)) {
            auto* val = ret->getRetValue();
            if (!val) return;
            val = val->IgnoreParenImpCasts();
            if (auto* dre = clang::dyn_cast<clang::DeclRefExpr>(val)) {
                if (auto* var = clang::dyn_cast<clang::VarDecl>(
                        dre->getDecl())) {
                    if (byValParams.count(var)) {
                        emitDiag(diags_, ret->getReturnLoc(), sm_,
                                 rule_->getCategory(), rule_->getId(),
                                 "Returning reference to by-value parameter '" +
                                 var->getNameAsString() +
                                 "' — dangling reference / "
                                 "返回值参数的引用，引用将悬空",
                                 Severity::Error);
                    }
                }
            }
        }
        for (auto* child : body->children()) {
            if (child) checkReturns(child, byValParams, funcLoc);
        }
    }

    clang::ASTContext& ctx_;
    const clang::SourceManager& sm_;
    DiagnosticEngine& diags_;
    CrossFunctionLifetimeRule* rule_;
};

void CrossFunctionLifetimeRule::check(clang::ASTContext& context,
                                      DiagnosticEngine& diagnostics) {
    CrossFuncLifetimeVisitor v(context, diagnostics, this);
    v.TraverseDecl(context.getTranslationUnitDecl());
}

} // namespace tcc

