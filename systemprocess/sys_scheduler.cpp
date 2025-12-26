// systemprocess/sys_scheduler.cpp
#include "../systemh/sys_scheduler.h"
#include "../systemh/sys_process.h"
SystemProcess* processes[10] = {nullptr}; 
int current_task = 0;
int process_count = 0;

extern "C" uint32_t schedule(uint32_t current_esp) {
    if (processes[current_task] == nullptr) {
        return current_esp;
    }
    processes[current_task]->save_context(current_esp);

    current_task = (current_task + 1) % 2; 

    return processes[current_task]->load_context();
}