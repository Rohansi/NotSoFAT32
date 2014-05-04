#include "Fat32File.hpp"
#include "Disk.hpp"
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Fat32AllocationTable.hpp"

Fat32File::Fat32File(std::shared_ptr<Fat32Disk> fat32, Fat32DirectoryEntry entry)
    : m_fat32(fat32)
{
    m_firstCluster = entry.firstCluster;
    m_clusterSize = fat32->getClusterSize();
    m_size = entry.size;

    m_buffer = std::make_unique<char[]>(m_clusterSize);
    m_cluster = FatFree;
    m_clusterPosition = 0;
    m_clusterOffset = 0;

    m_position = 0;
    m_eof = m_firstCluster == FatEof;
}

Fat32File::Fat32File(Fat32File &&other)
{
    m_fat32 = std::move(other.m_fat32);

    m_firstCluster = other.m_firstCluster;
    m_clusterSize = other.m_clusterSize;
    m_size = other.m_size;

    m_buffer = std::move(other.m_buffer);
    m_cluster = other.m_cluster;
    m_clusterPosition = other.m_clusterPosition;
    m_clusterOffset = other.m_clusterOffset;

    m_position = other.m_position;
    m_eof = other.m_eof;
}

Fat32File::~Fat32File()
{
    flush();
}

void Fat32File::flush()
{
    
}

void Fat32File::seek(int position)
{
    if (position < 0)
        throw std::exception("Seek to negative");

    m_position = position;
}

int Fat32File::read(char *buffer, int count)
{
    if (m_eof)
        return 0;

    checkSeekToPosition();

    int bytesRead = 0;
    while (bytesRead < count)
    {
        if (m_position >= m_size || !checkNextCluster())
        {
            m_eof = true;
            return bytesRead;
        }

        *buffer++ = m_buffer[m_clusterOffset++];
        m_position++;
        bytesRead++;
    }

    return bytesRead;
}

bool Fat32File::eof() const
{
    return m_eof;
}

// switches to the cluster at m_position if needed
// returns false if eof && !create
bool Fat32File::checkSeekToPosition(bool create)
{
    int clusterPositionOffset = m_clusterPosition + m_clusterOffset;
    if (m_position == clusterPositionOffset)
        return true;

    int positionIndex = m_position / m_clusterSize;
    int clusterIndex = clusterPositionOffset / m_clusterSize;

    if (m_position >= 0 && m_position < m_clusterSize) // moved to first cluster
    {
        if (m_firstCluster == FatEof)
        {
            if (!create)
                return false;

            // TODO
            throw std::exception("allocate first cluster");
        }

        m_cluster = m_firstCluster;
        m_clusterPosition = 0;
        m_clusterOffset = m_position;
    }
    else
    {
        if (m_position < clusterPositionOffset) // moved before current cluster, need to start over
        {
            if (m_firstCluster == FatEof)
            {
                if (!create)
                    return false;

                // TODO
                throw std::exception("allocate first cluster");
            }

            m_cluster = m_firstCluster;
            m_clusterPosition = 0;
        }

        int targetClusterPosition = positionIndex * m_clusterSize;

        while (m_clusterPosition != targetClusterPosition)
        {
            m_clusterOffset = m_clusterSize;
            checkNextCluster(create, false);
        }

        m_clusterOffset = m_position % m_clusterSize;
    }

    m_fat32->readCluster(m_cluster, m_buffer.get());

    return true;
}

// switches to the next cluster if needed
// returns false if eof && !create
bool Fat32File::checkNextCluster(bool create, bool read)
{
    if (m_clusterOffset < m_clusterSize)
        return;

    int currentCluster = m_cluster;

    if (m_cluster < 0)
        m_cluster = m_firstCluster;
    else
        m_cluster = m_fat32->m_fat.read(m_cluster);

    if (m_cluster == FatBad)
        throw std::exception("Bad cluster in chain");

    if (m_cluster == FatUnassign)
        throw std::exception("Unassigned cluster in chain");

    if (m_cluster == FatFree)
        throw std::exception("Free cluster in chain");

    if (m_cluster == FatEof)
    {
        if (!create)
            return false;
            
        // TODO
        throw std::exception("allocate cluster");
    }

    m_clusterPosition += m_clusterSize;
    m_clusterOffset = 0;

    if (read)
        m_fat32->readCluster(m_cluster, m_buffer.get());

    return true;
}
