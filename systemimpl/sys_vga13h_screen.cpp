#include "../systemh/sys_vga13h_screen.h"
#include "../systemh/sys_cli_task.h"
#include "../systemh/sys_input_output.h"
#include "../systemh/sys_vga13h_screen.h"
#include "../systemh/sys_vga_screen.h"
#include "../systemh/sys_window.h"
#include "../systemh/sys_ps2_mouse_driver.h"
#include "../systemh/sys_shell.h"

uint8_t backbuffer[320 * 200]; 
static FontEngine font;

void put_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= 320 || y < 0 || y >= 200) return;
    backbuffer[y * 320 + x] = color;
}

void vga_flip() {
    uint32_t* vga = (uint32_t*)0xA0000;
    uint32_t* src = (uint32_t*)backbuffer;
    for (int i = 0; i < 16000; i++) {
        vga[i] = src[i];
    }
}
void run_interface_vga13h() {
    vga_set_mode_13h();
    PS2Mouse::init(); 

    Window shell_window(50, 30, 180, 100, "SHELL", font);
    register_window(&shell_window);
    Shell shell(1, nullptr, 0, &shell_window);

    uint8_t CURSOR_COLOR = 7;
    Window* dragged_window = nullptr;
    int offset_x = 0, offset_y = 0;
    bool in_gui = true;
    int last_mx = 160, last_my = 100;
    bool needs_redraw = true;
    uint32_t frame_counter = 0;
    bool cursor_visible = true;
    uint8_t cursor_type = 0;

    cls();

    while (in_gui) {

        frame_counter++;

        if (frame_counter % 300 == 0) {
            cursor_visible = !cursor_visible;
        }
        
        *((uint8_t*)0xA0000 + 0) = (uint8_t)(kbd_write_ptr % 10) + 48;
        *((uint8_t*)0xA0000 + 2) = (uint8_t)(kbd_read_ptr % 10) + 48;

        int mx = PS2Mouse::get_x();
        int my = PS2Mouse::get_y();

        
        draw_pattern_background();
        
        font.draw_string(10, 10, "PORTA", VGA_COLOR_BLACK);
        
        uint8_t sc = scankey();
if (sc != 0 && !(sc & 0x80)) { // 0x80 prüft ob Taste gedrückt (nicht losgelassen)
    char c = scancode_to_ascii(sc);
    
    if (c == '\b') {
        shell.backspace();
    } else if (c == '\n') {
        // Hier könntest du shell.process_command() aufrufen
        // Für jetzt: Einfach Buffer leeren oder ignorieren
    } else if (c >= 32 && c <= 126) {
        shell.add_char(c);
    }
}

        for (int i = 0; i < window_count; i++) {
        windows[i]->draw();
        
        if (windows[i] == &shell_window) {
             windows[i]->write_content_text(0, 0, "PORTA ROOT SHELL", 10);
             windows[i]->write_content_text(0, 10, "$> ", 15);
             
             windows[i]->set_cursor_pos(24, 10);
             windows[i]->draw_cursor(cursor_visible);
        }
    }
        if(cursor_type == 0) {
            draw_cursor(mx, my, last_mx, last_my, CURSOR_COLOR);
        } else {
            draw_text_cursor(mx, my, last_mx, last_my, CURSOR_COLOR);
        }
        
        vga_flip();
        last_mx = mx;
        last_my = my;
        needs_redraw = false;

if (PS2Mouse::left_clicked()) {
    if (dragged_window == nullptr) {
        for (int i = window_count - 1; i >= 0; i--) {
            if (windows[i]->is_over_title_bar(mx, my)) {
                dragged_window = windows[i];
    
            for (int j = i; j < window_count - 1; j++) {
                windows[j] = windows[j + 1];
            }
            windows[window_count - 1] = dragged_window;
    
            offset_x = mx - dragged_window->get_x();
            offset_y = my - dragged_window->get_y();
            cursor_type = 0;
            break;
            }
        }
    }

    if (dragged_window != nullptr) {
        dragged_window->move(mx - offset_x, my - offset_y);
    }

    if (shell_window.is_inside_content(mx, my)) {

        int rel_x = mx - shell_window.get_x() - 4;
        int rel_y = my - shell_window.get_y() - 14;

        int grid_x = (rel_x / 8) * 8;
        int grid_y = (rel_y / 8) * 8;

        shell_window.set_cursor_pos(grid_x, grid_y);
        cursor_type = 1;
        }
} else {
    dragged_window = nullptr;
}
        //for(volatile int i = 0; i < 50000; i++);
        asm volatile("pause");
    }
}