// file_persistence.cpp
#ifndef INCLUDED_FILE_PERSISTENCE
#include <file_persistence.h>
#endif

#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>

namespace {
// Internal helper: load the entire file into a map
std::unordered_map<std::string, std::string> loadFile(const std::string& filename)
{
    std::unordered_map<std::string, std::string> data;
    std::ifstream f(filename);
    if (!f.is_open()) return data;

    std::string line;
    while (std::getline(f, line)) {
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            data[line.substr(0, pos)] = line.substr(pos + 1);
        }
    }
    return data;
}

// Internal helper: save a map to file
bool saveFile(const std::string& filename,
              const std::unordered_map<std::string, std::string>& data)
{
    std::ofstream f(filename);
    if (!f.is_open()) return false;
    for (const auto& [k, v] : data) {
        f << k << "=" << v << "\n";
    }
    return true;
}
}  // close anonymous namespace

FilePersistence::FilePersistence(const std::string& filename)
: d_filename(filename)
{
}

bool FilePersistence::save(const std::string& key, const std::string& value)
{
    auto data = loadFile(d_filename);
    data[key] = value;
    return saveFile(d_filename, data);
}

bool FilePersistence::load(const std::string& key, std::string* valueOut)
{
    auto data = loadFile(d_filename);
    auto it = data.find(key);
    if (it == data.end()) return false;
    if (valueOut) *valueOut = it->second;
    return true;
}

bool FilePersistence::remove(const std::string& key)
{
    auto data = loadFile(d_filename);
    if (!data.erase(key)) return false;
    return saveFile(d_filename, data);
}

bool FilePersistence::exists(const std::string& key)
{
    auto data = loadFile(d_filename);
    return data.count(key) > 0;
}

int FilePersistence::count()
{
    return static_cast<int>(loadFile(d_filename).size());
}
