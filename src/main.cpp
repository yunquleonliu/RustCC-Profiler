// Rust C/C++ Profiler - Main Entry Point
// Rust C/C++ 分析器 - 主入口
//
// Command-line interface for Rust C/C++ profiler
// Rust C/C++ 分析器的命令行接口
//
// File mode semantics / 文件模式语义:
//   _t.cc / @tcc annotation → ENFORCE mode: violations are errors, build fails
//   .cpp / .cc / .c         → ADVISORY mode: violations are reported, build continues
//   @tcc-no-ownership/lifetime/concurrency → per-file category opt-out (= Rust unsafe {})

#include "tcc/Core.h"
#include "tcc/Diagnostic.h"
#include "tcc/FileDetector.h"
#include "tcc/RuleEngine.h"

#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Frontend/FrontendActions.h>
#include <llvm/Support/CommandLine.h>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace tcc;
using namespace clang;
using namespace clang::tooling;
using namespace llvm;

// Command line options / 命令行选项
static cl::OptionCategory RCCCategory("Rust C/C++ Options / Rust C/C++ 选项");

static cl::opt<bool> ShowVersion(
    "rcc-version",
    cl::desc("Show version information / 显示版本信息"),
    cl::cat(RCCCategory)
);

static cl::opt<bool> Verbose(
    "verbose",
    cl::desc("Enable verbose output / 启用详细输出"),
    cl::cat(RCCCategory)
);

static cl::opt<bool> NoOwnership(
    "no-ownership",
    cl::desc("Disable ownership checks / 禁用所有权检查"),
    cl::cat(RCCCategory)
);

static cl::opt<bool> NoLifetime(
    "no-lifetime",
    cl::desc("Disable lifetime checks / 禁用生命周期检查"),
    cl::cat(RCCCategory)
);

static cl::opt<bool> NoConcurrency(
    "no-concurrency",
    cl::desc("Disable concurrency checks / 禁用并发检查"),
    cl::cat(RCCCategory)
);

static cl::opt<bool> AutoStdcppIncludes(
    "auto-stdcpp-includes",
    cl::desc("Append fallback C++17 standard-library include paths for standalone files / 为独立文件追加 C++17 标准库回退包含路径"),
    cl::cat(RCCCategory)
);

// Custom AST Consumer / 自定义 AST 消费者
class RCCASTConsumer : public ASTConsumer {
public:
    explicit RCCASTConsumer(RuleEngine& engine, DiagnosticEngine& diags,
                            bool enforceMode)
        : engine_(engine), diagnostics_(diags), enforceMode_(enforceMode) {}
    
    void HandleTranslationUnit(ASTContext& context) override {
        if (Verbose) {
            llvm::outs() << "Analyzing translation unit"
                         << (enforceMode_ ? " [ENFORCE]" : " [ADVISORY]")
                         << "...\n";
        }
        engine_.analyze(context, diagnostics_);
    }

private:
    RuleEngine& engine_;
    DiagnosticEngine& diagnostics_;
    bool enforceMode_;
};

// Custom Frontend Action / 自定义前端动作
class RCCFrontendAction : public ASTFrontendAction {
public:
    explicit RCCFrontendAction(RuleEngine& engine, DiagnosticEngine& diags,
                               bool enforceMode)
        : engine_(engine), diagnostics_(diags), enforceMode_(enforceMode) {}
    
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& CI, StringRef InFile) override {
        
        if (Verbose) {
            llvm::outs() << "Processing file: " << InFile.str()
                         << (enforceMode_ ? " [ENFORCE/_t.cc]" : " [ADVISORY]")
                         << "\n";
        }
        
        return std::make_unique<RCCASTConsumer>(engine_, diagnostics_, enforceMode_);
    }

private:
    RuleEngine& engine_;
    DiagnosticEngine& diagnostics_;
    bool enforceMode_;
};

// Frontend Action Factory / 前端动作工厂
class RCCActionFactory : public FrontendActionFactory {
public:
    explicit RCCActionFactory(RuleEngine& engine, DiagnosticEngine& diags,
                              bool enforceMode)
        : engine_(engine), diagnostics_(diags), enforceMode_(enforceMode) {}
    
    std::unique_ptr<FrontendAction> create() override {
        return std::make_unique<RCCFrontendAction>(engine_, diagnostics_, enforceMode_);
    }

private:
    RuleEngine& engine_;
    DiagnosticEngine& diagnostics_;
    bool enforceMode_;
};

// Print banner / 打印横幅
void printBanner() {
    llvm::outs() << "╔════════════════════════════════════════════════════════════╗\n";
    llvm::outs() << "║  Rust C/C++ Profiler - Pre-compilation Safety Analyzer      ║\n";
    llvm::outs() << "║  Rust C/C++ 分析器 - 预编译安全分析工具                      ║\n";
    llvm::outs() << "║  Version / 版本: " << VERSION << "                                   ║\n";
    llvm::outs() << "╚════════════════════════════════════════════════════════════╝\n\n";
}

