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

    IFat32Directory(std::shared_ptr<Fat32Disk> fat32, int firstCluster);
    IFat32Directory(IFat32Directory &&other);

    std::vector<DirectoryEntry> entries();

    Fat32Directory directory(const std::string &name);
    Fat32File file(const std::string &name);

    bool add(const std::string &name, int attributes);
    bool remove(const std::string &name);
    bool exists(const std::string &name);

    static bool isValidName(const std::string &name);

protected:

    void parse();

private:

    std::shared_ptr<Fat32Disk> m_fat32;
    int m_firstCluster;

    std::unordered_map<std::string, DirectoryEntry> m_entries;

};

#endif
