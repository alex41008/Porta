// systemprocess/sys_process.cpp
#include "../systemh/sys_process.h"
#include <stdint.h>

void k_strncpy(char* dest, const char* src, size_t n) {
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++)
        dest[i] = src[i];
    for (; i < n; i++)
        dest[i] = '\0';
}

extern "C" void launch_process(SystemProcess* proc) {
    proc->execute();
    proc->terminate();
}

SystemProcess::SystemProcess(uint8_t id, const char* name, uint8_t* stack, size_t stack_s) 
    : process_id(id), stack_base(stack), stack_size(stack_s), state(READY)
{
    k_strncpy(process_name, name, sizeof(process_name) - 1);

    uint32_t* st = (uint32_t*)(stack_base + stack_size);
    
    *(--st) = 0x202;    
    *(--st) = 0x08;            
    *(--st) = (uint32_t)launch_process;

    *(--st) = (uint32_t)this;    
    *(--st) = 0;                  
    *(--st) = 0;                    
    *(--st) = 0;                 
    *(--st) = 0;               
    *(--st) = 0;              
    *(--st) = 0;                    
    *(--st) = 0;                     

    this->esp = (uint32_t)st;
    this->ebp = (uint32_t)(stack_base + stack_size);
}
void SystemProcess::save_context(uint32_t current_esp) {
    this->esp = current_esp;
    this->state = READY;
}

uint32_t SystemProcess::load_context() {
    this->state = RUNNING;
    return this->esp;
}

