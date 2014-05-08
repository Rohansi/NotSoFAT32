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

size_t Fat32Disk::getClusterSize() const
{
    return m_bpb.sectorsPerCluster * m_bpb.bytesPerSector;
}

size_t Fat32Disk::getClusterCount() const
{
    return m_bpb.totalSectors / m_bpb.sectorsPerCluster;
}

void Fat32Disk::readCluster(fatcluster_t cluster, char *buffer)
{
    if (cluster < 0 || cluster >= getClusterCount())
        throw std::exception("Cluster out of range");

    int clusterOffset = m_bpb.reservedSectors + m_bpb.fatSize;
    int sector = clusterOffset + (cluster * m_bpb.sectorsPerCluster);

    for (int i = 0; i < m_bpb.sectorsPerCluster; i++)
    {
        m_disk->readSector(sector++, buffer);
        buffer += m_bpb.bytesPerSector;
    }
}

void Fat32Disk::writeCluster(fatcluster_t cluster, char *buffer)
{
    if (cluster < 0 || cluster >= getClusterCount())
        throw std::exception("Cluster out of range");

    int clusterOffset = m_bpb.reservedSectors + m_bpb.fatSize;
    int sector = clusterOffset + (cluster * m_bpb.sectorsPerCluster);

    for (int i = 0; i < m_bpb.sectorsPerCluster; i++)
    {
        m_disk->writeSector(sector++, buffer);
        buffer += m_bpb.bytesPerSector;
    }
}

std::shared_ptr<Fat32Root> Fat32Disk::root()
{
    if (!m_root)
    {
        m_root = std::make_shared<Fat32Root>(shared_from_this());
    }

    return m_root;
}

void Fat32Disk::format(const std::string &volumeLabel, size_t sectorsPerCluster)
{
    if (volumeLabel.length() > 16)
        throw std::exception("Invalid volume label");

    auto bytesPerCluster = m_disk->getSectorSize() * sectorsPerCluster;

    if (bytesPerCluster < 512 || sectorsPerCluster > 32 * 1024)
        throw std::exception("Invalid bytes per cluster");

    m_fat.reset();
    m_bpb = {};

    const std::string nsfat32 = "NSFAT32";

    std::copy(nsfat32.begin(), nsfat32.end(), m_bpb.fsysName);
    m_bpb.fsysVersion = 0;

    m_bpb.bytesPerSector = m_disk->getSectorSize();
    m_bpb.totalSectors = m_disk->getSectorCount();
    m_bpb.reservedSectors = 1; // the bpb

    m_bpb.sectorsPerCluster = sectorsPerCluster;

    auto totalClusters = m_bpb.totalSectors / m_bpb.sectorsPerCluster;
    m_bpb.fatSize = (totalClusters * sizeof(fatcluster_t)) / m_bpb.bytesPerSector; // TODO: fatSize is too large with this method as it includes the fat in the calculation

    m_bpb.rootCluster = 0;

    std::copy(volumeLabel.begin(), volumeLabel.end(), m_bpb.label);

    auto buffer = std::make_unique<char[]>(m_bpb.bytesPerSector);
    std::memcpy(buffer.get(), &m_bpb, sizeof(Fat32Bpb));
    m_disk->writeSector(0, buffer.get());

    auto fatEntriesPerSector = m_bpb.bytesPerSector / sizeof(fatcluster_t);
    auto fatBuffer = std::make_unique<fatcluster_t[]>(fatEntriesPerSector);

    for (size_t i = 0; i < fatEntriesPerSector; i++)
        fatBuffer[i] = FatFree;

    for (size_t i = 0; i < m_bpb.fatSize; i++)
    {
        if (i == 0)
            fatBuffer[0] = 0;

        m_disk->writeSector(m_bpb.reservedSectors + i, fatBuffer.get());

        if (i == 0)
            fatBuffer[0] = FatFree;
    }
}

std::shared_ptr<IFat32Directory> Fat32Disk::getOrAddDirectory(size_t firstCluster, std::function<IFat32Directory()> ctor)
{
    auto item = m_directories.find(firstCluster);
    std::shared_ptr<IFat32Directory> result;

    if (item == m_directories.end())
    {
        result = std::make_shared<IFat32Directory>(ctor());
        m_directories.insert(std::make_pair(firstCluster, result));
    }
    else
    {
        if (result = item->second.lock())
        {
            return result;
        }
        else
        {
            result = std::make_shared<IFat32Directory>(ctor());
            item->second = result;
        }
    }

    return result;
}
