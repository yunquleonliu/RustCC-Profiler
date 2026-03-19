// RCC Test: Safety pattern violations (should fail)
// RCC 测试：安全模式违规（应失败）
// @tcc
// Violations: TCC-PANIC-001, TCC-SAFE-001

#include <optional>
#include <cstdlib>
#include <vector>

void unsafeUnwrap(std::optional<int> opt) {
    // ✗ VIOLATION: .value() without has_value() check
    // 违规：未经 has_value() 检查直接调用 .value()
    int v = opt.value();  // TCC-PANIC-001
    (void)v;
}

void panicCall() {
    // ✗ VIOLATION: direct abort() call
    // 违规：直接调用 abort()
    abort();  // TCC-PANIC-002
}

void uncheckedIndex(const std::vector<int>& data) {
    // ✗ VIOLATION: unchecked operator[]
    // 违规：未检查的 operator[]
    int v = data[0];  // TCC-SAFE-001
    (void)v;
}

int main() {
    unsafeUnwrap(std::nullopt);
    panicCall();
    return 0;
}
