#include "Fat32.hpp"

#include "Fat32File.hpp"
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Interface/IFat32Directory.hpp"

Fat32::Fat32(std::shared_ptr<Fat32Disk> fat32)
    : IFat32Directory(fat32, fat32->m_bpb.rootCluster), m_fat32(fat32)
{
    parse();
}

Fat32::Fat32(Fat32 &&other)
    : IFat32Directory(std::move(other))
{
    m_fat32 = std::move(other.m_fat32);
}
