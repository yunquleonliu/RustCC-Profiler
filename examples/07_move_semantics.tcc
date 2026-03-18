// Tough C Example - Move Semantics (Should Pass)
// Tough C 示例 - 移动语义（应该通过）
// @tcc
// Demonstrates: TCC-OWN-005, TCC-OWN-006, TCC-OWN-007

#include <memory>
#include <string>
#include <vector>
#include <utility>

// RAII Resource with move semantics
// 带有移动语义的 RAII 资源
class Resource {
private:
    std::unique_ptr<int[]> data_;
    size_t size_;
    std::string name_;
    
public:
    // Constructor / 构造函数
    Resource(size_t size, const std::string& name)
        : data_(std::make_unique<int[]>(size))
        , size_(size)
        , name_(name) {}
    
    // ✓ Move constructor (transfer ownership) / 移动构造（转移所有权）
    Resource(Resource&& other) noexcept
        : data_(std::move(other.data_))
        , size_(other.size_)
        , name_(std::move(other.name_)) {
        other.size_ = 0;
    }
    
    // ✓ Move assignment (transfer ownership) / 移动赋值（转移所有权）
    Resource& operator=(Resource&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            size_ = other.size_;
            name_ = std::move(other.name_);
            other.size_ = 0;
        }
        return *this;
    }
    
    // Delete copy operations / 删除拷贝操作
    Resource(const Resource&) = delete;
    Resource& operator=(const Resource&) = delete;
    
    // Getters / 取值器
    size_t getSize() const { return size_; }
    const std::string& getName() const { return name_; }
    
    // Safe access / 安全访问
    int* getData() { return data_.get(); }
    const int* getData() const { return data_.get(); }
};

// ✓ Good: Consume ownership / 良好：消费所有权
void processResource(Resource&& res) {
    // Function takes ownership / 函数获得所有权
    std::cout << "Processing: " << res.getName() << "\n";
}

// ✓ Good: Return by move / 良好：通过移动返回
Resource createResource(size_t size) {
    Resource res(size, "created");
    return res;  // Automatic move / 自动移动
}

// ✓ Good: Move into container / 良好：移动到容器
void storeResources(std::vector<Resource>& storage) {
    Resource r1(100, "resource1");
    Resource r2(200, "resource2");
    
    // Move into vector / 移动到 vector
    storage.push_back(std::move(r1));
    storage.push_back(std::move(r2));
    
    // ✓ r1 and r2 are now moved-from (shouldn't use)
    // ✓ r1 和 r2 现在已移动（不应使用）
}

// ✓ Good: Swap without copies / 良好：无拷贝交换
void swapResources(Resource& a, Resource& b) {
    Resource temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

// Example: Proper move usage / 示例：正确的移动使用
void demonstrateMoveSemantics() {
    // Create resource / 创建资源
    Resource original(1000, "original");
    
    // ✓ Move to new owner / 移动到新所有者
    Resource moved = std::move(original);
    // original is now in moved-from state / original 现在处于已移动状态
    
    // ✓ Use the new owner / 使用新所有者
    std::cout << moved.getName() << " has size " << moved.getSize() << "\n";
    
    // ✓ Move to function / 移动到函数
    processResource(std::move(moved));
    // moved is now in moved-from state / moved 现在处于已移动状态
}

// Example: Container with move-only type / 示例：包含只移动类型的容器
void demonstrateContainerMoves() {
    std::vector<Resource> resources;
    
    // ✓ Emplace for efficiency / 就地构造以提高效率
    resources.emplace_back(100, "first");
    resources.emplace_back(200, "second");
    
    // ✓ Move existing resource / 移动现有资源
    Resource external(300, "external");
    resources.push_back(std::move(external));
    
    // ✓ Access elements / 访问元素
    for (const auto& res : resources) {
        std::cout << res.getName() << ": " << res.getSize() << "\n";
    }
}

// Example: Chained moves / 示例：链式移动
Resource chainedMoves(Resource&& input) {
    // ✓ Forward the moved parameter / 转发移动的参数
    Resource processed = std::move(input);
    
    // Do some work / 做一些工作
    
    // ✓ Return by move / 通过移动返回
    return processed;
}

int main() {
    // All examples demonstrate proper move semantics
    // 所有示例演示正确的移动语义
    
    demonstrateMoveSemantics();
    demonstrateContainerMoves();
    
    // ✓ Chained operations / 链式操作
    Resource initial(500, "initial");
    Resource final = chainedMoves(std::move(initial));
    
    return 0;
}
