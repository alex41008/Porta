

#define ATA_PORT_DATA       0x1F0
#define ATA_PORT_ERROR      0x1F1
#define ATA_PORT_SECTOR_COUNT 0x1F2
#define ATA_PORT_LBA_LOW    0x1F3
#define ATA_PORT_LBA_MID    0x1F4
#define ATA_PORT_LBA_HIGH   0x1F5
#define ATA_PORT_DRIVE_SEL  0x1F6
#define ATA_PORT_STATUS_CMD 0x1F7
#define ATA_PORT_ALT_STATUS_CTRL 0x3F6
#define ATA_PORT_ALT_STATUS_CTRL 0x3F6

#define ATA_CR_NRESET 0x00
#define ATA_CR_RESET  0x04
#define ATA_CR_NIEN   0x02

#define ATA_SR_BSY 0x80 
#define ATA_SR_DRQ 0x08 

#define ATA_MASTER 0xA0
#define ATA_SLAVE  0xB0

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_IDENTIFY 0xEC

#define ATA_MBR_SIGNATURE 0xAA55


uint16_t mbr_buffer[256];
uint16_t vbr_buffer[256]; 

#define VBR_SIGNATURE 0xAA55

void ata_wait_for_not_busy() {

    
    while (inb(ATA_PORT_STATUS_CMD) & ATA_SR_BSY) {
    }
}

bool ata_wait_for_data_request() {
    ata_wait_for_not_busy(); 

    inb(ATA_PORT_ALT_STATUS_CTRL);
    inb(ATA_PORT_ALT_STATUS_CTRL);
    inb(ATA_PORT_ALT_STATUS_CTRL);
    inb(ATA_PORT_ALT_STATUS_CTRL);

    uint8_t status;
    while(true) {
        status = inb(ATA_PORT_STATUS_CMD);
        if (status & 0x01) { // <-- ERR-Bit
            return false; // FEHLER: Führt zu "Disk not found"
        }
        if (status & ATA_SR_DRQ) { 
            return true;
        }
    }
}

void ata_software_reset() {
    // 1. Interrupts deaktivieren und Reset aktivieren
    outb(ATA_PORT_ALT_STATUS_CTRL, ATA_CR_RESET | ATA_CR_NIEN);
    
    // 2. Kurz warten (ca. 5 Mikrosekunden)
    // Wir verwenden vier Lesevorgänge, um eine kurze Verzögerung zu gewährleisten
    inb(ATA_PORT_ALT_STATUS_CTRL); 
    inb(ATA_PORT_ALT_STATUS_CTRL); 
    inb(ATA_PORT_ALT_STATUS_CTRL); 
    inb(ATA_PORT_ALT_STATUS_CTRL); 

    // 3. Reset beenden (De-Reset)
    outb(ATA_PORT_ALT_STATUS_CTRL, ATA_CR_NIEN); // Nur NIEN, RESET ist 0
    
    // 4. Warten, bis BSY gelöscht ist (bis zu 30 Sekunden)
    // Wir verwenden die bestehende Funktion
    ata_wait_for_not_busy();
    
    // 5. Status prüfen (Fehler-Bit sollte 0 sein, sonst Initialisierung fehlgeschlagen)
}

uint16_t byte_swap(uint16_t val) {
    return (val << 8) | (val >> 8);
}

