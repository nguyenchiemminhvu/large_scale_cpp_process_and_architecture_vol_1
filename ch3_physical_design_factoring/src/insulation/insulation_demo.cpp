// insulation_demo.cpp                                                -*-C++-*-
//
// Demonstrates compile-time coupling vs. insulation.
// Book reference: Lakos §3.6, §3.7

#ifndef INCLUDED_HEAVY_WIDGET
#include <heavy_widget.h>
#endif

#ifndef INCLUDED_INSULATED_WIDGET
#include <insulated_widget.h>
#endif

#include <cassert>
#include <iostream>
#include <string>

int main()
{
    std::cout << "=== Chapter 3: Compile-Time Coupling vs. Insulation ===\n\n";

    std::cout << "--- HeavyWidget (excessive coupling) ---\n";
    std::cout << "  Including heavy_widget.h forces this TU to parse:\n";
    std::cout << "    <algorithm>, <array>, <chrono>, <functional>,\n";
    std::cout << "    <map>, <set>, <unordered_map>, <vector>, ...\n\n";

    HeavyWidget hw("sensor-001");
    hw.addData(42);
    hw.addData(73);
    hw.addTag("critical");
    hw.addTag("live");

    assert(hw.name() == "sensor-001");
    assert(hw.dataSize() == 2);
    assert(hw.hasTag("critical"));
    assert(!hw.hasTag("archived"));

    std::cout << "  HeavyWidget name: " << hw.name() << "\n";
    std::cout << "  Data size: " << hw.dataSize() << "\n";
    std::cout << "  Has 'critical': " << (hw.hasTag("critical") ? "yes" : "no") << "\n\n";

    std::cout << "--- InsulatedWidget (minimal coupling) ---\n";
    std::cout << "  Including insulated_widget.h only requires:\n";
    std::cout << "    <memory>, <string>\n";
    std::cout << "  All heavy headers are hidden in insulated_widget.cpp\n\n";

    InsulatedWidget iw("sensor-002");
    iw.addData(100);
    iw.addData(200);
    iw.addData(300);
    iw.addTag("archived");

    assert(iw.name() == "sensor-002");
    assert(iw.dataSize() == 3);
    assert(!iw.hasTag("critical"));
    assert(iw.hasTag("archived"));

    std::cout << "  InsulatedWidget name: " << iw.name() << "\n";
    std::cout << "  Data size: " << iw.dataSize() << "\n";
    std::cout << "  Has 'archived': " << (iw.hasTag("archived") ? "yes" : "no") << "\n\n";

    // Move semantics
    InsulatedWidget iw2 = std::move(iw);
    std::cout << "  After move: iw2.name() = " << iw2.name() << "\n\n";

    std::cout << "=== Insulation Comparison ===\n\n";
    std::cout << "                    HeavyWidget   InsulatedWidget\n";
    std::cout << "Headers exposed:    Many (7+)     2 (<memory>, <string>)\n";
    std::cout << "Recompile on change: Yes           No (impl hidden)\n";
    std::cout << "Runtime overhead:   None          Heap alloc + pointer\n";
    std::cout << "ABI stability:      Low           High\n\n";

    std::cout << "RULE (Lakos §3.6):\n";
    std::cout << "  Insulate when:\n";
    std::cout << "    - The header includes many/heavy other headers\n";
    std::cout << "    - The implementation is likely to change\n";
    std::cout << "    - ABI stability is required (shared library)\n";
    std::cout << "  Don't insulate when:\n";
    std::cout << "    - The type is a simple value type (Date, Point)\n";
    std::cout << "    - Performance is critical (no heap alloc)\n";
    std::cout << "    - The header is already minimal\n";

    return 0;
}
