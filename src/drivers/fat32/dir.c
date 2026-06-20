#include "drivers/fat32/fat32.h"
#include "drivers/ata.h"
#include "string.h"
#include "stdio.h"
#include "kernel/utils.h"

int find_in_dir(fat32_fs_t* fs, uint32_t dir_cluster, const char* fat_name, fat32_dir_entry_t* out) {
    uint8_t buf[512];
    FOR_EACH_DIR_ENTRY(fs, dir_cluster, buf)
        if (entries[i].name[0] == 0x00) return 0;
        if (entries[i].name[0] == 0xE5) continue;
        if (entries[i].attributes == 0x0F) continue;
        if (memcmp(entries[i].name, fat_name, 11) == 0) {
            *out = entries[i];
            return 1;
        }
    END_DIR_ENTRY_LOOP(fs);
    return 0;
}


//TODO: make this function sprintf() into and return a const char* rather than directly
//interfacing with the terminal

void fat32_list_current_dir_cluster(fat32_fs_t* fs) {

    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return;
    }

    uint8_t buf[512];
    
    printf("Type      Size       Name\n");
    printf("--------------------------\n");

    FOR_EACH_DIR_ENTRY(fs, fs->current_dir_cluster, buf);
        if (entries[i].name[0] == FAT32_ENTRY_END) return;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        if (entries[i].attributes == 0x0F) continue;
        if (entries[i].attributes & FAT32_ATTR_DIRECTORY) {
            printf("DIR       %d ", 0);
        } else {
            printf("FILE      %d ", entries[i].file_size);
        }

        char size_char[32];
        itoa(entries[i].file_size, size_char, 10);
        for(int j = 0; j < (10 - strlen(size_char)); j++) {
            printf(" ");
        }

        for (int j = 0; j < 8; j++) {
            if (entries[i].name[j] != ' ') printf("%c", entries[i].name[j]);
        }
        if (entries[i].name[8] != ' ') {
            printf(".");
            for (int j = 8; j < 11; j++) {
                if (entries[i].name[j] != ' ') printf("%c", entries[i].name[j]);
            }
        }
        printf("\n");
    END_DIR_ENTRY_LOOP(fs)
}

int fat32_change_dir(fat32_fs_t* fs, const char* path) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 1;
    }

    char fat_name[11];

    to_fat_name(path, fat_name);

	fat32_dir_entry_t entry;
    if (find_in_dir(fs, fs->current_dir_cluster, fat_name, &entry)) {
        if (entry.attributes & FAT32_ATTR_DIRECTORY) {
            fs->current_dir_cluster = ((uint32_t)entry.cluster_high << 16)
                              | entry.cluster_low;
            if (fs->current_dir_cluster == 0)
                fs->current_dir_cluster = fs->bpb.root_cluster;
            return 1;
        } else {
            printf("FAT32: not a directory: %s\n", path);
        }
    } else {
        printf("FAT32: directory not found: %s\n", path);
    }
    return 0;
}

int fat32_create_dir_entry(fat32_fs_t* fs, uint32_t dir_cluster, fat32_dir_entry_t* entry) {
    uint8_t buf[512];

    FOR_EACH_DIR_ENTRY(fs, dir_cluster, buf)
        if(entries[i].name[0] == FAT32_ENTRY_FREE) {
            entries[i] = *entry;
            ata_write_sector(lba + s, buf);
            return 1;
        }
        if(entries[i].name[0] == FAT32_ENTRY_END) {
            entries[i] = *entry;

            if(i + 1 < 16) {
                entries[i+1].name[0] = FAT32_ENTRY_END;
            } else {
                uint8_t next_buf[512];
                uint32_t next_lba = lba + s + 1;

                if(s + 1 >= fs->sectors_per_cluster) {
                    uint32_t next_cluster = fat32_next_cluster(fs, cluster);
                    if(next_cluster < FAT32_CLUSTER_END) {
                        next_lba = cluster_to_lba(fs, next_cluster);
                    } else {
                        next_lba = 0;
                    }
                }
                if(next_lba) {
                    ata_read_sector(next_lba, next_buf);
                    next_buf[0] = FAT32_ENTRY_END;
                    ata_write_sector(next_lba, next_buf);
                }
            }
            ata_write_sector(lba + s, buf);
            return 1;
        }
    END_DIR_ENTRY_LOOP(fs)

    uint32_t new_cluster = fat32_alloc_cluster(fs);
    if(!new_cluster) return 0;
    fat32_write_entry(fs, cluster, new_cluster);

    uint8_t empty[512];
    memcpy(empty, 0, 512);
    fat32_dir_entry_t* entries = (fat32_dir_entry_t*)empty;
    entries[0] = *entry;
    entries[1].name[0] = FAT32_ENTRY_END;
    ata_write_sector(cluster_to_lba(fs, new_cluster), empty);
    return 1;
}
