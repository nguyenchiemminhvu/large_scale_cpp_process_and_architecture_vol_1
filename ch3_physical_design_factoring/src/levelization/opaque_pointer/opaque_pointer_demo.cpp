// opaque_pointer_demo.cpp                                            -*-C++-*-
//
// Demonstrates the Opaque Pointer (PIMPL) levelization technique.
// Book reference: Lakos §3.5.4 "Opaque Pointers"

#ifndef INCLUDED_DEVICE
#include <device.h>
#endif

#include <cassert>
#include <iostream>
#include <string>

int main()
{
    std::cout << "=== Chapter 3: Opaque Pointer (PIMPL) Demo ===\n\n";

    std::cout << "CONCEPT: device.h contains only a forward declaration of\n";
    std::cout << "  'DeviceImpl'. The actual implementation is hidden in device.cpp.\n";
    std::cout << "  Clients of Device have ZERO compile-time knowledge of DeviceImpl.\n\n";

    std::cout << "--- Physical Dependency Analysis ---\n";
    std::cout << "BEFORE opaque pointer:\n";
    std::cout << "  client.cpp → device.h → device_impl.h → <linux/i2c.h> (OS-specific)\n";
    std::cout << "  All clients are recompiled when DeviceImpl changes!\n\n";

    std::cout << "AFTER opaque pointer:\n";
    std::cout << "  client.cpp → device.h  (only sees forward decl of DeviceImpl)\n";
    std::cout << "  device.cpp → device_impl.h → <linux/i2c.h> (hidden!)\n";
    std::cout << "  Clients are NOT recompiled when DeviceImpl changes.\n\n";

    std::cout << "--- Demo: Using Device (client sees only device.h) ---\n";

    Device dev("/dev/sensor0");
    std::cout << "Device name: " << dev.name() << "\n";
    std::cout << "Is open: " << (dev.isOpen() ? "yes" : "no") << "\n\n";

    bool opened = dev.open();
    std::cout << "Opened: " << (opened ? "yes" : "no") << "\n";
    assert(dev.isOpen());

    dev.write("Hello, sensor!");
    dev.write("More data");
    std::cout << "Bytes written: " << dev.bytesWritten() << "\n\n";

    dev.close();
    assert(!dev.isOpen());

    // Move semantics (Device is movable, not copyable)
    std::cout << "--- Move Semantics ---\n";
    Device dev2 = std::move(dev);
    std::cout << "After move: dev2.name() = " << dev2.name() << "\n";
    dev2.open();
    dev2.write("After move");
    dev2.close();

    std::cout << "\n--- Key Takeaways ---\n";
    std::cout << "1. Opaque pointer breaks compile-time dependency on implementation\n";
    std::cout << "2. Destructor MUST be defined in .cpp (DeviceImpl must be complete)\n";
    std::cout << "3. Move operations can be defaulted in .cpp for same reason\n";
    std::cout << "4. Copy is usually deleted (or requires deep copy in .cpp)\n";
    std::cout << "5. Trade-off: heap allocation + pointer indirection at runtime\n";

    return 0;
}
