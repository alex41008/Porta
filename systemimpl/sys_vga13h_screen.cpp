#include "../systemh/sys_vga13h_screen.h"

uint8_t backbuffer[320 * 200]; 

void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 320 || y < 0 || y >= 200) return;
    backbuffer[y * 320 + x] = color;
}

void vga_flip() {
    uint32_t* vga = (uint32_t*)0xA0000;
    uint32_t* src = (uint32_t*)backbuffer;
    for (int i = 0; i < 16000; i++) {
        vga[i] = src[i];
    }
}