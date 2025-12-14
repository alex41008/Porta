#ifndef SYS_INPUT_OUTPUT_H
#define SYS_INPUT_OUTPUT_H

#include <stdint.h>
#include "sys_io.h"
//#include <cstddef>         
#include "sys_definition_list.h"
#include "sys_vga_screen.h" 
#include "sys_vec.h"

static bool global_left_shift_pressed = false;
static bool global_right_shift_pressed = false;

char scancode_to_ascii(uint8_t scancode, bool shift_pressed)
{
    if (scancode >= 58)
        return 0;
    return shift_pressed ? scancode_ascii_shifted[scancode] : scancode_ascii_normal[scancode];
}

void print_char(char c, bool inplace = false, int color = VGA_COLOR_LIGHT_GREY)
{
    if (c == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        uint16_t position = cursor_y * VGA_WIDTH + cursor_x;
        vga_buffer[position] = (color << 8) | c;

        if (!inplace)
        {
            cursor_x++;
            if (cursor_x >= VGA_WIDTH)
            {
                cursor_x = 0;
                cursor_y++;
                if (cursor_y >= VGA_HEIGHT)
                {
                    scroll_screen(vga_buffer);
                    cursor_y = VGA_HEIGHT - 1;
                }
            }
        }
    }
}

void print_string(const char *str, int color = VGA_COLOR_LIGHT_GREY)
{
    for (int i = 0; str[i] != '\0'; ++i)
    {
        print_char(str[i], false, color);
    }
}

void print_vector(vector<char> &v, int color = VGA_COLOR_LIGHT_GREY)
{
    for (auto &c : v)
    {
        print_char(c, false, color);
    }
}

void print_int(int x, int color = VGA_COLOR_LIGHT_GREY)
{
    int digits = 1;
    int a = x;
    while (a >= 10)
    {
        digits++;
        a = a / 10;
        cursor_x++;
    }

    do
    {
        print_char(48 + (x % 10), true, color);
        x = x / 10;
        cursor_x--;
    } while (x > 0);

    cursor_x += digits + 1;
}
char to_hex_char(uint8_t nibble) {
    if (nibble < 10) {
        return '0' + nibble;
    } else {
        return 'A' + (nibble - 10);
    }
}
void print_hex_32(uint32_t value) {
    
    bool non_zero_printed = false;

    for (int i = 7; i >= 0; --i) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        if (nibble != 0 || i == 0 || non_zero_printed) {
            print_char(to_hex_char(nibble));
            non_zero_printed = true;
        }
    }
}

void print_hex_16(uint16_t value) {
    for (int i = 3; i >= 0; --i) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        print_char(to_hex_char(nibble));
    }
}

void print_hex_8(uint32_t value) {
    auto to_hex_char = [](uint8_t nibble) -> char {
        if (nibble < 10) {
            return '0' + nibble;
        } else {
            return 'A' + (nibble - 10);
        }
    };
}

uint8_t scankey()
{
    if (kbd_read_ptr != kbd_write_ptr) {
        uint8_t scancode = kbd_buffer[kbd_read_ptr];
        kbd_read_ptr = (kbd_read_ptr + 1) % KBD_BUFFER_SIZE;
        return scancode;
    }
    return 0;
}

bool str_cmp(vector<char>&a, vector<char>&b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }

    return true;
}
bool str_cmp_literal(const vector<char>& a, const char* literal) {
    size_t i = 0;
    
    while (i < a.size() && literal[i] != '\0') {
        if (a[i] != literal[i]) {
            return false;
        }
        i++;
    }
    
    return (i == a.size() && literal[i] == '\0');
}
void flush_keyboard_buffer() {
    uint8_t status;
    while (inb(0x64) & 0x1) {
        inb(0x60); 
    }
}

void trim_vec(vector<char>& v) {
        if (v.empty()) return;

    size_t start = 0;
    while (start < v.size() && v[start] == ' ') {
        start++;
    }

    size_t end = v.size();
    while (end > start && v[end - 1] == ' ') {
        end--;
    }

    if (start == end) {
        v.clear(); 
        return;
    }

    if (start > 0) {
        for (size_t i = start; i < end; ++i) {
            v[i - start] = v[i];
        }
    }
    
    size_t new_size = end - start;
    while (v.size() > new_size) {
        v.pop_back(); 
    }
    }

    void input(vector<char> &v, int color = VGA_COLOR_LIGHT_GREY) {
    print_char('_', true, VGA_COLOR_DARK_GREY);

    auto scancode = scankey(); 

    if (scancode == 0) { 
        return; 
    }

    switch (scancode)
    {
    case ENTER:
        print_char(' ', true);
        return; 

    case SHIFT_PRESSED_LEFT:
        global_left_shift_pressed = true;
        break;

    case SHIFT_RELEASED_LEFT:
        global_left_shift_pressed = false;
        break;
        
    case SHIFT_PRESSED_RIGHT:
        global_right_shift_pressed = true;
        break;

    case SHIFT_RELEASED_RIGHT:
        global_right_shift_pressed = false;
        break;

    case BACKSPACE:
        if (!v.empty())
        {
            v.pop_back();
            print_char(' ', true);
            if (cursor_x > 0) {
                cursor_x--;
            } else if (cursor_y > 0) {
                cursor_y--;
                cursor_x = VGA_WIDTH - 1;
            }
            uint16_t position = cursor_y * VGA_WIDTH + cursor_x;
            vga_buffer[position] = (color << 8) | ' ';
        }
        break;

    default:
        if (!(scancode & 0x80) && scancode < KEY_LIMIT) 
        {
            bool shift_pressed = global_left_shift_pressed || global_right_shift_pressed;
            char c = scancode_to_ascii(scancode, shift_pressed);
            if (c)
            {
                v.push_back(c);
                print_char(c, false);
            }
        }
        break;
    }

    return;
    
}

#endif // SYS_INPUT_OUTPUT_H