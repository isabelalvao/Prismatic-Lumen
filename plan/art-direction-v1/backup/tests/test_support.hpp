#pragma once
#include <cmath>
#include <iostream>
#include <string>

inline int test_failures = 0;
inline void test_check(bool condition, const char* expression, int line) {
    if (!condition) {
        ++test_failures;
        std::cerr << "FAIL line " << line << ": " << expression << '\n';
    }
}
#define CHECK(x) test_check(static_cast<bool>(x), #x, __LINE__)
#define CHECK_NEAR(a,b,e) CHECK(std::abs((a)-(b)) <= (e))
inline int test_finish() {
    if (!test_failures) std::cout << "All checks passed\n";
    return test_failures ? 1 : 0;
}
