#include "../systemh/sys_window.h"
#include "../systemh/sys_definition_list.h"
#include "../systemh/sys_vga13h_screen.h"

#define MAX_WINDOWS 10
Window* windows[MAX_WINDOWS];
int window_count = 0;

void register_window(Window* w) {
    if (window_count < MAX_WINDOWS) {
        windows[window_count++] = w;
    }
}

void Window::draw() {
    clear_content(VGA_COLOR_BLACK);
    draw_border(1, 9);
    draw_title_bar(9);
    //draw_text(x_ + 4, y_ + 20, "Hello, World!", VGA_COLOR_LIGHT_GREEN);
}


void Window::draw_border(uint8_t thickness, uint8_t color) {
    for (int i = x_; i < x_ + width_; i++) {
        put_pixel(i, y_, color);                 
        put_pixel(i, y_ + height_ - thickness, color);    
    }

    for (int j = y_; j < y_ + height_; j++) {
        put_pixel(x_, j, color);                 
        put_pixel(x_ + width_ - thickness, j, color);     
    }
}

void Window::clear_content(uint8_t color) {
    for (int i = x_ + 1; i < x_ + width_ - 1; i++) {
        for (int j = y_ + 13; j < y_ + height_ - 1; j++) {
             put_pixel(i, j, color);
        }
    }
}

bool Window::is_over_title_bar(int mouse_x, int mouse_y) {
    return (mouse_x >= x_ && mouse_x < (x_ + width_) &&
            mouse_y >= y_ && mouse_y < (y_ + 12));
}

void Window::draw_title_bar(uint8_t color) {
    for (int i = x_; i < x_ + width_; i++) {
        for (int h = 0; h < 13; h++) {
             put_pixel(i, y_ + h, color);
        }
    }
    
    if (this->title_ != nullptr) {
        font_.draw_string(this->x_ + 4, this->y_ + 2, this->title_, 7);
    } else {
        font_.draw_string(this->x_ + 4, this->y_ + 2, "NULL POINTER", 15);
    }
}

void Window::move(int new_x, int new_y) {
    this->x_ = new_x;
    this->y_ = new_y;
}