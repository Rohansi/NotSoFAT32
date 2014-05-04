#ifndef __FAT32FILE_HPP
#define __FAT32FILE_HPP

#include "Fat32Common.hpp"
#include <memory>

class Fat32Disk;

class Fat32File
{

public:

    Fat32File(std::shared_ptr<Fat32Disk>, Fat32DirectoryEntry); // internal
    Fat32File(Fat32File &&other);

    ~Fat32File();

    int read(char* buffer, int count);
    bool eof() const;

private:

    std::shared_ptr<Fat32Disk> m_fat32;

    bool m_eof;
    int m_size;
    int m_position;

    int m_currentCluster;
    int m_clusterOffset;
    int m_firstCluster;
    int m_clusterSize;

    std::unique_ptr<char[]> m_cluster;
};

#endif
