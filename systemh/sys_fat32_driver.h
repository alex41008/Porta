#ifndef SYS_FAT32_DRIVER_H
#define SYS_FAT32_DRIVER_H

#include <stdint.h>

#define FAT32_ROOT_CLUSTER 2
#define FAT_ENTRY_SIZE 32
#define FAT_ENTRIES_PER_SECTOR 16
#define FAT_ATTR_ARCHIVE 0x20
#define FAT_ATTR_LFN 0x0F
#define FAT_ATTR_VOLUME_ID 0x08
#define FAT_ENTRY_DELETED 0xE5
#define FAT_ENTRY_UNUSED 0x00
#define FAT32_EOC_MARKER 0x0FFFFFFF

extern uint32_t g_partition_lba;
extern uint16_t g_reserved_sectors;
extern uint8_t g_fat_count;
extern uint32_t g_fat_size;

extern uint32_t g_root_dir_lba;
extern uint8_t g_sectors_per_cluster;

typedef struct {
    uint8_t filename[8];        // 0x00
    uint8_t ext[3];             // 0x08
    uint8_t attributes;         // 0x0B
    uint8_t reserved;           // 0x0C
    uint8_t creation_time_ms;   // 0x0D
    uint16_t creation_time;     // 0x0E
    uint16_t creation_date;     // 0x10
    uint16_t last_access_date;  // 0x12
    uint16_t first_cluster_high;// 0x14
    uint16_t last_write_time;   // 0x16
    uint16_t last_write_date;   // 0x18
    uint16_t first_cluster_low; // 0x1A
    uint32_t file_size;         // 0x1C
} __attribute__((packed)) FAT_Directory_Entry; 

struct __attribute__((packed)) FAT_VBR {
    uint8_t boot_jump[3];           // 0x00 - 0x02
    char oem_name[8];               // 0x03 - 0x0A
    uint16_t bytes_per_sector;      // 0x0B - 0x0C (512)
    uint8_t sectors_per_cluster;    // 0x0D
    uint16_t reserved_sectors;      // 0x0E - 0x0F (32)
    uint8_t fat_count;              // 0x10 (2)
    uint16_t root_dir_entries;  // 17-18
    uint16_t total_sectors_16;  // 19-20
    uint8_t media_type;         // 21
    uint16_t fat_size_16;       // 22-23

    // 0x0018 - 0x0023
    uint32_t sectors_per_fat;

    uint32_t root_cluster;
    
    uint8_t filler[462]; 

    uint16_t signature;
};

#endif