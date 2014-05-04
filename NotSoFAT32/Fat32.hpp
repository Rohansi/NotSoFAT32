#ifndef __FAT32_HPP
#define __FAT32_HPP

#include "Disk.hpp"
#include "Fat32File.hpp"
#include "Fat32Directory.hpp"
#include "Fat32Disk.hpp"
#include "Interface/IFat32Directory.hpp"

class Fat32 : public IFat32Directory
{

public:

    Fat32(std::shared_ptr<Fat32Disk> disk);

private:

    std::shared_ptr<Fat32Disk> m_fat32;
};

#endif
