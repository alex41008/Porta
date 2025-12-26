#pragma once
#ifndef SYS_FONT_H
#define SYS_FONT_H

#include <stdint.h>

extern const uint8_t font_8x8_data[1024];

class FontEngine {
public:
    static void draw_char(int x, int y, char c, uint8_t color);
    
    static void draw_string(int x, int y, const char* str, uint8_t color);
};

#endif