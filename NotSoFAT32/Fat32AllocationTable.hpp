#ifndef __FAT32ALLOCATIONTABLE_HPP
#define __FAT32ALLOCATIONTABLE_HPP

#include <memory>

class Fat32Disk;

class Fat32AllocationTable
{

public:

    Fat32AllocationTable(Fat32Disk*);
    ~Fat32AllocationTable();

    int read(int index);
    void write(int index, int value);
    int alloc();
    void free(int index);
    void reset();

private:

    Fat32Disk *m_fat32;

    bool m_cacheDirty;
    int m_cachedSector;
    std::unique_ptr<int[]> m_cache;

    int findFree(int startCluster);
    void flush();
    int getFatSector(int index);
    int getFatSectorOffset(int index);
};

#endif
