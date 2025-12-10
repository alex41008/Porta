#include <stdint.h>

#include "systemh/sys_definition_list.h"
#include "systemh/sys_vec.h"
#include "systemh/sys_mem.h"
#include "systemh/sys_vga13h_screen.h"
#include "systemh/sys_vga_screen.h"
#include "systemh/sys_io.h"
#include "systemh/sys_input_output.h"
#include "systemh/sys_cli_action_list.h"

kernel_program krnl;

extern "C" void kernel_main(multiboot_info *mbi)
{ 
    cls();
    krnl.k_init_cli();
    //krnl.k_open_file("TEST.TXT");
    
    while (true)
    {
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
    }
}