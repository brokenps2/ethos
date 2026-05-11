#include "drivers/fat32/fat32.h"
#include "drivers/ata.h"
#include "string.h"
#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

int fat32_init(FAT32* fs) {
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

    memcpy(&fs->bpb, buf, sizeof(FAT32BPB));

    fs->fatStart = bpb_sector + fs->bpb.reservedSectors;
    fs->dataStart = bpb_sector + fs->bpb.reservedSectors + fs->bpb.fatCount * fs->bpb.fatSize32;
    fs->sectorsPerCluster = fs->bpb.sectorsPerCluster;

    if (fs->bpb.sectorsPerCluster == 0 || fs->bpb.fatSize32 == 0 || fs->bpb.reservedSectors == 0) {
        printf("FAT32: bad BPB - one or more attribs equal 0\n");
        return 0;
    }

    fs->currentDirCluster = fs->bpb.rootCluster;

    printf("FAT32: mounted\n");
    fs->ready = true;
    return 1;
}

uint32_t cluster_to_lba(FAT32* fs, uint32_t cluster) {
    return fs->dataStart + (cluster - 2) * fs->sectorsPerCluster;
}

uint32_t fat32_next_cluster(FAT32* fs, uint32_t cluster) {
    uint32_t fatOffset = cluster * 4;
    uint32_t fatSector = fs->fatStart + fatOffset / 512;
    uint32_t offset = fatOffset % 512;
    
    uint8_t buf[512];
    ata_read_sector(fatSector, buf);

    uint32_t next = *(uint32_t*)(buf + offset) & 0x0fffffff;
    return next;
}

void to_fat_name(const char* name, char* fatName) {

    if (strcmp(name, ".") == 0) {
        memcpy(fatName, ".          ", 11);
        return;
    } else if (strcmp(name, "..") == 0) {
        memcpy(fatName, "..         ", 11);
        return;
    }

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



void fat32_write_entry(FAT32* fs, uint32_t cluster, uint32_t value) {
    uint32_t fatOffset = cluster * 4;
    uint32_t fatSector = fs->fatStart + fatOffset / 512;
    uint32_t offset = fatOffset % 512;

    uint8_t buf[512];
    ata_read_sector(fatSector, buf);
    *(uint32_t*)(buf + offset) = value & 0x0FFFFFFF;
    ata_write_sector(fatSector, buf);

    if(fs->bpb.fatCount > 1) {
        ata_read_sector(fatSector + fs->bpb.fatSize32, buf);
        *(uint32_t*)(buf + offset) = value & 0x0FFFFFFF;
        ata_write_sector(fatSector + fs->bpb.fatSize32, buf);
    }
}

uint32_t fat32_alloc_cluster(FAT32* fs) {
    uint8_t buf[512];
    uint32_t currentSector = 0xFFFFFFFF;

    for(uint32_t c = 2; c < fs->bpb.fatSize32 * 128; c++) {
        uint32_t fatOffset = c * 4;
        uint32_t fatSector = fs->fatStart + fatOffset / 512;
        uint32_t offset = fatOffset % 512;

        if(fatSector != currentSector) {
            ata_read_sector(fatSector, buf);
            currentSector = fatSector;
        }

        uint32_t entry = *(uint32_t*)(buf + offset) & 0x0FFFFFFF;
        if(entry == FAT32_FREE) {
            fat32_write_entry(fs, c, FAT32_CLUSTER_END);
            return c;
        }
    }
    printf("FAT32: disk full\n");
    return 0;
}

void fat32_free_chain(FAT32* fs, uint32_t cluster) {
    while(cluster < FAT32_CLUSTER_END) {
        uint32_t next = fat32_next_cluster(fs, cluster);
        fat32_write_entry(fs, cluster, FAT32_FREE);
        cluster = next;
    }
}


void fat32_write_cluster(FAT32* fs, uint32_t cluster, uint8_t* buf) {
    uint32_t lba = cluster_to_lba(fs, cluster);
    for(uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
        ata_write_sector(lba + s, buf + s * 512);
    }
}

int fat32_update_dir_entry(FAT32* fs, uint32_t dirCluster, const char* fatName, FAT32DirEntry* newEntry) {
    uint8_t buf[512];

    FOR_EACH_DIR_ENTRY(fs, dirCluster, buf)
                if(entries[i].name[0] == FAT32_ENTRY_END) return 0;
                if(entries[i].name[0] == FAT32_ENTRY_FREE) continue;
                if(memcmp(entries[i].name, fatName, 11) == 0) {
                    entries[i] = *newEntry;
                    ata_write_sector(lba + s, buf);
                    return 1;
                }
            }
        }
        cluster = fat32_next_cluster(fs, cluster);
    }
    return 0;
}





