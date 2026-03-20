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
#include <clang/Frontend/FrontendActions.h>
#include <llvm/Support/CommandLine.h>

#include <iostream>
#include <string>

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
    
    RCCActionFactory actionFactory(engine, diagnostics);
    int result = Tool.run(&actionFactory);
    
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

