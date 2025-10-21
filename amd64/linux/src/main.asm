%include "defines.inc"
%include "fileio.inc"
%include "file_reader.inc"
%include "stdio.inc"
%include "memory.inc"

section .bss
    filename: resb 64

section .data
    usage_error: db "Usage: pvcpu <[OPTIONS]> <[ARGS]>", ENDL, STREND
    data_ptr: dq 0
    file_size: dq 0
    fd: dq 0

section .text
    global _start

_start:
    ; Get info
.parse_args:
    ; Parse args
    mov rbx, [rsp] ; argc
    mov rsi, [rsp + 8] ; argv[0]
    cmp rbx, 2
    jl .usage_error
    mov rax, [rsp + 16]
    mov [rel filename], rax ; argv[1]
.con0: ; Continue
    mov rdi, [rel filename]
    call open_file
    mov [rel fd], rax

    lea rsi, [rel filename]
    call get_fsize

    mov [rel file_size], rax

    push rax
    call malloc
    mov [rel data_ptr], rax
    pop rsi

    mov rdi, [rel fd]
    mov rdx, [rel file_size]

    call read_file
    mov rdi, [rel fd]
    call close_file
    mov qword [rel fd], 0

.finish:
    mov rax, [rel file_size]
    mov rdi, 1
    mov rdx, rax
    mov rax, [rel data_ptr]
    call get_fheap
    mov rsi, rax
    call write_file
    push rax
    call free
    pop rax

    mov rdi, 0
    jmp exit
.usage_error:
    lea rdi, usage_error
    push rdi
    call print
    pop rdi
    mov rdi, USAGE_ERROR
    jmp exit

exit:
    mov rax, 60 ; Exit
    syscall