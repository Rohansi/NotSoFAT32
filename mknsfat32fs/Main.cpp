#include <iostream>
#include <filesystem>
#include <NotSoFAT32/Fat32Disk.hpp>

using namespace std::tr2::sys;

int main(int argc, char *argv[])
{
	try
	{
		auto &rootPath = absolute(path("root")); // TODO: options
		auto &rootPathStr = rootPath.string();

		if (!exists(rootPath))
		{
			std::cout << "root doesnt exist" << std::endl;
			return EXIT_FAILURE;
		}

		if (!is_directory(rootPath))
		{
			std::cout << "root isn't a directory" << std::endl;
			return EXIT_FAILURE;
		}

		auto &disk = Disk::create("disk.img", 10000); // TODO: options
		Fat32Disk::format(disk, "test disk"); // TODO: options

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

			auto &entryPathStr = absolute(entryPath).string();
			entryPathStr.erase(0, rootPathStr.length() + 1);

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
					throw std::exception("failed to open");

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
							throw std::exception("failed to read from file");
					}

					output.write(buffer, (size_t)bytesRead);
				}
			}
		}
	}
	catch (std::exception &e)
	{
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
