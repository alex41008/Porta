#ifndef SYSTEMH_SYS_CLOCK_TASK_H
#define SYSTEMH_SYS_CLOCK_TASK_H

#pragma once

#include "sys_process.h"

class Clock_Task : public SystemProcess {
public:
    Clock_Task(uint8_t id, uint8_t* stack, size_t stack_s);

    void execute() override;

    void terminate() override;
};

#endif // SYSTEMH_SYS_CLOCK_TASK_H