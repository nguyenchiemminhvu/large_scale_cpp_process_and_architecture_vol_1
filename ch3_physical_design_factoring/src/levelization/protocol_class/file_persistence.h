// file_persistence.h
#ifndef INCLUDED_FILE_PERSISTENCE
#define INCLUDED_FILE_PERSISTENCE

#ifndef INCLUDED_PERSISTENCE_ADAPTER
#include <persistence_adapter.h>
#endif

#include <string>

/// File-based implementation of PersistenceAdapter.
/// Stores key-value pairs in a simple text file.
class FilePersistence : public PersistenceAdapter {
    std::string d_filename;

  public:
    explicit FilePersistence(const std::string& filename);

    bool save(const std::string& key, const std::string& value) override;
    bool load(const std::string& key, std::string* valueOut) override;
    bool remove(const std::string& key) override;
    bool exists(const std::string& key) override;
    int  count() override;
};

#endif  // INCLUDED_FILE_PERSISTENCE
