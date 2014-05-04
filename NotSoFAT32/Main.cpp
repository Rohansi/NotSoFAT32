#include <iostream>
#include <fstream>
#include <string>
#include "Fat32.hpp"

int main(int argc, char *argv[])
{
    try
    {
        auto disk = Disk::create("disk.img", 10000);
        auto fat32Disk = std::make_shared<Fat32Disk>(disk);
        fat32Disk->format("Test Disk");

        Fat32 fat32(fat32Disk);

        auto file = fat32.file("text.txt");
        std::ofstream out("test.txt", std::ios::out | std::ios::trunc | std::ios::binary);

        char buffer[512];
        int bytesRead = 0;

        while (bytesRead = file.read(buffer, 512))
        {
            out.write(buffer, bytesRead);
        }

        out.close();
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}

