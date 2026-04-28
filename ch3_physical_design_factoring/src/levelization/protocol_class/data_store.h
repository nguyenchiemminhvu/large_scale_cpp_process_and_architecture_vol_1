// data_store.h                                                       -*-C++-*-
//
// A data store that depends ONLY on PersistenceAdapter (abstract).
// It never sees MemoryPersistence or FilePersistence.

#ifndef INCLUDED_DATA_STORE
#define INCLUDED_DATA_STORE

#ifndef INCLUDED_PERSISTENCE_ADAPTER
#include <persistence_adapter.h>
#endif

#include <string>
#include <vector>

/// A high-level data store that depends only on the abstract
/// PersistenceAdapter interface.
///
/// DEPENDENCY STRUCTURE:
///   DataStore → PersistenceAdapter  (abstract only!)
///
///   MemoryPersistence → PersistenceAdapter
///   FilePersistence   → PersistenceAdapter
///
///   Application → DataStore + MemoryPersistence (or FilePersistence)
///
/// DataStore can be tested using MemoryPersistence without any real I/O.
/// In production, swap in FilePersistence (or SqlitePersistence, etc.)
class DataStore {
    PersistenceAdapter& d_adapter;  // reference to abstract interface

  public:
    explicit DataStore(PersistenceAdapter& adapter);

    // BUSINESS LOGIC (uses abstract persistence)

    /// Stores a user session.
    bool storeSession(const std::string& userId, const std::string& token);

    /// Retrieves a session token for a user.
    std::string getSession(const std::string& userId);

    /// Invalidates a user's session.
    bool invalidateSession(const std::string& userId);

    /// Returns whether a user has an active session.
    bool hasSession(const std::string& userId);

    /// Returns all session keys (for admin use).
    int activeSessionCount();
};

#endif  // INCLUDED_DATA_STORE
