// Tough C Example - Borrow Violations (Should Fail)
// Tough C 示例 - 借用违规（应该失败）
// @tcc
// Violations: TCC-BORROW-001, TCC-BORROW-002, TCC-BORROW-003, TCC-BORROW-004

#include <vector>
#include <string>

class Data {
private:
    std::vector<int> values_;
public:
    std::vector<int>& getValues() { return values_; }
    const std::vector<int>& getValuesConst() const { return values_; }
    void addValue(int v) { values_.push_back(v); }
};

int main() {
    // ✗ VIOLATION 1: Mutable and immutable borrow simultaneously
    // ✗ 违规 1：同时存在可变和不可变借用
    // TCC-BORROW-001: Conflicting mutable and immutable borrows
    {
        Data data;
        
        std::vector<int>& mutable_ref = data.getValues();      // Mutable borrow / 可变借用
        const std::vector<int>& const_ref = data.getValuesConst();  // Immutable borrow / 不可变借用
        
        // ✗ ERROR: Can't have both! / 错误：不能同时存在！
        mutable_ref.push_back(42);  // TCC-BORROW-001: Conflicting borrows!
        int size = const_ref.size(); // TCC-BORROW-001: Conflicting borrows!
    }
    
    // ✗ VIOLATION 2: Multiple mutable borrows
    // ✗ 违规 2：多个可变借用
    // TCC-BORROW-003: Multiple mutable borrows detected
    {
        Data data;
        
        std::vector<int>& ref1 = data.getValues();  // First mutable borrow / 第一个可变借用
        std::vector<int>& ref2 = data.getValues();  // Second mutable borrow / 第二个可变借用
        
        // ✗ ERROR: Only one mutable borrow allowed!
        // ✗ 错误：只允许一个可变借用！
        ref1.push_back(1);  // TCC-BORROW-003: Multiple mutable borrows!
        ref2.push_back(2);  // TCC-BORROW-003: Multiple mutable borrows!
    }
    
    // ✗ VIOLATION 3: Borrow outlives owner
    // ✗ 违规 3：借用超出所有者生命周期
    // TCC-BORROW-002: Borrow outlives owner
    {
        const std::vector<int>* dangling_ref = nullptr;
        
        {
            Data temp_data;
            temp_data.addValue(100);
            dangling_ref = &temp_data.getValuesConst();
        } // temp_data destroyed here / temp_data 在此销毁
        
        // ✗ ERROR: temp_data is gone! / 错误：temp_data 已消失！
        int size = dangling_ref->size();  // TCC-BORROW-002: Borrow outlives owner!
    }
    
    // ✗ VIOLATION 4: Modify while borrowed immutably
    // ✗ 违规 4：在不可变借用期间修改
    // TCC-BORROW-004: Borrowed value modified
    {
        Data data;
        data.addValue(1);
        data.addValue(2);
        
        const std::vector<int>& borrowed = data.getValuesConst();
        
        // ✗ ERROR: Can't modify while borrowed! / 错误：借用期间不能修改！
        data.addValue(3);  // TCC-BORROW-004: Modifying while borrowed!
        
        // Using the borrow / 使用借用
        int size = borrowed.size();
    }
    
    // ✗ VIOLATION 5: Borrow through pointer, modify original
    // ✗ 违规 5：通过指针借用，修改原始数据
    {
        Data data;
        data.addValue(10);
        
        const std::vector<int>* ptr = &data.getValuesConst();
        
        // ✗ ERROR: Modifying while pointer exists / 错误：指针存在时修改
        data.addValue(20);  // TCC-BORROW-004: Borrowed via pointer!
        
        int value = (*ptr)[0];  // Using borrowed data / 使用借用的数据
    }
    
    // ✗ VIOLATION 6: Iterator invalidation (borrow conflict)
    // ✗ 违规 6：迭代器失效（借用冲突）
    {
        Data data;
        data.addValue(1);
        data.addValue(2);
        data.addValue(3);
        
        auto& values = data.getValues();
        auto it = values.begin();  // Immutable borrow via iterator / 通过迭代器不可变借用
        
        // ✗ ERROR: Modifying while iterating! / 错误：迭代时修改！
        values.push_back(4);  // TCC-BORROW-001: Mutable access while iterator active!
        
        int val = *it;  // Iterator might be invalid / 迭代器可能无效
    }
    
    // ✗ VIOLATION 7: Return reference to local
    // ✗ 违规 7：返回局部变量的引用
    // TCC-BORROW-002: Borrow outlives owner
    {
        auto getBadReference = []() -> const std::vector<int>& {
            Data local;
            local.addValue(42);
            return local.getValuesConst();  // TCC-BORROW-002: Returning ref to local!
        };
        
        // ✗ ERROR: Reference to destroyed object / 错误：引用已销毁对象
        // const auto& bad = getBadReference();
    }
    
    // ✗ VIOLATION 8: Overlapping mutable and immutable in function chain
    // ✗ 违规 8：函数链中重叠的可变和不可变借用
    {
        Data data;
        data.addValue(5);
        
        auto& mut_ref = data.getValues();
        
        // ✗ ERROR: Creating immutable while mutable exists
        // ✗ 错误：可变借用存在时创建不可变借用
        const auto& const_ref = data.getValuesConst();  // TCC-BORROW-001
        
        mut_ref[0] = 10;           // Mutable use / 可变使用
        int x = const_ref.size();  // Immutable use / 不可变使用
    }
    
    // ✗ VIOLATION 9: Borrow in condition, use different path
    // ✗ 违规 9：在条件中借用，不同路径使用
    {
        Data data;
        bool condition = true;
        
        if (condition) {
            auto& ref = data.getValues();
            ref.push_back(1);
        }
        
        // ✗ Potential issue: ref might still conceptually exist
        // ✗ 潜在问题：ref 在概念上可能仍然存在
        auto& another_ref = data.getValues();
        another_ref.push_back(2);  // Might overlap with previous borrow scope
    }
    
    return 0;
}
