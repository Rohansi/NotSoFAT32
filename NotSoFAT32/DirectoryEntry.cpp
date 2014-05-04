#include "DirectoryEntry.hpp"

DirectoryEntry::DirectoryEntry(std::shared_ptr<Fat32Disk> fat32, Fat32DirectoryEntry entry, int parentFirstCluster, int parentPosition)
{
    m_fat32 = fat32;
    m_entry = entry;
    m_parentFirstCluster = parentFirstCluster;
    m_parentPosition = parentPosition;
    m_name = std::string(m_entry.name, strnlen(m_entry.name, FatNameLength));
}

DirectoryEntry::DirectoryEntry(DirectoryEntry &other)
{
    m_fat32 = other.m_fat32;
    m_entry = other.m_entry;
    m_parentFirstCluster = other.m_parentFirstCluster;
    m_parentPosition = other.m_parentPosition;
    m_name = other.m_name;
}

const std::string& DirectoryEntry::getName() const
{
    return m_name;
}

int DirectoryEntry::getAttributes() const
{
    return m_entry.attrib;
}

size_t DirectoryEntry::getSize() const
{
    return m_entry.size;
}

void DirectoryEntry::save() const
{
    Fat32File file(m_fat32, m_parentFirstCluster);
    file.seek(m_parentPosition);
    file.write((char*)&m_entry, sizeof(Fat32DirectoryEntry));
}
