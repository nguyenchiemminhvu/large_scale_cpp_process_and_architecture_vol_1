// persistence_adapter.h                                              -*-C++-*-
//
// Demonstrates the PROTOCOL CLASS levelization technique.
// Book reference: Lakos §3.5.7 (Protocol Callbacks), §3.8 (Avoiding
// Inappropriate Link-Time Dependencies)
//
// CONCEPT:
//   Define an abstract interface (protocol class) that decouples the
//   logical contract from any specific implementation. The library
//   component depends only on the abstract interface.
//
//   This prevents "betting on a single technology" — e.g., hardcoding
//   a dependency on SQLite, Redis, or a specific OS API.

#ifndef INCLUDED_PERSISTENCE_ADAPTER
#define INCLUDED_PERSISTENCE_ADAPTER

#include <string>

/// Abstract interface for key-value persistence.
///
/// A "protocol class" in Lakos terminology:
///   - Pure abstract (all methods virtual and = 0)
///   - Virtual destructor
///   - No data members
///   - No non-virtual methods
///
/// Components that need persistence depend on THIS interface,
/// not on any concrete implementation.
class PersistenceAdapter {
  public:
    virtual ~PersistenceAdapter() = default;

    virtual bool save(const std::string& key, const std::string& value) = 0;
    virtual bool load(const std::string& key, std::string* valueOut) = 0;
    virtual bool remove(const std::string& key) = 0;
    virtual bool exists(const std::string& key) = 0;
    virtual int  count() = 0;
};

#endif  // INCLUDED_PERSISTENCE_ADAPTER
