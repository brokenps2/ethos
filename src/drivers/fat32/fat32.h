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
        for(uint32_t s = 0; s < fs->sectors_per_cluster; s++)          \
        {                                                            \
            ata_read_sector(lba + s, buf);                        \
            Fat32DirEntry* entries = (Fat32DirEntry*)buf;              \
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
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t fat_count;
    uint16_t root_entry_count;
    uint16_t total_sectors_16;
    uint8_t media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint32_t fat_size_32;
    uint16_t flags;
    uint16_t version;
    uint32_t root_cluster;
    uint16_t fs_info_sector;
    uint8_t reserved[32];
    uint8_t drive_number;
    uint8_t reserved2;
    uint8_t boot_signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t fs_type[8];
} __attribute__((packed)) Fat32Bpb;

typedef struct {
    uint8_t name[11];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t cluster_high;
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t cluster_low;
    uint32_t file_size;
} __attribute__((packed)) Fat32DirEntry;

typedef struct {
    uint32_t first_cluster;
    uint32_t size;
    uint32_t position;
} Fat32File;

typedef struct {
    Fat32Bpb bpb;
    uint32_t fat_start;
    uint32_t data_start;
    uint32_t sectors_per_cluster;
    uint32_t current_dir_cluster;
    bool ready;
} Fat32Context;


int fat32_init(Fat32Context* fs);
uint32_t cluster_to_lba(Fat32Context* fs, uint32_t cluster);
uint32_t fat32_next_cluster(Fat32Context* fs, uint32_t cluster);
void to_fat_name(const char* name, char* fat_name);
int find_in_dir(Fat32Context* fs, uint32_t cluster, const char* fat_name, Fat32DirEntry* out);
void fat32_list_current_dir_cluster(Fat32Context* fs);
int fat32_change_dir(Fat32Context* fs, const char* path);
void fat32_write_entry(Fat32Context* fs, uint32_t cluster, uint32_t value);
uint32_t fat32_alloc_cluster(Fat32Context* fs);
void fat32_free_chain(Fat32Context* fs, uint32_t cluster);
void fat32_write_cluster(Fat32Context* fs, uint32_t cluster, uint8_t* buf);
int fat32_update_dir_entry(Fat32Context* fs, uint32_t dir_cluster, const char* fat_name, Fat32DirEntry* new_entry);
int fat32_create_dir_entry(Fat32Context* fs, uint32_t dir_cluster, Fat32DirEntry* entry);

int fat32_open(Fat32Context* fs, Fat32File *file, const char *path);
int fat32_read(Fat32Context* fs, Fat32File *file, uint8_t* buf, uint32_t size);
int fat32_write_file(Fat32Context* fs, const char* path, uint8_t* buf, uint32_t size);
int fat32_delete_file(Fat32Context* fs, const char* path);
