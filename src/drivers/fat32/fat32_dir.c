#include "drivers/fat32/fat32.h"
#include "drivers/ata.h"
#include "string.h"
#include "stdio.h"
#include "kernel/utils.h"

int find_in_dir(FAT32* fs, uint32_t dirCluster, const char* fatName, FAT32DirEntry* out) {
    uint8_t buf[512];
    FOR_EACH_DIR_ENTRY(fs, dirCluster, buf)
        if (entries[i].name[0] == 0x00) return 0;
        if (entries[i].name[0] == 0xE5) continue;
        if (entries[i].attributes == 0x0F) continue;
        if (memcmp(entries[i].name, fatName, 11) == 0) {
            *out = entries[i];
            return 1;
        }
    END_DIR_ENTRY_LOOP(fs);
    return 0;
}


//TODO: make this function sprintf() into and return a const char* rather than directly
//interfacing with the terminal

void fat32_list_current_dir_cluster(FAT32* fs) {

    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return;
    }

    uint8_t buf[512];
    
    printf("Type      Size       Name\n");
    printf("--------------------------\n");

    FOR_EACH_DIR_ENTRY(fs, fs->currentDirCluster, buf);
        if (entries[i].name[0] == FAT32_ENTRY_END) return;
        if (entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        if (entries[i].attributes == 0x0F) continue;
        if (entries[i].attributes & FAT32_ATTR_DIRECTORY) {
            printf("DIR       %d ", 0);
        } else {
            printf("FILE      %d ", entries[i].fileSize);
        }

        char sizeChar[32];
        itoa(entries[i].fileSize, sizeChar, 10);
        for(int j = 0; j < (10 - strlen(sizeChar)); j++) {
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

int fat32_change_dir(FAT32* fs, const char* path) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 1;
    }

    char fatName[11];

    to_fat_name(path, fatName);

    FAT32DirEntry entry;
    if (find_in_dir(fs, fs->currentDirCluster, fatName, &entry)) {
        if (entry.attributes & FAT32_ATTR_DIRECTORY) {
            fs->currentDirCluster = ((uint32_t)entry.clusterHigh << 16)
                              | entry.clusterLow;
            if (fs->currentDirCluster == 0)
                fs->currentDirCluster = fs->bpb.rootCluster;
            return 1;
        } else {
            printf("FAT32: not a directory: %s\n", path);
        }
    } else {
        printf("FAT32: directory not found: %s\n", path);
    }
    return 0;
}

int fat32_create_dir_entry(FAT32* fs, uint32_t dirCluster, FAT32DirEntry* entry) {
    uint8_t buf[512];

    FOR_EACH_DIR_ENTRY(fs, dirCluster, buf)
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
                uint8_t nextBuf[512];
                uint32_t nextLBA = lba + s + 1;

                if(s + 1 >= fs->sectorsPerCluster) {
                    uint32_t nextCluster = fat32_next_cluster(fs, cluster);
                    if(nextCluster < FAT32_CLUSTER_END) {
                        nextLBA = cluster_to_lba(fs, nextCluster);
                    } else {
                        nextLBA = 0;
                    }
                }
                if(nextLBA) {
                    ata_read_sector(nextLBA, nextBuf);
                    nextBuf[0] = FAT32_ENTRY_END;
                    ata_write_sector(nextLBA, nextBuf);
                }
            }
            ata_write_sector(lba + s, buf);
            return 1;
        }
    END_DIR_ENTRY_LOOP(fs)

    uint32_t newCluster = fat32_alloc_cluster(fs);
    if(!newCluster) return 0;
    fat32_write_entry(fs, cluster, newCluster);

    uint8_t empty[512];
    memcpy(empty, 0, 512);
    FAT32DirEntry* entries = (FAT32DirEntry*)empty;
    entries[0] = *entry;
    entries[1].name[0] = FAT32_ENTRY_END;
    ata_write_sector(cluster_to_lba(fs, newCluster), empty);
    return 1;
}
