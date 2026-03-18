// TCC Test: Safe move semantics patterns (should pass)
// TCC 测试：安全的移动语义模式（应通过）
// @tcc

#include <memory>
#include <string>
#include <utility>
#include <vector>

// Properly move-constructible RAII type
// 正确实现移动构造的 RAII 类型
class Buffer {
    std::unique_ptr<int[]> data_;
    size_t size_;
public:
    explicit Buffer(size_t sz)
        : data_(std::make_unique<int[]>(sz)), size_(sz) {}

    Buffer(Buffer&& o) noexcept
        : data_(std::move(o.data_)), size_(o.size_) { o.size_ = 0; }

    Buffer& operator=(Buffer&& o) noexcept {
        data_ = std::move(o.data_);
        size_ = o.size_;
        o.size_ = 0;
        return *this;
    }

    ~Buffer() = default;
    size_t size() const { return size_; }
};

// Safe: variable not used after move
// 安全：变量移动后不再使用
void consume(Buffer buf) { (void)buf; }

void safeMove() {
    Buffer b1(16);
    Buffer b2 = std::move(b1);  // b1 not used after here
    consume(std::move(b2));     // b2 not used after here
}

// Safe: factory function returns by value (NRVO)
// 安全：工厂函数通过值返回（NRVO）
Buffer makeBuffer(size_t n) {
    return Buffer(n);
}

int main() {
    safeMove();
    auto buf = makeBuffer(32);
    (void)buf;
    return 0;
}
