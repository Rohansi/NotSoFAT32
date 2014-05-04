#include "Fat32.hpp"

#include "Fat32File.hpp"
#include "Fat32Common.hpp"
#include "Fat32Disk.hpp"
#include "Interface/IFat32Directory.hpp"

Fat32::Fat32(std::shared_ptr<Fat32Disk> fat32)
    : m_fat32(fat32)
{
    /*std::string name = "test.txt";

    Fat32DirectoryEntry entry = {};
    std::copy(name.begin(), name.end(), entry.name);
    entry.attrib = 0;
    entry.size = 0;
    entry.firstCluster = FatEof;

    Fat32File file(fat32, m_fat32->m_bpb.rootCluster);
    file.seek(0);
    file.write((char*)&entry, sizeof(Fat32DirectoryEntry));
    file.flush();*/

    parse(m_fat32, m_fat32->m_bpb.rootCluster);
}
