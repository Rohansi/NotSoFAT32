#ifndef __FAT32FILE_HPP
#define __FAT32FILE_HPP

#include <memory>
#include "Fat32Common.hpp"
#include "DirectoryEntry.hpp"

class Fat32
;
class DirectoryEntry;

class Fat32File
{

public:

    Fat32File(std::shared_ptr<Fat32Disk> fat32, std::shared_ptr<DirectoryEntry> entry); // internal
    Fat32File(std::shared_ptr<Fat32Disk> fat32, int firstCluster); // internal
    Fat32File(Fat32File &&other);

    ~Fat32File();

    void flush();
    void seek(int position);
    int tell() const;

    int read(char *buffer, int count);
    void write(const char *buffer, int count);

    bool eof() const;

private:

    std::shared_ptr<Fat32Disk> m_fat32;

    std::shared_ptr<DirectoryEntry> m_entry;
    bool m_entryDirty;

    int m_firstCluster;
    int m_clusterSize;
    int m_originalSize;
    int m_size;

    std::unique_ptr<char[]> m_buffer;
    int m_cluster;
    int m_clusterPosition;
    int m_clusterOffset;
    bool m_clusterDirty;

    int m_position;

    bool checkSeekToPosition(bool alloc = false);
    bool checkNextCluster(bool alloc = false, bool read = true);
    bool checkHasCluster(bool alloc = false);

};

#endif
