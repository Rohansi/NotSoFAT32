#ifndef __FAT32COMMON_HPP
#define __FAT32COMMON_HPP

const int FatNameLength = 23;

const int FatEof = 0xFFFFFFFC;
const int FatBad = 0xFFFFFFFD;
const int FatUnassign = 0xFFFFFFFE;
const int FatFree = 0xFFFFFFFF;

const int FatAttribFile = 0x00;
const int FatAttribReadOnly = 0x01;
const int FatAttribHidden = 0x02;
const int FatAttribSystem = 0x04;
const int FatAttribDirectory = 0x08;

#pragma pack(push,1)

struct Fat32Bpb
{
    char        jump[8];                // Jump code, ignored
    char        fsysName[7];            // Must be "NSFAT32"
    char        fsysVersion;            // Filesystem version, currently 0

    char        driveNumber;            // Drive device id
    short       bytesPerSector;         // Bytes per sector
    short       numOfHeads;             // Number of heads
    short       sectorsPerTrack;        // Sectors per track
    int         totalSectors;           // Sector count
    short       reservedSectors;        // Reserved sectors

    char        sectorsPerCluster;      // Sectors per cluster
    int         fatSize;                // Size of the FAT, in sectors
    int         rootCluster;            // Offset of root directory
    char        label[16];              // Volume label

    char        code[456];              // Boot code
    short       bootSig;                // Boot signature, should be 0xAA55
};

struct Fat32DirectoryEntry
{
    char        name[FatNameLength];    // Entry name
    char        attrib;                 // Attributes
    int         size;                   // Entry size
    int         firstCluster;           // First cluster
};

#pragma pack(pop)

static_assert(sizeof(Fat32Bpb) == 512, "Fat32Bpb must be 512 bytes");
static_assert(sizeof(Fat32DirectoryEntry) == 32, "Fat32DirectotryEntry must be 32 bytes");

#endif
