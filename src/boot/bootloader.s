[bits 16]
[org 0x7C00]

cli
xor ax, ax
mov es, ax
mov ds, ax

mov si, dap
mov ah, 42H
int 13h

jc err

jmp 00:9000h

err:
    mov ah, 0Eh
    mov si, str_t
.print_char:
    lodsb
    cmp al, 0
    je .hang
    int 0x10
    jmp .print_char

.hang:
    hlt
    jmp .hang

str_t:
    db "", 0

dap:
    db 10h
    db 0
    dw KERNEL_SECTORS
    dd 0x9000
    dq 1

times 510 - ($ - $$) db 0
dw 0xAA55