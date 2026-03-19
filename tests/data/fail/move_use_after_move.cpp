// RCC Test: Use-after-move violation (should fail)
// RCC 测试：移动后使用违规（应失败）
// @tcc
// Violation: TCC-OWN-005

#include <memory>
#include <string>
#include <utility>

void consume(std::unique_ptr<int> p) { (void)p; }

int main() {
    // ✗ VIOLATION: use ptr after std::move
    // 违规：在 std::move 之后使用 ptr
    auto ptr = std::make_unique<int>(42);
    consume(std::move(ptr));
    int val = *ptr;  // TCC-OWN-005: use after move
    (void)val;
    return 0;
}
