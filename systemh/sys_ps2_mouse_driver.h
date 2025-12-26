#ifndef SYS_PS2_MOUSE_H
#define SYS_PS2_MOUSE_H

#include <stdint.h>
#include "sys_io.h"

class PS2Mouse {
public:
    static void init();
    static void handle_interrupt();
    static void process_byte(uint8_t data);

    static int get_x() { return mouse_x; }
    static int get_y() { return mouse_y; }
    static bool left_clicked() { return (buttons & 0x01); }

private:
    static void wait_write();
    static void wait_read();
    static void write_command(uint8_t command);
    static void write_data(uint8_t data);
    static uint8_t read_data();

    static int mouse_x, mouse_y;
    static uint8_t buttons;
    static uint8_t cycle;
    static uint8_t packet[3];
};

#endif