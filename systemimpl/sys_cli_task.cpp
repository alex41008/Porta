// systemimpl/cli_task.cpp
#include "../systemh/sys_cli_task.h"
#include "../systemh/sys_input_output.h"
#include "../systemh/sys_vga13h_screen.h"
#include "../systemh/sys_vga_screen.h"
#include "../systemh/sys_window.h"
#include "../systemh/sys_ps2_mouse_driver.h"


#define CURSOR_COLOR 7
static FontEngine font;

void CLI_Task::execute() {
    Window test_window(40, 30, 180, 100, "WINDOW", font);
    Window test_window_2(70, 80, 130, 70, "SHELL", font);

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

    bool in_gui = true;
    int last_mx = 160, last_my = 100;
    static bool is_dragging = false;
    static bool is_dragging_2 = false;
static int offset_x = 0;
static int offset_y = 0;


    cls();
    test_window.draw();

    while (in_gui) {
 
        *((uint8_t*)0xA0000 + 0) = (uint8_t)(kbd_write_ptr % 10) + 48;
        *((uint8_t*)0xA0000 + 2) = (uint8_t)(kbd_read_ptr % 10) + 48;
        
        uint8_t sc;
            if (sc == 0x01) {
                in_gui = false;
                put_pixel(0, 0, 4);
                put_pixel(1, 0, 4);
                put_pixel(2, 0, 4);
                put_pixel(3, 0, 4);
                put_pixel(4, 0, 4);
                put_pixel(5, 0, 4);
                put_pixel(6, 0, 4);
            }

        int mx = PS2Mouse::get_x();
        int my = PS2Mouse::get_y();

        if (mx != last_mx || my != last_my) {

            for(int ox = 0; ox <= 10; ox++) {
                for(int oy = 0; oy <= 16; oy++) {
                    put_pixel(last_mx + ox, last_my + oy, 0);
                }
            }


            clear_screen();
            font.draw_string(10, 10, "PORTA 1.01", 3);
            test_window.draw(); 
            test_window_2.draw(); 

            put_pixel(mx, my + 1, CURSOR_COLOR);
            put_pixel(mx, my + 2, CURSOR_COLOR);
            put_pixel(mx, my + 3, CURSOR_COLOR);
            put_pixel(mx, my + 4, CURSOR_COLOR);
            put_pixel(mx, my + 5, CURSOR_COLOR);
            put_pixel(mx, my + 6, CURSOR_COLOR);
            put_pixel(mx, my + 7, CURSOR_COLOR);
            put_pixel(mx, my + 8, CURSOR_COLOR);
            put_pixel(mx, my + 9, CURSOR_COLOR);
            put_pixel(mx, my + 10, CURSOR_COLOR);
            put_pixel(mx, my + 11, CURSOR_COLOR);
            put_pixel(mx, my + 12, CURSOR_COLOR);
            put_pixel(mx+1, my + 12, CURSOR_COLOR);
            put_pixel(mx+2, my + 11, CURSOR_COLOR);
            put_pixel(mx+3, my + 10, CURSOR_COLOR);
            put_pixel(mx+4, my + 11, CURSOR_COLOR);
            put_pixel(mx+4, my + 12, CURSOR_COLOR);
            put_pixel(mx+5, my + 13, CURSOR_COLOR);
            put_pixel(mx+5, my + 14, CURSOR_COLOR);
            put_pixel(mx+6, my + 15, CURSOR_COLOR);
            put_pixel(mx+7, my + 15, CURSOR_COLOR);
            put_pixel(mx+8, my + 14, CURSOR_COLOR);
            put_pixel(mx+8, my + 13, CURSOR_COLOR);
            put_pixel(mx+7, my + 12, CURSOR_COLOR);
            put_pixel(mx+7, my + 11, CURSOR_COLOR);
            put_pixel(mx+6, my + 10, CURSOR_COLOR);
            put_pixel(mx+6, my + 9, CURSOR_COLOR);
            put_pixel(mx+7, my + 9, CURSOR_COLOR);
            put_pixel(mx+8, my + 9, CURSOR_COLOR);
            put_pixel(mx+9, my + 9, CURSOR_COLOR);

            put_pixel(mx + 1, my, CURSOR_COLOR);
            put_pixel(mx + 2, my + 1, CURSOR_COLOR);
            put_pixel(mx + 3, my + 2, CURSOR_COLOR);
            put_pixel(mx + 4, my + 3, CURSOR_COLOR);
            put_pixel(mx + 5, my + 4, CURSOR_COLOR);
            put_pixel(mx + 6, my + 5, CURSOR_COLOR);
            put_pixel(mx + 7, my + 6, CURSOR_COLOR);
            put_pixel(mx + 8, my + 7, CURSOR_COLOR);
            put_pixel(mx + 9, my + 8, CURSOR_COLOR);

            last_mx = mx;
            last_my = my;
        }

        if (PS2Mouse::left_clicked()) {
    if (!is_dragging) {
        if (test_window.is_over_title_bar(mx, my)) {
            is_dragging = true;
            offset_x = mx - test_window.get_x();
            offset_y = my - test_window.get_y();
        }
        if (test_window_2.is_over_title_bar(mx, my)) {
            is_dragging_2 = true;
            offset_x = mx - test_window_2.get_x();
            offset_y = my - test_window_2.get_y();
        }
    }

    if (is_dragging) {
        test_window.move(mx - offset_x, my - offset_y);
    }
} else {
    is_dragging = false;
}
    if (is_dragging_2) {
        test_window_2.move(mx - offset_x, my - offset_y);
    }
 else {
    is_dragging_2 = false;
}

        asm volatile("pause");
    }

    // vga_set_mode_3(); 
    cls();      
    print_string("Zurueck im Porta-Shell...\n");
    }
    }
}

void drawCursor() {
    
}

void CLI_Task::terminate() {
    print_string("\nShell stopped. System awaiting further instructions...\n");
    while(1); 
}