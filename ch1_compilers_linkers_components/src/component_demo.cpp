// component_demo.cpp                                                  -*-C++-*-
//
// Full demonstration of Chapter 1 concepts:
//   - The four Component Properties
//   - Level numbers and the dependency DAG
//   - Include guards
//   - How to read the dependency graph
//
// This executable links against both my_stack and linkage_demo components,
// showing how components are combined at the application level.
//
// Book reference: Chapter 1, §1.6–1.12

#ifndef INCLUDED_MY_STACK
#include <my_stack.h>
#endif

#ifndef INCLUDED_LINKAGE_DEMO
#include <linkage_demo.h>
#endif

#ifndef INCLUDED_INCLUDE_GUARD_DEMO
#include <include_guard_demo.h>
#endif

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

// ============================================================
// SECTION 1: Demonstrating Component Properties
// ============================================================

static void demoComponentProperties()
{
    std::cout << "--- Component Properties (Lakos §1.6) ---\n\n";

    std::cout << "Property 1: Self-Sufficient Header\n";
    std::cout << "  my_stack.h can be compiled standalone.\n";
    std::cout << "  Verified by: it is the FIRST #include in my_stack.cpp\n\n";

    std::cout << "Property 2: No Symbol Leakage\n";
    std::cout << "  linkage_demo.cpp defines internal helpers in anonymous\n";
    std::cout << "  namespace — they cannot be accessed from here.\n\n";

    std::cout << "Property 3: Clients Use Only the .h File\n";
    std::cout << "  This file includes my_stack.h, never my_stack.cpp\n\n";

    std::cout << "Property 4: Dependencies via #include\n";
    std::cout << "  This component's dependencies:\n";
    std::cout << "    - my_stack.h      (provides my::Stack)\n";
    std::cout << "    - linkage_demo.h  (provides linkage::LinkageExample)\n";
    std::cout << "    - include_guard_demo.h\n";
    std::cout << "    - Standard library headers\n\n";
}

// ============================================================
// SECTION 2: Demonstrating Level Numbers
// ============================================================

static void demoLevelNumbers()
{
    std::cout << "--- Level Numbers (Lakos §1.10) ---\n\n";

    std::cout << "Level numbers describe position in the dependency DAG:\n\n";

    std::cout << "  Level 1: Components with NO dependencies in the codebase\n";
    std::cout << "           Examples: my_stack, linkage_demo\n";
    std::cout << "           (depend only on standard library)\n\n";

    std::cout << "  Level 2: Components that depend on Level 1 components\n";
    std::cout << "           Example: A 'Date' component using a math utility\n\n";

    std::cout << "  Level N: Depends on Level N-1 and lower\n\n";

    std::cout << "  RULE: If you cannot assign level numbers, you have cycles!\n";
    std::cout << "  This 'component_demo' is at Level 2 (uses Level 1 components)\n\n";

    // Practical demonstration: build a dependency trace
    std::cout << "  Dependency graph for this demo:\n";
    std::cout << "    component_demo  (Level 2)\n";
    std::cout << "      ↓ uses\n";
    std::cout << "    my_stack        (Level 1) → std::vector\n";
    std::cout << "    linkage_demo    (Level 1) → std::string\n";
    std::cout << "    include_guard_demo (Level 1) → std::string\n\n";
}

// ============================================================
// SECTION 3: Practical Usage of Stacked Components
// ============================================================

static void demoUsage()
{
    std::cout << "--- Practical Usage ---\n\n";

    // Using my_stack component
    my::Stack<std::string> callStack;
    callStack.push("main");
    callStack.push("demoUsage");

    std::cout << "Call stack simulation:\n";
    std::cout << "  Top: " << callStack.top() << "\n";
    callStack.pop();
    std::cout << "  After pop: " << callStack.top() << "\n\n";

    // Using linkage_demo component
    linkage::LinkageExample le(42);
    std::cout << "LinkageExample value: " << le.getValue() << "\n";
    le.increment();
    std::cout << "After increment: " << le.getValue() << "\n\n";

    // Using include_guard_demo component
    auto practices = guard_demo::bestPractices();
    std::cout << "Include Guard Best Practices:\n";
    for (const auto& p : practices) {
        std::cout << "  - " << p << "\n";
    }
    std::cout << "\n";
}