// Find fallback include directories for standalone file analysis.
// 为独立文件分析查找回退头文件目录。
std::vector<std::string> buildFallbackStdcppArgs() {
    std::vector<std::string> args;
    args.emplace_back("-std=c++17");

    std::vector<std::filesystem::path> candidates;
#ifdef _WIN32
    // Prefer INCLUDE from a Developer Command Prompt / vcvars64 environment.
    // 优先使用 Developer Command Prompt / vcvars64 环境中的 INCLUDE。
    if (const char* includeEnv = std::getenv("INCLUDE")) {
        std::string includeStr(includeEnv);
        size_t start = 0;
        while (start <= includeStr.size()) {
            size_t sep = includeStr.find(';', start);
            const auto token = includeStr.substr(start, sep - start);
            if (!token.empty()) {
                candidates.emplace_back(token);
            }
            if (sep == std::string::npos) {
                break;
            }
            start = sep + 1;
        }
    }
#else
    const std::array<const char*, 7> versions = {"14", "13", "12", "11", "10", "9", "8"};

    for (const auto* version : versions) {
        const std::filesystem::path base = std::filesystem::path("/usr/include/c++") / version;
        if (std::filesystem::exists(base)) {
            candidates.push_back(base);
            candidates.push_back(base / "backward");
            candidates.push_back(std::filesystem::path("/usr/include/x86_64-linux-gnu/c++") / version);
            candidates.push_back(std::filesystem::path("/usr/lib/gcc/x86_64-linux-gnu") / version / "include");
            break;
        }
    }

    candidates.push_back("/usr/local/include");
    candidates.push_back("/usr/include/x86_64-linux-gnu");
    candidates.push_back("/usr/include");
#endif

    std::unordered_set<std::string> seen;
    for (const auto& path : candidates) {
        if (!std::filesystem::exists(path)) {
            continue;
        }
        const auto normalized = path.string();
        if (seen.insert(normalized).second) {
            args.emplace_back("-I" + normalized);
        }
    }

    return args;
}

