// Test: TCC-BORROW-003 clean — single mutable borrow at a time
// Should NOT trigger BORROW-003

int main() {
    int x = 5;
    { int* p1 = &x; *p1 = 10; }
    { int* p2 = &x; *p2 = 20; }
    return 0;
}
