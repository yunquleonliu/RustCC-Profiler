// Tough C Example - Option and Result Patterns (Should Pass)
// Tough C 示例 - Option 和 Result 模式（应该通过）
// @tcc
// Demonstrates: TCC-OPTION-001, TCC-OPTION-002, TCC-RESULT-001

#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <iostream>

// ============================================================================
// Option<T> Pattern - Handling absence of value
// Option<T> 模式 - 处理值的缺失
// ============================================================================

// ✓ Good: Use std::optional for nullable returns / 使用 std::optional 表示可空返回
std::optional<int> findValue(const std::vector<int>& vec, int target) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] == target) {
            return static_cast<int>(i);  // Found / 找到
        }
    }
    return std::nullopt;  // Not found / 未找到
}

// ✓ Good: Optional with custom type / 自定义类型的 optional
struct User {
    std::string name;
    int age;
};

std::optional<User> findUser(const std::vector<User>& users, const std::string& name) {
    for (const auto& user : users) {
        if (user.name == name) {
            return user;
        }
    }
    return std::nullopt;
}

// ============================================================================
// Result<T, E> Pattern - Explicit error handling
// Result<T, E> 模式 - 显式错误处理
// ============================================================================

// Error types / 错误类型
enum class ParseError {
    EmptyString,
    InvalidFormat,
    OutOfRange
};

enum class FileError {
    NotFound,
    PermissionDenied,
    IoError
};

// ✓ Good: Result type using std::variant / 使用 std::variant 的 Result 类型
template<typename T, typename E>
using Result = std::variant<T, E>;

// ✓ Good: Parse with error handling / 带错误处理的解析
Result<int, ParseError> parseInt(const std::string& str) {
    if (str.empty()) {
        return ParseError::EmptyString;
    }
    
    try {
        int value = std::stoi(str);
        return value;
    } catch (...) {
        return ParseError::InvalidFormat;
    }
}

// ✓ Good: File operations with Result / 带 Result 的文件操作
Result<std::string, FileError> readFile(const std::string& path) {
    if (path.empty()) {
        return FileError::NotFound;
    }
    
    // Simulate file reading / 模拟文件读取
    if (path == "forbidden.txt") {
        return FileError::PermissionDenied;
    }
    
    return std::string("file contents");
}

// ============================================================================
// Safe usage patterns / 安全使用模式
// ============================================================================

// ✓ Good: Check before use (Option) / 使用前检查（Option）
void demonstrateOptionUsage() {
    std::vector<int> numbers = {10, 20, 30, 40, 50};
    
    // Pattern 1: if statement / 模式 1：if 语句
    if (auto result = findValue(numbers, 30)) {
        std::cout << "Found at index: " << *result << "\n";
    } else {
        std::cout << "Not found\n";
    }
    
    // Pattern 2: value_or / 模式 2：value_or
    int index = findValue(numbers, 100).value_or(-1);
    std::cout << "Index: " << index << "\n";
    
    // Pattern 3: has_value check / 模式 3：has_value 检查
    auto opt = findValue(numbers, 20);
    if (opt.has_value()) {
        int val = opt.value();  // ✓ Safe: checked first / 安全：先检查
        std::cout << "Value: " << val << "\n";
    }
}

// ✓ Good: Pattern matching for Result / Result 的模式匹配
void demonstrateResultUsage() {
    // Handle parse result / 处理解析结果
    auto result = parseInt("42");
    
    // ✓ Must check which variant / 必须检查是哪个变体
    if (std::holds_alternative<int>(result)) {
        int value = std::get<int>(result);
        std::cout << "Parsed: " << value << "\n";
    } else {
        ParseError error = std::get<ParseError>(result);
        // Handle error / 处理错误
        switch (error) {
            case ParseError::EmptyString:
                std::cout << "Error: Empty string\n";
                break;
            case ParseError::InvalidFormat:
                std::cout << "Error: Invalid format\n";
                break;
            case ParseError::OutOfRange:
                std::cout << "Error: Out of range\n";
                break;
        }
    }
}

