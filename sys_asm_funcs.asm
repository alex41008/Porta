section .text
    global idt_flush
    global timer_handler_asm

idt_flush:
    mov eax, [esp+4]
    lidt [eax]   
    ret

extern timer_handler_c

timer_handler_asm:
    pusha           
    
    ; WICHTIG: Erlaube verschachtelte Interrupts WÄHREND des C++-Aufrufs (Optional, aber oft notwendig)
    sti ; <-- Füge 'sti' (Set Interrupt Flag) HIER hinzu

    call timer_handler_c 
    
    cli ; <-- Füge 'cli' (Clear Interrupt Flag) HIER hinzu, um den Rücksprung zu sichern

    add esp, 4      
    
    mov al, 0x20
    out 0x20, al    

    popa            
    iret

global irq1_handler_asm
irq1_handler_asm:
    pusha
    
    ; Sende EOI an den Master PIC (IRQs 0-7)
    mov al, 0x20
    out 0x20, al 

    popa
    iret

global default_interrupt_handler
default_interrupt_handler:
    cli
.halt_loop:
    hlt 
    jmp .halt_loop