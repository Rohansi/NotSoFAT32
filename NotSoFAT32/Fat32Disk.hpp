#ifndef __FAT32DISK_HPP
#define __FAT32DISK_HPP

#include <functional>
#include <unordered_map>
#include <unordered_set>
#include "Disk.hpp"
#include "Fat32Common.hpp"
#include "Fat32AllocationTable.hpp"
#include "Fat32Root.hpp"
#include "DirectoryEntry.hpp"

class DirectoryEntry;
class Fat32File;
class Disk;

class Fat32Disk : public std::enable_shared_from_this<Fat32Disk>
{
    friend class Fat32AllocationTable;
    friend class IFat32Directory;
    friend class Fat32File;
    friend class Fat32Root;

public:

    Fat32Disk::Fat32Disk(std::shared_ptr<Disk> disk);

    std::shared_ptr<Disk> getDisk() const;

    int getClusterSize() const;
    int getClusterCount() const;
    void readCluster(int cluster, char *buffer);
    void writeCluster(int cluster, char *buffer);

    std::shared_ptr<Fat32Root> root();

    void format(const std::string &volumeLabel, int sectorsPerCluster = 1);

private:

    std::shared_ptr<Disk> m_disk;
    Fat32Bpb m_bpb;
    Fat32AllocationTable m_fat;
    std::shared_ptr<Fat32Root> m_root;

    std::unordered_map<int, std::weak_ptr<IFat32Directory>> m_directories;
    std::shared_ptr<IFat32Directory> getOrAddDirectory(int firstCluster, std::function<IFat32Directory()> ctor);

};

#endif
