#include "Disk.hpp"
#include "Fat32AllocationTable.hpp"
#include "Fat32Disk.hpp"

Fat32AllocationTable::Fat32AllocationTable(Fat32Disk *fat32)
    : m_fat32(fat32)
{
    m_cacheDirty = false;
    m_cachedSector = -1;

    auto entriesPerSector = fat32->getDisk()->getSectorSize() / sizeof(fatcluster_t);
    m_cache = std::make_unique<fatcluster_t[]>(entriesPerSector);
    m_entryCount = m_fat32->m_bpb.fatSize * entriesPerSector;
}

Fat32AllocationTable::~Fat32AllocationTable()
{
    flush();
}

fatcluster_t Fat32AllocationTable::read(fatcluster_t index)
{
    if (index >= m_entryCount)
        throw std::exception("FAT index out of range");

    auto sector = getFatSector(index);
    auto offset = getFatSectorOffset(index);
    auto disk = m_fat32->getDisk();

    if (sector != m_cachedSector)
    {
        flush();
        disk->readSector(sector, m_cache.get());
        m_cachedSector = sector;
    }

    return m_cache[offset];
}

void Fat32AllocationTable::write(fatcluster_t index, fatcluster_t value)
{
    if (index >= m_entryCount)
        throw std::exception("FAT index out of range");

    auto sector = getFatSector(index);
    auto offset = getFatSectorOffset(index);
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

fatcluster_t Fat32AllocationTable::alloc()
{
    auto index = findFree(0);
    write(index, FatUnassign);
    return index;
}

void Fat32AllocationTable::free(fatcluster_t index)
{
    write(index, FatFree);
}

void Fat32AllocationTable::reset()
{
    flush();
    m_cachedSector = FatFree;
}

fatcluster_t Fat32AllocationTable::findFree(fatcluster_t startCluster)
{
    auto lastCluster = m_fat32->getClusterCount();
    auto cluster = startCluster;

    while (true)
    {
        if (cluster >= lastCluster)
            throw std::exception("No free clusters");

        auto value = read(cluster);
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

size_t Fat32AllocationTable::getFatSector(fatcluster_t index)
{
    auto fatOffset = m_fat32->m_bpb.reservedSectors;
    return fatOffset + (index / (m_fat32->m_bpb.bytesPerSector / sizeof(fatcluster_t)));
}

size_t Fat32AllocationTable::getFatSectorOffset(fatcluster_t index)
{
    return index % (m_fat32->m_bpb.bytesPerSector / sizeof(fatcluster_t));
}
