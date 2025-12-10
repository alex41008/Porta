#include <stdint.h>
#include <stddef.h>
// HINWEIS: Hier KEIN #include "sys_io.h" oder andere I/O-Header, um Konflikte zu vermeiden.

// 1. GLOBALE VARIABLEN: DEFINITION (KEIN extern)
inline alignas(16) char memory_pool[1024 * 1024]; // 1MB pool (Definiert HIER)
inline size_t pool_index = 0;

// 2. OPERATOREN: Definition (Inline wird ignoriert, da es die einzige Definition ist)
// Hinzufügen von noexcept zu new/delete
inline void *operator new[](size_t size) noexcept 
{
    if (pool_index + size > sizeof(memory_pool))
    {
        return nullptr;
    }
    void *ptr = &memory_pool[pool_index];
    pool_index += size;
    return ptr;
}

inline void operator delete[](void *ptr) noexcept
{
    // Implementierung (ist leer, wie Sie es haben)
}

struct multiboot_info
{
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
    uint32_t drives_length;
    uint32_t drives_addr;
    uint32_t config_table;
    uint32_t boot_loader_name;
    uint32_t apm_table;
    uint32_t vbe_control_info;
    uint32_t vbe_mode_info;
    uint16_t vbe_mode;
    uint16_t vbe_interface_seg;
    uint16_t vbe_interface_off;
    uint16_t vbe_interface_len;
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t framebuffer_bpp;
    uint8_t framebuffer_type;
    uint8_t color_info[6];
};

struct mmap_entry
{
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed));

extern "C" uint32_t mboot_info_ptr;

inline uint32_t get_total_ram_mb(multiboot_info *mbi)
{

    if (!(mbi->flags & 0x1))
    {
        return 0;
    }

    uint32_t mem_kb = mbi->mem_lower + mbi->mem_upper;
    uint32_t mem_mb = mem_kb / 1024;

    if (mbi->flags & 0x40)
    {
        mem_mb = 0;
        mmap_entry *entry = (mmap_entry *)mbi->mmap_addr;
        mmap_entry *end = (mmap_entry *)((uint32_t)mbi->mmap_addr + mbi->mmap_length);

        while (entry < end)
        {
            if (entry->type == 1)
            {
                mem_mb += (entry->len / 1048576); // BYTE TO MB CONVERSION
            }
            entry = (mmap_entry *)((uint32_t)entry + entry->size + 4);
        }
    }

    return mem_mb;
}