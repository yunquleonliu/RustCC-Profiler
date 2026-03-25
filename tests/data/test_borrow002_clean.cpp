// Test: TCC-BORROW-002 clean — no outliving borrows
// Should NOT trigger BORROW-002

int main() {
    int local = 42;
    int* p = &local;  // Same scope: no outlives issue
    *p = 10;
    return 0;
}
