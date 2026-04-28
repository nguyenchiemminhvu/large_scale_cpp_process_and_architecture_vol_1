// device.h                                                           -*-C++-*-
//
// Demonstrates the OPAQUE POINTER (PIMPL) levelization technique.
// Book reference: Lakos §3.5.4
//
// PROBLEM BEING SOLVED:
//   Class 'Device' used to include 'device_impl.h' in its header, pulling in
//   heavy system headers (OS-specific, hardware-specific) transitively into
//   every client's translation unit.
//
// SOLUTION: Opaque Pointer (PIMPL = Pointer to IMPLementation)
//   - The 'Device' header only forward-declares 'DeviceImpl'
//   - The actual DeviceImpl is defined only in device.cpp
//   - Clients that include device.h have ZERO knowledge of DeviceImpl
//
// BEFORE (strong header dependency):
//   device.h includes device_impl.h
//   device.h includes <linux/i2c.h>      ← platform-specific!
//   device.h includes <sys/ioctl.h>      ← OS-specific!
//   Every user of Device must compile with all of this.
//
// AFTER (opaque pointer):
//   device.h: class DeviceImpl;  ← forward declaration only
//   device.cpp: #include "device_impl.h"  ← kept private
//
// TRADE-OFF:
//   + Compile-time isolation (clients don't see DeviceImpl)
//   + Can change DeviceImpl without recompiling clients
//   + Hides platform-specific headers from the public interface
//   - Runtime overhead: extra heap allocation + pointer indirection
//   - Slightly more complex implementation (must manage lifetime)
//
// INCLUDE GUARD: INCLUDED_DEVICE

#ifndef INCLUDED_DEVICE
#define INCLUDED_DEVICE

#include <memory>   // std::unique_ptr
#include <string>

// Forward declaration — clients of device.h know DeviceImpl EXISTS
// but know NOTHING about its structure. This is the "opaque" part.
class DeviceImpl;

/// A hardware device abstraction.
///
/// Clients of this class cannot see DeviceImpl's implementation details.
/// The physical dependency on DeviceImpl is hidden in device.cpp.
class Device {
    std::unique_ptr<DeviceImpl> d_impl_p;  // opaque pointer

  public:
    /// Creates a device with the given device name.
    explicit Device(const std::string& name);

    /// Destructor must be defined in .cpp (where DeviceImpl is complete).
    ~Device();

    // Non-copyable (unique ownership of impl)
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // Movable
    Device(Device&&) noexcept;
    Device& operator=(Device&&) noexcept;

    // MANIPULATORS

    /// Opens the device for I/O.
    bool open();

    /// Closes the device.
    void close();

    /// Writes data to the device.
    bool write(const std::string& data);

    // ACCESSORS

    /// Returns the device name.
    std::string name() const;

    /// Returns true if the device is currently open.
    bool isOpen() const;

    /// Returns the number of bytes written since opening.
    int bytesWritten() const;
};

#endif  // INCLUDED_DEVICE
