#ifndef __FAT32ALLOCATIONTABLE_HPP
#define __FAT32ALLOCATIONTABLE_HPP

#include <memory>
#include "Fat32Common.hpp"

class Fat32Disk;

class Fat32AllocationTable
{

public:

    Fat32AllocationTable(Fat32Disk* fat32);
    ~Fat32AllocationTable();

    fatcluster_t read(fatcluster_t index);
    void write(fatcluster_t index, fatcluster_t value);
    fatcluster_t alloc();
    void free(fatcluster_t index);
    void reset();

private:

    Fat32Disk *m_fat32;

    bool m_cacheDirty;
    size_t m_cachedSector;
    std::unique_ptr<fatcluster_t[]> m_cache;
    size_t m_entryCount;

    fatcluster_t findFree(fatcluster_t startCluster);
    void flush();
    size_t getFatSector(fatcluster_t index);
    size_t getFatSectorOffset(fatcluster_t index);
};

#endif
