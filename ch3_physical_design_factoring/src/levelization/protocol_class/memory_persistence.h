// memory_persistence.h
#ifndef INCLUDED_MEMORY_PERSISTENCE
#define INCLUDED_MEMORY_PERSISTENCE

#ifndef INCLUDED_PERSISTENCE_ADAPTER
#include <persistence_adapter.h>
#endif

#include <unordered_map>
#include <string>

/// In-memory implementation of PersistenceAdapter.
/// Used for testing and development without real storage.
class MemoryPersistence : public PersistenceAdapter {
    std::unordered_map<std::string, std::string> d_store;

  public:
    bool save(const std::string& key, const std::string& value) override;
    bool load(const std::string& key, std::string* valueOut) override;
    bool remove(const std::string& key) override;
    bool exists(const std::string& key) override;
    int  count() override;
};

#endif  // INCLUDED_MEMORY_PERSISTENCE