void ata_read_sector(uint32_t lba, uint16_t* buffer) {
    // 1. Warten auf BSY-Clear, bevor wir mit dem Setup beginnen.
    ata_wait_for_not_busy(); 
    
    // 2. LBA-Register setzen
    outb(ATA_PORT_SECTOR_COUNT, 1);
    outb(ATA_PORT_LBA_LOW, (uint8_t)lba);
    outb(ATA_PORT_LBA_MID, (uint8_t)(lba >> 8));
    outb(ATA_PORT_LBA_HIGH, (uint8_t)(lba >> 16));

    // 3. Drive Select (inkl. LBA 24-27)
    outb(ATA_PORT_DRIVE_SEL, 0xE0 | ((lba >> 24) & 0x0F));
    
    // 4. Befehl senden (READ PIO)
    outb(ATA_PORT_STATUS_CMD, ATA_CMD_READ_PIO);

    // 5. Warten auf Abschluss (BSY-Clear)
    ata_wait_for_not_busy(); 
    
    // 6. 400ns Delay (Zwingend nach BSY-Clear und vor Status-Check)
    // Dies stellt sicher, dass der Controller den endgültigen Status gemeldet hat.
    inb(ATA_PORT_ALT_STATUS_CTRL); 
    inb(ATA_PORT_ALT_STATUS_CTRL); 
    inb(ATA_PORT_ALT_STATUS_CTRL); 
    inb(ATA_PORT_ALT_STATUS_CTRL);

    // 7. Finalen Status prüfen
    uint8_t status = inb(ATA_PORT_STATUS_CMD);
    
    // 8. Fehlerbehandlung (MUSS VOR dem Lesen erfolgen)
    if (status & 0x01) { // ERR-Bit gesetzt
        uint8_t error_code = inb(ATA_PORT_ERROR);
        print_string("Disk Read Error. Error Register: 0x");
        print_hex_8(error_code); 
        print_char('\n');
        return; 
    }
    
    // 9. Warten auf Data Request (DRQ)
    if (!(status & ATA_SR_DRQ)) {
        // Obwohl wir BSY gewartet haben, ist es möglich, dass DRQ noch nicht gesetzt ist.
        // Wir verwenden ata_wait_for_data_request, um dies robust zu prüfen.
        if (!ata_wait_for_data_request()) {
            print_string("Disk Read Error: Data Request (DRQ) missing.\n");
            return;
        }
    }

    // 10. Daten lesen (NUR wenn kein Fehler und DRQ gesetzt ist)
    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(ATA_PORT_DATA);
    }
}

bool ata_identify_master_drive() {
    uint16_t data_buffer[256];

    ata_wait_for_not_busy();

    outb(ATA_PORT_SECTOR_COUNT, 0);
    outb(ATA_PORT_LBA_LOW, 0);
    outb(ATA_PORT_LBA_MID, 0);
    outb(ATA_PORT_LBA_HIGH, 0);

    outb(ATA_PORT_DRIVE_SEL, 0xA0); 
    ata_wait_for_not_busy();

    outb(ATA_PORT_STATUS_CMD, ATA_CMD_IDENTIFY); 

    uint8_t status = inb(ATA_PORT_STATUS_CMD);
    if (status == 0x00) {
        return false;
    }

    ata_wait_for_not_busy(); 

    uint8_t lba_mid = inb(ATA_PORT_LBA_MID);
    uint8_t lba_high = inb(ATA_PORT_LBA_HIGH);

    if (lba_mid != 0x00 || lba_high != 0x00) {
        return false;
    }

    if (!ata_wait_for_data_request()) {
        return false;
    }

    for (int i = 0; i < 256; ++i) {
        inw(ATA_PORT_DATA);
    }

    return true;
}

void ata_command_readmbr() {

    if (!ata_identify_master_drive()) {
        print_string("Error: No ATA-Disk found \n");
        return;
    }

    ata_read_sector(0, mbr_buffer); 
    
    uint16_t signature_val = mbr_buffer[255];

    if (signature_val != ATA_MBR_SIGNATURE) {
        print_string("Error: MBR Signatur nicht gefunden.\n");
        print_string("Erwartet: 0xAA55, Gelesen: 0x");
        print_hex_16(signature_val);
        print_char('\n');
        return;
    }

    if (mbr_buffer[255] != ATA_MBR_SIGNATURE) {
        print_string("Error: ATA_MBR_SIGNATURE (0xAA55) not found\n");
        return;
    }

    print_string("ATA_MBR_SIGNATURE read: 0x");
    print_string("AA55\n");

    uint8_t* mbr_bytes = (uint8_t*)mbr_buffer;

    uint32_t first_partition_lba = *(uint32_t*)&mbr_bytes[0x1C6];
    
    print_string("First partition at LBA: ");
    print_int(first_partition_lba);
    print_string("\n"); 
    
    print_string("Reading VBR from LBA ");
    print_int(first_partition_lba); 
    print_string("...\n");

    ata_read_sector(first_partition_lba, vbr_buffer);
    
    uint16_t vbr_signature_val = vbr_buffer[255]; 
    
    if (vbr_signature_val != VBR_SIGNATURE) {
        print_string("Error: VBR-Signature not found\n");
        print_string("Expected: 0xAA55, Read: 0x");
        print_hex_16(vbr_signature_val); 
        print_char('\n');
    } else {
        print_string("VBR-Signature read: 0xAA55\n");
    }

}