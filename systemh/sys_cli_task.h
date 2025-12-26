#ifndef SYSTEMH_SYS_CLI_TASK_H
#define SYSTEMH_SYS_CLI_TASK_H

#pragma once

#include "sys_process.h"
#include "sys_font.h"

class CLI_Task : public SystemProcess {
public:
    CLI_Task(uint8_t id, uint8_t* stack, size_t stack_s) 
        : SystemProcess(id, "Shell", stack, stack_s) {}

    void execute() override;

    void terminate() override;
};

#endif // SYSTEMH_SYS_CLI_TASK_H