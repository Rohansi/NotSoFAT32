#include "IFat32Directory.hpp"

#include "../Fat32Common.hpp"
#include "../Fat32Directory.hpp"
#include "../Fat32File.hpp"

IFat32Directory::IFat32Directory(std::shared_ptr<Fat32Disk> fat32, int firstCluster)
{
    m_fat32 = fat32;
    m_firstCluster = firstCluster;
}

IFat32Directory::IFat32Directory(IFat32Directory &&other)
{
    m_fat32 = std::move(other.m_fat32);
    m_firstCluster = other.m_firstCluster;

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

bool IFat32Directory::add(const std::string &name, int attributes)
{
    if (!isValidName(name))
        return false;

    if (exists(name))
        return false;

    Fat32File file(m_fat32, m_firstCluster);
    Fat32DirectoryEntry entry;
    int entryPosition;

    // seek to first free entry
    while (true)
    {
        entryPosition = file.tell();

        if (file.read((char*)&entry, sizeof(Fat32DirectoryEntry)) != sizeof(Fat32DirectoryEntry))
            break;

        if (entry.name[0] == 0x00) // stop
            break;

        if (entry.name[0] == (char)0x01) // free
            break;
    }

    // write new entry
    entry = {};
    std::copy(name.begin(), name.end(), entry.name);
    entry.attrib = 0;
    entry.size = 0;
    entry.firstCluster = FatEof;

    file.seek(entryPosition); // just in case of a partial read
    file.write((char*)&entry, sizeof(Fat32DirectoryEntry));

    // add to entry list
    DirectoryEntry dirEntry(m_fat32, entry, m_firstCluster, entryPosition);
    m_entries.insert(std::make_pair(dirEntry.getName(), dirEntry));

    return true;
}

bool IFat32Directory::remove(const std::string &name)
{
    auto item = m_entries.find(name);
    if (item == m_entries.end())
        return false;

    throw std::exception("not implemented");

    auto &entry = item->second;

    if (entry.getAttributes() & FatAttribDirectory)
    {
        // need to recursively remove
    }
    
    // need to free clusters

    // mark the entry as free
    entry.m_name[0] = 0x01;
    entry.save();

    m_entries.erase(item);

    return true;
}

bool IFat32Directory::exists(const std::string &name) const
{
    auto item = m_entries.find(name);
    return item != m_entries.end();
}

void IFat32Directory::parse()
{
    m_entries.clear();

    Fat32File file(m_fat32, m_firstCluster);
    Fat32DirectoryEntry entry;

    while (true)
    {
        int entryPosition = file.tell();

        if (file.read((char*)&entry, sizeof(Fat32DirectoryEntry)) != sizeof(Fat32DirectoryEntry))
            break;

        if (entry.name[0] == 0x00) // stop
            break;

        if (entry.name[0] == (char)0x01) // free
            continue;

        DirectoryEntry dirEntry(m_fat32, entry, m_firstCluster, entryPosition);
        m_entries.insert(std::make_pair(dirEntry.getName(), dirEntry));
    }
}

static const std::string illegalChars =
{
    '"', '*', '/', ':', '<', '>', '?', '\\',
    '|', 127, '+', ',', ';', '=', '[', ']'
};

bool IFat32Directory::isValidName(const std::string &name)
{
    if (name.length() > FatNameLength)
        return false;

    for (const char &ch : name)
    {
        for (const char &illegal : illegalChars)
        {
            if (ch == illegal || ch <= 31)
                return false;
        }
    }

    return true;
}
