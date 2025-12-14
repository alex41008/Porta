#ifndef SYS_IO_H
#define SYS_IO_H
#include <stdint.h> 

#define KBD_BUFFER_SIZE 256
extern uint8_t kbd_buffer[KBD_BUFFER_SIZE];
extern uint16_t kbd_read_ptr;            
extern uint16_t kbd_write_ptr;         

extern "C" void kbd_buffer_push(uint8_t scancode);

extern "C" {
    inline uint8_t inb(uint16_t port) {
        uint8_t value;
        asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }
    inline uint16_t inw(uint16_t port) {
        uint16_t value;
        asm volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }
    inline void outb(uint16_t port, uint8_t val) {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
    }
    inline void outw(uint16_t port, uint16_t val) {
        asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
    }

    inline void io_wait() {
        asm volatile("outb %%al, $0x80" : : "a"(0));
    }

}
#endif