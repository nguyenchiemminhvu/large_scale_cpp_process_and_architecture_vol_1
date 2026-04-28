// insulated_widget.h                                                 -*-C++-*-
//
// Demonstrates compile-time INSULATION via PIMPL.
// Book reference: Lakos §3.6, §3.7
//
// The "insulated" version of HeavyWidget exposes only what clients need.
// All implementation details (heavy includes) are hidden in the .cpp.
//
// BENEFIT: Clients that include insulated_widget.h pay ONLY for:
//   - <memory> (for unique_ptr)
//   - <string>
// Not for <unordered_map>, <set>, <chrono>, <map>, etc.

#ifndef INCLUDED_INSULATED_WIDGET
#define INCLUDED_INSULATED_WIDGET

#include <memory>
#include <string>

// Forward declaration of impl — no #include of any heavy header needed!
class InsulatedWidgetImpl;

/// An insulated widget — same interface as HeavyWidget but with
/// compile-time isolation via the PIMPL idiom.
///
/// INSULATION BENEFIT:
///   Changing any implementation detail (e.g., switching from
///   unordered_map to sorted vector) does NOT require recompiling
///   any client that includes this header.
class InsulatedWidget {
    std::unique_ptr<InsulatedWidgetImpl> d_impl_p;

  public:
    explicit InsulatedWidget(const std::string& name);
    ~InsulatedWidget();

    InsulatedWidget(InsulatedWidget&&) noexcept;
    InsulatedWidget& operator=(InsulatedWidget&&) noexcept;

    InsulatedWidget(const InsulatedWidget&) = delete;
    InsulatedWidget& operator=(const InsulatedWidget&) = delete;

    // PUBLIC INTERFACE (minimal, client-oriented)
    std::string name() const;
    void        addData(int value);
    int         dataSize() const;
    void        addTag(const std::string& tag);
    bool        hasTag(const std::string& tag) const;
};

#endif  // INCLUDED_INSULATED_WIDGET
