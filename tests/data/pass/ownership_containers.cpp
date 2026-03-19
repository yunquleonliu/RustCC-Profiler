// Test file for correct container usage
// 正确容器使用测试文件
// @tcc

#include <memory>
#include <vector>
#include <string>

class Data {
public:
    Data(int val) : value_(val) {}
    int getValue() const { return value_; }
private:
    int value_;
};

// GOOD: Using std::vector instead of malloc / 良好：使用 std::vector 而不是 malloc
std::vector<int> createIntArray(size_t size) {
    return std::vector<int>(size, 0);
}

// GOOD: Using std::string instead of malloc / 良好：使用 std::string 而不是 malloc
std::string createString(const char* text) {
    return std::string(text);
}

// GOOD: Container of smart pointers / 良好：智能指针容器
std::vector<std::unique_ptr<Data>> createDataList(size_t count) {
    std::vector<std::unique_ptr<Data>> result;
    for (size_t i = 0; i < count; ++i) {
        result.push_back(std::make_unique<Data>(static_cast<int>(i)));
    }
    return result;
}

// GOOD: Return unique_ptr instead of raw pointer / 良好：返回 unique_ptr 而不是原始指针
std::unique_ptr<Data> createData(int value) {
    return std::make_unique<Data>(value);
}

int main() {
    // All memory management is automatic / 所有内存管理都是自动的
    auto intArray = createIntArray(100);
    auto str = createString("Hello RCC");
    auto dataList = createDataList(10);
    auto data = createData(42);
    
    // No manual cleanup needed / 不需要手动清理
    return 0;
}
