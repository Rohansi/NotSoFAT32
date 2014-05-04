#include "IFat32Directory.hpp"

#include "../Fat32Common.hpp"
#include "../Fat32Directory.hpp"
#include "../Fat32File.hpp"

IFat32Directory::IFat32Directory()
{

}

IFat32Directory::IFat32Directory(IFat32Directory &&other)
{
    m_fat32 = std::move(other.m_fat32);
    m_entries = std::move(other.m_entries);
}

Fat32Directory IFat32Directory::directory(const std::string &name)
{
    auto item = m_entries.find(name);
    if (item == m_entries.end())
        throw std::exception("Entry doesn't exist");

    int firstCluster = item->second.m_entry.firstCluster;

    if ((item->second.getAttributes() & FatAttribDirectory) == 0)
        throw std::exception("Not a directory");

    return Fat32Directory(m_fat32, firstCluster);
}

Fat32File IFat32Directory::file(const std::string &name)
{
    auto item = m_entries.find(name);
    if (item == m_entries.end())
        throw std::exception("Entry doesn't exist");

    if ((item->second.getAttributes() & FatAttribDirectory) != 0)
        throw std::exception("Not a file");

    return Fat32File(m_fat32, std::make_shared<DirectoryEntry>(item->second));
}

bool IFat32Directory::exists(const std::string &name)
{
    auto item = m_entries.find(name);
    return item != m_entries.end();
}

void IFat32Directory::parse(std::shared_ptr<Fat32Disk> fat32, int firstCluster)
{
    Fat32File file(fat32, firstCluster);

    m_fat32 = fat32;
    m_entries.clear();

    while (true)
    {
        Fat32DirectoryEntry entry;
        int entryPosition = file.tell();
        file.read((char*)&entry, sizeof(Fat32DirectoryEntry));

        if (file.eof() || entry.name[0] == 0x00)
            break;

        if (entry.name[0] == (char)0x01)
            continue; // free

        DirectoryEntry dirEntry(fat32, entry, firstCluster, entryPosition);
        m_entries.insert(std::make_pair(dirEntry.getName(), dirEntry));
    }
}
