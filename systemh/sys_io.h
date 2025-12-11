#ifndef SYS_IO_H
#define SYS_IO_H
#include <stdint.h> 

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

}
#endif