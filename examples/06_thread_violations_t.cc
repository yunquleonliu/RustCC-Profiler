// Rust C/C++ Example - Thread Safety Violations (Should Fail)
// Rust C/C++ 示例 - 线程安全违规（应该失败）
// @tcc

#include <thread>
#include <vector>

// BAD: Unsynchronized shared mutable state / 错误：未同步的共享可变状态
int global_counter = 0;  // RCC WARNING: Global mutable state
                         // RCC 警告：全局可变状态

class UnsafeCounter {
public:
    // BAD: Race condition / 错误：竞态条件
    void increment() {
        ++count_;  // RCC ERROR: Unsynchronized access to shared mutable state
                   // RCC 错误：对共享可变状态的非同步访问
    }
    
    int get() const { return count_; }
    
private:
    int count_ = 0;  // RCC ERROR: Shared mutable without synchronization
                     // RCC 错误：没有同步的共享可变
};

// BAD: Capturing non-const reference in thread lambda
// 错误：在线程 lambda 中捕获非 const 引用
void processData(int& data) {
    std::thread t([&data]() {  // RCC ERROR: Capturing non-const reference
                               // RCC 错误：捕获非 const 引用
        data = 42;  // Potential race / 潜在竞态
    });
    
    data = 100;  // Race with thread / 与线程竞态
    t.join();
}

// BAD: Sharing raw pointers across threads
// 错误：跨线程共享原始指针
void sharePointer() {
    int* ptr = new int(0);  // Already bad: raw new / 已经不好：原始 new
    
    std::thread t1([ptr]() {  // RCC ERROR: Sharing raw pointer across threads
        *ptr = 1;             // RCC 错误：跨线程共享原始指针
    });
    
    std::thread t2([ptr]() {
        *ptr = 2;  // Race condition / 竞态条件
    });
    
    t1.join();
    t2.join();
    delete ptr;  // Already bad: manual delete / 已经不好：手动 delete
}

int main() {
    UnsafeCounter counter;
    
    std::vector<std::thread> threads;
    
    // BAD: Multiple threads modifying without synchronization
    // 错误：多个线程修改而没有同步
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 100; ++j) {
                counter.increment();  // RCC ERROR: Data race
                                      // RCC 错误：数据竞争
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}

// This file should trigger RCC errors:
// 此文件应该触发 RCC 错误：
// 1. Unsynchronized shared mutable state / 未同步的共享可变状态
// 2. Capturing non-const reference in thread / 在线程中捕获非 const 引用
// 3. Sharing raw pointers across threads / 跨线程共享原始指针
//
// Fix: Use std::atomic, std::mutex, or message passing
// 修复：使用 std::atomic、std::mutex 或消息传递

