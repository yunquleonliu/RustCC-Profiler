// Rust C/C++ Example - Lifetime Safety (Should Pass)
// Rust C/C++ 示例 - 生命周期安全（应该通过）
// @tcc

#include <memory>
#include <string>
#include <vector>

class Person {
public:
    Person(std::string name) : name_(std::move(name)) {}
    
    // Good: Return by value / 良好：按值返回
    std::string getName() const { return name_; }
    
    // Good: Return const reference to member / 良好：返回成员的 const 引用
    const std::string& getNameRef() const { return name_; }
    
private:
    std::string name_;
};

// Good: Value semantics / 良好：值语义
Person createPerson(const std::string& name) {
    Person p(name);
    return p;  // RVO/move will happen / 将发生 RVO/移动
}

// Good: Clear lifetime through smart pointers / 良好：通过智能指针明确生命周期
std::unique_ptr<Person> createPersonPtr(const std::string& name) {
    return std::make_unique<Person>(name);
}

int main() {
    // All lifetimes are clear / 所有生命周期都是明确的
    auto person1 = createPerson("Alice");
    auto person2 = createPersonPtr("Bob");
    
    // Safe reference (object still alive) / 安全引用（对象仍然存活）
    const std::string& name = person1.getNameRef();
    
    // Container of values / 值的容器
    std::vector<Person> people;
    people.push_back(createPerson("Charlie"));
    
    return 0;
}

