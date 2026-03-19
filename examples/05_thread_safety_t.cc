// Rust C/C++ Example - Thread Safety (Should Pass)
// Rust C/C++ 示例 - 线程安全（应该通过）
// @tcc

#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

class ThreadSafeCounter {
public:
    ThreadSafeCounter() : count_(0) {}
    
    // Good: Using atomic for shared state / 良好：对共享状态使用 atomic
    void increment() {
        count_.fetch_add(1, std::memory_order_relaxed);
    }
    
    int get() const {
        return count_.load(std::memory_order_relaxed);
    }
    
private:
    std::atomic<int> count_;
};

class ThreadSafeData {
public:
    // Good: Mutex-protected access / 良好：互斥锁保护的访问
    void setValue(int value) {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ = value;
    }
    
    int getValue() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return value_;
    }
    
private:
    mutable std::mutex mutex_;
    int value_ = 0;
};

int main() {
    ThreadSafeCounter counter;
    ThreadSafeData data;
    
    std::vector<std::thread> threads;
    
    // Safe: Each thread increments atomically / 安全：每个线程原子地递增
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 100; ++j) {
                counter.increment();
            }
        });
    }
    
    // Safe: Mutex-protected access / 安全：互斥锁保护的访问
    threads.emplace_back([&data]() {
        data.setValue(42);
    });
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}

