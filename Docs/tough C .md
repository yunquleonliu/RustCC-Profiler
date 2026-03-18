# Tough C：所有权与生命周期增强规范 (Draft v1.0)

> Note / 说明:
> This document describes design semantics and direction.
> 本文档描述设计语义与方向。
>
> Current implementation completeness is tracked in `Working Track .md`.
> 当前实现完成度请以 `Working Track .md` 为准。

## 1. 核心哲学：语义升级 (Semantic Upgrade)

我们不增加新关键字，而是通过 Profiler 对现有 C/C++ 语法进行严格约束：

| **现有语法** | **标准 C++ 语义** | **Tough C 增强语义 (Profiler 强制)** |
| --- | --- | --- |
| `T*` | 裸指针，无所有权概念 | **Borrowed (可变借用)**。受生命周期检查。 |
| `const T*` | 常量指针 | **Shared Ref (只读借用)**。禁止任何修改。 |
| `T&&` | 右值引用 (仅建议移动) | **Owned (唯一所有权)**。一旦赋值，原变量失效。 |
| `RAII Object` | 栈对象 | **Scope Owner**。超出作用域自动注入清理逻辑。 |

## 2. 内存模型：借用检查规则 (Borrow Checker)

Profiler 将对代码进行静态着色分析，强制执行以下规则：

1. **唯一所有权：** 标记为 `T&&` 的变量在赋值或作为参数传递后，Profiler 标记原符号为 `DELETED`状态，后续访问报错。

2. **读写互斥：** 同一逻辑路径下，若存在 `T*` (可变)，则不允许存在任何其他的 `const T*` (只读)。

3. **生命周期绑定：** 借用指针（`T*`）的生命周期跨度严禁超过其来源的所有者（`T&&` 或栈对象）。

## 3. Profiler 整合方案：三位一体流水线

我们将 Profiler 嵌入到构建流程中，分为 **Static Linter**、**Runtime Sentinel** 和 **Code Generator**。

### A. 静态阶段：语义提取 (The Annotator)

工程师编写看起来像标准 C++ 的代码，但 Profiler 进行深度静态分析。
C++
```
// Tough C 源码 (工程师视角)

void

process_packet
(Packet&& pkt)

{ 
// 语义：我拿走了所有权

    log_id(pkt.id);
} 

int

main
()

{
    Packet&& my_pkt = alloc_p(); 
    process_packet(
std
::move(my_pkt)); 

// my_pkt.id = 1; // Profiler 静态报错：Use after move!

}

```

### B. 动态阶段：影子内存 (The Sentinel) （这部分以后可以作为服务提供的）

在 安全检查的 “仿真” 模式下，Profiler 自动在生成的代码中注入“内存哨兵”：

- **破坏性移动：** 在 `move` 发生后，插入 `ptr = nullptr`。

- **边界检查：** 自动为数组访问注入边界检查代码。

### C. 生成阶段：零成本 C++ (The Generator)

通过检查后，生成纯净的高性能 C++ 代码：

- 移除所有权检查逻辑（因为静态阶段已保证安全）。

- 将 `T&&` 优化为最快的寄存器传递方式。

- 自动插入 `restrict` 关键字协助编译器进行指令重排。

## 4. 整合进 Tough C 项目的路线图

### 第一步：定义宏约束 (The Guard)

为了不引入新关键字且兼容标准编译器，我们定义一组语义宏：
C++
```
#
define
 TC_OWN &&       
// 代表所有权转移

#
define
 TC_MUT * 
// 代表可变借用

#
define
 TC_REF const * 
// 代表只读借用

```

### 第二步：开发 Clang-based Profiler

利用 Clang 的 AST (抽象语法树) 插件：

1. **追踪变量状态：** 在遍历语法树时，记录每个变量的 `Owned/Borrowed/Dead` 状态。

2. **分析生命周期：** 检查指针赋值是否跨越了其对象的有效作用域。

### 第三步：集成构建系统 (Makefile/CMake)
Bash
```
# 执行流程

tc-profiler ./src/main.cpp --check  
# 1. 静态安全检查

g++ ./src/main.cpp -o app           
# 2. 标准编译 (无感知转换)

```

## 5. 下一步行动

1. **确定 Minimal Viable Semantics (MVS)：** 我们先只针对“指针移动 (Move)”和“空指针 (Null)”进行 Profiler 开发。

2. **编写一些 Tough C 实例：** 建议用 常见的 嵌入系统Embedded C ，常用的C++_ 算法的常见内存管理任务做试点。

3. **集成 Valgrind/ASan：** 作为 Profiler 的动态验证补充，确保静态逻辑的正确性。

** 写具体的 Clang AST 检查逻辑伪代码，展示 Profiler 是如何识别出“所有权非法二次使用”**