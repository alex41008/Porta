#include "../systemh/sys_ps2_mouse_driver.h"
#include "../systemh/sys_vga13h_screen.h"

int PS2Mouse::mouse_x = 160;
int PS2Mouse::mouse_y = 100;
uint8_t PS2Mouse::buttons = 0;
uint8_t PS2Mouse::cycle = 0;
uint8_t PS2Mouse::packet[3];

void PS2Mouse::wait_write() {
    while ((inb(0x64) & 0x02)); // Warte, bis der Input-Buffer leer ist
}

void PS2Mouse::wait_read() {
    while (!(inb(0x64) & 0x01)); // Warte, bis Daten zum Lesen da sind
}

void PS2Mouse::write_command(uint8_t cmd) {
    wait_write();
    outb(0x64, cmd);
}

uint8_t PS2Mouse::read_data() {
    wait_read();
    return inb(0x60);
}

void PS2Mouse::write_data(uint8_t data) {
    wait_write();
    outb(0x64, 0xD4);
    wait_write();
    outb(0x60, data);
}

void PS2Mouse::init() {
    // 1. Hilfsvariable, um Puffer zu leeren
    while (inb(0x64) & 0x01) inb(0x60);

    // 2. Maus-Port aktivieren
    outb(0x64, 0xA8); 

    // 3. Controller-Status-Byte lesen und IRQ 12 aktivieren
    outb(0x64, 0x20);
    wait_read();
    uint8_t status = inb(0x60);
    status |= 0x02;       // Bit 1: Aktiviere Maus-Interrupt (IRQ 12)
    status &= ~(0x20);    // Bit 5: Deaktiviere Maus-Clock-Sperre
    
    outb(0x64, 0x60);
    wait_write();
    outb(0x60, status);

    // 4. Maus sagen, sie soll Standardwerte laden und Streamen aktivieren
    write_data(0xF6); // Set Defaults
    read_data();      // ACK abholen
    
    write_data(0xF4); // Enable Streaming
    read_data();      // ACK abholen
    
    // 5. Finales Leeren, falls während der Init Daten reinkamen
    while (inb(0x64) & 0x01) inb(0x60);
}
void PS2Mouse::process_byte(uint8_t data) {
    switch(cycle) {
        case 0:
            if (!(data & 0x08)) return; // Sync-Bit Check
            packet[0] = data;
            cycle++;
            break;
        case 1:
            packet[1] = data;
            cycle++;
            break;
        case 2:
            packet[2] = data;
            cycle = 0;

            // 9-Bit Vorzeichen-Erweiterung für flüssige Bewegung
            int16_t x_rel = (int16_t)packet[1];
            int16_t y_rel = (int16_t)packet[2];
            if (packet[0] & 0x10) x_rel |= 0xFF00;
            if (packet[0] & 0x20) y_rel |= 0xFF00;

            mouse_x += x_rel;
            mouse_y -= y_rel; // VGA invertiert

            // Clipping
            if (mouse_x < 0) mouse_x = 0;
            if (mouse_y < 0) mouse_y = 0;
            if (mouse_x >= 320) mouse_x = 319;
            if (mouse_y >= 200) mouse_y = 199;

            buttons = packet[0] & 0x07; // Bit 0, 1, 2 sind L, R, M Klick
            break;
    }
}

// Der Interrupt-Handler ruft jetzt einfach process_byte auf
void PS2Mouse::handle_interrupt() {
    uint8_t data = inb(0x60);
    process_byte(data);
}