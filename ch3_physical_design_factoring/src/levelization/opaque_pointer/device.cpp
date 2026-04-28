// device.cpp                                                         -*-C++-*-
//
// Component Property 1: device.h is the FIRST include.
//
// CRITICAL: DeviceImpl is ONLY defined here, never in device.h.
// This is what creates the compile-time isolation.

#ifndef INCLUDED_DEVICE
#include <device.h>
#endif

#include <iostream>
#include <string>

// ============================================================
// DeviceImpl: The "hidden" implementation class
//
// In a real system, this might include platform-specific headers
// like <linux/i2c-dev.h>, <windows.h>, <sys/ioctl.h> etc.
// By keeping it here (in the .cpp), we prevent those headers
// from polluting client compilation units.
// ============================================================

class DeviceImpl {
    std::string d_name;
    bool        d_isOpen;
    int         d_bytesWritten;

    // In a real system: platform-specific handle here
    // e.g.:  int d_fd;          // POSIX file descriptor
    //        HANDLE d_handle;   // Windows HANDLE

  public:
    explicit DeviceImpl(const std::string& name)
    : d_name(name)
    , d_isOpen(false)
    , d_bytesWritten(0)
    {
    }

    bool open() {
        // Real code: open("/dev/..." , O_RDWR); or CreateFile(...)
        if (d_isOpen) return false;
        d_isOpen = true;
        std::cout << "    [DeviceImpl] Opened device: " << d_name << "\n";
        return true;
    }

    void close() {
        if (d_isOpen) {
            // Real code: ::close(d_fd); or CloseHandle(d_handle);
            d_isOpen = false;
            std::cout << "    [DeviceImpl] Closed device: " << d_name << "\n";
        }
    }

    bool write(const std::string& data) {
        if (!d_isOpen) return false;
        // Real code: ::write(d_fd, data.data(), data.size());
        d_bytesWritten += static_cast<int>(data.size());
        std::cout << "    [DeviceImpl] Wrote " << data.size()
                  << " bytes to " << d_name << "\n";
        return true;
    }

    const std::string& name()         const { return d_name; }
    bool               isOpen()       const { return d_isOpen; }
    int                bytesWritten() const { return d_bytesWritten; }
};


// ============================================================
// Device public interface implementation
// (Delegates all work to DeviceImpl via opaque pointer)
// ============================================================

Device::Device(const std::string& name)
: d_impl_p(std::make_unique<DeviceImpl>(name))
{
}

// Destructor MUST be defined here (not in header) because:
// - std::unique_ptr<T>::~unique_ptr() calls T's destructor
// - At the point of destruction, T (DeviceImpl) must be a COMPLETE type
// - DeviceImpl is complete only in this .cpp file
Device::~Device() = default;

Device::Device(Device&&) noexcept = default;
Device& Device::operator=(Device&&) noexcept = default;

bool Device::open()
{
    return d_impl_p->open();
}

void Device::close()
{
    d_impl_p->close();
}

bool Device::write(const std::string& data)
{
    return d_impl_p->write(data);
}

std::string Device::name() const
{
    return d_impl_p->name();
}

bool Device::isOpen() const
{
    return d_impl_p->isOpen();
}

int Device::bytesWritten() const
{
    return d_impl_p->bytesWritten();
}
