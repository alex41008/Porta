section .text
    global idt_flush
    global timer_handler_asm

idt_flush:
    mov eax, [esp+4]
    lidt [eax]   
    ret

extern timer_handler_c

extern schedule

timer_handler_asm:
    pusha

    mov ax, ds
    push eax
    mov ax, 0x10
    mov ds, ax
    mov es, ax


    call timer_handler_c 


    mov al, 0x20
    out 0x20, al 

  
    push esp
    call schedule    
    mov esp, eax

  
    pop eax
    mov ds, ax
    mov es, ax
    
    popa
    iret

global irq1_handler_asm
extern keyboard_handler_c
extern ps2_dispatcher_c

irq1_handler_asm:
    pusha
    push ds
    push es
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    call ps2_dispatcher_c  

    mov al, 0x20
    out 0x20, al          
    
    pop es
    pop ds
    popa
    iret

global irq12_handler_asm
extern mouse_handler_c

irq12_handler_asm:
    pusha
    push ds
    push es
    mov ax, 0x10
    mov ds, ax
    mov es, ax

    call ps2_dispatcher_c  

    mov al, 0x20          
    out 0xA0, al
    out 0x20, al
    
    pop es
    pop ds
    popa
    iret

global default_interrupt_handler
default_interrupt_handler:
    pusha

    popa
    iret ; SICHERE RÜCKKEHR