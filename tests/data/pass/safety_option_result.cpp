// TCC Test: Safe optional/result usage (should pass)
// TCC 测试：安全的 optional/result 使用（应通过）
// @tcc

#include <optional>
#include <string>
#include <variant>

// Safe: check before using optional value
// 安全：使用 optional 值之前检查
std::optional<int> divide(int a, int b) {
    if (b == 0) return std::nullopt;
    return a / b;
}

void safeOptional() {
    auto result = divide(10, 2);
    if (result.has_value()) {
        int v = result.value();  // safe: guarded by has_value()
        (void)v;
    }
}

// Safe: Result via std::variant, always match
// 安全：通过 std::variant 实现 Result，始终进行匹配
using Result = std::variant<int, std::string>;

Result tryParse(const std::string& s) {
    if (s.empty()) return std::string("empty input");
    return 42;
}

void safeResult() {
    auto r = tryParse("42");
    std::visit([](auto&& v) { (void)v; }, r);
}

int main() {
    safeOptional();
    safeResult();
    return 0;
}