// ✓ Good: Error propagation / 错误传播
Result<int, ParseError> parseAndDouble(const std::string& str) {
    auto result = parseInt(str);
    
    // ✓ Propagate error / 传播错误
    if (std::holds_alternative<ParseError>(result)) {
        return std::get<ParseError>(result);
    }
    
    // ✓ Process success value / 处理成功值
    int value = std::get<int>(result);
    return value * 2;
}

// ✓ Good: Chaining operations / 链式操作
Result<int, ParseError> parseMultiple(const std::vector<std::string>& strings) {
    int sum = 0;
    
    for (const auto& str : strings) {
        auto result = parseInt(str);
        
        // ✓ Early return on error / 错误时提前返回
        if (std::holds_alternative<ParseError>(result)) {
            return std::get<ParseError>(result);
        }
        
        sum += std::get<int>(result);
    }
    
    return sum;
}

// ✓ Good: transform for Option / Option 的 transform
std::optional<std::string> getUserName(int id) {
    if (id < 0) {
        return std::nullopt;
    }
    return "User_" + std::to_string(id);
}

void transformOption() {
    auto name = getUserName(42);
    
    // ✓ Transform if present / 如果存在则转换
    if (name) {
        std::string upper = *name;
        // Transform to uppercase / 转换为大写
        std::cout << "Name: " << upper << "\n";
    }
}

// ✓ Good: and_then for Option chaining / Option 链式的 and_then
std::optional<int> getAge(const std::string& name) {
    if (name == "Alice") return 30;
    if (name == "Bob") return 25;
    return std::nullopt;
}

std::optional<bool> isAdult(int age) {
    return age >= 18;
}

void chainOptions() {
    auto name = getUserName(10);
    
    if (name) {
        auto age = getAge(*name);
        if (age) {
            auto adult = isAdult(*age);
            if (adult && *adult) {
                std::cout << *name << " is an adult\n";
            }
        }
    }
}

// ✓ Good: Result with custom error types / 带自定义错误类型的 Result
struct ValidationError {
    std::string field;
    std::string message;
};

using ValidationResult = Result<bool, ValidationError>;

ValidationResult validateAge(int age) {
    if (age < 0) {
        return ValidationError{"age", "Age cannot be negative"};
    }
    if (age > 150) {
        return ValidationError{"age", "Age is unrealistic"};
    }
    return true;
}

ValidationResult validateName(const std::string& name) {
    if (name.empty()) {
        return ValidationError{"name", "Name cannot be empty"};
    }
    if (name.length() > 100) {
        return ValidationError{"name", "Name too long"};
    }
    return true;
}

// ✓ Good: Combining validations / 组合验证
ValidationResult validateUser(const std::string& name, int age) {
    // Validate name / 验证名称
    auto nameResult = validateName(name);
    if (std::holds_alternative<ValidationError>(nameResult)) {
        return std::get<ValidationError>(nameResult);
    }
    
    // Validate age / 验证年龄
    auto ageResult = validateAge(age);
    if (std::holds_alternative<ValidationError>(ageResult)) {
        return std::get<ValidationError>(ageResult);
    }
    
    return true;
}

int main() {
    // Demonstrate Option patterns / 演示 Option 模式
    demonstrateOptionUsage();
    transformOption();
    chainOptions();
    
    // Demonstrate Result patterns / 演示 Result 模式
    demonstrateResultUsage();
    
    // Error propagation / 错误传播
    auto doubled = parseAndDouble("21");
    if (std::holds_alternative<int>(doubled)) {
        std::cout << "Result: " << std::get<int>(doubled) << "\n";
    }
    
    // Multiple operations / 多个操作
    std::vector<std::string> nums = {"1", "2", "3"};
    auto sum = parseMultiple(nums);
    if (std::holds_alternative<int>(sum)) {
        std::cout << "Sum: " << std::get<int>(sum) << "\n";
    }
    
    // Validation / 验证
    auto validation = validateUser("Alice", 30);
    if (std::holds_alternative<bool>(validation)) {
        std::cout << "User valid\n";
    } else {
        auto error = std::get<ValidationError>(validation);
        std::cout << "Validation error in " << error.field 
                  << ": " << error.message << "\n";
    }
    
    return 0;
}
