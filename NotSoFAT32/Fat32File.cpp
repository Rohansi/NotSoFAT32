#include "Fat32File.hpp"
#include "Disk.hpp"
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Fat32AllocationTable.hpp"

Fat32File::Fat32File(std::shared_ptr<Fat32Disk> fat32, std::shared_ptr<DirectoryEntry> entry)
    : m_fat32(fat32), m_entry(entry)
{
    m_entryDirty = false;

    if (m_entry)
    {
        m_firstCluster = entry->m_entry.firstCluster;
        m_size = entry->getSize();
    }

    m_clusterSize = fat32->getClusterSize();
    m_originalSize = m_size;

    m_buffer = std::make_unique<char[]>(m_clusterSize);
    m_cluster = FatEof;
    m_clusterPosition = -1;
    m_clusterOffset = -1;
    m_clusterDirty = false;

    m_position = 0;
}

Fat32File::Fat32File(std::shared_ptr<Fat32Disk> fat32, int firstCluster)
    : Fat32File(fat32, std::shared_ptr<DirectoryEntry>())
{
    m_firstCluster = firstCluster;
    m_size = std::numeric_limits<int>::max();
}

Fat32File::Fat32File(Fat32File &&other)
{
    m_fat32 = std::move(other.m_fat32);

    m_entry = std::move(other.m_entry);
    m_entryDirty = other.m_entryDirty;

    m_firstCluster = other.m_firstCluster;
    m_clusterSize = other.m_clusterSize;
    m_size = other.m_size;
    m_originalSize = other.m_originalSize;

    m_buffer = std::move(other.m_buffer);
    m_cluster = other.m_cluster;
    m_clusterPosition = other.m_clusterPosition;
    m_clusterOffset = other.m_clusterOffset;
    m_clusterDirty = other.m_clusterDirty;

    m_position = other.m_position;
}

Fat32File::~Fat32File()
{
    flush();

    if (m_entry && m_entryDirty)
    {
        m_entry->m_entry.size = m_size;
        m_entry->m_entry.firstCluster = m_firstCluster;
        m_entry->save();
    }
}

void Fat32File::flush()
{
    if (!m_clusterDirty)
        return;

    m_fat32->writeCluster(m_cluster, m_buffer.get());
    m_clusterDirty = false;
}

void Fat32File::seek(int position)
{
    if (position < 0)
        throw std::exception("Seek to negative");

    m_position = position;
}

int Fat32File::tell() const
{
    return m_position;
}

int Fat32File::read(char *buffer, int count)
{
    if (eof())
        return 0;

    if (!checkSeekToPosition())
        return 0;

    int bytesRead = 0;
    while (bytesRead < count)
    {
        if (m_position >= m_size || !checkNextCluster())
            return bytesRead;

        *buffer++ = m_buffer[m_clusterOffset++];
        m_position++;
        bytesRead++;
    }

    return bytesRead;
}

void Fat32File::write(const char *buffer, int count)
{
    checkSeekToPosition(true);

    int bytesWritten = 0;
    while (bytesWritten < count)
    {
        checkNextCluster(true);

        m_buffer[m_clusterOffset++] = *buffer++;
        m_clusterDirty = true;
        m_position++;
        bytesWritten++;
    }

    if (m_position >= m_size)
    {
        m_size = m_position;
        m_entryDirty = true;
    }
}

bool Fat32File::eof() const
{
    return m_position >= m_size;
}

// switches to the cluster at m_position if needed
// returns false if eof && !alloc
bool Fat32File::checkSeekToPosition(bool alloc)
{
    int clusterPositionOffset = m_clusterPosition + m_clusterOffset;
    if (m_position == clusterPositionOffset)
        return true;

    flush();

    int positionIndex = m_position / m_clusterSize;
    int clusterIndex = clusterPositionOffset / m_clusterSize;

    if (m_position >= 0 && m_position < m_clusterSize) // moved to first cluster
    {
        if (!checkHasCluster(alloc))
            return false;

        m_cluster = m_firstCluster;
        m_clusterPosition = 0;
        m_clusterOffset = m_position;
    }
    else
    {
        if (m_position < clusterPositionOffset) // moved before current cluster, need to start over
        {
            if (!checkHasCluster(alloc))
                return false;

            m_cluster = m_firstCluster;
            m_clusterPosition = 0;
        }

        int targetClusterPosition = positionIndex * m_clusterSize;

        while (m_clusterPosition != targetClusterPosition)
        {
            m_clusterOffset = m_clusterSize;
            checkNextCluster(alloc, false);
        }

        m_clusterOffset = m_position % m_clusterSize;
    }

    m_fat32->readCluster(m_cluster, m_buffer.get());

    return true;
}

// switches to the next cluster if needed
// returns false if eof && !alloc
bool Fat32File::checkNextCluster(bool alloc, bool read)
{
    if (m_clusterOffset < m_clusterSize)
        return true;

    flush();

    int currentCluster = m_cluster;

    if (m_firstCluster < 0)
        throw std::exception("First cluster is invalid in checkNextCluster");

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
        if (!alloc)
            return false;
            
        auto &fat = m_fat32->m_fat;

        int nextCluster = fat.alloc();
        fat.write(currentCluster, nextCluster);
        fat.write(nextCluster, FatEof);

        m_cluster = nextCluster;
    }

    m_clusterPosition += m_clusterSize;
    m_clusterOffset = 0;

    if (read)
        m_fat32->readCluster(m_cluster, m_buffer.get());

    return true;
}

// returns false if eof && !alloc
bool Fat32File::checkHasCluster(bool alloc)
{
    if (m_firstCluster != FatEof)
        return true;

    if (!alloc)
        return false;

    if (!m_entry)
        throw std::exception("Tried to allocate on an empty entry-less file");

    m_firstCluster = m_fat32->m_fat.alloc();
    m_fat32->m_fat.write(m_firstCluster, FatEof);

    m_entryDirty = true;

    return true;
}
