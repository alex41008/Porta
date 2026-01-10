#ifndef SYS_WINDOW_H
#define SYS_WINDOW_H

#include <stdint.h>
#include "sys_font.h"


class Window {
    private:
    int cursor_x_ = 0;
    int cursor_y_ = 0;
    bool show_cursor_ = true;
    int x_, y_;
    int width_, height_;
    const char* title_;
    FontEngine& font_;
    
    void draw_border(uint8_t thickness, uint8_t color);
    void draw_title_bar(uint8_t color);
    void clear_content(uint8_t color);
    void draw_text(int x, int y, const char* text, uint8_t color);
    
    public:
    Window(int x, int y, int width, int height, const char* title, FontEngine& font) : font_(font)
    {
        this->x_ = x;
        this->y_ = y;
        this->width_ = width;
        this->height_ = height;
        this->title_ = title;
    }
    
    ~Window() {}
    
    void draw();
    void move(int new_x, int new_y);
    bool is_over_title_bar(int mouse_x, int mouse_y);
    void put_pixel_in_window(int rel_x, int rel_y, uint8_t color);
    void write_content_text(int relative_x, int relative_y, const char* text, uint8_t color);
    void set_cursor_pos(int rel_x, int rel_y) {cursor_x_ = rel_x; cursor_y_ = rel_y;}
    void draw_cursor(bool blink_state);
    bool is_inside_content(int mx, int my);
    
    int get_x() { return x_; }
    int get_y() { return y_; }
    
    
};

#define MAX_WINDOWS 10
extern Window* windows[MAX_WINDOWS];
extern int window_count;

void register_window(Window* w);

#endif // SYS_WINDOW_H