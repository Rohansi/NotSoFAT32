#include "Fat32Root.hpp"

#include "Fat32File.hpp"
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Interface/IFat32Directory.hpp"

Fat32Root::Fat32Root(std::shared_ptr<Fat32Disk> fat32)
    : IFat32Directory(fat32)
{
    m_fat32 = fat32;
}

Fat32Root::Fat32Root(Fat32Root &&other)
    : IFat32Directory(std::move(other))
{
    m_fat32 = std::move(other.m_fat32);
}

void Fat32Root::initialize()
{
    Fat32DirectoryEntry dirEntry = {};
    dirEntry.firstCluster = m_fat32->m_bpb.rootCluster;
    dirEntry.size = std::numeric_limits<int>::max();

    m_entry = std::shared_ptr<DirectoryEntry>(new DirectoryEntry(m_fat32, shared_from_this(), -1, dirEntry));

    IFat32Directory::initialize();
}
