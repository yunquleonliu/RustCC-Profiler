// Test: TCC-OWN-008 — double-free via pointer aliasing
// Should trigger: one OWN-008 error

int main() {
    int* p = new int(42);
    int* q = p;    // q aliases p's allocation
    delete p;      // first delete
    delete q;      // ERROR: double-free — q still aliases deleted allocation
    return 0;
}
