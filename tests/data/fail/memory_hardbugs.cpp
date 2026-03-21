// Test fixture: severe memory-risk patterns
// 测试样例：严重内存风险模式
// @tcc

#include <cstdlib>

int* make_raw_owner() {
    int* p = new int(1); // TCC-OWN-001
    return p;            // TCC-OWN-004
}

void free_twice() {
    int* p = new int(2); // TCC-OWN-001
    delete p;            // TCC-OWN-002
    delete p;            // severe runtime bug pattern
}

int* dangling() {
    int local = 3;
    return &local; // TCC-LIFE-002
}

void c_alloc() {
    int* p = static_cast<int*>(std::malloc(sizeof(int))); // TCC-OWN-003
    std::free(p);                                          // TCC-OWN-003
}

int main() {
    make_raw_owner();
    free_twice();
    dangling();
    c_alloc();
    return 0;
}
