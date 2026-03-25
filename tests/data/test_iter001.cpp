// Test: TCC-ITER-001 — container modified during range-based for loop
// Should trigger: one ITER-001 error

#include <vector>

int main() {
    std::vector<int> v = {1, 2, 3};
    for (auto& x : v) {
        v.push_back(x * 2);  // ERROR: modifies v while iterating it
    }
    return 0;
}
