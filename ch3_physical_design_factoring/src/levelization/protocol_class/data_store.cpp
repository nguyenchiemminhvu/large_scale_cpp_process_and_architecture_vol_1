// data_store.cpp
#ifndef INCLUDED_DATA_STORE
#include <data_store.h>
#endif

DataStore::DataStore(PersistenceAdapter& adapter)
: d_adapter(adapter)
{
}

bool DataStore::storeSession(const std::string& userId, const std::string& token)
{
    return d_adapter.save("session:" + userId, token);
}

std::string DataStore::getSession(const std::string& userId)
{
    std::string token;
    d_adapter.load("session:" + userId, &token);
    return token;
}

bool DataStore::invalidateSession(const std::string& userId)
{
    return d_adapter.remove("session:" + userId);
}

bool DataStore::hasSession(const std::string& userId)
{
    return d_adapter.exists("session:" + userId);
}

int DataStore::activeSessionCount()
{
    return d_adapter.count();
}
