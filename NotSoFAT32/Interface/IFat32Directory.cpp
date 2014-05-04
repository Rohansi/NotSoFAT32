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
    std::string nameLower(name);
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

    auto item = m_entries.find(nameLower);
    if (item == m_entries.end())
        throw std::exception("Entry doesn't exist");

    auto &entry = item->second.entry;
    int firstCluster = entry.firstCluster;

    if ((entry.attrib & FatAttribDirectory) == 0)
        throw std::exception("Not a directory");

    return Fat32Directory(m_fat32, firstCluster);
}

Fat32File IFat32Directory::file(const std::string &name)
{
    std::string nameLower(name);
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

    auto item = m_entries.find(nameLower);
    if (item == m_entries.end())
        throw std::exception("Entry doesn't exist");

    auto &entry = item->second.entry;

    return Fat32File(m_fat32, item->second.entry);
}

bool IFat32Directory::exists(const std::string &name)
{
    std::string nameLower(name);
    std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

    auto item = m_entries.find(nameLower);
    return item != m_entries.end();
}

void IFat32Directory::parse(std::shared_ptr<Fat32Disk> fat32, Fat32File &file)
{
    Fat32DirectoryEntry entry;
    DirectoryEntry dirEntry;

    m_fat32 = fat32;
    m_entries.clear();

    while (true)
    {
        file.read((char*)&entry, sizeof(Fat32DirectoryEntry));

        if (file.eof() || entry.name[0] == 0x00)
            break;

        if (entry.name[0] == (char)0x01)
            continue; // free

        std::string name(entry.name, sizeof(entry.name));
        dirEntry.name = name;

        dirEntry.entry = entry;
        dirEntry.attributes = entry.attrib;
        dirEntry.size = entry.size;

        m_entries.insert({ name, dirEntry });
    }
}
