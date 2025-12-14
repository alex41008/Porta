#include <stdint.h>

#include "systemh/sys_definition_list.h"
#include "systemh/sys_vec.h"
#include "systemh/sys_mem.h"
#include "systemh/sys_vga13h_screen.h"
#include "systemh/sys_vga_screen.h"
#include "systemh/sys_io.h"
#include "systemh/sys_input_output.h"
#include "systemh/sys_cli_action_list.h"

uint64_t g_timer_ticks = 0;
uint64_t last_tick = 0;
volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
uint16_t cursor_x = 0;
uint16_t cursor_y = 0;
kernel_program krnl;

extern "C" void kernel_main(multiboot_info *mbi)
{ 
    cls();
    krnl.k_init_cli();
    
    while (true)
    {

        asm volatile("hlt");
        vector<char> i;
        krnl.k_input(i);
        krnl.k_clear(i);
        krnl.k_help(i);
        krnl.k_show_files(i);
        krnl.k_enter_gui(i);
        krnl.k_draw_test(i);
        krnl.k_file_dialogue_open(i);
        krnl.k_disktest(i);
        krnl.k_readmbr(i);
        krnl.k_info(i, mbi);
        if (g_timer_ticks != last_tick) {
            asm volatile("cli"); 
            
            cursor_x = 0;
            cursor_y = 5;
            print_string("Ticks: ");
            print_int(g_timer_ticks, VGA_COLOR_LIGHT_GREY);
            last_tick = g_timer_ticks;
            
            asm volatile("sti"); 
        }
    }
    
}