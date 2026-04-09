#include <stdint.h>
#pragma once

typedef struct {
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t fatCount;
    uint16_t rootEntryCount;
    uint16_t totalSectors16;
    uint8_t mediaType;
    uint16_t fatSize16;
    uint16_t sectorsPerTrack;
    uint16_t headCount;
    uint32_t hiddenSectors;
    uint32_t totalSectors32;
    uint32_t fatSize32;
    uint16_t flags;
    uint16_t version;
    uint32_t rootCluster;
    uint16_t fsInfoSector;
    uint8_t reserved[32];
    uint8_t driveNumber;
    uint8_t reserved2;
    uint8_t bootSignature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];
    uint8_t fsType[8];
} __attribute__((packed)) FAT32BPB;

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t createTimeTenth;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t accessDate;
    uint16_t clusterHigh;
    uint16_t modifyTime;
    uint16_t modifyDate;
    uint16_t clusterLow;
    uint32_t fileSize;
} __attribute__((packed)) FAT32DirEntry;

int fat32_init();
int fat32_read_file(const char* path, uint8_t* buffer, uint32_t* size);
void fat32_list_dir();
int fat32_change_dir(const char* path);
int fat32_get_file_size(const char* path);
