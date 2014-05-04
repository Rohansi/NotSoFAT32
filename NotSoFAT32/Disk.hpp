#ifndef __DISK_HPP
#define __DISK_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <memory>
#include <limits>

const int MinBytesPerSector = 512;
const int MaxBytesPerSector = 4096;

class Disk
{

public:

    Disk(const std::string &filename, int sectorSize = MinBytesPerSector);
    Disk(std::fstream &file, int sectorCount, int sectorSize);

    void writeSector(int sector, void *buffer);
    void readSector(int sector, void *buffer);

    int getSectorCount() const;
    int getSectorSize() const;

    static std::shared_ptr<Disk> create(const std::string &filename, int sectorCount, int sectorSize = MinBytesPerSector);

private:

    std::fstream m_file;

    int m_sectorCount;
    int m_sectorSize;
};

#endif
