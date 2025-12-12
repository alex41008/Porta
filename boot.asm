; Bootloader 32-bit

section .multiboot
align 4
    ; Multiboot header
    dd 0x1BADB002         
    dd 0x00000003                
    dd -(0x1BADB002+0x00000003) 

section .data
align 4
mboot_info_ptr:
    dd 0                         

section .bss
align 16
stack_bottom:
    resb 16384                   
stack_top:

section .text
global _start
global mboot_info_ptr           
extern kernel_main

_start:
    mov [mboot_info_ptr], ebx
    
    mov esp, stack_top
    
    cli

    lgdt [gdt_descriptor]

    jmp 0x08:reload_segments     
    
reload_segments:
    mov ax, 0x10               
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    in al, 0x92
    or al, 2
    out 0x92, al

    push mboot_info_ptr          
    call kernel_main
    
hang:
    cli                      
    hlt                       
    jmp hang                  

align 8
gdt:
    dq 0

    dw 0xFFFF     
    dw 0          
    db 0           
    db 10011010b   
    db 11001111b    
    db 0           

    dw 0xFFFF       
    dw 0          
    db 0           
    db 10010010b   
    db 11001111b   
    db 0           

gdt_descriptor:
    dw gdt_descriptor - gdt - 1
    dd gdt                     