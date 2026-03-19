// Rust C/C++ Example - Lifetime Violations (Should Fail)
// Rust C/C++ 示例 - 生命周期违规（应该失败）
// @tcc

#include <string>
#include <vector>

// BAD: Returning reference to local / 错误：返回局部变量的引用
const std::string& getBadString() {
    std::string local = "temporary";
    return local;  // RCC ERROR: Returning reference to local variable
                   // RCC 错误：返回局部变量的引用
}  // 'local' is destroyed here / 'local' 在此处销毁

// BAD: Returning pointer to local / 错误：返回局部变量的指针
int* getBadPointer() {
    int value = 42;
    return &value;  // RCC ERROR: Returning pointer to local variable
                    // RCC 错误：返回局部变量的指针
}

class Container {
public:
    // BAD: Storing raw pointer / 错误：存储原始指针
    void addItem(int* item) {
        items_.push_back(item);  // RCC ERROR: Storing raw pointer
                                 // RCC 错误：存储原始指针
    }
    
private:
    std::vector<int*> items_;  // RCC ERROR: Container of raw pointers
                               // RCC 错误：原始指针的容器
};

int main() {
    // These will create dangling references / 这些将创建悬空引用
    const std::string& bad1 = getBadString();  // Dangling! / 悬空！
    int* bad2 = getBadPointer();  // Dangling! / 悬空！
    
    return 0;
}

// This file should trigger RCC errors:
// 此文件应该触发 RCC 错误：
// 1. Returning reference to local variable / 返回局部变量的引用
// 2. Returning pointer to local variable / 返回局部变量的指针
// 3. Container storing raw pointers / 存储原始指针的容器
//
// Fix: Use value semantics or smart pointers
// 修复：使用值语义或智能指针

