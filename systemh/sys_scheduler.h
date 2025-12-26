#include "sys_process.h"

class Scheduler {
public:
    void add_process(SystemProcess* proc);
    uint32_t schedule(uint32_t current_esp);

private:
};
extern SystemProcess* processes[10];
extern int current_task;
extern int process_count;

// Die C-Schnittstelle für den Assembler
extern "C" uint32_t schedule(uint32_t current_esp);