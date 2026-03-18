// Tough C Profiler - Diagnostic Helper Utilities
// Tough C 分析器 - 诊断辅助工具
//
// Convenience helpers for reporting diagnostics from inner AST visitors
// that hold a clang::ASTContext reference but need to emit tcc::Diagnostic.
// 从持有 clang::ASTContext 引用的内部 AST 访问者中报告诊断的便捷辅助函数。

#pragma once

#include "tcc/Core.h"
#include "tcc/Diagnostic.h"

#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <string>

namespace tcc {

/// Convert a Clang SourceLocation to a TCC SourceLocation.
/// 将 Clang SourceLocation 转换为 TCC SourceLocation。
inline SourceLocation makeSourceLoc(clang::SourceLocation loc,
                                    const clang::SourceManager& sm) {
    if (loc.isInvalid()) return {};
    auto pl = sm.getPresumedLoc(loc);
    if (pl.isInvalid()) return {};
    return SourceLocation(pl.getFilename(), pl.getLine(), pl.getColumn());
}

/// Emit a single diagnostic from an inner AST visitor.
/// 从内部 AST 访问者发出单个诊断。
inline void emitDiag(DiagnosticEngine& diags,
                     clang::SourceLocation loc,
                     const clang::SourceManager& sm,
                     RuleCategory cat,
                     const std::string& ruleId,
                     const std::string& message,
                     Severity severity) {
    diags.report(Diagnostic(severity, message, makeSourceLoc(loc, sm), cat, ruleId));
}

} // namespace tcc
