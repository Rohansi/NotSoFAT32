#ifndef __FAT32COMMON_HPP
#define __FAT32COMMON_HPP

#include <cstdint>

typedef uint32_t fatcluster_t;

const size_t FatNameLength = 23;

const fatcluster_t FatEof = 0xFFFFFFFC;
const fatcluster_t FatBad = 0xFFFFFFFD;
const fatcluster_t FatUnassign = 0xFFFFFFFE;
const fatcluster_t FatFree = 0xFFFFFFFF;

namespace FatAttrib
{
    enum
    {
        File = 0x00,
        ReadOnly = 0x01,
        Hidden = 0x02,
        System = 0x04,
        Directory = 0x08
    };
}

#pragma pack(push,1)

struct Fat32Bpb
{
    uint8_t     jump[8];                // Jump code, ignored
    char        fsysName[7];            // Must be "NSFAT32"
    uint8_t     fsysVersion;            // Filesystem version, currently 0

    uint8_t     driveNumber;            // Drive device id
    uint16_t    bytesPerSector;         // Bytes per sector
    uint16_t    numOfHeads;             // Number of heads
    uint16_t    sectorsPerTrack;        // Sectors per track
    uint32_t    totalSectors;           // Sector count
    uint16_t    reservedSectors;        // Reserved sectors

    uint8_t     sectorsPerCluster;      // Sectors per cluster
    uint32_t    fatSize;                // Size of the FAT, in sectors
    uint32_t    rootCluster;            // Offset of root directory
    char        label[16];              // Volume label

    uint8_t      code[456];              // Boot code
    uint16_t     bootSig;                // Boot signature, should be 0xAA55
};

struct Fat32DirectoryEntry
{
    char        name[FatNameLength];    // Entry name
    uint8_t     attrib;                 // Attributes
    uint32_t    size;                   // Entry size
    uint32_t    firstCluster;           // First cluster
};

#pragma pack(pop)

static_assert(sizeof(Fat32Bpb) == 512, "Fat32Bpb must be 512 bytes");
static_assert(sizeof(Fat32DirectoryEntry) == 32, "Fat32DirectotryEntry must be 32 bytes");

#endif
