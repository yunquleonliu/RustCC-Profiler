# Rust C/C++ 中的 Rust 抽象集成
# Rust Abstractions in Rust C/C++

> Implementation note / 实现说明:
> This document contains design intent plus partially implemented features.
> 本文档包含设计目标和部分已实现功能。
>
> Some listed rules are still under active implementation.
> 文档中的部分规则仍在持续实现中。

## 概述 / Overview

本文档描述了如何将经过验证的 Rust 内存安全抽象集成到 Rust C/C++ (Rust C) Profiler 中。这些抽象提供了编译时内存安全保证，同时保持与 C/C++ 的兼容性。

This document describes how validated Rust memory safety abstractions are integrated into the Rust C/C++ (Rust C) Profiler. These abstractions provide compile-time memory safety guarantees while maintaining C/C++ compatibility.

---

## 核心 Rust 抽象 / Core Rust Abstractions

### 1. 所有权系统 (Ownership System)

#### 原理 / Principle
Rust 的所有权系统确保：
- 每个值都有唯一的所有者
- 当所有者超出作用域时，值被释放
- 值可以移动（move）或借用（borrow）

Rust's ownership system ensures:
- Each value has a unique owner
- When the owner goes out of scope, the value is freed
- Values can be moved or borrowed

#### Rust C/C++ 实现 / Implementation in Rust C/C++

```cpp
// TCC-OWN-005: Move Semantics Tracking
// 移动语义追踪

class Resource {
    int* data_;
public:
    // 构造函数 / Constructor
    Resource(int size) : data_(new int[size]) {}
    
    // 移动构造 / Move constructor
    Resource(Resource&& other) noexcept 
        : data_(other.data_) {
        other.data_ = nullptr;  // ✓ 原对象失效 / Original invalidated
    }
    
    // 禁止拷贝 / No copy
    Resource(const Resource&) = delete;
    
    ~Resource() { delete[] data_; }
};

void consume(Resource&& res) {
    // res 获得所有权 / res takes ownership
}

int main() {
    Resource r(100);
    consume(std::move(r));
    // r.use();  // ✗ TCC-OWN-005: Use after move! / 移动后使用！
}
```

---

### 2. 借用检查器 (Borrow Checker)

#### 原理 / Principle
Rust 强制执行借用规则：
- 多个不可变引用（&T） OR 一个可变引用（&mut T）
- 引用的生命周期不能超过所有者

Rust enforces borrowing rules:
- Multiple immutable references (&T) OR one mutable reference (&mut T)
- References cannot outlive their owner

#### Rust C/C++ 实现 / Implementation in Rust C/C++

```cpp
// TCC-BORROW-001: Mutable Aliasing Detection
// 可变别名检测

void read_data(const int* data) {
    // 只读访问 / Read-only access
}

void write_data(int* data) {
    // 可写访问 / Write access
    *data = 42;
}

int main() {
    int value = 10;
    
    // ✓ 允许：多个只读借用 / OK: Multiple immutable borrows
    const int* r1 = &value;
    const int* r2 = &value;
    read_data(r1);
    read_data(r2);
    
    // ✗ 禁止：同时存在可变和不可变引用
    // ✗ Forbidden: Simultaneous mutable and immutable references
    int* w = &value;
    const int* r3 = &value;  // TCC-BORROW-001: Conflicting borrows!
    write_data(w);
    read_data(r3);
}
```

---

### 3. Option<T> 模式 (Option Type Pattern)

#### 原理 / Principle
消除空指针错误，强制处理"无值"情况。

Eliminates null pointer errors by forcing handling of "no value" cases.

#### Rust C/C++ 实现 / Implementation in Rust C/C++

