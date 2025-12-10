volatile uint16_t *vga_buffer = (uint16_t *)0xB8000;
uint16_t cursor_x, cursor_y;

void scroll_screen(volatile uint16_t *vga_buffer)
{
    for (uint16_t y = 0; y < VGA_HEIGHT - 1; y++)
    {
        for (uint16_t x = 0; x < VGA_WIDTH; x++)
        {
            vga_buffer[y * VGA_WIDTH + x] = vga_buffer[(y + 1) * VGA_WIDTH + x];
        }
    }
    for (uint16_t x = 0; x < VGA_WIDTH; x++)
    {
        vga_buffer[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = (VGA_COLOR << 8) | ' ';
    }
}

void cls()
{
    const uint16_t color_attribute = (VGA_COLOR_BLACK << 4) | VGA_COLOR_LIGHT_GREY;

    const uint16_t blank = (color_attribute << 8) | ' '; 

    for (uint16_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
    {
        vga_buffer[i] = blank;
    }
    cursor_x = 0;
    cursor_y = 0;
}