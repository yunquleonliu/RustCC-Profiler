// Test file for correct ownership patterns
// 正确所有权模式测试文件
// This should pass all RCC checks
// 这应该通过所有 RCC 检查
// @tcc

#include <memory>

class TestClass {
public:
    TestClass(int val) : value(val) {}
    int value;
};

// GOOD: Using std::make_unique / 良好：使用 std::make_unique
std::unique_ptr<TestClass> createWithUnique() {
    return std::make_unique<TestClass>(42);
}

// GOOD: Using std::make_shared / 良好：使用 std::make_shared
std::shared_ptr<TestClass> createWithShared() {
    return std::make_shared<TestClass>(100);
}

// GOOD: RAII - automatic cleanup / 良好：RAII - 自动清理
void processObject(const TestClass& obj) {
    // Work with object / 使用对象
    int val = obj.value;
}

int main() {
    auto obj1 = createWithUnique();
    auto obj2 = createWithShared();
    
    processObject(*obj1);
    processObject(*obj2);
    
    // Automatic cleanup / 自动清理
    return 0;
}