```cpp
// TCC-OPTION-001: Enforce explicit null checks
// 强制显式空值检查

#include <optional>

// ✓ 推荐：使用 std::optional / Recommended: Use std::optional
std::optional<int> find_value(const std::vector<int>& vec, int target) {
    for (int val : vec) {
        if (val == target) return val;
    }
    return std::nullopt;  // 明确表示"无值" / Explicitly "no value"
}

int main() {
    std::vector<int> data = {1, 2, 3};
    
    // ✓ 强制检查 / Forced check
    if (auto result = find_value(data, 5)) {
        int value = *result;  // 安全解引用 / Safe dereference
    } else {
        // 处理无值情况 / Handle no-value case
    }
    
    // ✗ 禁止：直接返回可能为空的指针 / Forbidden: Return potentially null pointer
    // int* find_value_bad(const std::vector<int>& vec, int target); // TCC-OPTION-001
}
```

---

### 4. Result<T, E> 模式 (Result Type Pattern)

#### 原理 / Principle
强制错误处理，避免异常传播。

Forces error handling, avoiding exception propagation.

#### Rust C/C++ 实现 / Implementation in Rust C/C++

```cpp
// TCC-RESULT-001: Enforce error handling
// 强制错误处理

#include <variant>
#include <string>

template<typename T, typename E>
using Result = std::variant<T, E>;

enum class FileError {
    NotFound,
    PermissionDenied,
    IoError
};

// ✓ 返回 Result 类型 / Return Result type
Result<std::string, FileError> read_file(const std::string& path) {
    // 模拟文件读取 / Simulate file reading
    if (path.empty()) {
        return FileError::NotFound;
    }
    return std::string("file contents");
}

int main() {
    // ✓ 必须处理两种情况 / Must handle both cases
    auto result = read_file("test.txt");
    
    if (std::holds_alternative<std::string>(result)) {
        std::string content = std::get<std::string>(result);
        // 使用内容 / Use content
    } else {
        FileError error = std::get<FileError>(result);
        // 处理错误 / Handle error
    }
    
    // ✗ 禁止：忽略返回值 / Forbidden: Ignore return value
    // read_file("test.txt"); // TCC-RESULT-001: Unused Result!
}
```

---

### 5. 生命周期标注 (Lifetime Annotations)

#### 原理 / Principle
明确引用的有效性范围，防止悬空引用。

Explicitly specify validity scope of references to prevent dangling references.

#### Rust C/C++ 实现 / Implementation in Rust C/C++

```cpp
// TCC-LIFE-005: Lifetime binding analysis
// 生命周期绑定分析

class StringView {
    const char* data_;
    size_t size_;
    
public:
    // 标注：此 view 的生命周期绑定到 str
    // Annotation: This view's lifetime is bound to str
    StringView(const std::string& str) 
        : data_(str.data()), size_(str.size()) {}
    
    const char* data() const { return data_; }
    size_t size() const { return size_; }
};

// ✗ 危险：返回的 view 引用了局部对象
// ✗ Dangerous: Returned view references local object
StringView create_view_bad() {
    std::string local = "temp";
    return StringView(local);  // TCC-LIFE-005: Lifetime violation!
}  // local 超出作用域 / local goes out of scope

// ✓ 安全：view 的来源生命周期足够长
// ✓ Safe: Source outlives the view
StringView create_view_good(const std::string& source) {
    return StringView(source);  // OK: source 的生命周期由调用者保证
}                                 // source lifetime guaranteed by caller
```

---

### 6. 内部可变性 (Interior Mutability)

#### 原理 / Principle
在不可变上下文中提供可变性，同时保持安全性。

Provide mutability within immutable context while maintaining safety.

#### Rust C/C++ 实现 / Implementation in Rust C/C++

```cpp
// TCC-MUT-001: Interior mutability pattern
// 内部可变性模式

#include <mutex>
#include <atomic>

class ThreadSafeCounter {
    mutable std::mutex mutex_;  // ✓ 允许：在 const 方法中修改
    mutable std::atomic<int> cache_;  // ✓ Allowed: Modify in const methods
    int count_;
    
public:
    ThreadSafeCounter() : count_(0), cache_(0) {}
    
    // const 方法，但内部状态可变
    // const method, but internal state is mutable
    int get() const {
        std::lock_guard<std::mutex> lock(mutex_);
        cache_.store(count_, std::memory_order_relaxed);
        return count_;
    }
    
    void increment() {
        std::lock_guard<std::mutex> lock(mutex_);
        ++count_;
    }
};

// ✗ 禁止：不受保护的内部可变性
// ✗ Forbidden: Unprotected interior mutability
class UnsafeCache {
    mutable int unprotected_;  // TCC-MUT-001: Unsafe mutable in const context!
public:
    int get() const {
        return ++unprotected_;  // 数据竞争！/ Data race!
    }
};
```

