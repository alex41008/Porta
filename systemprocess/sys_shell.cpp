#include "../systemh/sys_shell.h"

#define SHELL_BUFFER_MAX 64


void Shell::add_char(char c) {
    if (input_index < SHELL_BUFFER_MAX - 1) {
        input_buffer[input_index] = c;
        input_index++;
        input_buffer[input_index] = '\0';
    }
}

void Shell::backspace() {
    if (input_index > 0) {
        input_index--;
        input_buffer[input_index] = '\0';
    }
}

const char* Shell::get_buffer() {
    return input_buffer;
}

int Shell::get_input_length() {
    return input_index;
}

void Shell::execute() {

}

void Shell::terminate() {

}