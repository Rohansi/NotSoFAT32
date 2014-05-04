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

    void flush();
    void seek(int position);
    int read(char *buffer, int count);
    void write(char *buffer, int count);

    bool eof() const;

private:

    std::shared_ptr<Fat32Disk> m_fat32;

    int m_firstCluster;
    int m_clusterSize;
    int m_size;

    std::unique_ptr<char[]> m_buffer;
    int m_cluster;
    int m_clusterPosition;
    int m_clusterOffset;

    int m_position;
    bool m_eof;

    bool checkSeekToPosition(bool create = false);
    bool checkNextCluster(bool create = false, bool read = true);

};

#endif
