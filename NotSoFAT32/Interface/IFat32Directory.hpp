#ifndef __INTERFACE_FAT32DIRECTORY_HPP
#define __INTERFACE_FAT32DIRECTORY_HPP

#include "../Fat32Common.hpp"

#include <memory>
#include <string>
#include <unordered_map>
#include <algorithm>

class Fat32Disk;

struct DirectoryEntry
{
    std::string name;
    size_t size;
    int attributes;
    time_t creationTime;
    time_t lastAccessTime;
    time_t lastWriteTime;

    Fat32DirectoryEntry entry;
};

class Fat32Directory;
class Fat32File;

class IFat32Directory
{

public:

    IFat32Directory();
    IFat32Directory(IFat32Directory &&other);

    Fat32Directory directory(const std::string &name);
    Fat32File file(const std::string &name);
    bool exists(const std::string &name);

protected:

    void parse(std::shared_ptr<Fat32Disk> fat32, Fat32File &file);

private:

    std::shared_ptr<Fat32Disk> m_fat32;
    std::unordered_map<std::string, DirectoryEntry> m_entries;
};

#endif
