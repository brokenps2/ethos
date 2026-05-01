#include "drivers/fat32.h"
#include "drivers/ata.h"
#include "drivers/terminal.h"
#include "string.h"
#include "stdio.h"
#include "kernel/utils.h"
#include <stdint.h>
#include <stdbool.h>

#define FAT32_EOC  0x0FFFFFFF
#define FAT32_FREE 0x00000000

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

    if (fs->bpb.sectorsPerCluster == 0) {
        printf("FAT32: bad BPB - sectorsPerCluster is 0\n");
        return 0;
    }
    if (fs->bpb.fatSize32 == 0) {
        printf("FAT32: bad BPB - fatSize32 is 0\n");
        return 0;
    }
    if (fs->bpb.reservedSectors == 0) {
        printf("FAT32: bad BPB - reservedSectors is 0\n");
        return 0;
    }

    fs->currentDirCluster = fs->bpb.rootCluster;

    printf("FAT32: mounted\n");
    fs->ready = true;
    return 1;
}

static uint32_t cluster_to_lba(FAT32* fs, uint32_t cluster) {
    return fs->dataStart + (cluster - 2) * fs->sectorsPerCluster;
}

static uint32_t fat32_next_cluster(FAT32* fs, uint32_t cluster) {
    uint32_t fatOffset = cluster * 4;
    uint32_t fatSector = fs->fatStart + fatOffset / 512;
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

static int find_in_dir(FAT32* fs, uint32_t cluster, const char* fatName, FAT32DirEntry* out) {
    uint8_t buf[512];
    while (cluster < 0x0FFFFFF8) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        for (uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
            ata_read_sector(lba + s, buf);
            FAT32DirEntry *entries = (FAT32DirEntry*)buf;
            for (int i = 0; i < 16; i++) {
                if (entries[i].name[0] == 0x00) return 0;
                if (entries[i].name[0] == 0xE5) continue;
                if (entries[i].attributes == 0x0F) continue;
                if (memcmp(entries[i].name, fatName, 11) == 0) {
                    *out = entries[i];
                    return 1;
                }
            }
        }
        cluster = fat32_next_cluster(fs, cluster);
    }
    return 0;
}

uint32_t originalFG;
extern uint32_t termFG;
extern uint32_t termBG;

void fat32_list_dir(FAT32* fs) {

    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return;
    }

    uint32_t cluster = fs->currentDirCluster;
    uint8_t buf[512];
    
    printf("Type      Size       Name\n");
    printf("--------------------------\n");

    while (cluster < 0x0FFFFFF8) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        
        for (uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
            ata_read_sector(lba + s, buf);
            FAT32DirEntry *entries = (FAT32DirEntry*)buf;
            
            for (int i = 0; i < 16; i++) {
                if (entries[i].name[0] == 0x00) return;
                if (entries[i].name[0] == 0xE5) continue;
                if (entries[i].attributes == 0x0F) continue;
                if (entries[i].attributes & 0x10) {
                    printf("DIR       %d ", 0);
                    originalFG = termFG;
                    term_set_color(0x004040EE, termBG);
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

                if(entries[i].attributes & 0x10) term_set_color(originalFG, termBG);
                printf("\n");
            }
        }
        cluster = fat32_next_cluster(fs, cluster);
    }
}

int fat32_change_dir(FAT32* fs, const char* path) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 1;
    }

    char fatName[11];

    if (strcmp(path, ".") == 0) {
        memcpy(fatName, ".          ", 11);
    } else if (strcmp(path, "..") == 0) {
        memcpy(fatName, "..         ", 11);
    } else {
        to_fat_name(path, fatName);
    }

    FAT32DirEntry entry;
    if (find_in_dir(fs, fs->currentDirCluster, fatName, &entry)) {
        if (entry.attributes & 0x10) {
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

int fat32_open(FAT32* fs, FAT32File *file, const char *path) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 0;
    }
    if(path[0] == '/') path++;

    char fatName[11];
    to_fat_name(path, fatName);

    FAT32DirEntry entry;
    if(!find_in_dir(fs, fs->currentDirCluster, fatName, &entry)) {
        printf("FAT32: file not found\n");
        return 0;
    }

    file->size = entry.fileSize;
    file->firstCluster = ((uint32_t)entry.clusterHigh << 16) | entry.clusterLow;
    file->position = 0;

    return 1;
}