---

### 7. 线程安全类型 (Send/Sync Traits)

#### 原理 / Principle
静态标记类型是否可以安全地在线程间传递或共享。

Statically mark whether types can be safely transferred or shared between threads.

#### Rust C/C++ 实现 / Implementation in Rust C/C++

```cpp
// TCC-SYNC-001: Thread safety annotations
// 线程安全标注

#include <thread>
#include <memory>

// ✓ 标记为线程安全 / Marked as thread-safe
class [[tcc::send]] [[tcc::sync]] ThreadSafeData {
    std::mutex mutex_;
    int value_;
public:
    void set(int v) {
        std::lock_guard<std::mutex> lock(mutex_);
        value_ = v;
    }
};

// ✗ 未标记，不应跨线程使用
// ✗ Not marked, should not be used across threads
class UnsafeData {
    int value_;  // 无保护 / No protection
public:
    void set(int v) { value_ = v; }
};

void worker(ThreadSafeData& safe, UnsafeData& unsafe) {
    safe.set(42);  // ✓ OK
    unsafe.set(42);  // ✗ TCC-SYNC-001: Non-sync type used in thread!
}

int main() {
    ThreadSafeData safe;
    UnsafeData unsafe;
    
    std::thread t1([&]() { worker(safe, unsafe); });  // 警告检测
    t1.join();
}
```

---

## 集成规则清单 / Integrated Rules Summary

| 规则 ID / Rule ID | 类别 / Category | 描述 / Description |
|------------------|----------------|-------------------|
| TCC-OWN-005 | Ownership | 移动后使用检测 / Use-after-move detection |
| TCC-BORROW-001 | Borrowing | 可变别名冲突 / Mutable aliasing conflicts |
| TCC-BORROW-002 | Borrowing | 借用生命周期超出所有者 / Borrow outlives owner |
| TCC-OPTION-001 | Safety | 强制空值检查 / Enforce null checks |
| TCC-RESULT-001 | Safety | 强制错误处理 / Enforce error handling |
| TCC-LIFE-005 | Lifetime | 生命周期绑定分析 / Lifetime binding analysis |
| TCC-MUT-001 | Concurrency | 内部可变性安全性 / Interior mutability safety |
| TCC-SYNC-001 | Concurrency | 线程安全类型检查 / Thread safety type checking |

---

## Profiler 实现策略 / Profiler Implementation Strategy

### 阶段 1: 静态分析 / Phase 1: Static Analysis

```cpp
// 使用 Clang AST 追踪变量状态
// Use Clang AST to track variable states

enum class VariableState {
    Owned,      // 拥有所有权 / Has ownership
    Moved,      // 已移动 / Has been moved
    Borrowed,   // 被借用 / Is borrowed
    Invalid     // 无效（已释放或移动后）/ Invalid (freed or post-move)
};

class StateTracker {
    std::map<const clang::VarDecl*, VariableState> states_;
    
public:
    void markMoved(const clang::VarDecl* var) {
        states_[var] = VariableState::Moved;
    }
    
    bool checkUsage(const clang::DeclRefExpr* usage) {
        auto it = states_.find(usage->getDecl());
        if (it != states_.end() && it->second == VariableState::Moved) {
            // 报告错误：移动后使用
            // Report error: use after move
            return false;
        }
        return true;
    }
};
```

### 阶段 2: 借用图构建 / Phase 2: Borrow Graph Construction

