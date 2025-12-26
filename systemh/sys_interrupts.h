#ifndef SYS_INTERRUPTS_H
#define SYS_INTERRUPTS_H

#include <stdint.h>
#include "sys_io.h" 
#include "sys_ps2_mouse_driver.h"

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1

#define ICW1_INIT 0x11      
#define ICW4_8086 0x01      

#define PIC_MASTER_OFFSET 0x20 
#define PIC_SLAVE_OFFSET 0x28 

#define PIC_EOI 0x20

inline void pic_send_eoi(uint8_t irq_number) {
    if (irq_number >= 8) {
        outb(0xA0, 0x20); 
    }
    outb(0x20, 0x20);   
}

inline void pic_remap(void) {
    uint8_t a1 = inb(PIC1_DATA);
    uint8_t a2 = inb(PIC2_DATA);

    outb(PIC1_COMMAND, ICW1_INIT);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT); 
    io_wait();

    outb(PIC1_DATA, PIC_MASTER_OFFSET); 
    io_wait();
    outb(PIC2_DATA, PIC_SLAVE_OFFSET);
    io_wait();

    outb(PIC1_DATA, 0x04);
    io_wait();
    outb(PIC2_DATA, 0x02);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0xF8); // 0, 1, 2 frei
    outb(PIC2_DATA, 0xEF); // 12 frei
}

#define PIT_DATA 0x40
#define PIT_COMMAND 0x43

#define PIT_BASE_FREQUENCY 1193180

inline void pit_set_frequency(uint16_t hz) {
    uint32_t divisor = PIT_BASE_FREQUENCY / hz;
    
    outb(PIT_COMMAND, 0x36);
    io_wait();
    
    uint8_t l = (uint8_t)(divisor & 0xFF);
    outb(PIT_DATA, l);
    io_wait();

    uint8_t h = (uint8_t)((divisor >> 8) & 0xFF);
    outb(PIT_DATA, h);
    io_wait();
}

extern "C" uint64_t g_timer_ticks; 

extern "C" void timer_handler_c() {
    g_timer_ticks++;
    //pic_send_eoi(0);

}
extern "C" void keyboard_handler_c() {
    uint8_t status = inb(0x64);

    // Wir lesen ALLES aus, was gerade im Hardware-Puffer liegt
    while (status & 0x01) {
        uint8_t data = inb(0x60);

        if (status & 0x20) {
            // Hoppla! Das ist ein Maus-Byte, obwohl wir im Keyboard-IRQ sind
            PS2Mouse::process_byte(data);
        } else {
            // Echtes Tastatur-Byte
            kbd_buffer_push(data);
        }
        status = inb(0x64); // Status neu prüfen
    }

    pic_send_eoi(1);
}

extern "C" void mouse_handler_c() {
    PS2Mouse::handle_interrupt();
    pic_send_eoi(12);
}

extern "C" void ps2_dispatcher_c() {
    uint8_t status = inb(0x64);
    
    // Solange der Output-Buffer des 8042 voll ist
    while (status & 0x01) {
        uint8_t data = inb(0x60);
        
        if (status & 0x20) {
            // Bit 5 gesetzt -> Es ist definitiv ein MAUS-Byte
            PS2Mouse::process_byte(data);
        } else {
            // Bit 5 nicht gesetzt -> Es ist ein TASTATUR-Byte
            kbd_buffer_push(data);
        }
        
        // Status erneut lesen, falls mehrere Bytes warten
        status = inb(0x64);
    }
}

#endif // SYS_INTERRUPTS_H