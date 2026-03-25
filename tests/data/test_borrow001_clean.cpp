// Test: TCC-BORROW-001 clean — no conflicting borrows
// Should NOT trigger BORROW-001

int main() {
    int x = 10;
    // Only one kind of borrow at a time
    { int* p = &x; *p = 20; }
    { const int* cp = &x; int v = *cp; (void)v; }
    return 0;
}