int fat32_read(FAT32* fs, FAT32File *file, uint8_t *buf, uint32_t size) {
    if(!fs->ready) return 0;

    if(file->position >= file->size) return 0;

    if(size > file->size - file->position)
        size = file->size - file->position;

    uint32_t cluster = file->firstCluster;
    uint32_t bytesRead = 0;

    for(uint32_t i = 0; i < file->position / (fs->sectorsPerCluster * 512); i++) {
        cluster = fat32_next_cluster(fs, cluster);
    }

    uint32_t offset = file->position % (fs->sectorsPerCluster * 512);

    while(cluster < 0x0FFFFFF8 && bytesRead < size) {
        for(uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
            uint8_t sector[512];
            ata_read_sector(cluster_to_lba(fs, cluster) + s, sector);

            uint32_t start = offset;
            uint32_t copy = 512 - start;

            if(copy > size - bytesRead) copy = size - bytesRead;

            memcpy(buf + bytesRead, sector + start, copy);

            bytesRead += copy;
            offset = 0;

            if(bytesRead >= size) break;
        }
        cluster = fat32_next_cluster(fs, cluster);
    }
    file->position += bytesRead;
    return bytesRead;
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

static uint32_t fat32_alloc_cluster(FAT32* fs) {
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
            fat32_write_entry(fs, c, FAT32_EOC);
            return c;
        }
    }
    printf("FAT32: disk full\n");
    return 0;
}

static void fat32_free_chain(FAT32* fs, uint32_t cluster) {
    while(cluster < 0x0FFFFFF8) {
        uint32_t next = fat32_next_cluster(fs, cluster);
        fat32_write_entry(fs, cluster, FAT32_FREE);
        cluster = next;
    }
}


static void fat32_write_cluster(FAT32* fs, uint32_t cluster, uint8_t* buf) {
    uint32_t lba = cluster_to_lba(fs, cluster);
    for(uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
        ata_write_sector(lba + s, buf + s * 512);
    }
}

