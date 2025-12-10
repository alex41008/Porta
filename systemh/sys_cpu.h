struct cpuid_result {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
};

cpuid_result cpuid(uint32_t leaf) {
    cpuid_result result;

    asm volatile (
        "cpuid"
        : "=a" (result.eax), "=b" (result.ebx), "=c" (result.ecx), "=d" (result.edx)
        : "a" (leaf)
    );
    return result;
}

void print_cpu_vendor() {
    cpuid_result result = cpuid(0);
    
    char* ebx_chars = (char*)&result.ebx;
    for (int i = 0; i < 4; ++i) {
        print_char(ebx_chars[i]);
    }

    char* edx_chars = (char*)&result.edx;
    for (int i = 0; i < 4; ++i) {
        print_char(edx_chars[i]);
    }
    
    char* ecx_chars = (char*)&result.ecx;
    for (int i = 0; i < 4; ++i) {
        print_char(ecx_chars[i]);
    }
}

void print_cpu_brand() {
    uint32_t leaves[] = {0x80000002, 0x80000003, 0x80000004};

    for (int j = 0; j < 3; ++j) {
        cpuid_result result = cpuid(leaves[j]);
        
        uint32_t* regs[] = {&result.eax, &result.ebx, &result.ecx, &result.edx};

        for (int i = 0; i < 4; ++i) {
            char* chars = (char*)regs[i];

            for (int k = 0; k < 4; ++k) {

                if (chars[k] == '\0') {
                    return;
                }
                print_char(chars[k]);
            }
        }
    }
}