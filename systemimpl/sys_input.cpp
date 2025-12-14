// sys_input.cpp

#include <stdint.h>
#include "../systemh/sys_io.h"

uint8_t kbd_buffer[KBD_BUFFER_SIZE];
uint16_t kbd_read_ptr = 0;
uint16_t kbd_write_ptr = 0;

extern "C" void kbd_buffer_push(uint8_t scancode) {
    
    uint16_t next_write_ptr = (kbd_write_ptr + 1) % KBD_BUFFER_SIZE;

    if (next_write_ptr == kbd_read_ptr) {
        return; 
    }

    kbd_buffer[kbd_write_ptr] = scancode;

    kbd_write_ptr = next_write_ptr;
}