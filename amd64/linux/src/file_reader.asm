%include "fileio.inc"

section .text
global open_file
global close_file
global write_file
global read_file
global open_input_file

open_file:
    mov rax, SYS_openat
    mov r10, MODE_644
    syscall
    ret

close_file:
    mov rax, SYS_close
    syscall
    ret

write_file:
    mov rax, SYS_write
    syscall
    ret

read_file:
    mov rax, SYS_read
    syscall
    ret

open_input_file:
    ; rdi - file name, rsi - out buff addr, rdx - out fd buff, r10 - count
    ; save
    push rdi
    push rsi
    push rdx
    push r10

    mov rsi, rdi ; arg 2
    mov rdi, AT_FDCWD ; arg 1
    mov rdx, O_RDONLY ; arg 3
    call open_file

    ; fd in rax
    ; restore
    pop r10
    pop rdx
    pop rsi

    mov qword [rdx], rax ; Save fd

    ; Save again
    push rsi
    push rdx
    push r10

    ; read now
    mov rdi, rax
    ; rsi already points to out buff
    mov rdx, r10 ; count
    call read_file

    cmp rax, 1
    jl .open_input_file__unabletoread
    pop r10
    pop rdx
    pop rsi
    pop rdi

    mov rax, 0
    ret
.open_input_file__unabletoread:
    pop r10
    pop rdx
    pop rsi
    pop rdi

    mov rax, 1
    ret