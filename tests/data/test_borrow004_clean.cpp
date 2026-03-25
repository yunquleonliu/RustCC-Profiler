// Test: TCC-BORROW-004 clean — modify before borrowing
// Should NOT trigger BORROW-004

int main() {
    int x = 10;
    x = 20;            // modification before borrow — safe
    const int& r = x;  // immutable borrow after modification
    int v = r;
    (void)v;
    return 0;
}
