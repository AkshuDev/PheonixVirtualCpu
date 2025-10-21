%include "fileio.inc"

section .data
    stat_struct: db 144

section .text
    global open_file
    global close_file
    global write_file
    global read_file
    global get_fsize

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

get_fsize: ; rsi -> char* filename -> rax (file size)
    mov rax, SYS_newfstatat
    mov rdi, AT_FDCWD
    lea rdx, [rel stat_struct]
    xor r10, r10
    syscall

    cmp rax, 0
    js .err
    mov rax, [stat_struct + 48]
    ret
.err:
    ret