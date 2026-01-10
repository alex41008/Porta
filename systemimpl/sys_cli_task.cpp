// systemimpl/cli_task.cpp
#include "../systemh/sys_cli_task.h"
#include "../systemh/sys_input_output.h"
#include "../systemh/sys_vga13h_screen.h"
#include "../systemh/sys_vga_screen.h"
#include "../systemh/sys_window.h"
#include "../systemh/sys_ps2_mouse_driver.h"

void CLI_Task::execute() {

    while (true) {
        print_string("\nPorta-Shell $> ");

        char command_buffer[64];
        for(int i=0; i<64; i++) command_buffer[i] = 0; 
        int index = 0;
        bool command_ready = false;

        while (!command_ready) {
            uint8_t scancode = 0;
            while((scancode = scankey()) == 0) {
                asm volatile("pause"); 
            }

            char c = scancode_to_ascii(scancode);

            if (c == 0) continue;

            if (c == '\n') {
                command_buffer[index] = '\0';
                command_ready = true;
                print_char('\n');
            } 
            else if (c == '\b' && index > 0) {
                index--;                    
                command_buffer[index] = 0;  

                print_char('\b'); 
                print_char(' '); 
                print_char('\b');
            }
            else if (index < 63) {
                command_buffer[index++] = c;
                print_char(c);
            }
        }

        if (str_cmp(command_buffer, "help")) {
            print_string("\nCommands: help, cls, gui, info");
        } 
        else if (str_cmp(command_buffer, "cls")) {
            cls();
        }
        else if (str_cmp(command_buffer, "info")) {
            print_string("\nPorta - Version 0.1 (Process-based)");
        }
       else if (str_cmp(command_buffer, "gui")) {
            //run_interface_vga13h();
        }
        else {
            print_string("\nUnknown command: ");
            print_string(command_buffer);
        }
    }
}

void CLI_Task::execute_windowed(FontEngine& font) {
    Window shell_window(70, 80, 130, 70, "SHELL", font);
    register_window(&shell_window);
}

void CLI_Task::terminate() {
    print_string("\nShell stopped. System awaiting further instructions...\n");
    while(1); 
}