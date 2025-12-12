#ifndef SYS_IDT_H
#define SYS_IDT_H

#include <stdint.h>
#include "sys_io.h" 

struct IDTEntry {
    uint16_t base_low;  
    uint16_t selector;  
    uint8_t zero;        
    uint8_t flags;        
    uint16_t base_high;   
} __attribute__((packed)); 

struct IDTPointer {
    uint16_t limit;    
    uint32_t base;        
} __attribute__((packed));

#define IDT_ENTRIES 256
#define KERNEL_CS 0x08

IDTEntry idt[IDT_ENTRIES];
IDTPointer idt_ptr;

inline void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags; 
}

extern "C" void timer_handler_asm(); 
extern "C" void idt_flush(uint32_t);
extern "C" void irq1_handler_asm();
extern "C" void default_interrupt_handler();

inline void idt_init() {
    idt_ptr.limit = (sizeof(IDTEntry) * IDT_ENTRIES) - 1;
    idt_ptr.base = (uint32_t)&idt;

    // 1. Initialisiere alle 256 Einträge (mit dem SICHEREN default_interrupt_handler)
    for (int i = 0; i < IDT_ENTRIES; i++) {
        idt_set_gate(i, (uint32_t)&default_interrupt_handler, KERNEL_CS, 0x8E); // MUSS default_interrupt_handler sein!
    }

    // 2. Überschreibe Timer (INT 32)
    idt_set_gate(PIC_MASTER_OFFSET + 0, (uint32_t)&timer_handler_asm, KERNEL_CS, 0x8E);
    
    // 3. Überschreibe Tastatur (INT 33)
    idt_set_gate(PIC_MASTER_OFFSET + 1, (uint32_t)&irq1_handler_asm, KERNEL_CS, 0x8E);

    // 4. Lade die IDT nur EINMAL und am Ende
    idt_flush((uint32_t)&idt_ptr);
}

#endif // SYS_IDT_H