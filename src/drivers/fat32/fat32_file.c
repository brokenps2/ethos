#include "drivers/fat32/fat32.h"
#include "drivers/ata.h"
#include "string.h"
#include "stdio.h"

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

    while(cluster < FAT32_CLUSTER_END && bytesRead < size) {
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

    FOR_EACH_DIR_ENTRY(fs, fs->currentDirCluster, buf);
   
        if(entries[i].name[0] == FAT32_ENTRY_END) {
            printf("FAT32: file not found\n");
            return 0;
        }
        if(entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        if(entries[i].attributes == 0x0F) continue;

        if(memcmp(entries[i].name, fatName, 11) == 0) {
            if(entries[i].attributes & FAT32_ATTR_DIRECTORY) {
                printf("FAT32: is a directory\n");
                return 0;
            }

            uint32_t first = ((uint32_t)entries[i].clusterHigh << 16) | entries[i].clusterLow;
            if(first) fat32_free_chain(fs, first);

            entries[i].name[0] = FAT32_ENTRY_FREE;
            ata_write_sector(lba + s, buf);

            return 1;
        }

    END_DIR_ENTRY_LOOP(fs);

    printf("FAT32: file not found\n");
    return 0;
}
