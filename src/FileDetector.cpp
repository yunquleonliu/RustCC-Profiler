// Rust C/C++ Profiler - File Detector Implementation
// Rust C/C++ 分析器 - 文件检测器实现

#include "tcc/FileDetector.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace tcc {

bool FileDetector::shouldAnalyze(const std::string& filename) {
    return hasTCCExtension(filename) || hasTCCAnnotation(filename);
}

bool FileDetector::hasTCCExtension(const std::string& filename) {
    // Check for _t.cc extension / 检查 _t.cc 扩展名
    if (filename.length() >= 5) {
        std::string ext = filename.substr(filename.length() - 5);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        return ext == "_t.cc";
    }
    return false;
}

bool FileDetector::hasTCCAnnotation(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // Read first 100 lines to look for @tcc / 读取前100行查找 @tcc
    std::string line;
    int lineCount = 0;
    while (std::getline(file, line) && lineCount < 100) {
        // Look for // @tcc or /* @tcc */
        if (line.find("@tcc") != std::string::npos) {
            return true;
        }
        ++lineCount;
    }
    
    return false;
}

std::optional<TCCConfig> FileDetector::parseConfig(const std::string& filename) {
    if (!shouldAnalyze(filename)) {
        return std::nullopt;
    }
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    return parseAnnotation(content);
}

TCCConfig FileDetector::parseAnnotation(const std::string& content) {
    TCCConfig config;
    config.enabled = true;
    
    // Parse configuration options from annotations
    // 从注解解析配置选项
    // Format: // @tcc: option=value
    
    // Look for specific disables / 查找特定的禁用选项
    if (content.find("@tcc-no-ownership") != std::string::npos) {
        config.ownershipChecks = false;
    }
    if (content.find("@tcc-no-lifetime") != std::string::npos) {
        config.lifetimeChecks = false;
    }
    if (content.find("@tcc-no-concurrency") != std::string::npos) {
        config.concurrencyChecks = false;
    }
    
    return config;
}

} // namespace tcc

