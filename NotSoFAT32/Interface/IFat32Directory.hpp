#ifndef __INTERFACE_FAT32DIRECTORY_HPP
#define __INTERFACE_FAT32DIRECTORY_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "../Fat32Common.hpp"
#include "../DirectoryEntry.hpp"

class Fat32Disk;
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

    void parse(std::shared_ptr<Fat32Disk> fat32, int firstCluster);

private:

    std::shared_ptr<Fat32Disk> m_fat32;
    std::unordered_map<std::string, DirectoryEntry> m_entries;

};

#endif
