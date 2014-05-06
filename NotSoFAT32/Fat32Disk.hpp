#ifndef __FAT32DISK_HPP
#define __FAT32DISK_HPP

#include "Disk.hpp"
#include "Fat32Common.hpp"
#include "Fat32AllocationTable.hpp"

class Fat32File;
class Disk;

class Fat32Disk
{
    friend class Fat32AllocationTable;
    friend class IFat32Directory;
    friend class Fat32File;
    friend class Fat32;

public:

    Fat32Disk::Fat32Disk(std::shared_ptr<Disk> disk);

    std::shared_ptr<Disk> getDisk() const;

    int getClusterSize() const;
    int getClusterCount() const;
    void readCluster(int cluster, char *buffer);
    void writeCluster(int cluster, char *buffer);

    void format(const std::string &volumeLabel, int sectorsPerCluster = 1);

private:

    std::shared_ptr<Disk> m_disk;
    Fat32Bpb m_bpb;
    Fat32AllocationTable m_fat;

};

#endif
