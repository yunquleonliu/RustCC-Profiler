// Rust C/C++ Profiler - Main Entry Point
// Rust C/C++ 分析器 - 主入口
//
// Command-line interface for Rust C/C++ profiler
// Rust C/C++ 分析器的命令行接口

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
    explicit RCCASTConsumer(RuleEngine& engine, DiagnosticEngine& diags)
        : engine_(engine), diagnostics_(diags) {}
    
    void HandleTranslationUnit(ASTContext& context) override {
        if (Verbose) {
            llvm::outs() << "Analyzing translation unit...\n";
            llvm::outs() << "分析翻译单元中...\n";
        }
        
        engine_.analyze(context, diagnostics_);
    }

private:
    RuleEngine& engine_;
    DiagnosticEngine& diagnostics_;
};

// Custom Frontend Action / 自定义前端动作
class RCCFrontendAction : public ASTFrontendAction {
public:
    explicit RCCFrontendAction(RuleEngine& engine, DiagnosticEngine& diags)
        : engine_(engine), diagnostics_(diags) {}
    
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& CI, StringRef InFile) override {
        
        if (Verbose) {
            llvm::outs() << "Processing file: " << InFile.str() << "\n";
            llvm::outs() << "处理文件: " << InFile.str() << "\n";
        }
        
        return std::make_unique<RCCASTConsumer>(engine_, diagnostics_);
    }

private:
    RuleEngine& engine_;
    DiagnosticEngine& diagnostics_;
};

// Frontend Action Factory / 前端动作工厂
class RCCActionFactory : public FrontendActionFactory {
public:
    explicit RCCActionFactory(RuleEngine& engine, DiagnosticEngine& diags)
        : engine_(engine), diagnostics_(diags) {}
    
    std::unique_ptr<FrontendAction> create() override {
        return std::make_unique<RCCFrontendAction>(engine_, diagnostics_);
    }

private:
    RuleEngine& engine_;
    DiagnosticEngine& diagnostics_;
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
    
    // Initialize rule engine / 初始化规则引擎
    RuleEngine engine;
    engine.initializeDefaultRules();
    
    // Configure rule categories / 配置规则类别
    if (NoOwnership) {
        engine.enableCategory(RuleCategory::Ownership, false);
        if (Verbose) {
            llvm::outs() << "Ownership checks disabled\n";
            llvm::outs() << "所有权检查已禁用\n";
        }
    }
    if (NoLifetime) {
        engine.enableCategory(RuleCategory::Lifetime, false);
        if (Verbose) {
            llvm::outs() << "Lifetime checks disabled\n";
            llvm::outs() << "生命周期检查已禁用\n";
        }
    }
    if (NoConcurrency) {
        engine.enableCategory(RuleCategory::Concurrency, false);
        if (Verbose) {
            llvm::outs() << "Concurrency checks disabled\n";
            llvm::outs() << "并发检查已禁用\n";
        }
    }
    
    if (Verbose) {
        llvm::outs() << "Active rules: " << engine.getActiveRuleCount() << "\n";
        llvm::outs() << "活动规则数: " << engine.getActiveRuleCount() << "\n\n";
    }
    
    // Create diagnostic engine / 创建诊断引擎
    DiagnosticEngine diagnostics;
    
    // Run tool / 运行工具
    ClangTool Tool(OptionsParser.getCompilations(),
                   OptionsParser.getSourcePathList());

    if (AutoStdcppIncludes) {
        auto fallbackArgs = buildFallbackStdcppArgs();
        if (!fallbackArgs.empty()) {
            Tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(
                fallbackArgs, ArgumentInsertPosition::END));

            if (Verbose) {
                llvm::outs() << "Fallback include mode enabled with args:\n";
                llvm::outs() << "已启用回退包含路径模式，参数如下：\n";
                for (const auto& arg : fallbackArgs) {
                    llvm::outs() << "  " << arg << "\n";
                }
            }
        }
    }
    
    RCCActionFactory actionFactory(engine, diagnostics);
    int result = Tool.run(&actionFactory);

    if (result != 0) {
        llvm::errs() << "\n✗ Failed to process one or more translation units.\n";
        llvm::errs() << "✗ 一个或多个翻译单元处理失败。\n";

        if (!diagnostics.getDiagnostics().empty()) {
            diagnostics.printAll(std::cerr);
        }

        return static_cast<int>(ExitCode::CompilationError);
    }
    
    // Print diagnostics / 打印诊断
    if (diagnostics.getDiagnostics().empty()) {
        llvm::outs() << "\n✓ All checks passed! Code is RCC-compliant.\n";
        llvm::outs() << "✓ 所有检查通过！代码符合 RCC 规范。\n";
        return static_cast<int>(ExitCode::Success);
    }
    
    llvm::errs() << "\n✗ RCC rule violations found:\n";
    llvm::errs() << "✗ 发现 RCC 规则违规:\n\n";
    diagnostics.printAll(std::cerr);
    
    if (diagnostics.hasErrors()) {
        llvm::errs() << "\nErrors: " << diagnostics.getErrorCount() << "\n";
        llvm::errs() << "错误数: " << diagnostics.getErrorCount() << "\n";
        return static_cast<int>(ExitCode::RuleViolation);
    }
    
    return static_cast<int>(ExitCode::Success);
}

