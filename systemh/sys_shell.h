#pragma once 
#include "../systemh/sys_process.h"
#include "../systemh/sys_window.h"

class Shell : public SystemProcess {
public:
    Shell(uint8_t id, uint8_t* stack, size_t stack_s, Window* win) 
        : SystemProcess(id, "Shell", stack, stack_s), window_(win), input_index(0) {
        input_buffer[0] = '\0';
    } 

    void execute() override;
    void terminate() override;
    void add_char(char c);
    void backspace();
    const char* get_buffer();
    int get_input_length();

private:
    Window* window_;
    char input_buffer[64]; 
    int input_index;       
};