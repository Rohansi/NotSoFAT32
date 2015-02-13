#include <iostream>
#include <sstream>
#include <filesystem>
#include <NotSoFAT32/Fat32Disk.hpp>

using namespace std::tr2::sys;

size_t sizeStringToSectorCount(const std::string &value)
{
    std::stringstream stream(value);

    size_t size;
    char unit;

    stream >> size;

    if (stream.fail())
        throw std::exception("invalid size");

    stream >> unit;

    if (stream.fail())
        unit = 0;

    switch (unit)
    {
        case 0:
            return size;

        case 'k':
        case 'K':
            return (size * 1024) / 512;

        case 'm':
        case 'M':
            return (size * 1024 * 1024) / 512;

        case 'g':
        case 'G':
            return (size * 1024 * 1024 * 1024) / 512;

        default:
            throw std::exception("invalid size unit");
    }
}

int makeImage(
    const std::string &image,
    const std::string &size,
    const std::string &volumeLabel,
    const std::string &bootloader,
    const std::string &root)
{
    try
    {
        auto &rootPath = path(root);

        if (!exists(rootPath))
            throw std::exception("root doesn't exist");

        if (!is_directory(rootPath))
            throw std::exception("root must be a directory");

        auto &disk = Disk::create(image, sizeStringToSectorCount(size));
        Fat32Disk::format(disk, volumeLabel);

        auto &fat32Disk = std::make_shared<Fat32Disk>(disk);

        auto &it = recursive_directory_iterator(rootPath);
        auto &end = recursive_directory_iterator();

        auto &currentDir = fat32Disk->root();
        auto currentDepth = 0;

        for (; it != end; it++)
        {
            auto &e = *it;

            auto &entryPath = e.path();
            auto &entryFileName = entryPath.filename().string();

            while (currentDepth > it.depth())
            {
                currentDir = currentDir->up();
                currentDepth--;
            }

            if (is_directory(entryPath))
            {
                currentDepth++;

                if (!currentDir->add(entryFileName, FatAttrib::Directory))
                    throw std::exception("failed to create directory entry");

                currentDir = currentDir->directory(entryFileName);
                continue;
            }

            {
                if (!currentDir->add(entryFileName, FatAttrib::File))
                    throw std::exception("failed to create file entry");

                auto &output = currentDir->file(entryFileName);

                std::ifstream input(entryPath, std::ios::binary);

                if (!input.is_open())
                    throw std::exception("failed to open input file");

                while (true)
                {
                    char buffer[4096];
                    input.read(buffer, sizeof(buffer));
                    auto bytesRead = input.gcount();

                    if (bytesRead == 0)
                    {
                        if (input.eof())
                            break;

                        if (input.fail())
                            throw std::exception("failed to read from input file");
                    }

                    output.write(buffer, (size_t)bytesRead);
                }
            }
        }

        if (bootloader != "")
        {
            std::ifstream bootFile(bootloader);
            if (!bootFile.is_open())
                throw std::exception("failed to open bootloader");  

            Fat32Bpb currentBpb;
            disk->readSector(0, &currentBpb);

            Fat32Bpb newBpb;
            bootFile.read((char*)&newBpb, sizeof(Fat32Bpb));

            if (!bootFile.good())
                throw std::exception("failed to read bootloader");

            std::memcpy(currentBpb.jump, newBpb.jump, sizeof(Fat32Bpb::jump));
            std::memcpy(currentBpb.code, newBpb.code, sizeof(Fat32Bpb::code));
            currentBpb.bootSig = newBpb.bootSig;

            disk->writeSector(0, &currentBpb);
        }
    }
    catch (std::exception &e)
    {
        std::cout << "error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void usage()
{
    std::cout << "usage: mknsfat32fs [-o output.img] [-s size] [-l volumeLabel] [-b boot.bin] path" << std::endl;
}

int main(int argc, char *argv[])
{
    std::string image = "disk.img";
    std::string size = "10M";
    std::string volumeLabel;
    std::string bootloader;
    std::string root;

    if (argc < 1)
    {
        usage();
        return EXIT_FAILURE;
    }

    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];

        if (arg == "-o" || arg == "-s" || arg == "-l" || arg == "-b")
        {
            i++;

            if (i >= argc - 1)
            {
                usage();
                return EXIT_FAILURE;
            }

            if (arg == "-o")
                image = argv[i];
            else if (arg == "-s")
                size = argv[i];
            else if (arg == "-l")
                volumeLabel = argv[i];
            else if (arg == "-b")
                bootloader = argv[i];
            else
            {
                usage();
                return EXIT_FAILURE;
            }

            continue;
        }

        root = arg;
        break;
    }

    if (root == "")
    {
        usage();
        return EXIT_FAILURE;
    }

    return makeImage(image, size, volumeLabel, bootloader, root);
}
