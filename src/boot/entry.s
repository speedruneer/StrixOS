[bits 16]
global entry_start
extern vesa_mode_info
extern font
extern base_font

entry_start:

times 16 nop

mov bx, 0x411B
mov ax, 0x4F01
mov di, 0x8000
mov cx, bx
int 10h

mov ax, 0x4F02
int 10h

mov di, base_font
push    ds
push    es
mov	    ax, 1130h
mov	    bh, 6
int	    10h
push    es
pop	    ds
pop	    es
mov	    si, bp
mov	    cx, 256*16/4
rep     movsd
pop	    ds

lgdt [gdt_descriptor]

mov eax, cr0
or  eax, 1
mov cr0, eax

jmp 0x08:pm_start

gdt_start:
    dq 0
    dd 0x0000FFFF
    dw 1001101000000000b
    dw 11001111b
    dd 0x0000FFFF
    dw 1001001000000000b
    dw 11001111b ; this is held up by hopes and dreams

gdt_descriptor:
    dw gdt_descriptor - gdt_start - 1
    dd gdt_start

[bits 32]
extern main
pm_start:
    mov al, 0
    mov dx, 0x3F2
    out dx, al
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x8FFF    ; setup stack somewhere unsafe (the whole code is loaded at 0x9000 btw)

    mov ecx, 50
.loop:
    mov al, [0x8000+ecx]
    mov [vesa_mode_info+ecx], al
    loop .loop

    call main

global load_idt

section .text
load_idt:
    mov eax, [esp + 4]
    lidt [eax]
    sti
    ret

global isr_stub

isr_stub:
    pushad
    popad
    iret