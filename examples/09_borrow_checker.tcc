// Tough C Example - Borrow Checker (Should Pass)
// Tough C 示例 - 借用检查器（应该通过）
// @tcc
// Demonstrates: TCC-BORROW-001, TCC-BORROW-002, TCC-BORROW-003

#include <vector>
#include <string>
#include <algorithm>

class Data {
private:
    std::vector<int> values_;
    std::string name_;
    
public:
    Data(const std::string& name) : name_(name) {}
    
    // ✓ Multiple immutable borrows are OK / 多个不可变借用是可以的
    const std::vector<int>& getValues() const { return values_; }
    const std::string& getName() const { return name_; }
    
    // ✓ Single mutable borrow / 单个可变借用
    std::vector<int>& getValuesMut() { return values_; }
    
    void addValue(int v) { values_.push_back(v); }
};

// ✓ Good: Read-only access / 良好：只读访问
void printData(const Data& data) {
    const auto& values = data.getValues();
    std::cout << data.getName() << " has " << values.size() << " values\n";
}

// ✓ Good: Multiple readers / 良好：多个读取器
void multipleReaders(const Data& data) {
    // ✓ Multiple immutable references are fine
    // ✓ 多个不可变引用没问题
    const auto& values1 = data.getValues();
    const auto& values2 = data.getValues();
    const auto& name = data.getName();
    
    // All read-only operations / 都是只读操作
    std::cout << "Size: " << values1.size() << "\n";
    std::cout << "Size: " << values2.size() << "\n";
    std::cout << "Name: " << name << "\n";
}

// ✓ Good: Mutable borrow isolated / 良好：可变借用隔离
void modifyData(Data& data) {
    // ✓ Single mutable borrow / 单个可变借用
    auto& values = data.getValuesMut();
    values.push_back(42);
    values.push_back(100);
    
    // ✓ After mutable borrow ends, can read again
    // ✓ 可变借用结束后，可以再次读取
    const auto& name = data.getName();
    std::cout << "Modified " << name << "\n";
}

// ✓ Good: Sequential borrows (non-overlapping) / 良好：顺序借用（不重叠）
void sequentialBorrows(Data& data) {
    // Phase 1: Read / 阶段 1：读取
    {
        const auto& values = data.getValues();
        std::cout << "Current size: " << values.size() << "\n";
    } // Immutable borrow ends here / 不可变借用在此结束
    
    // Phase 2: Write / 阶段 2：写入
    {
        auto& values = data.getValuesMut();
        values.push_back(50);
    } // Mutable borrow ends here / 可变借用在此结束
    
    // Phase 3: Read again / 阶段 3：再次读取
    {
        const auto& values = data.getValues();
        std::cout << "New size: " << values.size() << "\n";
    }
}

// ✓ Good: Proper lifetime nesting / 良好：正确的生命周期嵌套
void nestedBorrows() {
    Data outer("outer");
    outer.addValue(1);
    outer.addValue(2);
    
    {
        // ✓ Borrow within owner's scope / 在所有者作用域内借用
        const auto& values = outer.getValues();
        
        // Use within scope / 在作用域内使用
        for (int v : values) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    } // Reference goes out of scope / 引用超出作用域
    
    // ✓ Owner still valid / 所有者仍然有效
    outer.addValue(3);
}

// ✓ Good: Borrow for function call / 良好：为函数调用借用
void processValues(const std::vector<int>& values) {
    int sum = 0;
    for (int v : values) {
        sum += v;
    }
    std::cout << "Sum: " << sum << "\n";
}

void demonstrateSafeBorrow() {
    Data data("demo");
    data.addValue(10);
    data.addValue(20);
    
    // ✓ Borrow for function / 为函数借用
    const auto& values = data.getValues();
    processValues(values);
    
    // ✓ Original still valid / 原始数据仍然有效
    data.addValue(30);
}

// ✓ Good: Iterator pattern (safe borrowing) / 良好：迭代器模式（安全借用）
void iteratorBorrow(Data& data) {
    const auto& values = data.getValues();
    
    // ✓ Iterate over immutable borrow / 迭代不可变借用
    auto it = values.begin();
    auto end = values.end();
    
    while (it != end) {
        std::cout << *it << " ";
        ++it;
    }
    std::cout << "\n";
}

// ✓ Good: Const correctness / 良好：const 正确性
class Container {
private:
    mutable std::vector<int> cache_;  // ✓ Interior mutability with mutable
    int count_;
    
public:
    Container() : count_(0) {}
    
    // ✓ Const method can modify mutable members / const 方法可以修改 mutable 成员
    int getCount() const {
        cache_.push_back(count_);  // ✓ OK: mutable member
        return count_;
    }
    
    void increment() {
        ++count_;
    }
};

int main() {
    // All examples demonstrate safe borrowing
    // 所有示例演示安全的借用
    
    Data data("test");
    data.addValue(1);
    data.addValue(2);
    data.addValue(3);
    
    // ✓ Multiple parallel reads / 多个并行读取
    multipleReaders(data);
    
    // ✓ Sequential read-write-read / 顺序读-写-读
    sequentialBorrows(data);
    
    // ✓ Scoped borrows / 作用域借用
    nestedBorrows();
    
    // ✓ Function borrowing / 函数借用
    demonstrateSafeBorrow();
    
    // ✓ Iterator borrowing / 迭代器借用
    iteratorBorrow(data);
    
    // ✓ Interior mutability / 内部可变性
    Container c;
    c.increment();
    int count = c.getCount();  // ✓ Const method modifies cache
    
    return 0;
}