static int fat32_update_dir_entry(FAT32* fs, uint32_t dirCluster, const char* fatName, FAT32DirEntry* newEntry) {
    uint8_t buf[512];
    uint32_t cluster = dirCluster;

    while(cluster < 0x0FFFFFF8) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        for(uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
            ata_read_sector(lba + s, buf);
            FAT32DirEntry* entries = (FAT32DirEntry*)buf;
            for(int i = 0; i < 16; i++) {
                if(entries[i].name[0] == 0x00) return 0;
                if(entries[i].name[0] == 0xE5) continue;
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

static int fat32_create_dir_entry(FAT32* fs, uint32_t dirCluster, FAT32DirEntry* entry) {
    uint8_t buf[512];
    uint32_t cluster = dirCluster;

    while(cluster < 0x0FFFFFF8) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        for(uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
            ata_read_sector(lba + s, buf);
            FAT32DirEntry* entries = (FAT32DirEntry*)buf;
            for(int i = 0; i < 16; i++) {
                if(entries[i].name[0] == 0xE5) {
                    entries[i] = *entry;
                    ata_write_sector(lba + s, buf);
                    return 1;
                }
                if(entries[i].name[0] == 0x00) {
                    entries[i] = *entry;

                    if(i + 1 < 16) {
                        entries[i+1].name[0] = 0x00;
                    } else {
                        uint8_t nextBuf[512];
                        uint32_t nextLBA = lba + s + 1;

                        if(s + 1 >= fs->sectorsPerCluster) {
                            uint32_t nextCluster = fat32_next_cluster(fs, cluster);
                            if(nextCluster < 0x0FFFFFF8) {
                                nextLBA = cluster_to_lba(fs, nextCluster);
                            } else {
                                nextLBA = 0;
                            }
                        }
                        if(nextLBA) {
                            ata_read_sector(nextLBA, nextBuf);
                            nextBuf[0] = 0x00;
                            ata_write_sector(nextLBA, nextBuf);
                        }
                    }
                    ata_write_sector(lba + s, buf);
                    return 1;
                }
            }
        }
        cluster = fat32_next_cluster(fs, cluster);
    }

    uint32_t newCluster = fat32_alloc_cluster(fs);
    if(!newCluster) return 0;
    fat32_write_entry(fs, cluster, newCluster);

    uint8_t empty[512];
    memcpy(empty, 0, 512);
    FAT32DirEntry* entries = (FAT32DirEntry*)empty;
    entries[0] = *entry;
    entries[1].name[0] = 0x00;
    ata_write_sector(cluster_to_lba(fs, newCluster), empty);
    return 1;
}

int fat32_write_file(FAT32* fs, const char* path, uint8_t* buf, uint32_t size) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 0;
    }
    if(path[0] == '/') path++;
    char fatName[11];
    to_fat_name(path, fatName);

    FAT32DirEntry entry;
    int exists = find_in_dir(fs, fs->currentDirCluster, fatName, &entry);

    if(exists) {
        uint32_t oldCluster = ((uint32_t)entry.clusterHigh << 16) | entry.clusterLow;
        if(oldCluster) fat32_free_chain(fs, oldCluster);
    } else {
        memset(&entry, 0, sizeof(entry));
        memcpy(entry.name, fatName, 11);
        entry.attributes = 0x20;
    }

    uint32_t firstCluster = 0;
    uint32_t prevCluster = 0;
    uint32_t written = 0;
    uint32_t clusterSize = fs->sectorsPerCluster * 512;

    while(written < size) {
        uint32_t cluster = fat32_alloc_cluster(fs);
        if(!cluster) return 0;

        if(!firstCluster) firstCluster = cluster;
        if(prevCluster) fat32_write_entry(fs, prevCluster, cluster);

        uint8_t cbuf[clusterSize];
        memset(cbuf, 0, clusterSize);
        uint32_t chunk = size - written;
        if(chunk > clusterSize) chunk = clusterSize;
        memcpy(cbuf, buf + written, chunk);
        fat32_write_cluster(fs, cluster, cbuf);

        written += chunk;
        prevCluster = cluster;
    }

    entry.clusterHigh = (firstCluster >> 16) & 0xFFFF;
    entry.clusterLow = (firstCluster) & 0xFFFF;
    entry.fileSize = size;

    if(exists) {
        fat32_update_dir_entry(fs, fs->currentDirCluster, fatName, &entry);
    } else {
        fat32_create_dir_entry(fs, fs->currentDirCluster, &entry);
    }

    return 1;
}

int fat32_delete_file(FAT32* fs, const char* path) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 0;
    }
    if(path[0] == '/') path++;

    char fatName[11];
    to_fat_name(path, fatName);

    uint8_t buf[512];
    uint32_t cluster = fs->currentDirCluster;

    while(cluster < 0x0FFFFFF8) {
        uint32_t lba = cluster_to_lba(fs, cluster);
        for(uint32_t s = 0; s < fs->sectorsPerCluster; s++) {
            ata_read_sector(lba + s, buf);
            FAT32DirEntry* entries = (FAT32DirEntry*)buf;
            for(int i = 0; i < 16; i++) {
                if(entries[i].name[0] == 0x00) {
                    printf("FAT32: file not found\n");
                    return 0;
                }
                if(entries[i].name[0] == 0xE5) continue;
                if(entries[i].attributes == 0x0F) continue;

                if(memcmp(entries[i].name, fatName, 11) == 0) {
                    if(entries[i].attributes & 0x10) {
                        printf("FAT32: is a directory\n");
                        return 0;
                    }

                    uint32_t first = ((uint32_t)entries[i].clusterHigh << 16) | entries[i].clusterLow;
                    if(first) fat32_free_chain(fs, first);

                    entries[i].name[0] = 0xE5;
                    ata_write_sector(lba + s, buf);

                    return 1;
                }
            }
        }
        cluster = fat32_next_cluster(fs, cluster);
    }

    printf("FAT32: file not found\n");
    return 0;
}

