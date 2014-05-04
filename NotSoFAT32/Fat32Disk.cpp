#include "Disk.hpp"
#include "Fat32Disk.hpp"
#include "Fat32Directory.hpp"
#include "Fat32File.hpp"
#include "Fat32AllocationTable.hpp"
#include "Fat32Common.hpp"

Fat32Disk::Fat32Disk(std::shared_ptr<Disk> disk)
    : m_disk(disk), m_fat(this)
{
    auto buffer = std::make_unique<char[]>(m_disk->getSectorSize());
    m_disk->readSector(0, buffer.get());
    m_bpb = *(Fat32Bpb*)buffer.get();
}

std::shared_ptr<Disk> Fat32Disk::getDisk() const
{
    return m_disk;
}

int Fat32Disk::getClusterSize() const
{
    return m_bpb.sectorsPerCluster * m_bpb.bytesPerSector;
}

int Fat32Disk::getClusterCount() const
{
    return m_bpb.totalSectors / m_bpb.sectorsPerCluster;
}

void Fat32Disk::readCluster(int cluster, char *buffer)
{
    if (cluster >= getClusterCount())
        throw std::exception("Cluster out of range");

    int clusterOffset = m_bpb.reservedSectors + m_bpb.fatSize;
    int sector = clusterOffset + (cluster * m_bpb.sectorsPerCluster);

    for (int i = 0; i < m_bpb.sectorsPerCluster; i++)
    {
        m_disk->readSector(sector++, buffer);
        buffer += m_bpb.bytesPerSector;
    }
}

void Fat32Disk::format(const std::string &volumeLabel, int sectorsPerCluster)
{
    if (volumeLabel.length() > 16)
        throw std::exception("Invalid volume label");

    int bytesPerCluster = m_disk->getSectorSize() * sectorsPerCluster;

    if (bytesPerCluster < 512 || sectorsPerCluster > 32 * 1024)
        throw std::exception("Invalid bytes per cluster");

    m_fat.reset();
    m_bpb = {};

    m_bpb.bytesPerSector = m_disk->getSectorSize();
    m_bpb.totalSectors = m_disk->getSectorCount();
    m_bpb.reservedSectors = 1; // the bpb

    m_bpb.sectorsPerCluster = sectorsPerCluster;

    int totalClusters = m_bpb.totalSectors / m_bpb.sectorsPerCluster;
    m_bpb.fatSize = (totalClusters * sizeof(int)) / m_bpb.bytesPerSector; // TODO: fatSize is too large with this method as it includes the fat in the calculation

    m_bpb.rootCluster = 0;

    std::copy(volumeLabel.begin(), volumeLabel.end(), m_bpb.label);

    auto buffer = std::make_unique<char[]>(m_bpb.bytesPerSector);
    std::memcpy(buffer.get(), &m_bpb, sizeof(Fat32Bpb));
    m_disk->writeSector(0, buffer.get());

    int fatEntriesPerSector = m_bpb.bytesPerSector / sizeof(int);
    auto fatBuffer = std::make_unique<int[]>(fatEntriesPerSector);

    for (int i = 0; i < fatEntriesPerSector; i++)
        fatBuffer[i] = FatFree;

    for (int i = 0; i < m_bpb.fatSize; i++)
    {
        if (i == 0)
            fatBuffer[0] = 0;

        m_disk->writeSector(m_bpb.reservedSectors + i, fatBuffer.get());

        if (i == 0)
            fatBuffer[0] = FatFree;
    }
}
