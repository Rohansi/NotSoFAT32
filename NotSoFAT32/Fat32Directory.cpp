#include "Interface/IFat32Directory.hpp"
#include "Fat32Directory.hpp"
#include "Fat32File.hpp"
#include "Fat32.hpp"

#include <algorithm>
#include <string>

Fat32Directory::Fat32Directory(std::shared_ptr<Fat32Disk> fat32, int firstCluster)
    : IFat32Directory(fat32, firstCluster), m_fat32(fat32)
{
    parse();
}

Fat32Directory::Fat32Directory(Fat32Directory &&other)
    : IFat32Directory(std::move(other))
{
    m_fat32 = std::move(other.m_fat32);
}
