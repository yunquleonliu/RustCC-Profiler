// Rust C/C++ Example - Smart Pointer Usage (Should Pass)
// Rust C/C++ 示例 - 智能指针使用（应该通过）
// @tcc

#include <memory>
#include <vector>

// Example of RCC-compliant code / RCC 合规代码示例
// This file uses smart pointers and avoids raw ownership
// 此文件使用智能指针并避免原始所有权

class Widget {
public:
    Widget(int value) : value_(value) {}
    int getValue() const { return value_; }
    
private:
    int value_;
};

// Good: Using unique_ptr for ownership / 良好：使用 unique_ptr 表示所有权
std::unique_ptr<Widget> createWidget(int value) {
    return std::make_unique<Widget>(value);
}

// Good: Using shared_ptr when shared ownership needed
// 良好：需要共享所有权时使用 shared_ptr
std::shared_ptr<Widget> createSharedWidget(int value) {
    return std::make_shared<Widget>(value);
}

// Good: Passing by reference for borrowing / 良好：通过引用传递以借用
void processWidget(const Widget& widget) {
    // Read-only access / 只读访问
    int value = widget.getValue();
}

int main() {
    // All ownership is explicit / 所有所有权都是显式的
    auto widget1 = createWidget(42);
    auto widget2 = createSharedWidget(100);
    
    // Borrowing is clear / 借用是明确的
    processWidget(*widget1);
    processWidget(*widget2);
    
    // RAII handles cleanup / RAII 处理清理
    return 0;
}

