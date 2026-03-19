// Rust C/C++ Example - Option/Result Violations (Should Fail)
// Rust C/C++ 示例 - Option/Result 违规（应该失败）
// @tcc
// Violations: TCC-OPTION-001, TCC-RESULT-001, TCC-PANIC-001, TCC-SAFE-001

#include <optional>
#include <variant>
#include <vector>
#include <string>

template<typename T, typename E>
using Result = std::variant<T, E>;

enum class Error {
    NotFound,
    Invalid
};

// ============================================================================
// Option Violations / Option 违规
// ============================================================================

// ✗ VIOLATION 1: Using .value() without checking
// ✗ 违规 1：未检查就使用 .value()
// TCC-PANIC-001: Unsafe unwrap detected
void unsafeUnwrap() {
    std::optional<int> maybe_value = std::nullopt;
    
    // ✗ ERROR: Calling .value() without checking has_value()
    // ✗ 错误：未检查 has_value() 就调用 .value()
    int value = maybe_value.value();  // TCC-PANIC-001: Unchecked unwrap!
    // This will throw std::bad_optional_access / 这将抛出 std::bad_optional_access
}

// ✗ VIOLATION 2: Dereferencing without checking
// ✗ 违规 2：未检查就解引用
// TCC-OPTION-001: Pointer must be checked before use
void unsafeDeref() {
    std::optional<int> opt = std::nullopt;
    
    // ✗ ERROR: Dereferencing without checking / 错误：未检查就解引用
    int value = *opt;  // TCC-PANIC-001: Unchecked dereference!
    // Undefined behavior / 未定义行为
}

// ✗ VIOLATION 3: Returning nullable pointer instead of optional
// ✗ 违规 3：返回可空指针而非 optional
// TCC-OPTION-002: Use std::optional instead of nullable pointer
int* findInArray(int* arr, size_t size, int target) {  // TCC-OPTION-002: Should return std::optional<int>!
    for (size_t i = 0; i < size; ++i) {
        if (arr[i] == target) {
            return &arr[i];
        }
    }
    return nullptr;  // ✗ Returning null pointer / 返回空指针
}

// ✗ VIOLATION 4: Using pointer without null check
// ✗ 违规 4：未经空值检查使用指针
// TCC-OPTION-001: Pointer must be checked before use
void usePointerUnsafely() {
    int arr[] = {1, 2, 3};
    int* ptr = findInArray(arr, 3, 10);  // Returns nullptr / 返回 nullptr
    
    // ✗ ERROR: Using pointer without null check / 错误：未经空值检查使用指针
    int value = *ptr;  // TCC-OPTION-001: Unchecked null pointer dereference!
}

// ============================================================================
// Result Violations / Result 违规
// ============================================================================

Result<int, Error> parseNumber(const std::string& str) {
    if (str.empty()) {
        return Error::Invalid;
    }
    return 42;  // Simplified / 简化版
}

// ✗ VIOLATION 5: Ignoring Result return value
// ✗ 违规 5：忽略 Result 返回值
// TCC-RESULT-001: Result type must be handled
void ignoreResult() {
    // ✗ ERROR: Not checking or using the result / 错误：未检查或使用结果
    parseNumber("123");  // TCC-RESULT-001: Result ignored!
    
    // The result must be handled / 结果必须被处理
}

// ✗ VIOLATION 6: Unwrapping Result without checking
// ✗ 违规 6：未检查就展开 Result
// TCC-PANIC-001: Unsafe unwrap detected
void unwrapResultUnsafely() {
    auto result = parseNumber("");  // Returns Error / 返回错误
    
    // ✗ ERROR: Accessing value without checking variant
    // ✗ 错误：未检查变体就访问值
    int value = std::get<int>(result);  // TCC-PANIC-001: Unchecked get!
    // This will throw std::bad_variant_access / 这将抛出 std::bad_variant_access
}

// ✗ VIOLATION 7: Not propagating errors
// ✗ 违规 7：未传播错误
// TCC-RESULT-001: Result type must be handled
int processUnsafely(const std::string& str) {
    auto result = parseNumber(str);
    
    // ✗ ERROR: Assuming success without checking
    // ✗ 错误：未检查就假设成功
    return std::get<int>(result);  // TCC-PANIC-001: Might fail!
}

// ============================================================================
// Array Safety Violations / 数组安全违规
// ============================================================================

