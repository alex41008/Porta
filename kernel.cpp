#include <stdint.h>

#include "systemh/sys_definition_list.h"
#include "systemh/sys_vec.h"
#include "systemh/sys_mem.h"
#include "systemh/sys_vga13h_screen.h"
#include "systemh/sys_vga_screen.h"
#include "systemh/sys_io.h"
#include "systemh/sys_input_output.h"
#include "systemh/sys_cli_action_list.h"
#include "systemh/sys_cli_task.h"
#include "systemh/sys_clock_task.h"
#include "systemh/sys_scheduler.h"
#include "systemh/sys_font.h"

uint64_t g_timer_ticks = 0;
uint64_t last_tick = 0;
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
uint16_t cursor_x = 0;
uint16_t cursor_y = 0;
kernel_program krnl;

uint8_t shell_stack[4096];
uint8_t clock_stack[4096];
uint8_t gui_stack[8192];

CLI_Task shell(1, shell_stack, 4096);
Clock_Task clock_task(2, clock_stack, 4096);

extern "C" int __cxa_guard_acquire(long long int *g) { return 0; }
extern "C" void __cxa_guard_release(long long int *g) { (void)g; }
extern "C" {
    void* __dso_handle = (void*) &__dso_handle;

    int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {
        return 0;
    }

    void operator delete(void* ptr, unsigned int size) {
    }
    
    void operator delete(void* ptr) {
    }
}

extern "C" void kernel_main(multiboot_info *mbi)
{ 
    cls(); 
    
    idt_init();       
    pic_remap();       
    //PS2Mouse::init();  
    pit_set_frequency(100);

    asm volatile("sti");

    krnl.k_init_cli(); 

    shell.execute(); 

    while (true) {
        asm volatile("hlt");
    }
}
