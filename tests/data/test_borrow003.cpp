// Test: TCC-BORROW-003 — multiple mutable borrows of same variable
// Should trigger: one BORROW-003 error

int main() {
    int x = 5;
    int* p1 = &x;  // first mutable borrow
    int* p2 = &x;  // second mutable borrow — ERROR
    *p1 = 10;
    *p2 = 20;
    return 0;
}
