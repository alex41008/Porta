#include "sys_cpu.h"
#include "sys_ata_driver.h"
#include "sys_fat32_driver.h"
//#include "sys_vga_screen.h"

uint32_t g_partition_lba = 0;
uint16_t g_reserved_sectors = 0;
uint8_t g_fat_count = 0;
uint32_t g_fat_size = 0;
uint32_t g_root_dir_lba = 0;

struct kernel_program {
public:
    
    // ... (Andere Funktionen wie k_help, k_readmbr, k_info, k_disktest, k_clear bleiben unverändert) ...

    // Hilfsfunktion zur Namensformatierung (für k_open_file)
    // -------------------------------------------------------------------------
// 1. fat_format_name (Rein funktional, keine Ausgabe)
// -------------------------------------------------------------------------
void fat_format_name(const char* filename, char* output) {
    // Füllt den 11-Byte-Puffer mit Leerzeichen
    for (int i = 0; i < 11; i++) output[i] = ' '; 

    int input_pos = 0;
    int output_pos = 0;
    bool ext_started = false;

    while (filename[input_pos] != '\0' && output_pos < 11) {
        char c = filename[input_pos];

        if (c == '.') {
            // Springe zum Erweiterungs-Teil (Offset 8)
            output_pos = 8;
            ext_started = true;
        } else {
            // Nur Großbuchstaben (FAT ist case-insensitive)
            if (c >= 'a' && c <= 'z') c -= 32;

            if (output_pos < 8 || (ext_started && output_pos < 11)) {
                output[output_pos] = c;
                output_pos++;
            }
        }
        input_pos++;
    }
}

// -------------------------------------------------------------------------
// 2. __k_init_filesystem__ (Minimale Ausgabe)
// -------------------------------------------------------------------------
void __k_init_filesystem__() {
    
    // --- SCHRITT 1: CONTROLLER STABILISIEREN & MBR LESEN ---
    //print_string("Attempting ATA Controller stabilization...\n");
    ata_command_readmbr(); 
    
    for (volatile int i = 0; i < 10000; i++); 

    uint8_t* mbr_bytes = (uint8_t*)mbr_buffer; 
    uint32_t first_partition_lba = *(uint32_t*)&mbr_bytes[0x1C6]; 
    
    g_partition_lba = first_partition_lba;
    
    // --- SCHRITT 2: VBR LESEN (Volume Boot Record) ---
    uint16_t vbr_buffer[256]; 
    //print_string("Reading VBR from LBA ");
    //print_int(g_partition_lba); 
    //print_string("...\n");
    
    ata_read_sector(g_partition_lba, vbr_buffer);
    
    // --- SCHRITT 3: VBR PRÜFEN & METADATEN PARSEN ---
    uint8_t* byte_ptr = (uint8_t*)vbr_buffer; 
    uint16_t actual_signature = *(uint16_t*)&byte_ptr[510]; 

    if (actual_signature != 0xAA55) {
        print_string("Error: VBR-Signature invalid. Read: 0x");
        print_hex_16(actual_signature); 
        print_char('\n');
        return;
    }
    
    g_fat_size = *(uint32_t*)&byte_ptr[36];     
    g_reserved_sectors = *(uint16_t*)&byte_ptr[14]; 
    g_fat_count = byte_ptr[16]; 

    // Root Directory LBA berechnen und global speichern
    g_root_dir_lba = g_partition_lba + g_reserved_sectors + (g_fat_count * g_fat_size);
    
    //print_string("FAT Root Directory LBA calculated to start at: ");
    //print_int(g_root_dir_lba); 
    //print_char('\n');
    
    // --- SCHRITT 4: ROOT DIRECTORY LESEN & PARSEN (Ohne vollständigen Debug) ---

    uint16_t root_dir_buffer[256]; 
    ata_read_sector(g_root_dir_lba, root_dir_buffer);

    FAT_Directory_Entry* entries = (FAT_Directory_Entry*)root_dir_buffer;

    print_string("\n--- Root Directory Entries ---\n");

    // Nur gültige Dateien auflisten (die die Filterung in k_open_file überstehen würden)
    for (int i = 0; i < 16; i++) {
        
        if (entries[i].filename[0] == 0x00) break; 
        
        // Ignoriere: gelöschte Dateien (0xE5) und Volume Labels/LFN (0x08 oder 0x0F)
        if (entries[i].filename[0] == 0xE5 || (entries[i].attributes & 0x08) || (entries[i].attributes == 0x0F)) {
            continue;
        }
        
        // Dateinamen ausgeben
        print_string("File: ");
        
        // Dateiname (8 Bytes)
        for (int j = 0; j < 8; j++) {
            if (entries[i].filename[j] == ' ') break;
            print_char(entries[i].filename[j]);
        }
        
        // Erweiterung (3 Bytes)
        if (entries[i].ext[0] != ' ') {
            print_char('.');
            for (int j = 0; j < 3; j++) {
                if (entries[i].ext[j] == ' ') break;
                print_char(entries[i].ext[j]);
            }
        }

        // Größe ausgeben
        print_string(" (Size: ");
        print_int(entries[i].file_size);
        print_string(" Bytes)\n");
    }

    print_string("------------------------------\n");
}


// -------------------------------------------------------------------------
// 3. k_open_file (Minimale Ausgabe)
// -------------------------------------------------------------------------
FAT_Directory_Entry* k_open_file(const char* filename) {
    
    uint32_t root_dir_lba = g_root_dir_lba;
    
    uint16_t root_dir_buffer[256]; 
    ata_read_sector(root_dir_lba, root_dir_buffer);

    FAT_Directory_Entry* entries = (FAT_Directory_Entry*)root_dir_buffer;

    char search_name[11];
    this->fat_format_name(filename, search_name); 

    // Nur Ausgabe, wenn gesucht wird (kann für den "schlanken" Modus entfernt werden)
    // print_string("\nSearching for file: ");
    // print_string(filename);
    // print_string("...\n");

    for (int i = 0; i < FAT_ENTRIES_PER_SECTOR; i++) {
        
        if (entries[i].filename[0] == FAT_ENTRY_UNUSED) break; 
        
        // Ignoriere LFN, Volume Label, gelöschte Dateien
        if (entries[i].filename[0] == FAT_ENTRY_DELETED || 
            (entries[i].attributes & FAT_ATTR_VOLUME_ID) || 
            (entries[i].attributes == FAT_ATTR_LFN)) {
            continue;
        }

        // Getrennter Vergleich von Name (8 Bytes) und Extension (3 Bytes)
        bool name_match = true;
        
        // Teil 1: Name (8 Bytes)
        for (int j = 0; j < 8; j++) {
            if (entries[i].filename[j] != search_name[j]) {
                name_match = false;
                break;
            }
        }
        
        if (!name_match) continue;

        // Teil 2: Extension (3 Bytes)
        for (int j = 0; j < 3; j++) {
            if (entries[i].ext[j] != search_name[8 + j]) {
                name_match = false;
                break;
            }
        }

        if (name_match) {
            // Ausgabe: Datei gefunden und Inhalt wird gelesen
            print_string("Reading File ");
            print_string(filename);
            print_string(": \n");

            // --- Lesen des ersten Clusters ---
            uint32_t first_cluster = entries[i].first_cluster_low; 
            
            if (first_cluster == 0) first_cluster = FAT32_ROOT_CLUSTER; 
            
            // LBA des Daten-Clusters (angenommen 1 Sektor pro Cluster)
            uint32_t cluster_lba = g_root_dir_lba + (first_cluster - FAT32_ROOT_CLUSTER); 

            char cluster_buffer[512];
            ata_read_sector(cluster_lba, (uint16_t*)cluster_buffer);
            
            uint32_t size = entries[i].file_size;
            if (size > 512) size = 512; 

            for(uint32_t k = 0; k < size; k++) {
                print_char(cluster_buffer[k]);
            }
            print_char('\n');
            
            return &entries[i];
        }
    }

    print_string("Error: File not found.\n");
    return nullptr;
}


void k_readmbr(const vector<char>& a) {
    if(str_cmp_literal(a, "readmbr")) {
        ata_command_readmbr();
    }
}
void k_info(const vector<char>& a, multiboot_info *mbi) {
    if(str_cmp_literal(a, "info")) {
        print_string("Free Memory: ");
        print_int(mbi->mem_lower + mbi->mem_upper);
        print_string(" MB\n");
        print_string("CPU Vendor: ");
        print_cpu_vendor();
        print_char('\n');
        print_string("CPU Brand: ");
        print_cpu_brand();
        print_char('\n');
    }
}
void k_disktest(const vector<char>& a) {
    if (str_cmp_literal(a, "disktest")) {
        print_string("Checking primary master (0x1F0)... ");
        if (ata_identify_master_drive()) {
            print_string("ATA-Disk found\n");
        }  else {
            print_string("Error: No ATA-Disk found\n");
        }
    }
}
void k_help(const vector<char>& a) {
    if(str_cmp_literal(a, "help")) {
        print_string("Kernel Functions: help, info, disktest, readmbr, openfile, showfiles, gui, drawtest, clear\n");
    }
}
void k_show_files(const vector<char>& a) {
    if(str_cmp_literal(a, "showfiles")) {
        __k_init_filesystem__();
    }
}
    void k_print_info() {
        print_string("Spectrum 1.0\n", VGA_COLOR_LIGHT_RED);
        print_string("Operating System (Kernel CLI) - Alexander Slama\n");
        print_string("-----------------------------------------------\n");
    }
    void k_input(vector<char>& i) {
        print_string("- ", VGA_COLOR_LIGHT_RED);
        input(i);      
        print_char('\n');
    }
    void k_clear(const vector<char>& a) {
        if (str_cmp_literal(a, "clear")) {
            print_string("CLEARING");
            cls();
            k_init_cli();
        }
    }
    void k_init_cli() {
        k_print_info();
        //__k_init_filesystem__();
    }
    void k_enter_gui(const vector<char>& a) {
    if (str_cmp_literal(a, "gui")) {
        print_string("Switched to VGA Mode 13h (320x200). CLI functions are now broken.\n");
        vga_set_mode_13h();
    }
    }
    void k_draw_test(const vector<char>& a) {
    if (str_cmp_literal(a, "drawtest")) {
        
        // Stellen Sie sicher, dass wir im Grafikmodus sind, bevor wir zeichnen
        vga_set_mode_13h(); 

        // Zeichne einen roten Rahmen (Index 4 ist typischerweise Rot)
        for (int x = 0; x < VGA_WIDTH; x++) {
            put_pixel(x, 0, 4); // Oberer Rand
            put_pixel(x, VGA_HEIGHT - 1, 4); // Unterer Rand
        }
        for (int y = 0; y < VGA_HEIGHT; y++) {
            put_pixel(0, y, 4); // Linker Rand
            put_pixel(VGA_WIDTH - 1, y, 4); // Rechter Rand
        }
        
        // Zeichne ein blaues Quadrat in der Mitte (Index 1 ist typischerweise Blau)
        for (int y = 50; y < 150; y++) {
            for (int x = 50; x < 270; x++) {
                put_pixel(x, y, 1); 
            }
        }
    }
}
    void k_file_dialogue_open(const vector<char>& a) {
    if (str_cmp_literal(a, "openfile")) {
        
        print_string("Enter Filename: ");
        
        // Vektor für die Benutzereingabe (Dateiname)
        vector<char> input_vec;
        
        // Ruft Ihre input-Funktion auf, die den Cursor bewegt und '\n' hinzufügt
        // Hinweis: print_char('\n') am Ende von k_input wird das Trennzeichen sein.
        this->k_input(input_vec); 
        
        // 1. Eingabe trimmen (Leerzeichen entfernen)
        // Voraussetzung: trim_vec() ist verfügbar (wie in Ihrem ursprünglichen Code gesehen)
        // trim_vec(input_vec); // Kann optional sein, wenn Nutzer Leerzeichen eingeben könnten
        
        // 2. Vektor in null-terminierten String umwandeln
        // Wir benötigen einen Puffer auf dem Stack. +1 für das Null-Byte.
        // MAX_FILENAME_LENGTH sollte groß genug sein (z.B. 12 oder mehr).
        const size_t MAX_FILENAME_LENGTH = 16; 
        char filename_buffer[MAX_FILENAME_LENGTH];
        
        size_t len = input_vec.size();
        if (len >= MAX_FILENAME_LENGTH) {
            len = MAX_FILENAME_LENGTH - 1; // Sicherstellen, dass Platz für '\0' ist
        }

        // Vektorinhalt kopieren
        for (size_t k = 0; k < len; ++k) {
            filename_buffer[k] = input_vec[k];
        }
        
        // Null-Terminierung hinzufügen
        filename_buffer[len] = '\0'; 

        // 3. k_open_file mit dem erstellten String aufrufen
        // Die Funktion gibt den Inhalt direkt auf dem Bildschirm aus.
        this->k_open_file(filename_buffer);
    }
}

};