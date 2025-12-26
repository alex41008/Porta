// systemh/sys_process.h
#pragma once

#include <stdint.h>
#include <stddef.h> 

enum ProcessState {
    READY,  
    RUNNING,       
    BLOCKED,     
    TERMINATED    
};

struct InterruptContext {
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed));

class SystemProcess {
public:

    SystemProcess(uint8_t id, const char* name, uint8_t* stack, size_t stack_s);

    virtual void execute() = 0;
    virtual void terminate() = 0;
    inline virtual ~SystemProcess() {}

    void save_context(uint32_t current_esp);
    uint32_t load_context();

protected:
    uint8_t process_id;
    char process_name[32];

    uint8_t* stack_base;  
    size_t stack_size;
    
    uint32_t esp;     
    uint32_t ebp;     
    uint32_t eip;    
    ProcessState state;
};

// systemh/sys_process.h