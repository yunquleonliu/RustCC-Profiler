# Tough C Tests / Tough C 测试
# Test suite for TCC profiler
# TCC 分析器测试套件

# Enable testing / 启用测试
enable_testing()

# Test utilities / 测试工具
set(TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

# Helper function to add TCC test / 添加 TCC 测试的辅助函数
function(add_tcc_test test_name source_file should_pass)
    add_test(
        NAME ${test_name}
        COMMAND tcc-check ${TEST_DATA_DIR}/${source_file}
    )
    
    if(should_pass)
        # Test should pass (exit code 0) / 测试应该通过（退出码0）
        set_tests_properties(${test_name} PROPERTIES
            PASS_REGULAR_EXPRESSION "All checks passed"
        )
    else()
        # Test should fail (exit code 1) / 测试应该失败（退出码1）
        set_tests_properties(${test_name} PROPERTIES
            WILL_FAIL TRUE
        )
    endif()
endfunction()

# Ownership tests / 所有权测试
add_tcc_test(ownership_pass_smart_pointers "pass/ownership_smart_pointers.cpp" TRUE)
add_tcc_test(ownership_pass_containers "pass/ownership_containers.cpp" TRUE)
add_tcc_test(ownership_fail_new_delete "fail/ownership_new_delete.cpp" FALSE)
add_tcc_test(ownership_fail_malloc_free "fail/ownership_malloc_free.cpp" FALSE)
add_tcc_test(ownership_fail_raw_owning_ptr "fail/ownership_raw_owning_ptr.cpp" FALSE)

# Lifetime tests / 生命周期测试
add_tcc_test(lifetime_pass_correct_patterns "pass/lifetime_correct_patterns.cpp" TRUE)
add_tcc_test(lifetime_fail_dangling "fail/lifetime_dangling.cpp" FALSE)
add_tcc_test(lifetime_fail_raw_ptr_container "fail/lifetime_raw_ptr_container.cpp" FALSE)
add_tcc_test(lifetime_fail_ref_member "fail/lifetime_ref_member.cpp" FALSE)

# Concurrency tests / 并发测试
add_tcc_test(concurrency_pass_safe "pass/concurrency_safe.cpp" TRUE)
add_tcc_test(concurrency_fail_unsafe "fail/concurrency_unsafe.cpp" FALSE)

# Complete test suite for MVP / MVP 完整测试套件
# Total: 12 tests (6 pass, 6 fail) / 总计：12 个测试（6 个通过，6 个失败）

# Move semantics tests (Rust-inspired: TCC-OWN-005/006/007)
# 移动语义测试（Rust 风格：TCC-OWN-005/006/007）
add_tcc_test(move_pass_safe "pass/move_semantics_safe.cpp" TRUE)
add_tcc_test(move_fail_use_after_move "fail/move_use_after_move.cpp" FALSE)

# Safety pattern tests (Rust-inspired: TCC-OPTION, TCC-RESULT, TCC-PANIC, TCC-SAFE)
# 安全模式测试（Rust 风格：TCC-OPTION、TCC-RESULT、TCC-PANIC、TCC-SAFE）
add_tcc_test(safety_pass_option_result "pass/safety_option_result.cpp" TRUE)
add_tcc_test(safety_fail_violations "fail/safety_violations.cpp" FALSE)
