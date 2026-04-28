// linkage_demo_main.cpp                                               -*-C++-*-
//
// Demonstrates the linkage concepts from linkage_demo component.
// This is a standalone executable showing external vs. internal linkage.

#ifndef INCLUDED_LINKAGE_DEMO
#include <linkage_demo.h>
#endif

#include <iostream>
#include <string>

int main()
{
    std::cout << "=== Chapter 1: Linkage Demo ===\n\n";

    // --- External linkage function ---
    std::cout << "External linkage function:\n";
    std::cout << "  " << linkage::externalLinkageFunction() << "\n\n";

    // --- Global extern variable ---
    std::cout << "Global extern variable (g_externalCounter):\n";
    std::cout << "  Before: " << linkage::g_externalCounter << "\n";
    ++linkage::g_externalCounter;
    std::cout << "  After:  " << linkage::g_externalCounter << "\n\n";

    // --- Class with static member ---
    std::cout << "Static class member:\n";
    std::cout << "  Instance count before: "
              << linkage::LinkageExample::s_instanceCount << "\n";

    linkage::LinkageExample obj1(10);
    linkage::LinkageExample obj2(20);

    std::cout << "  Instance count after 2 objects: "
              << linkage::LinkageExample::s_instanceCount << "\n";
    std::cout << "  obj1.getValue() = " << obj1.getValue() << "\n";
    std::cout << "  obj2.getValue() = " << obj2.getValue() << "\n";

    obj1.increment();
    std::cout << "  After increment: obj1.getValue() = "
              << obj1.getValue() << "\n\n";

    // --- Inline function ---
    std::cout << "Inline function (inlineAdd):\n";
    std::cout << "  inlineAdd(3, 4) = " << linkage::inlineAdd(3, 4) << "\n\n";

    std::cout << "=== Key Linkage Concepts ===\n";
    std::cout << "\n1. EXTERNAL LINKAGE:\n";
    std::cout << "   Functions/vars declared in .h, defined in .cpp\n";
    std::cout << "   Visible across ALL translation units\n";
    std::cout << "   Single definition required (One Definition Rule)\n";

    std::cout << "\n2. INTERNAL LINKAGE (static / anonymous namespace):\n";
    std::cout << "   Visible ONLY within the .cpp that defines it\n";
    std::cout << "   Cannot be accessed from other translation units\n";
    std::cout << "   Safe from name collisions with other .cpp files\n";

    std::cout << "\n3. INLINE FUNCTIONS:\n";
    std::cout << "   Defined in header, but SAME definition in all TUs\n";
    std::cout << "   ODR requires all definitions be identical\n";
    std::cout << "   Linker merges duplicate copies\n";

    std::cout << "\n4. COMPONENT PROPERTY 2 (No Symbol Leakage):\n";
    std::cout << "   .cpp files must NOT define external-linkage symbols\n";
    std::cout << "   that are NOT declared in the corresponding .h file.\n";
    std::cout << "   Use anonymous namespace or 'static' for helpers.\n";

    return 0;
}
