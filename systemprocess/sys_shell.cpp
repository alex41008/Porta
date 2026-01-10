#include <stdint.h>
#include "../systemh/sys_window.h"
#include "../systemh/sys_cli_task.h"
#include "../systemh/sys_vga13h_screen.h"
#include "../systemh/sys_process.h"
#include "../systemh/sys_definition_list.h"

class Shell : public SystemProcess {
private:
    Window* window_;
    char shell_history[10][40];
    int cursor_row = 0;

public:
    Shell(uint8_t id, uint8_t* stack, size_t stack_s, Window* win) 
        : SystemProcess(id, "Shell", stack, stack_s), window_(win) {}

    void execute_windowed() {
        window_->write_content_text(0, cursor_row * 10, "Porta-Shell $> ", VGA_COLOR_LIGHT_GREEN);
    }
};