// Tough C Profiler - AST Visitor
// Tough C 分析器 - AST 访问者
//
// Base class for traversing Clang AST and applying rules
// 用于遍历 Clang AST 和应用规则的基类

#pragma once

#include "tcc/Core.h"
#include "tcc/Diagnostic.h"
#include "tcc/Rule.h"

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/Expr.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/ExprCXX.h>
#include <set>

namespace tcc {

// Main AST visitor that applies all rules / 应用所有规则的主 AST 访问者
class TCCASTVisitor : public clang::RecursiveASTVisitor<TCCASTVisitor> {
public:
    explicit TCCASTVisitor(clang::ASTContext& context,
                          const std::vector<std::unique_ptr<Rule>>& rules,
                          DiagnosticEngine& diagnostics)
        : context_(context)
        , rules_(rules)
        , diagnostics_(diagnostics) {}
    
    // Visit function declarations / 访问函数声明
    bool VisitFunctionDecl(clang::FunctionDecl* decl);
    
    // Visit variable declarations / 访问变量声明
    bool VisitVarDecl(clang::VarDecl* decl);
    
    // Visit new expressions / 访问 new 表达式
    bool VisitCXXNewExpr(clang::CXXNewExpr* expr);
    
    // Visit delete expressions / 访问 delete 表达式
    bool VisitCXXDeleteExpr(clang::CXXDeleteExpr* expr);
    
    // Visit return statements / 访问 return 语句
    bool VisitReturnStmt(clang::ReturnStmt* stmt);
    
    // Visit lambda expressions / 访问 lambda 表达式
    bool VisitLambdaExpr(clang::LambdaExpr* expr);
    
    // Visit thread creation / 访问线程创建
    bool VisitCXXConstructExpr(clang::CXXConstructExpr* expr);
    
    // Move semantics: track std::move calls / 移动语义：追踪 std::move 调用
    bool VisitCallExpr(clang::CallExpr* expr);

    // Move semantics: detect use-after-move / 移动语义：检测移动后使用
    bool VisitDeclRefExpr(clang::DeclRefExpr* expr);

    // Safety: detect unsafe .value()/.get() unwrap / 安全：检测不安全的 .value()/.get() 调用
    bool VisitCXXMemberCallExpr(clang::CXXMemberCallExpr* expr);

    // Safety: detect unchecked array subscript / 安全：检测未检查的数组下标
    bool VisitArraySubscriptExpr(clang::ArraySubscriptExpr* expr);

    // Safety: detect vector::operator[] (suggest .at()) / 安全：检测 vector::operator[]（建议使用 .at()）
    bool VisitCXXOperatorCallExpr(clang::CXXOperatorCallExpr* expr);

    // Helper: Get source location / 辅助函数：获取源位置
    SourceLocation getSourceLocation(clang::SourceLocation loc) const;
    
    // Helper: Check if location is in main file / 辅助函数：检查位置是否在主文件
    bool isInMainFile(clang::SourceLocation loc) const;

    // Performance: skip compiler-generated implicit code / 性能：跳过编译器生成的隐式代码
    bool shouldVisitImplicitCode() const { return false; }

private:
    clang::ASTContext& context_;
    const std::vector<std::unique_ptr<Rule>>& rules_;
    DiagnosticEngine& diagnostics_;

    // Deduplication: track already-reported locations / 去重：追踪已报告的位置
    mutable std::set<std::pair<unsigned, unsigned>> reportedMoveUses_;
};

} // namespace tcc
