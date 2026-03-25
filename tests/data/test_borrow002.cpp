// Test: TCC-BORROW-002 — borrow outlives owner (pointer escapes inner scope)
// Should trigger: one BORROW-002 error

int main() {
    int* outer_ptr = nullptr;
    {
        int local = 42;
        outer_ptr = &local;  // ERROR: outer_ptr will outlive local
    }
    // outer_ptr now dangles
    return 0;
}
