// Test: TCC-ITER-001 clean — no modification during iteration
// Should NOT trigger ITER-001

#include <vector>

int main() {
    std::vector<int> v = {1, 2, 3};
    std::vector<int> out;
    for (auto& x : v) {
        out.push_back(x * 2);  // modifying a DIFFERENT container — safe
    }
    return 0;
}