// Main function / 主函数
int main(int argc, const char** argv) {
    // Parse command line / 解析命令行
    auto ExpectedParser = CommonOptionsParser::create(
        argc, argv, RCCCategory,
        cl::Optional,
        "Rust C/C++ Profiler - enforces safety rules on C/C++ code\n"
        "Rust C/C++ 分析器 - 对 C/C++ 代码强制执行安全规则"
    );
    
    if (!ExpectedParser) {
        llvm::errs() << "Error parsing command line arguments:\n";
        llvm::errs() << "命令行参数解析错误:\n";
        llvm::errs() << toString(ExpectedParser.takeError()) << "\n";
        return static_cast<int>(ExitCode::InvalidArguments);
    }
    
    CommonOptionsParser& OptionsParser = ExpectedParser.get();

    // Validate input files early / 提前验证输入文件
    const auto& sourcePaths = OptionsParser.getSourcePathList();
    if (sourcePaths.empty()) {
        llvm::errs() << "Error: No source files provided.\n";
        llvm::errs() << "错误：未提供源文件。\n";
        llvm::errs() << "Usage example: rcc-check -p <build-dir> <file1.cc> [file2.cc ...]\n";
        llvm::errs() << "用法示例：rcc-check -p <构建目录> <file1.cc> [file2.cc ...]\n";
        return static_cast<int>(ExitCode::InvalidArguments);
    }

    for (const auto& sourcePath : sourcePaths) {
        if (!std::filesystem::exists(sourcePath)) {
            llvm::errs() << "Error: Source file not found: " << sourcePath << "\n";
            llvm::errs() << "错误：未找到源文件：" << sourcePath << "\n";
            return static_cast<int>(ExitCode::FileNotFound);
        }
    }
    
    // Show version if requested / 如果请求则显示版本
    if (ShowVersion) {
        printBanner();
        return static_cast<int>(ExitCode::Success);
    }
    
    printBanner();

    // -----------------------------------------------------------------------
    // Determine per-file enforce/advisory mode via FileDetector.
    // 通过 FileDetector 确定每个文件的强制/建议模式。
    //
    // - _t.cc extension  → ENFORCE: violations are errors (exit nonzero)
    // - // @tcc           → ENFORCE: annotation opts file in explicitly
    // - anything else    → ADVISORY: violations printed but exit 0
    //
    // Per-file @tcc-no-* annotations are applied BEFORE the RuleEngine runs,
    // disabling the annotated category for that TU only.
    // -----------------------------------------------------------------------

    // If ANY file is _t.cc / @tcc, run in enforce mode overall.
    // When mixing enforced and advisory files on one command line, we run two
    // passes: advisory first (best-effort, no-fail), then enforce (strict).
    // For the common case of all-same-mode, one pass is used.

    std::vector<std::string> enforcePaths;
    std::vector<std::string> advisoryPaths;

    for (const auto& path : sourcePaths) {
        if (FileDetector::shouldAnalyze(path)) {
            enforcePaths.push_back(path);
        } else {
            advisoryPaths.push_back(path);
        }
    }

    if (Verbose) {
        llvm::outs() << "File mode classification:\n";
        for (const auto& p : enforcePaths) {
            llvm::outs() << "  [ENFORCE] " << p << "\n";
        }
        for (const auto& p : advisoryPaths) {
            llvm::outs() << "  [ADVISORY] " << p << "\n";
        }
        llvm::outs() << "\n";
    }

    // Helper: build + configure a RuleEngine from CLI flags + optional per-file config.
    // 辅助函数：根据 CLI 标志和可选的每文件配置构建并配置规则引擎。
    auto makeEngine = [&](const std::optional<TCCConfig>& fileCfg) {
        RuleEngine engine;
        engine.initializeDefaultRules();

        // Global CLI overrides / 全局 CLI 覆盖
        if (NoOwnership) engine.enableCategory(RuleCategory::Ownership, false);
        if (NoLifetime)  engine.enableCategory(RuleCategory::Lifetime,  false);
        if (NoConcurrency) engine.enableCategory(RuleCategory::Concurrency, false);

        // Per-file @tcc-no-* opt-out (= Rust unsafe {} escape hatch)
        // 每文件 @tcc-no-* 退出选项（= Rust unsafe {} 逃生门）
        if (fileCfg) {
            if (!fileCfg->ownershipChecks)
                engine.enableCategory(RuleCategory::Ownership, false);
            if (!fileCfg->lifetimeChecks)
                engine.enableCategory(RuleCategory::Lifetime, false);
            if (!fileCfg->concurrencyChecks)
                engine.enableCategory(RuleCategory::Concurrency, false);
        }

        if (Verbose) {
            llvm::outs() << "Active rules: " << engine.getActiveRuleCount() << "\n\n";
        }
        return engine;
    };

    // Helper: run a set of files through the tool, return 0 or error code.
    // 辅助函数：对一组文件运行工具，返回 0 或错误码。
    auto runPass = [&](const std::vector<std::string>& paths,
                       bool enforceMode) -> int {
        if (paths.empty()) return 0;

        DiagnosticEngine diagnostics;

        // For single-file runs, parse per-file config; for multi-file, use
        // global config only (common case when all files share the same
        // category opt-outs, which is enforced by _t.cc convention).
        std::optional<TCCConfig> fileCfg;
        if (paths.size() == 1) {
            fileCfg = FileDetector::parseConfig(paths[0]);
        }

        RuleEngine engine = makeEngine(fileCfg);
        ClangTool Tool(OptionsParser.getCompilations(), paths);

        if (AutoStdcppIncludes) {
            auto fallbackArgs = buildFallbackStdcppArgs();
            if (!fallbackArgs.empty()) {
                Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
                    fallbackArgs, ArgumentInsertPosition::END));
                if (Verbose) {
                    llvm::outs() << "Fallback include mode enabled.\n";
                }
            }
        }

        RCCActionFactory actionFactory(engine, diagnostics, enforceMode);
        int result = Tool.run(&actionFactory);

        if (result != 0) {
            llvm::errs() << "\n✗ Failed to process one or more translation units.\n";
            if (!diagnostics.getDiagnostics().empty()) {
                diagnostics.printAll(std::cerr);
            }
            return static_cast<int>(ExitCode::CompilationError);
        }

        if (diagnostics.getDiagnostics().empty()) {
            if (enforceMode) {
                llvm::outs() << "✓ [ENFORCE] All checks passed! Code is RCC-compliant.\n";
            } else {
                llvm::outs() << "✓ [ADVISORY] No issues found.\n";
            }
            return 0;
        }

        diagnostics.printAll(std::cerr);

        if (enforceMode && diagnostics.hasErrors()) {
            llvm::errs() << "\n✗ [ENFORCE] Errors: " << diagnostics.getErrorCount()
                         << " — build failed (file is _t.cc / @tcc)\n";
            llvm::errs() << "✗ [强制模式] 错误数: " << diagnostics.getErrorCount()
                         << " — 构建失败（文件为 _t.cc / @tcc）\n";
            return static_cast<int>(ExitCode::RuleViolation);
        }

        // Advisory violations or enforce-mode warnings only — continue.
        return 0;
    };

    // Advisory pass (no-fail even if violations found)
    // 建议模式（即使发现违规也不失败）
    int advisoryResult = runPass(advisoryPaths, /*enforceMode=*/false);
    (void)advisoryResult; // advisory never fails the build

    // Enforce pass (_t.cc / @tcc — violations → nonzero exit)
    // 强制模式（_t.cc / @tcc — 违规 → 非零退出）
    int enforceResult = runPass(enforcePaths, /*enforceMode=*/true);

    if (enforceResult != 0) {
        return enforceResult;
    }

    return static_cast<int>(ExitCode::Success);
}

