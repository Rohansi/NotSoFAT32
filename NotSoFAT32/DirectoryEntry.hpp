#ifndef __DIRECTORYENTRY_HPP
#define __DIRECTORYENTRY_HPP

#include <string>
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Fat32File.hpp"

class DirectoryEntry
{
    friend class IFat32Directory;
    friend class Fat32File;

public:

    DirectoryEntry(const DirectoryEntry &other);

    const std::string& getName() const;
    char getAttributes() const;
    size_t getSize() const;

private:

    DirectoryEntry(std::shared_ptr<Fat32Disk> fat32, Fat32DirectoryEntry entry, int parentFirstCluster, int parentPosition);

    void save() const;

    std::shared_ptr<Fat32Disk> m_fat32;
    Fat32DirectoryEntry m_entry;
    int m_parentFirstCluster;
    int m_parentPosition;
    std::string m_name;

};

#endif
