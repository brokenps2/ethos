#include "drivers/fat32/fat32.h"
#include "drivers/ata.h"
#include "string.h"
#include "stdio.h"

int fat32_open(fat32_fs_t* fs, fat32_file_t *file, const char *path) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 0;
    }
    if(path[0] == '/') path++;

    char fat_name[11];
    to_fat_name(path, fat_name);

    fat32_dir_entry_t entry;
    if(!find_in_dir(fs, fs->current_dir_cluster, fat_name, &entry)) {
        printf("FAT32: file not found\n");
        return 0;
    }

    file->size = entry.file_size;
    file->first_cluster = ((uint32_t)entry.cluster_high << 16) | entry.cluster_low;
    file->position = 0;

    return 1;
}

int fat32_read(fat32_fs_t* fs, fat32_file_t *file, uint8_t *buf, uint32_t size) {
    if(!fs->ready) return 0;

    if(file->position >= file->size) return 0;

    if(size > file->size - file->position)
        size = file->size - file->position;

    uint32_t cluster = file->first_cluster;
    uint32_t bytes_read = 0;

    for(uint32_t i = 0; i < file->position / (fs->sectors_per_cluster * 512); i++) {
        cluster = fat32_next_cluster(fs, cluster);
    }

    uint32_t offset = file->position % (fs->sectors_per_cluster * 512);

    while(cluster < FAT32_CLUSTER_END && bytes_read < size) {
        for(uint32_t s = 0; s < fs->sectors_per_cluster; s++) {
            uint8_t sector[512];
            ata_read_sector(cluster_to_lba(fs, cluster) + s, sector);

            uint32_t start = offset;
            uint32_t copy = 512 - start;

            if(copy > size - bytes_read) copy = size - bytes_read;

            memcpy(buf + bytes_read, sector + start, copy);

            bytes_read += copy;
            offset = 0;

            if(bytes_read >= size) break;
        }
        cluster = fat32_next_cluster(fs, cluster);
    }
    file->position += bytes_read;
    return bytes_read;
}

int fat32_write_file(fat32_fs_t* fs, const char* path, uint8_t* buf, uint32_t size) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 0;
    }
    if(path[0] == '/') path++;
    char fat_name[11];
    to_fat_name(path, fat_name);

    fat32_dir_entry_t entry;
    int exists = find_in_dir(fs, fs->current_dir_cluster, fat_name, &entry);

    if(exists) {
        uint32_t oldCluster = ((uint32_t)entry.cluster_high << 16) | entry.cluster_low;
        if(oldCluster) fat32_free_chain(fs, oldCluster);
    } else {
        memset(&entry, 0, sizeof(entry));
        memcpy(entry.name, fat_name, 11);
        entry.attributes = 0x20;
    }

    uint32_t first_cluster = 0;
    uint32_t prev_cluster = 0;
    uint32_t written = 0;
    uint32_t cluster_size = fs->sectors_per_cluster * 512;

    while(written < size) {
        uint32_t cluster = fat32_alloc_cluster(fs);
        if(!cluster) return 0;

        if(!first_cluster) first_cluster = cluster;
        if(prev_cluster) fat32_write_entry(fs, prev_cluster, cluster);

        uint8_t cbuf[cluster_size];
        memset(cbuf, 0, cluster_size);
        uint32_t chunk = size - written;
        if(chunk > cluster_size) chunk = cluster_size;
        memcpy(cbuf, buf + written, chunk);
        fat32_write_cluster(fs, cluster, cbuf);

        written += chunk;
        prev_cluster = cluster;
    }

    entry.cluster_high = (first_cluster >> 16) & 0xFFFF;
    entry.cluster_low = (first_cluster) & 0xFFFF;
    entry.file_size = size;

    if(exists) {
        fat32_update_dir_entry(fs, fs->current_dir_cluster, fat_name, &entry);
    } else {
        fat32_create_dir_entry(fs, fs->current_dir_cluster, &entry);
    }

    return 1;
}

int fat32_delete_file(fat32_fs_t* fs, const char* path) {
    if(!fs->ready) {
        printf("FAT32: not ready\n");
        return 0;
    }
    if(path[0] == '/') path++;

    char fat_name[11];
    to_fat_name(path, fat_name);

    uint8_t buf[512];

    FOR_EACH_DIR_ENTRY(fs, fs->current_dir_cluster, buf);
   
        if(entries[i].name[0] == FAT32_ENTRY_END) {
            printf("FAT32: file not found\n");
            return 0;
        }
        if(entries[i].name[0] == FAT32_ENTRY_FREE) continue;
        if(entries[i].attributes == 0x0F) continue;

        if(memcmp(entries[i].name, fat_name, 11) == 0) {
            if(entries[i].attributes & FAT32_ATTR_DIRECTORY) {
                printf("FAT32: is a directory\n");
                return 0;
            }

            uint32_t first = ((uint32_t)entries[i].cluster_high << 16) | entries[i].cluster_low;
            if(first) fat32_free_chain(fs, first);

            entries[i].name[0] = FAT32_ENTRY_FREE;
            ata_write_sector(lba + s, buf);

            return 1;
        }

    END_DIR_ENTRY_LOOP(fs);

    printf("FAT32: file not found\n");
    return 0;
}
