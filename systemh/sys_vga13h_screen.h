#ifndef SYS_VGA13H_SCREEN_H
#define SYS_VGA13H_SCREEN_H

#include <stdint.h>
#include "sys_io.h"

#define VGA_WIDTH_13H 320
#define VGA_HEIGHT_13H 200
#define VGA_COLOR_COUNT 256

#define FRAMEBUFFER_ADDR 0xA0000

volatile uint8_t* g_vga_framebuffer = (volatile uint8_t*)FRAMEBUFFER_ADDR;

static void vga_set_register(uint16_t port, uint8_t index, uint8_t value) {
    outb(port, index);
    outb(port + 1, value);
}

void put_pixel(int x, int y, uint8_t color_index) {
    
    if (x >= VGA_WIDTH_13H || y >= VGA_HEIGHT_13H || x < 0 || y < 0) {
        return; 
    }

    uint32_t offset = (uint32_t)y * VGA_WIDTH_13H + x;

    g_vga_framebuffer[offset] = color_index;
}

void vga_set_mode_13h() {
    
    vga_set_register(0x3D4, 0x11, 0x00); 

    uint8_t crtc_values[] = { 
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, // 0x00 to 0x07
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x08 to 0x0F
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, // 0x10 to 0x17
        0xFF                                             // 0x18
    };

    for (int i = 0; i <= 0x18; i++) {
        if (i != 0x08) { 
            vga_set_register(0x3D4, i, crtc_values[i]);
        }
    }
    vga_set_register(0x3C4, 0x00, 0x03); 
    vga_set_register(0x3C4, 0x01, 0x01); 
    vga_set_register(0x3C4, 0x02, 0x0F);
    vga_set_register(0x3C4, 0x03, 0x00); 
    vga_set_register(0x3C4, 0x04, 0x0E);

    vga_set_register(0x3CE, 0x05, 0x00); 
    vga_set_register(0x3CE, 0x06, 0x05);

    outb(0x3C2, 0xE3); 

    inb(0x3DA);
    outb(0x3C0, 0x20);
    /*for (int i = 0; i < VGA_COLOR_COUNT; i++) {
        outb(0x3C0, i);
        outb(0x3C0, i); 
        outb(0x3C0, i); 
    }
    outb(0x3C0, 0x20);  */
    for (int i = 0; i < VGA_WIDTH_13H * VGA_HEIGHT_13H; i++) {
        g_vga_framebuffer[i] = 0;
    }
}

#endif // SYS_VGA13H_SCREEN_H