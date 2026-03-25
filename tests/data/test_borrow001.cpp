// Test: TCC-BORROW-001 — conflicting mutable and immutable borrows
// Should trigger: one BORROW-001 error

int main() {
    int x = 10;
    int* mutable_ptr = &x;         // mutable borrow
    const int* immutable_ptr = &x; // immutable borrow — CONFLICT
    *mutable_ptr = 20;
    int v = *immutable_ptr;
    (void)v;
    return 0;
}
