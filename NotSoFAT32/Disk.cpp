#include "Disk.hpp"

Disk::Disk(const std::string &filename, size_t sectorSize)
    : m_file(filename, std::fstream::in | std::fstream::out | std::fstream::binary)
{
    if (!m_file.is_open())
        throw std::exception("Failed to open disk");

    if (sectorSize < MinBytesPerSector || sectorSize > MaxBytesPerSector)
        throw std::exception("Invalid sector size");

    m_file.seekg(0, std::fstream::end);

    auto fileSize = m_file.tellg();
    auto maxFileSize = std::numeric_limits<int>::max() * (std::streamoff)sectorSize;

    if (fileSize > maxFileSize)
        throw std::exception("Disk too large");

    m_sectorCount = (int)fileSize / sectorSize;
    m_sectorSize = sectorSize;
}

Disk::Disk(std::fstream &file, size_t sectorCount, size_t sectorSize)
    : m_file(std::move(file))
{
    m_sectorCount = sectorCount;
    m_sectorSize = sectorSize;
}

size_t Disk::getSectorSize() const
{
    return m_sectorSize;
}

size_t Disk::getSectorCount() const
{
    return m_sectorCount;
}

void Disk::readSector(size_t sector, void *buffer)
{
    if (sector < 0 || sector >= m_sectorCount)
        throw std::exception("Sector doesn't exist");

    auto offset = (std::streamoff)sector * (std::streamoff)m_sectorSize;
    m_file.seekg(offset, std::fstream::beg);
    m_file.read((char*)buffer, m_sectorSize);

    if (m_file.fail())
        throw std::exception("Failed to read sector");
}

void Disk::writeSector(size_t sector, void *buffer)
{
    if (sector < 0 || sector >= m_sectorCount)
        throw std::exception("Sector doesn't exist");

    auto offset = (std::streamoff)sector * (std::streamoff)m_sectorSize;
    m_file.seekp(offset, std::fstream::beg);
    m_file.write((char*)buffer, m_sectorSize);

    if (m_file.fail())
        throw std::exception("Failed to write sector");
}

std::shared_ptr<Disk> Disk::create(const std::string &filename, size_t sectorCount, size_t sectorSize)
{
    if (sectorCount < 100)
        throw std::exception("Invalid sector count");

    if (sectorSize < MinBytesPerSector || sectorSize > MaxBytesPerSector)
        throw std::exception("Invalid sector size");

    std::fstream file(filename, std::fstream::in | std::fstream::out | std::fstream::trunc | std::fstream::binary);

    if (!file.is_open())
        throw std::exception("Failed to open disk");

    auto lastByte = (std::streamoff)sectorCount * (std::streamoff)sectorSize - 1;

    file.seekp(lastByte);
    file << (char)0;

    if (file.fail())
        throw std::exception("Failed to allocate disk space");

    return std::make_shared<Disk>(file, sectorCount, sectorSize);
}