// ============================================================
// SECTION 4: Demonstrating ODR (One Definition Rule)
// ============================================================

static void demoODR()
{
    std::cout << "--- One Definition Rule (ODR) ---\n\n";

    std::cout << "The ODR states:\n";
    std::cout << "  - A variable/function may be DECLARED multiple times\n";
    std::cout << "  - A variable/function may be DEFINED only once\n";
    std::cout << "  - EXCEPTION: inline functions/templates may have\n";
    std::cout << "    multiple identical definitions (one per TU)\n\n";

    std::cout << "Inline function test (inlineAdd is defined in header):\n";
    // This call is safe even though linkage_demo.h is included here
    // AND potentially included from other TUs — the ODR allows this
    // because inlineAdd is inline.
    int result = linkage::inlineAdd(5, 7);
    std::cout << "  inlineAdd(5, 7) = " << result << "\n\n";

    std::cout << "g_externalCounter (global, external linkage):\n";
    // This is ONE variable shared across all TUs — modifying it here
    // is visible everywhere.
    std::cout << "  Before: " << linkage::g_externalCounter << "\n";
    ++linkage::g_externalCounter;
    std::cout << "  After:  " << linkage::g_externalCounter << "\n\n";
}

// ============================================================
// SECTION 5: Include Guard Verification
// ============================================================

static void demoIncludeGuards()
{
    std::cout << "--- Include Guards ---\n\n";

    // Including the same header multiple times is safe because of guards
    // (The #include at the top of this file already included these headers)
    // The external guard pattern (used at the top of this file) prevents
    // even the file-open cost on subsequent includes:
    //   #ifndef INCLUDED_MY_STACK
    //   #include <my_stack.h>
    //   #endif

    std::cout << "Include guard format (Lakos convention):\n";
    std::cout << "  #ifndef INCLUDED_<COMPONENT_NAME_IN_UPPERCASE>\n";
    std::cout << "  #define INCLUDED_<COMPONENT_NAME_IN_UPPERCASE>\n";
    std::cout << "  // ... header content ...\n";
    std::cout << "  #endif  // INCLUDED_<COMPONENT_NAME_IN_UPPERCASE>\n\n";

    std::cout << "External guard pattern (build optimization):\n";
    std::cout << "  In .cpp files:\n";
    std::cout << "    #ifndef INCLUDED_SOME_HEADER\n";
    std::cout << "    #include <some_header.h>\n";
    std::cout << "    #endif\n\n";

    std::cout << guard_demo::describeIncludeGuards() << "\n\n";
}

// ============================================================
// Main
// ============================================================

int main()
{
    std::cout << "=== Chapter 1: Compilers, Linkers, and Components Demo ===\n\n";

    demoComponentProperties();
    demoLevelNumbers();
    demoUsage();
    demoODR();
    demoIncludeGuards();

    std::cout << "=== Summary of Key Rules ===\n";
    std::cout << "\nComponent Properties:\n";
    std::cout << "  CP1: .h file is self-sufficient (compilable standalone)\n";
    std::cout << "  CP2: .cpp exports no symbols absent from .h\n";
    std::cout << "  CP3: Clients #include only the .h\n";
    std::cout << "  CP4: Depends-On relation = #include chain\n";

    std::cout << "\nDesign Rules:\n";
    std::cout << "  DR: .h must be the FIRST #include in its .cpp\n";
    std::cout << "  DR: Every header must have include guards\n";
    std::cout << "  DR: No 'using namespace' in headers (outside functions)\n";
    std::cout << "  DR: No cyclic #include dependencies\n";

    return 0;
}
