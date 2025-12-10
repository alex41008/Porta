// In sys_vga_screen.h (oder neuem header)
#include <stdint.h>
#include "sys_io.h"

#define VGA_WIDTH 320
#define VGA_HEIGHT 200
#define VGA_COLOR_COUNT 256

// Die Speicheradresse des Framebuffers im Modus 13h
#define FRAMEBUFFER_ADDR 0xA0000

volatile uint8_t* g_vga_framebuffer = (volatile uint8_t*)FRAMEBUFFER_ADDR;

static void vga_set_register(uint16_t port, uint8_t index, uint8_t value) {
    outb(port, index);
    outb(port + 1, value);
}

void put_pixel(int x, int y, uint8_t color_index) {
    
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT || x < 0 || y < 0) {
        return; 
    }

    // Offset-Berechnung: y * 320 + x
    uint32_t offset = (uint32_t)y * VGA_WIDTH + x;

    g_vga_framebuffer[offset] = color_index;
}

void vga_set_mode_13h() {
    
    // -------------------------------------------------------------
    // Die Registrierungs-Werte für Modus 13h (320x200, 256 Farben)
    // Quelle: Standardwerte für VGA-Modus 13h
    // -------------------------------------------------------------

    // --- CRTC Registers (0x3D4) ---
    // Deaktiviere die Schreibsperre für Register 0x00 - 0x07 (V-Retrace Register 0x11)
    vga_set_register(0x3D4, 0x11, 0x00); 

    // Eine Liste von kritischen CRTC-Werten (nur ein Subset!)
    uint8_t crtc_values[] = { 
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, // 0x00 bis 0x07
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x08 bis 0x0F
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, // 0x10 bis 0x17
        0xFF                                             // 0x18
    };

    for (int i = 0; i <= 0x18; i++) {
        if (i != 0x08) { // Register 0x08 und 0x11 (entsperrt) überspringen
            vga_set_register(0x3D4, i, crtc_values[i]);
        }
    }

    // --- Sequencer Registers (0x3C4) ---
    vga_set_register(0x3C4, 0x00, 0x03); 
    vga_set_register(0x3C4, 0x01, 0x01); 
    vga_set_register(0x3C4, 0x02, 0x0F); // Map Mask
    vga_set_register(0x3C4, 0x03, 0x00); 
    vga_set_register(0x3C4, 0x04, 0x0E); // Memory Mode (Sequential)

    // --- Graphics Controller Registers (0x3CE) ---
    vga_set_register(0x3CE, 0x05, 0x00); 
    vga_set_register(0x3CE, 0x06, 0x05); // Miscellaneous Register

    // --- Miscellaneous Output Register (0x3C2) ---
    outb(0x3C2, 0xE3); 

    // --- Attribute Controller Registers (0x3C0) ---
    // Dies ist der komplizierteste Teil, da der Port 0x3C0 ein Umschalt-Register ist.
    inb(0x3DA); // Lesen, um Index/Data-Flip-Flop zu löschen (Reset)
    outb(0x3C0, 0x20); // Schaltet den Adressen-Modus des AR-Controllers ein.

    // Lösche den Bildschirm (Setze Framebuffer auf Schwarz)
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        g_vga_framebuffer[i] = 0;
    }
}