```cpp
// 构建借用关系图
// Build borrow relationship graph

class BorrowGraph {
    struct BorrowEdge {
        const clang::VarDecl* borrower;
        const clang::VarDecl* owner;
        bool is_mutable;
        clang::SourceLocation location;
    };
    
    std::vector<BorrowEdge> edges_;
    
public:
    void addBorrow(const clang::VarDecl* borrower,
                  const clang::VarDecl* owner,
                  bool is_mutable,
                  clang::SourceLocation loc) {
        edges_.push_back({borrower, owner, is_mutable, loc});
    }
    
    bool detectConflicts() {
        // 检测同一所有者的可变/不可变借用冲突
        // Detect mutable/immutable borrow conflicts for same owner
        // ...
        return true;
    }
};
```

### 阶段 3: 生命周期求解 / Phase 3: Lifetime Solving

```cpp
// 生命周期约束求解器
// Lifetime constraint solver

class LifetimeSolver {
    struct Constraint {
        const clang::VarDecl* reference;
        const clang::VarDecl* referent;
        std::string relation;  // "outlives", "equals", etc.
    };
    
    std::vector<Constraint> constraints_;
    
public:
    void addConstraint(const clang::VarDecl* ref,
                      const clang::VarDecl* referent,
                      const std::string& relation) {
        constraints_.push_back({ref, referent, relation});
    }
    
    bool solve() {
        // 使用拓扑排序检查生命周期约束
        // Use topological sort to check lifetime constraints
        // ...
        return true;
    }
};
```

---

## 与现有规则的集成 / Integration with Existing Rules

### 更新规则引擎 / Update Rule Engine

```cpp
// 在 RuleEngine 中注册新规则
// Register new rules in RuleEngine

void RuleEngine::registerAllRules() {
    // 现有规则 / Existing rules
    registerRule(std::make_unique<ForbidNewRule>());
    registerRule(std::make_unique<ForbidDeleteRule>());
    registerRule(std::make_unique<ForbidMallocFreeRule>());
    
    // 新增：Rust 风格规则 / New: Rust-style rules
    registerRule(std::make_unique<MoveSemanticRule>());        // TCC-OWN-005
    registerRule(std::make_unique<BorrowConflictRule>());      // TCC-BORROW-001
    registerRule(std::make_unique<LifetimeBindingRule>());     // TCC-LIFE-005
    registerRule(std::make_unique<OptionEnforcementRule>());   // TCC-OPTION-001
    registerRule(std::make_unique<ResultEnforcementRule>());   // TCC-RESULT-001
    registerRule(std::make_unique<InteriorMutabilityRule>());  // TCC-MUT-001
    registerRule(std::make_unique<ThreadSafetyRule>());        // TCC-SYNC-001
}
```

---

## 优势总结 / Benefits Summary

### 编译时保证 / Compile-Time Guarantees
- ✅ 无数据竞争 / No data races
- ✅ 无悬空指针 / No dangling pointers
- ✅ 无内存泄漏 / No memory leaks
- ✅ 无空指针解引用 / No null pointer dereferences

### 零运行时开销 / Zero Runtime Cost
- ✅ 所有检查在编译时完成 / All checks at compile time
- ✅ 生成的代码与手写 C++ 性能相同 / Generated code performs like hand-written C++

### 渐进式采用 / Gradual Adoption
- ✅ 可以逐文件启用 / Can be enabled file-by-file
- ✅ 与现有 C++ 代码兼容 / Compatible with existing C++ code
- ✅ 保留所有 C++ 能力 / Retains all C++ capabilities

---

## 下一步 / Next Steps

1. **完善规则实现** / Complete rule implementations
   - 实现所有 8 个新规则的 AST 检查逻辑
   - Implement AST checking logic for all 8 new rules

2. **扩展示例库** / Expand example library
   - 创建每个抽象的正面和反面示例
   - Create positive and negative examples for each abstraction

3. **性能优化** / Performance optimization
   - 优化 AST 遍历和状态追踪
   - Optimize AST traversal and state tracking

4. **工具集成** / Tool integration
   - 集成到 CMake 构建流程
   - Integrate into CMake build process
   - 提供 VS Code 插件支持
   - Provide VS Code plugin support

---

**Rust C/C++ = C++ 的表达力 + Rust 的安全性**
**Rust C/C++ = C++ Expressiveness + Rust Safety**

