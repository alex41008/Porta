#ifndef SYS_INTERRUPTS_H
#define SYS_INTERRUPTS_H

#include <stdint.h>
#include "sys_io.h" 

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
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
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

    outb(PIC1_DATA, a1); 
    outb(PIC2_DATA, a2); 
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

#endif // SYS_INTERRUPTS_H