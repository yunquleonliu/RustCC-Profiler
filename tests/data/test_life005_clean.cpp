// Test: TCC-LIFE-005 clean — no cross-function lifetime escape
// Should NOT trigger LIFE-005

static int g_val = 100;

// Pattern C variant (safe): storing address of GLOBAL (non-local) — OK
void storeGlobal(int** out) {
    *out = &g_val;  // g_val is global — safe
}

// Returning const ref to a member — safe (caller keeps object alive)
struct S { int x; };
const int& getX(const S& s) { return s.x; }  // ref param, not value param

int main() {
    int* p = nullptr;
    storeGlobal(&p);
    S s{42};
    const int& r = getX(s);
    (void)r;
    return 0;
}
