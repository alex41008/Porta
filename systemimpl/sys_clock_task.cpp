#include "../systemh/sys_cli_task.h"
#include "../systemh/sys_clock_task.h"
#include "../systemh/sys_input_output.h"

extern uint64_t g_timer_ticks; 

Clock_Task::Clock_Task(uint8_t id, uint8_t* stack, size_t stack_s) 
    : SystemProcess(id, "Clock", stack, stack_s) 
{
}

void Clock_Task::execute() {
    const char* hex_chars = "0123456789ABCDEF";
    volatile uint16_t* vga = (volatile uint16_t*)0xB8000;

    while (true) {
        uint32_t ticks = (uint32_t)g_timer_ticks;

        for (int i = 0; i < 8; i++) {
            uint8_t nibble = (ticks >> (i * 4)) & 0x0F;
            vga[79 - i] = (0x0E << 8) | hex_chars[nibble];
        }

        for(volatile int i = 0; i < 1000000; i++); 
    }
}

void Clock_Task::terminate() {

}