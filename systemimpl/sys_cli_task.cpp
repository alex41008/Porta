// systemimpl/cli_task.cpp
#include "../systemh/sys_cli_task.h"
#include "../systemh/sys_input_output.h"
#include "../systemh/sys_vga13h_screen.h"
#include "../systemh/sys_vga_screen.h"
#include "../systemh/sys_window.h"
#include "../systemh/sys_ps2_mouse_driver.h"

static FontEngine font;

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
    vga_set_mode_13h();
    PS2Mouse::init(); 

    Window test_window(40, 30, 180, 100, "WINDOW", font);
    Window test_window_2(70, 80, 130, 70, "SHELL", font);
    register_window(&test_window);
    register_window(&test_window_2);
    uint8_t CURSOR_COLOR = 7;
    Window* dragged_window = nullptr;
    int offset_x = 0, offset_y = 0;
    bool in_gui = true;
    int last_mx = 160, last_my = 100;
    bool needs_redraw = true;

    cls();

    while (in_gui) {
        
        *((uint8_t*)0xA0000 + 0) = (uint8_t)(kbd_write_ptr % 10) + 48;
        *((uint8_t*)0xA0000 + 2) = (uint8_t)(kbd_read_ptr % 10) + 48;
        uint8_t sc;
            if (sc == 0x01) {
                in_gui = false;
            }

        int mx = PS2Mouse::get_x();
        int my = PS2Mouse::get_y();

        if (mx != last_mx || my != last_my || dragged_window != nullptr || needs_redraw) {
        
        draw_pattern_background();
        
        font.draw_string(10, 10, "PORTA 1.01", 3);
        
        for (int i = 0; i < window_count; i++) {
            windows[i]->draw();
        }

        draw_cursor(mx, my, last_mx, last_my, CURSOR_COLOR);
        
        last_mx = mx;
        last_my = my;
        needs_redraw = false;
    }

if (PS2Mouse::left_clicked()) {
    if (dragged_window == nullptr) {
        for (int i = window_count - 1; i >= 0; i--) {
            if (windows[i]->is_over_title_bar(mx, my)) {
                dragged_window = windows[i];
                offset_x = mx - dragged_window->get_x();
                offset_y = my - dragged_window->get_y();
                
                // TODO: Z-Order
                break; 
            }
        }
    }

    if (dragged_window != nullptr) {
        dragged_window->move(mx - offset_x, my - offset_y);
    }
} else {
    dragged_window = nullptr;
}
        //for(volatile int i = 0; i < 50000; i++);
        asm volatile("pause");
    }

    // TODO: vga_set_mode_3(); 
    cls();      
    print_string("Zurueck im Porta-Shell...\n");
    }
    }
}

void CLI_Task::terminate() {
    print_string("\nShell stopped. System awaiting further instructions...\n");
    while(1); 
}