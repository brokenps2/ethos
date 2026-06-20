#include "drivers/fat32/fat32.h"
#include "drivers/ata.h"
#include "string.h"
#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>

int fat32_init(fat32_fs_t* fs) {
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

    memcpy(&fs->bpb, buf, sizeof(fat32_bpb_t));

    fs->fat_start = bpb_sector + fs->bpb.reserved_sectors;
    fs->data_start = bpb_sector + fs->bpb.reserved_sectors + fs->bpb.fat_count * fs->bpb.fat_size_32;
    fs->sectors_per_cluster = fs->bpb.sectors_per_cluster;

    if (fs->bpb.sectors_per_cluster == 0 || fs->bpb.fat_size_32 == 0 || fs->bpb.reserved_sectors == 0) {
        printf("FAT32: bad BPB - one or more attribs equal 0\n");
        return 0;
    }

    fs->current_dir_cluster = fs->bpb.root_cluster;

    printf("FAT32: mounted\n");
    fs->ready = true;
    return 1;
}

uint32_t cluster_to_lba(fat32_fs_t* fs, uint32_t cluster) {
    return fs->data_start + (cluster - 2) * fs->sectors_per_cluster;
}

uint32_t fat32_next_cluster(fat32_fs_t* fs, uint32_t cluster) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fs->fat_start + fat_offset / 512;
    uint32_t offset = fat_offset % 512;
    
    uint8_t buf[512];
    ata_read_sector(fat_sector, buf);

    uint32_t next = *(uint32_t*)(buf + offset) & 0x0fffffff;
    return next;
}

void to_fat_name(const char* name, char* fat_name) {

    if (strcmp(name, ".") == 0) {
        memcpy(fat_name, ".          ", 11);
        return;
    } else if (strcmp(name, "..") == 0) {
        memcpy(fat_name, "..         ", 11);
        return;
    }

    memset(fat_name, ' ', 11);

    int i = 0, j = 0;

    while(name[i] && name[i] != '.' && j < 8) {
        char ch = name[i++];
        fat_name[j++] = (ch >= 'a' && ch <= 'z') ? ch - 32 : ch;
    }

    while (name[i] && name[i] != '.') {
        i++;
    }

    if(name[i] == '.') {
        i++;
        j = 8;
        while(name[i] && j < 11) {
            fat_name[j++] = (name[i] >= 'a' && name[i] <= 'z') ? name[i++] - 32 : name[i++];
        }
    }   
}



void fat32_write_entry(fat32_fs_t* fs, uint32_t cluster, uint32_t value) {
    uint32_t fat_offset = cluster * 4;
    uint32_t fat_sector = fs->fat_start + fat_offset / 512;
    uint32_t offset = fat_offset % 512;

    uint8_t buf[512];
    ata_read_sector(fat_sector, buf);
    *(uint32_t*)(buf + offset) = value & 0x0FFFFFFF;
    ata_write_sector(fat_sector, buf);

    if(fs->bpb.fat_count > 1) {
        ata_read_sector(fat_sector + fs->bpb.fat_size_32, buf);
        *(uint32_t*)(buf + offset) = value & 0x0FFFFFFF;
        ata_write_sector(fat_sector + fs->bpb.fat_size_32, buf);
    }
}

uint32_t fat32_alloc_cluster(fat32_fs_t* fs) {
    uint8_t buf[512];
    uint32_t current_sector = 0xFFFFFFFF;

    for(uint32_t c = 2; c < fs->bpb.fat_size_32 * 128; c++) {
        uint32_t fat_offset = c * 4;
        uint32_t fat_sector = fs->fat_start + fat_offset / 512;
        uint32_t offset = fat_offset % 512;

        if(fat_sector != current_sector) {
            ata_read_sector(fat_sector, buf);
            current_sector = fat_sector;
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

void fat32_free_chain(fat32_fs_t* fs, uint32_t cluster) {
    while(cluster < FAT32_CLUSTER_END) {
        uint32_t next = fat32_next_cluster(fs, cluster);
        fat32_write_entry(fs, cluster, FAT32_FREE);
        cluster = next;
    }
}


void fat32_write_cluster(fat32_fs_t* fs, uint32_t cluster, uint8_t* buf) {
    uint32_t lba = cluster_to_lba(fs, cluster);
    for(uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
        ata_write_sector(lba + s, buf + s * 512);
    }
}

int fat32_update_dir_entry(fat32_fs_t* fs, uint32_t dir_cluster, const char* fat_name, fat32_dir_entry_t* new_entry) {
    uint8_t buf[512];

    FOR_EACH_DIR_ENTRY(fs, dir_cluster, buf)
                if(entries[i].name[0] == FAT32_ENTRY_END) return 0;
                if(entries[i].name[0] == FAT32_ENTRY_FREE) continue;
                if(memcmp(entries[i].name, fat_name, 11) == 0) {
                    entries[i] = *new_entry;
                    ata_write_sector(lba + s, buf);
                    return 1;
                }
            }
        }
        cluster = fat32_next_cluster(fs, cluster);
    }
    return 0;
}





