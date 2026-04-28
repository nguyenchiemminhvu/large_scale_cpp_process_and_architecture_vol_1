// memory_persistence.cpp
#ifndef INCLUDED_MEMORY_PERSISTENCE
#include <memory_persistence.h>
#endif

bool MemoryPersistence::save(const std::string& key, const std::string& value)
{
    d_store[key] = value;
    return true;
}

bool MemoryPersistence::load(const std::string& key, std::string* valueOut)
{
    auto it = d_store.find(key);
    if (it == d_store.end()) return false;
    if (valueOut) *valueOut = it->second;
    return true;
}

bool MemoryPersistence::remove(const std::string& key)
{
    return d_store.erase(key) > 0;
}

bool MemoryPersistence::exists(const std::string& key)
{
    return d_store.count(key) > 0;
}

int MemoryPersistence::count()
{
    return static_cast<int>(d_store.size());
}
