// Test: TCC-LIFE-005 — cross-function lifetime: returning ref to by-value param
// Should trigger: one LIFE-005 error

// Pattern A: return reference to by-value parameter (local copy)
const int& getRef(int val) {
    return val;  // ERROR: val is a local copy, reference will dangle
}

int main() {
    const int& r = getRef(42);
    (void)r;
    return 0;
}
