#include <stdint.h>
#include <stdbool.h>
#pragma once

#define FAT32_CLUSTER_END      0x0FFFFFF8
#define FAT32_ATTR_DIRECTORY   0x10
#define FAT32_ATTR_LFN         0x0F
#define FAT32_ENTRY_FREE       0xE5
#define FAT32_ENTRY_END        0x00
#define FAT32_SECTOR_SIZE      512
#define FAT32_ENTRIES_PER_SECTOR 16
#define FAT32_FREE 0x00000000

#define FOR_EACH_DIR_ENTRY(fs, startCluster, buf)     \
    uint32_t cluster = startCluster;                                 \
    while(cluster < FAT32_CLUSTER_END)                               \
    {                                                                \
        uint32_t lba = cluster_to_lba(fs, cluster);               \
        for(uint32_t s = 0; s < fs->sectorsPerCluster; s++)          \
        {                                                            \
            ata_read_sector(lba + s, buf);                        \
            FAT32DirEntry* entries = (FAT32DirEntry*)buf;              \
            for(int i = 0; i < 16; i++)                              \
            {

#define END_DIR_ENTRY_LOOP(fs)                                       \
            }                                                        \
        }                                                            \
        cluster = fat32_next_cluster(fs, cluster);                   \
    }

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

typedef struct {
    uint32_t firstCluster;
    uint32_t size;
    uint32_t position;
} FAT32File;

typedef struct {
    FAT32BPB bpb;
    uint32_t fatStart;
    uint32_t dataStart;
    uint32_t sectorsPerCluster;
    uint32_t currentDirCluster;
    bool ready;
} FAT32;


int fat32_init(FAT32* fs);
uint32_t cluster_to_lba(FAT32* fs, uint32_t cluster);
uint32_t fat32_next_cluster(FAT32* fs, uint32_t cluster);
void to_fat_name(const char* name, char* fatName);
int find_in_dir(FAT32* fs, uint32_t cluster, const char* fatName, FAT32DirEntry* out);
void fat32_list_current_dir_cluster(FAT32* fs);
int fat32_change_dir(FAT32* fs, const char* path);
void fat32_write_entry(FAT32* fs, uint32_t cluster, uint32_t value);
uint32_t fat32_alloc_cluster(FAT32* fs);
void fat32_free_chain(FAT32* fs, uint32_t cluster);
void fat32_write_cluster(FAT32* fs, uint32_t cluster, uint8_t* buf);
int fat32_update_dir_entry(FAT32* fs, uint32_t dirCluster, const char* fatName, FAT32DirEntry* newEntry);
int fat32_create_dir_entry(FAT32* fs, uint32_t dirCluster, FAT32DirEntry* entry);

int fat32_open(FAT32* fs, FAT32File *file, const char *path);
int fat32_read(FAT32* fs, FAT32File *file, uint8_t *buf, uint32_t size);
int fat32_write_file(FAT32* fs, const char* path, uint8_t* buf, uint32_t size);
int fat32_delete_file(FAT32* fs, const char* path);
