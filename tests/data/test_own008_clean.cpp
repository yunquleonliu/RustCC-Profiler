// Test: TCC-OWN-008 clean — no double-free
// Should NOT trigger OWN-008

int main() {
    int* p = new int(42);
    delete p;
    p = nullptr;
    // p is nulled — no double-free risk tracked by alias analysis
    return 0;
}
