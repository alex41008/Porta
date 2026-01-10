#pragma once 

class Shell : public SystemProcess {
public:
    Shell(uint8_t id, uint8_t* stack, size_t stack_s, Window* win) 
        : SystemProcess(id, "Shell", stack, stack_s), window_(win) {}     

    void execute() override {

    }

    void terminate() override {

    }

private:
    Window* window_;
};