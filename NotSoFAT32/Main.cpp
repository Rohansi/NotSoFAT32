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

        /*auto disk = std::make_shared<Disk>("disk.img");
        auto fat32Disk = std::make_shared<Fat32Disk>(disk);*/

        const std::string fileName = "test.txt";
        const std::string hello = "hello world!!\r\n";

        Fat32 fat32(fat32Disk);
        fat32.add(fileName, FatAttribFile);

        {
            auto file = fat32.file(fileName);

            for (int i = 0; i < 100; i++)
                file.write(hello.c_str(), hello.length());

            file.seek(0);

            std::ofstream out(fileName, std::ios::out | std::ios::trunc | std::ios::binary);

            char buffer[512];
            int bytesRead = 0;

            while (bytesRead = file.read(buffer, 512))
            {
                out.write(buffer, bytesRead);
            }

            out.close();
        }

        std::cout << "done!";
        fat32.remove("test.txt");
    }
    catch (std::exception &e)
    {
        std::cout << e.what() << std::endl;
    }

    return 0;
}

