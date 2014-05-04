#ifndef __FAT32DIRECTORY_HPP
#define __FAT32DIRECTORY_HPP

#include "Interface\IFat32Directory.hpp"
#include "Fat32Common.hpp"

#include <string>

class Fat32Disk;

class Fat32Directory : public IFat32Directory
{

public:

    Fat32Directory(std::shared_ptr<Fat32Disk>, int); // internal
    Fat32Directory(Fat32Directory &&other);

private:

    std::shared_ptr<Fat32Disk> m_fat32;
};

bool Fat32IsValidName(const std::string &name);

#endif
