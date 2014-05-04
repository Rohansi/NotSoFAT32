#include "Fat32File.hpp"
#include "Disk.hpp"
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Fat32AllocationTable.hpp"

Fat32File::Fat32File(std::shared_ptr<Fat32Disk> fat32, Fat32DirectoryEntry entry)
    : m_fat32(fat32)
{
    m_firstCluster = entry.firstCluster;
    m_currentCluster = FatFree;
    m_clusterSize = fat32->getClusterSize();
    m_clusterOffset = 0;

    m_eof = m_firstCluster >= FatEof;
    m_size = entry.size;

    m_cluster = std::make_unique<char[]>(m_clusterSize);
}

Fat32File::Fat32File(Fat32File &&other)
{
    m_fat32 = std::move(other.m_fat32);

    m_firstCluster = other.m_firstCluster;
    m_currentCluster = other.m_currentCluster;
    m_clusterSize = other.m_clusterSize;
    m_clusterOffset = other.m_clusterOffset;

    m_eof = other.m_eof;
    m_size = other.m_size;

    m_position = other.m_position;
    m_cluster = std::move(other.m_cluster);
}

Fat32File::~Fat32File()
{
    // flush n stuff
}

int Fat32File::read(char *buffer, int count)
{
    if (m_eof)
        return 0;

    int bytesRead = 0;
    while (bytesRead < count)
    {
        if (m_position >= m_size)
        {
            m_eof = true;
            return bytesRead;
        }

        if (m_clusterOffset >= m_clusterSize)
        {
            if (m_currentCluster < 0)
                m_currentCluster = m_firstCluster;
            else
                m_currentCluster = m_fat32->m_fat.read(m_currentCluster);

            if (m_currentCluster == FatBad)
                throw std::exception("Bad cluster in chain");

            if (m_currentCluster == FatUnassign)
                throw std::exception("Unassigned cluster in chain");

            if (m_currentCluster == FatFree)
                throw std::exception("Free cluster in chain");

            if (m_currentCluster == FatEof)
            {
                m_eof = true;
                return bytesRead;
            }

            m_clusterOffset = 0;
            m_fat32->readCluster(m_currentCluster, m_cluster.get());
        }

        *buffer++ = m_cluster[m_clusterOffset++];
        m_position++;
        bytesRead++;
    }

    return bytesRead;
}

bool Fat32File::eof() const
{
    return m_eof;
}
