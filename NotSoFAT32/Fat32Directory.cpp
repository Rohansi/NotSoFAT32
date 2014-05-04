#include "Interface/IFat32Directory.hpp"
#include "Fat32Directory.hpp"
#include "Fat32File.hpp"
#include "Fat32.hpp"

#include <algorithm>
#include <string>

Fat32Directory::Fat32Directory(std::shared_ptr<Fat32Disk> fat32, int cluster)
    : m_fat32(fat32)
{
    parse(fat32, cluster);
}

Fat32Directory::Fat32Directory(Fat32Directory &&other)
    : IFat32Directory(std::move(other))
{
    m_fat32 = std::move(other.m_fat32);
}

static const std::string illegalChars =
{
    '"', '*', '/', ':', '<', '>', '?', '\\',
    '|', 127, '+', ',', ';', '=', '[', ']'
};

bool Fat32IsValidName(const std::string &name)
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
