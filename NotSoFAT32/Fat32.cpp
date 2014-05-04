#include "Fat32.hpp"

#include "Fat32File.hpp"
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Interface/IFat32Directory.hpp"

Fat32::Fat32(std::shared_ptr<Fat32Disk> fat32)
    : m_fat32(fat32)
{
    Fat32DirectoryEntry rootEntry;
    rootEntry.firstCluster = m_fat32->m_bpb.rootCluster;
    rootEntry.size = std::numeric_limits<int>::max(); // hope this doesnt cause problems

    Fat32File rootDir(fat32, rootEntry);
    parse(m_fat32, rootDir);
}
