#include "drivers/fat32.h"
#include "drivers/ata.h"
#include "string.h"
#include "stdio.h"
#include <stdint.h>

static FAT32BPB bpb;
static uint32_t fatStart;
static uint32_t dataStart;
static uint32_t sectorsPerCluster;

int fat32_init() {
    uint8_t buf[512];
    ata_read_sector(0, buf);

    uint32_t bpb_sector = 0;

    uint8_t part_type = buf[0x1BE + 4];
    uint32_t part_lba = *(uint32_t*)(buf + 0x1BE + 8);

    if (buf[510] == 0x55 && buf[511] == 0xAA && part_type != 0) {
        printf("FAT32: partition type %x at LBA %d\n", part_type, part_lba);
        bpb_sector = part_lba;
        ata_read_sector(bpb_sector, buf);
    }

    memcpy(&bpb, buf, sizeof(FAT32BPB));

    fatStart  = bpb_sector + bpb.reservedSectors;
    dataStart = bpb_sector + bpb.reservedSectors
              + bpb.fatCount * bpb.fatSize32;
    sectorsPerCluster = bpb.sectorsPerCluster;

    // sanity checks
    if (bpb.sectorsPerCluster == 0) {
        printf("FAT32: bad BPB - sectorsPerCluster is 0\n");
        return 0;
    }
    if (bpb.fatSize32 == 0) {
        printf("FAT32: bad BPB - fatSize32 is 0\n");
        return 0;
    }
    if (bpb.reservedSectors == 0) {
        printf("FAT32: bad BPB - reservedSectors is 0\n");
        return 0;
    }

    printf("FAT32: mounted\n");
    return 1;
}

static uint32_t cluster_to_lba(uint32_t cluster) {
    return dataStart + (cluster - 2) * sectorsPerCluster;
}

static uint32_t fat_next_cluster(uint32_t cluster) {
    uint32_t fatOffset = cluster * 4;
    uint32_t fatSector = fatStart + fatOffset / 512;
    uint32_t offset = fatOffset % 512;
    
    uint8_t buf[512];
    ata_read_sector(fatSector, buf);

    uint32_t next = *(uint32_t*)(buf + offset) & 0x0fffffff;
    return next;
}

static void to_fat_name(const char* name, char* fatName) {
    memset(fatName, ' ', 11);

    int i = 0, j = 0;

    while(name[i] && name[i] != '.' && j < 8) {
        char ch = name[i++];
        fatName[j++] = (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
    }

    while (name[i] && name[i] != '.') {
        i++;
    }

    if(name[i] == '.') {
        i++;
        j = 8;
        while(name[i] && j < 11) {
            fatName[j++] = (name[i] >= 'a' && name[i] <= 'z') ? name[i++] - 32 : name[i++];
        }
    }   
}

static int find_in_dir(uint32_t cluster, const char* fatName, FAT32DirEntry* out) {
    uint8_t buf[512];
    int chain_limit = 1000;
    while (cluster < 0x0FFFFFF8 && --chain_limit) {
        uint32_t lba = cluster_to_lba(cluster);
        for (uint32_t s = 0; s < sectorsPerCluster; s++) {
            ata_read_sector(lba + s, buf);
            FAT32DirEntry *entries = (FAT32DirEntry*)buf;
            for (int i = 0; i < 512 / 32; i++) {
                if (entries[i].name[0] == 0x00) return 0;
                if (entries[i].name[0] == 0xE5) continue;
                if (entries[i].attributes == 0x0F) continue;
                if (memcmp(entries[i].name, fatName, 11) == 0) {
                    *out = entries[i];
                    return 1;
                }
            }
        }
        cluster = fat_next_cluster(cluster);
    }
    if (!chain_limit) printf("FAT32: cluster chain loop detected\n");
    return 0;
}

int fat32_read_file(const char* path, uint8_t* buf, uint32_t* size) {
    if(path[0] == '/') path++;

    char fatName[11];
    to_fat_name(path, fatName);

    FAT32DirEntry entry;
    if(!find_in_dir(bpb.rootCluster, fatName, &entry)) {
        printf("FAT32: file not found\n");
        return 0;
    }

    *size = entry.fileSize;
    uint32_t cluster = ((uint32_t)entry.clusterHigh << 16) | entry.clusterLow;
    uint32_t bytesRead = 0;

    while(cluster < 0x0FFFFFF8 && bytesRead < *size) {
        uint32_t lba = cluster_to_lba(cluster);
        for(uint32_t s = 0; s < sectorsPerCluster; s++) {
            ata_read_sector(lba + s, buf + bytesRead);
            bytesRead += 512;
            if(bytesRead >= *size) break;
        }
        cluster = fat_next_cluster(cluster);
    }
    return 1;
}
