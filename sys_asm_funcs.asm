section .text
    global idt_flush
    global timer_handler_asm

idt_flush:
    mov eax, [esp+4]
    lidt [eax]   
    ret

extern timer_handler_c

global timer_handler_asm
timer_handler_asm:
    pusha
    
    ; Rufe die C-Funktion auf, die g_timer_ticks inkrementiert
    extern timer_handler_c
    call timer_handler_c
    
    ; WICHTIG: EOI (End-of-Interrupt) an den Master PIC senden
    mov al, 0x20
    out 0x20, al 

    popa
    iret

global irq1_handler_asm
irq1_handler_asm:
    pusha
    
    ; 1. Scancode vom Port 0x60 lesen (entleert den Hardware-Puffer)
    in al, 0x60
    push eax ; Scancode auf den Stack legen (als Argument für die C-Funktion)
    
    ; 2. Rufe die C-Funktion auf, die den Scancode in den globalen Puffer speichert
    extern kbd_buffer_push
    call kbd_buffer_push
    
    pop eax ; Argument vom Stack entfernen
    
    ; 3. EOI senden
    mov al, 0x20
    out 0x20, al 

    popa
    iret

global default_interrupt_handler
default_interrupt_handler:
    pusha
    
    ; Optional: Debug-Code hier (wenn Sie wissen, welcher Vektor gecrashed ist)
    
    ; Wichtig: KEIN CLI/HLT, um Blockade zu vermeiden!
    
    ; EOI an den PIC senden, falls es ein IRQ war (nicht notwendig für Exceptions)
    ; mov al, 0x20
    ; out 0x20, al 

    popa
    iret ; SICHERE RÜCKKEHR