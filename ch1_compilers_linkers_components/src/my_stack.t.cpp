// my_stack.t.cpp                                                      -*-C++-*-
//
// TEST DRIVER for the my_stack component.
//
// LAKOS CONVENTION: Test drivers are named <component>.t.cpp
//
// This file demonstrates:
//   1. The "Hierarchical Testability Requirement" (§2.14)
//   2. Test driver structure (black-box + white-box tests)
//   3. Uniform test driver invocation interface (returns 0=pass, non-zero=fail)
//
// KEY POINT (Lakos §2.14.3):
//   A test driver must be buildable using ONLY the dependencies
//   of the component under test. It cannot depend on things the
//   component itself doesn't depend on. This keeps test isolation clean.
//
// For my_stack, dependencies are: only standard library.
// Therefore this test driver also only depends on the standard library.

#ifndef INCLUDED_MY_STACK
#include <my_stack.h>
#endif

#include <cassert>
#include <iostream>
#include <string>

// ============================================================
// TEST CASES
// Lakos convention: numbered test cases, most complex last.
// A test driver typically has:
//   - "positive test cases" (correct usage)
//   - "negative test cases" (boundary conditions, error handling)
// ============================================================

static int testCase1_DefaultConstruction()
{
    // Test: Default-constructed stack is empty
    my::Stack<int> s;
    assert(s.empty());
    assert(s.size() == 0);
    std::cout << "  [PASS] Test 1: Default construction\n";
    return 0;
}

static int testCase2_PushAndTop()
{
    // Test: Push elements and access top
    my::Stack<int> s;
    s.push(10);
    assert(!s.empty());
    assert(s.size() == 1);
    assert(s.top() == 10);

    s.push(20);
    assert(s.size() == 2);
    assert(s.top() == 20);  // LIFO: 20 is on top

    s.push(30);
    assert(s.top() == 30);

    std::cout << "  [PASS] Test 2: Push and top\n";
    return 0;
}

static int testCase3_Pop()
{
    // Test: Pop removes top element (LIFO order)
    my::Stack<int> s;
    s.push(1);
    s.push(2);
    s.push(3);

    assert(s.top() == 3);
    s.pop();
    assert(s.top() == 2);
    s.pop();
    assert(s.top() == 1);
    s.pop();
    assert(s.empty());

    std::cout << "  [PASS] Test 3: Pop (LIFO order)\n";
    return 0;
}

static int testCase4_ExceptionOnEmptyPop()
{
    // Test: Pop on empty stack throws
    my::Stack<int> s;
    bool caught = false;
    try {
        s.pop();
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    std::cout << "  [PASS] Test 4: Exception on empty pop\n";
    return 0;
}

static int testCase5_ExceptionOnEmptyTop()
{
    // Test: Top on empty stack throws
    my::Stack<int> s;
    bool caught = false;
    try {
        const int& t = s.top();
        (void)t;
    } catch (const std::out_of_range&) {
        caught = true;
    }
    assert(caught);

    std::cout << "  [PASS] Test 5: Exception on empty top\n";
    return 0;
}

static int testCase6_MoveSemantics()
{
    // Test: Push by move
    my::Stack<std::string> s;
    std::string hello = "hello";
    s.push(std::move(hello));
    assert(s.top() == "hello");
    assert(hello.empty() || true);  // moved-from state is valid but unspecified

    s.push(std::string("world"));
    assert(s.top() == "world");

    std::cout << "  [PASS] Test 6: Move semantics\n";
    return 0;
}

static int testCase7_InitialCapacity()
{
    // Test: Stack with initial capacity hint
    my::Stack<int> s(100);
    assert(s.empty());
    assert(s.size() == 0);

    for (int i = 0; i < 100; ++i) {
        s.push(i);
    }
    assert(s.size() == 100);
    assert(s.top() == 99);

    std::cout << "  [PASS] Test 7: Initial capacity\n";
    return 0;
}

// ============================================================
// Main test runner
//
// Lakos convention (§2.14.7): Uniform invocation interface —
// the test driver accepts no arguments and returns 0 on success.
// This allows automated test infrastructure to treat all test
// drivers uniformly: just run them and check exit code.
// ============================================================

int main()
{
    std::cout << "=== my_stack Component Test Driver ===\n\n";
    std::cout << "Running test cases (most specific to most general):\n";

    int failures = 0;
    failures += testCase1_DefaultConstruction();
    failures += testCase2_PushAndTop();
    failures += testCase3_Pop();
    failures += testCase4_ExceptionOnEmptyPop();
    failures += testCase5_ExceptionOnEmptyTop();
    failures += testCase6_MoveSemantics();
    failures += testCase7_InitialCapacity();

    if (failures == 0) {
        std::cout << "\n[ALL TESTS PASSED]\n";
        std::cout << "\nKey points demonstrated:\n";
        std::cout << "  - Test driver only depends on my_stack.h (+ std library)\n";
        std::cout << "  - Each test case is independent and numbered\n";
        std::cout << "  - Uniform invocation: return 0 = all tests pass\n";
        std::cout << "  - This satisfies Hierarchical Testability (Lakos §2.14)\n";
    } else {
        std::cout << "\n[" << failures << " TEST(S) FAILED]\n";
    }

    return failures;  // 0 = success, non-zero = failure(s)
}
