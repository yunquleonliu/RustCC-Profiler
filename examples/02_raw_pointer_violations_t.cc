// Rust C/C++ Example - Raw Pointer Violations (Should Fail)
// Rust C/C++ 示例 - 原始指针违规（应该失败）
// @tcc

#include <vector>

class Data {
public:
    Data(int value) : value_(value) {}
    
private:
    int value_;
};

// BAD: Raw new (ownership unclear) / 错误：原始 new（所有权不明确）
Data* createData(int value) {
    return new Data(value);  // RCC ERROR: Use std::make_unique instead
                             // RCC 错误：请使用 std::make_unique
}

// BAD: Manual delete / 错误：手动 delete
void processData(Data* data) {
    // ... do something
    delete data;  // RCC ERROR: Manual memory management forbidden
                  // RCC 错误：禁止手动内存管理
}

int main() {
    // BAD: Raw owning pointer / 错误：原始所有权指针
    Data* data = createData(42);  // RCC ERROR: Raw pointer ownership
                                  // RCC 错误：原始指针所有权
    
    processData(data);
    
    return 0;
}

// This file should trigger RCC errors:
// 此文件应该触发 RCC 错误：
// 1. Use of 'new' operator / 使用 'new' 操作符
// 2. Use of 'delete' operator / 使用 'delete' 操作符
// 3. Raw pointer with ownership semantics / 具有所有权语义的原始指针
//
// Escape path: Remove @tcc annotation or move to .cpp file
// 逃生路径：移除 @tcc 注解或移动到 .cpp 文件

