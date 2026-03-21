// Demo: Typical severe memory risks in legacy C/C++
// 演示：传统 C/C++ 中典型严重内存风险
// @tcc
//
// This file is intentionally unsafe for demonstration.
// 本文件故意包含不安全写法用于演示。

#include <cstdlib>
#include <iostream>

struct Buffer {
    int* data;
    size_t n;

    explicit Buffer(size_t count) : data(new int[count]), n(count) {} // TCC-OWN-001

    ~Buffer() {
        delete[] data; // TCC-OWN-002
    }
};

int* leak_raw_owner(size_t n) {
    // Raw owning pointer escape (leak-prone API boundary)
    // 原始所有权指针逃逸（高泄漏风险 API 边界）
    int* p = new int[n]; // TCC-OWN-001
    return p;            // TCC-OWN-004 (raw owning pointer return)
}

void double_free_pattern() {
    int* p = new int(42); // TCC-OWN-001
    delete p;             // TCC-OWN-002
    delete p;             // Double-free pattern (severe runtime risk)
}

int* dangling_pointer_return() {
    int local = 7;
    return &local; // TCC-LIFE-002
}

void c_allocation_mismatch() {
    int* p = static_cast<int*>(std::malloc(sizeof(int) * 4)); // TCC-OWN-003
    if (!p) {
        return;
    }
    std::free(p); // TCC-OWN-003
}

int main() {
    Buffer b(8);

    int* leaked = leak_raw_owner(16);
    (void)leaked; // intentionally leaked for demo

    double_free_pattern();
    int* d = dangling_pointer_return();
    std::cout << "Dangling value: " << *d << '\n';

    c_allocation_mismatch();
    return 0;
}
