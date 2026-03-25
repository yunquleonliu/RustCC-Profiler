// Test: TCC-BORROW-004 — owner modified while immutably borrowed
// Should trigger: one BORROW-004 error

int main() {
    int x = 10;
    const int& r = x;  // immutable borrow
    x = 20;            // ERROR: modification while borrowed
    int v = r;
    (void)v;
    return 0;
}
