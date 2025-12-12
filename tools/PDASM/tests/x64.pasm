:section .data
    msg!ubyte[] = "Hello World!", 10

:section .text
    :global main

main:
    lea %rax, [msg]
    mov %rbx, 13
    call $print
    jmp exit

exit:
    mov %rdi, %rax
    mov %rax, 60
    syscall

.func print:
    mov %rsi, %rax
    mov %rdx, %rbx
    jmp $print.sysprint
sysprint:
    mov %rax, 1
    mov %rdi, 1
    syscall
    ret
.endfunc
