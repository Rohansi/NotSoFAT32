#include "Disk.hpp"
#include "Fat32AllocationTable.hpp"
#include "Fat32Disk.hpp"

Fat32AllocationTable::Fat32AllocationTable(Fat32Disk *fat32)
    : m_fat32(fat32)
{
    m_cacheDirty = false;
    m_cachedSector = -1;
    m_cache = std::make_unique<int[]>(fat32->getDisk()->getSectorSize() / sizeof(int));
}

Fat32AllocationTable::~Fat32AllocationTable()
{
    flush();
}

int Fat32AllocationTable::read(int index)
{
    int sector = getFatSector(index);
    int offset = getFatSectorOffset(index);
    auto disk = m_fat32->getDisk();

    if (sector != m_cachedSector)
    {
        flush();
        disk->readSector(sector, m_cache.get());
        m_cachedSector = sector;
    }

    return m_cache[offset];
}

void Fat32AllocationTable::write(int index, int value)
{
    int sector = getFatSector(index);
    int offset = getFatSectorOffset(index);
    auto disk = m_fat32->getDisk();

    if (sector != m_cachedSector)
    {
        flush();
        disk->readSector(sector, m_cache.get());
        m_cachedSector = sector;
    }

    m_cache[offset] = value;
    m_cacheDirty = true;
}

int Fat32AllocationTable::alloc()
{
    int firstFree = findFree(0);
    int index = findFree(firstFree);

    write(index, FatUnassign);

    return index;
}

void Fat32AllocationTable::free(int index)
{
    write(index, FatFree);
}

void Fat32AllocationTable::reset()
{
    flush();
    m_cachedSector = FatFree;
}

int Fat32AllocationTable::findFree(int startCluster)
{
    int lastCluster = m_fat32->getClusterCount();
    int cluster = startCluster;

    while (true)
    {
        if (cluster >= lastCluster)
            throw std::exception("No free clusters");

        int value = read(cluster);
        if (value == FatFree)
            return cluster;

        cluster++;
    }
}

void Fat32AllocationTable::flush()
{
    if (!m_cacheDirty)
        return;

    m_fat32->getDisk()->writeSector(m_cachedSector, m_cache.get());
    m_cacheDirty = false;
}

int Fat32AllocationTable::getFatSector(int index)
{
    int fatOffset = m_fat32->m_bpb.reservedSectors;
    return fatOffset + (index / (m_fat32->m_bpb.bytesPerSector / sizeof(int)));
}

int Fat32AllocationTable::getFatSectorOffset(int index)
{
    return index % (m_fat32->m_bpb.bytesPerSector / sizeof(int));
}