// ✗ VIOLATION 8: Array access without bounds check
// ✗ 违规 8：未经边界检查的数组访问
// TCC-SAFE-001: Array access requires bounds check
void unsafeArrayAccess() {
    std::vector<int> vec = {1, 2, 3};
    int index = 10;
    
    // ✗ ERROR: No bounds check / 错误：无边界检查
    int value = vec[index];  // TCC-SAFE-001: Unchecked array access!
    // Out of bounds / 越界
}

// ✗ VIOLATION 9: Using operator[] instead of .at()
// ✗ 违规 9：使用 operator[] 而非 .at()
// TCC-SAFE-001: Consider using .at() for bounds-checked access
void preferAtMethod() {
    std::vector<int> vec = {1, 2, 3};
    
    // ✗ WARNING: Should use .at() for safety / 警告：应使用 .at() 以保证安全
    int value = vec[0];  // TCC-SAFE-001: Prefer .at() method!
    
    // ✓ Better: / 更好：
    // int value = vec.at(0);  // Throws if out of bounds / 越界时抛出异常
}

// ============================================================================
// Panic/Abort Violations / Panic/Abort 违规
// ============================================================================

// ✗ VIOLATION 10: Direct abort call
// ✗ 违规 10：直接调用 abort
// TCC-PANIC-002: Direct panic/abort call forbidden
void callAbort() {
    bool condition = false;
    
    if (!condition) {
        std::abort();  // TCC-PANIC-002: Forbidden panic function!
    }
}

// ✗ VIOLATION 11: Using exit
// ✗ 违规 11：使用 exit
// TCC-PANIC-002: Direct panic/abort call forbidden
void callExit() {
    std::exit(1);  // TCC-PANIC-002: Use proper error handling instead!
}

// ✗ VIOLATION 12: Using std::terminate
// ✗ 违规 12：使用 std::terminate
// TCC-PANIC-002: Direct panic/abort call forbidden  
void callTerminate() {
    std::terminate();  // TCC-PANIC-002: Forbidden!
}

// ============================================================================
// Complex Violation Scenarios / 复杂违规场景
// ============================================================================

// ✗ VIOLATION 13: Chained unsafe operations
// ✗ 违规 13：链式不安全操作
void chainedUnsafeOps() {
    std::optional<std::vector<int>> maybe_vec = std::nullopt;
    
    // ✗ Multiple errors in one line / 一行中的多个错误
    int value = (*maybe_vec)[0];  // TCC-PANIC-001: Unchecked unwrap!
                                  // TCC-SAFE-001: Unchecked array access!
}

// ✗ VIOLATION 14: Ignoring [[nodiscard]] attribute
// ✗ 违规 14：忽略 [[nodiscard]] 属性
// TCC-RESULT-002: Function return value must be checked
[[nodiscard]] bool criticalOperation() {
    return false;
}

void ignoreNodiscard() {
    // ✗ ERROR: Return value must be checked / 错误：返回值必须被检查
    criticalOperation();  // TCC-RESULT-002: [[nodiscard]] ignored!
}

// ✗ VIOLATION 15: Unsafe pattern with optional in condition
// ✗ 违规 15：条件中 optional 的不安全模式
void conditionalUnsafe() {
    std::optional<int> opt = std::nullopt;
    
    // ✗ Pattern that looks safe but isn't / 看起来安全但实际不安全的模式
    if (true) {  // Not checking opt! / 未检查 opt！
        int value = *opt;  // TCC-PANIC-001: Unchecked!
    }
}

// ✗ VIOLATION 16: Creating optional, immediately unwrapping
// ✗ 违规 16：创建 optional，立即展开
void pointlessOptional() {
    std::optional<int> opt = 42;
    
    // ✗ If you know it has a value, don't use optional
    // ✗ 如果你知道它有值，就不要使用 optional
    int value = opt.value();  // Still needs check technically / 技术上仍需检查
}

int main() {
    // All these examples violate safety rules
    // 所有这些示例都违反了安全规则
    
    // ✗ Don't run these - they will crash / 不要运行这些 - 它们会崩溃
    // unsafeUnwrap();
    // unsafeDeref();
    // usePointerUnsafely();
    // unwrapResultUnsafely();
    // unsafeArrayAccess();
    // callAbort();
    // callExit();
    // callTerminate();
    // chainedUnsafeOps();
    // conditionalUnsafe();
    
    // These demonstrate bad patterns / 这些演示不良模式
    ignoreResult();
    ignoreNodiscard();
    preferAtMethod();
    
    return 0;
}

