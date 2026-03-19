// Rust C/C++ Example - Move Semantic Violations (Should Fail)
// Rust C/C++ 示例 - 移动语义违规（应该失败）
// @tcc
// Violations: TCC-OWN-005, TCC-OWN-006

#include <memory>
#include <string>
#include <utility>

class Resource {
private:
    std::unique_ptr<int[]> data_;
    size_t size_;
    
public:
    Resource(size_t size) : data_(std::make_unique<int[]>(size)), size_(size) {}
    
    Resource(Resource&& other) noexcept
        : data_(std::move(other.data_)), size_(other.size_) {
        other.size_ = 0;
    }
    
    Resource& operator=(Resource&& other) noexcept {
        data_ = std::move(other.data_);
        size_ = other.size_;
        other.size_ = 0;
        return *this;
    }
    
    size_t getSize() const { return size_; }
    int* getData() { return data_.get(); }
};

void consume(Resource&& res) {
    // Takes ownership / 获得所有权
}

int main() {
    // ✗ VIOLATION 1: Use after move / 违规 1：移动后使用
    // TCC-OWN-005: Use after move detected
    {
        Resource r(100);
        Resource moved = std::move(r);
        
        // ✗ ERROR: r has been moved! / 错误：r 已被移动！
        size_t size = r.getSize();  // TCC-OWN-005: Variable used after move!
        int* data = r.getData();     // TCC-OWN-005: Variable used after move!
    }
    
    // ✗ VIOLATION 2: Double move / 违规 2：双重移动
    // TCC-OWN-006: Variable moved more than once
    {
        Resource r(200);
        Resource first = std::move(r);
        Resource second = std::move(r);  // TCC-OWN-006: Moved again!
    }
    
    // ✗ VIOLATION 3: Use after passing to function / 违规 3：传递给函数后使用
    {
        Resource r(300);
        consume(std::move(r));
        
        // ✗ ERROR: r was moved into consume()
        // ✗ 错误：r 已移动到 consume()
        size_t s = r.getSize();  // TCC-OWN-005: Use after move!
    }
    
    // ✗ VIOLATION 4: Multiple reads after move / 违规 4：移动后多次读取
    {
        Resource r(400);
        Resource moved = std::move(r);
        
        // ✗ All of these are errors / 所有这些都是错误
        if (r.getData() != nullptr) {    // TCC-OWN-005
            size_t s = r.getSize();       // TCC-OWN-005
            int* p = r.getData();         // TCC-OWN-005
        }
    }
    
    // ✗ VIOLATION 5: Move in condition, use in body / 违规 5：在条件中移动，在体中使用
    {
        Resource r(500);
        if (auto moved = std::move(r); true) {
            // ✗ ERROR: r moved in condition / 错误：r 在条件中移动
            size_t s = r.getSize();  // TCC-OWN-005
        }
    }
    
    // ✗ VIOLATION 6: Move in loop / 违规 6：在循环中移动
    {
        Resource r(600);
        for (int i = 0; i < 2; ++i) {
            Resource temp = std::move(r);  // TCC-OWN-006: First iteration OK, second is error
            // On second iteration, r is already moved!
            // 在第二次迭代时，r 已经被移动！
        }
    }
    
    // ✗ VIOLATION 7: Conditional move without reset / 违规 7：条件移动未重置
    {
        Resource r(700);
        bool condition = true;
        
        if (condition) {
            Resource moved = std::move(r);
        }
        
        // ✗ ERROR: r might have been moved / 错误：r 可能已被移动
        size_t s = r.getSize();  // TCC-OWN-005: Possibly use after move!
    }
    
    // ✗ VIOLATION 8: Move and then assign / 违规 8：移动后再赋值
    {
        Resource r1(800);
        Resource r2(900);
        
        Resource moved = std::move(r1);
        r1 = std::move(r2);  // This is OK actually - reinitializes r1
        
        // ✗ But r2 is now moved / 但 r2 现在已移动
        size_t s = r2.getSize();  // TCC-OWN-005: Use after move!
    }
    
    return 0;
}

